#include "LightingPass.h"
#include <d3dx12.h>
#include <d3dcompiler.h>
#include <stdexcept>

struct VoxelParams
{
    XMFLOAT3 voxelGridOrigin;
    float voxelSize;
};

struct PixelInput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 normal : TEXCOORD2;
    float3 viewDir : TEXCOORD3;
};


bool LightingPass::Initialize(ID3D12Device* device, DXGI_FORMAT backBufferFormat) {
    FrameParams frame = {};
    frame.invProj = XMMatrixInverse(nullptr, camera.GetProjectionMatrix());
    frame.prevViewProj = prevViewProj;
    frame.currViewProj = camera.GetViewProj();
    frame.screenSize = { width, height };
    frame.blendingFactor = 0.9f;

    cmdList->SetComputeRoot32BitConstants(slot, sizeof(FrameParams)/4, &frame, 0);


    SSRParams ssr = {};
    ssr.screenSize = { (float)width, (float)height };
    ssr.projMatrix = camera.GetProjectionMatrix();
    ssr.invProjMatrix = camera.GetInvProjectionMatrix();
    ssr.maxRayDist = 30.0f;

    cmdList->SetGraphicsRoot32BitConstants(1, sizeof(SSRParams)/4, &ssr, 0);

    cmdList->SetComputeRootDescriptorTable(0, m_voxelUAV); 
    cmdList->SetComputeRootDescriptorTable(1, m_voxelSRV); 

    cmdList->Dispatch(16, 16, 16);


    VoxelParams params = {};
    params.voxelGridOrigin = { -64.0f, -64.0f, -64.0f };
    params.voxelSize = 1.0f;

    cmdList->SetGraphicsRoot32BitConstants(slot, sizeof(VoxelParams) / 4, &params, 0);
    D3D12_RESOURCE_DESC volumeDesc = {};
    volumeDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    volumeDesc.Width = 128;
    volumeDesc.Height = 128;
    volumeDesc.DepthOrArraySize = 128;
    volumeDesc.MipLevels = 1;
    volumeDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
    volumeDesc.SampleDesc.Count = 1;
    volumeDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    device->CreateCommittedResource(
    &heapProps, D3D12_HEAP_FLAG_NONE, &volumeDesc,
    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
    nullptr, IID_PPV_ARGS(&m_voxelGrid));

    m_voxelUAV = descriptorHeap.AllocUAV();
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = volumeDesc.Format;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
    uavDesc.Texture3D.WSize = 128;
    device->CreateUnorderedAccessView(m_voxelGrid.Get(), nullptr, &uavDesc, m_voxelUAV);

    m_voxelSRV = descriptorHeap.AllocSRV();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = volumeDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture3D.MipLevels = 1;
    device->CreateShaderResourceView(m_voxelGrid.Get(), &srvDesc, m_voxelSRV);


    CD3DX12_DESCRIPTOR_RANGE srvTable;
    srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0); // t0-t4


    CD3DX12_ROOT_PARAMETER rootParams[3];

    rootParams[0].InitAsDescriptorTable(1, &texTable); 
    rootParams[1].InitAsConstantBufferView(1);         
    rootParams[2].InitAsConstantBufferView(0);          

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Init(_countof(rootParams), rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> sigBlob, errorBlob;
    D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob);
    device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob;
    D3DCompileFromFile(L"shaders/lighting.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, nullptr);
    D3DCompileFromFile(L"shaders/lighting.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, nullptr);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { nullptr, 0 };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = backBufferFormat;
    psoDesc.SampleDesc.Count = 1;

    device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC cbDesc = CD3DX12_RESOURCE_DESC::Buffer(1024);
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = m_width;
    texDesc.Height = m_height;
    texDesc.MipLevels = 1;
    texDesc.DepthOrArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    
    D3D12_CLEAR_VALUE clearVal = {};
    clearVal.Format = texDesc.Format;
    clearVal.Color[0] = 0.0f; clearVal.Color[1] = 0.0f; clearVal.Color[2] = 0.0f; clearVal.Color[3] = 1.0f;
    
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    
    device->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clearVal, IID_PPV_ARGS(&m_prevGITarget));
    
    m_prevGISRV = descriptorHeap.AllocSRV();
    device->CreateShaderResourceView(m_prevGITarget.Get(), &srvDesc, m_prevGISRV);
    
    
    m_giSRV = descriptorHeap.AllocSRV();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    device->CreateShaderResourceView(m_giTarget.Get(), &srvDesc, m_giSRV);



    return true;
}

void LightingPass::Render(ID3D12GraphicsCommandList* cmdList, GBuffer* gbuffer) {
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_giTarget.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->ResourceBarrier(1, &barrier);

    cmdList->OMSetRenderTargets(1, &m_giRTV, FALSE, nullptr);
    cmdList->ClearRenderTargetView(m_giRTV, clearColor, 0, nullptr);

    cmdList->SetGraphicsRootConstantBufferView(2, m_cameraCB->GetGPUVirtualAddress()); 

    // Transition to SRV
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_giTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(1, &barrier);

    // Bind back buffer
    cmdList->OMSetRenderTargets(1, &backBufferRTV, FALSE, nullptr);

    // Set t3 = m_giSRV
    cmdList->SetGraphicsRootDescriptorTable(3, m_giSRV);



    ID3D12DescriptorHeap* heaps[] = { gbuffer->GetSRVHeap() };
    cmdList->SetDescriptorHeaps(1, heaps);

    cmdList->SetGraphicsRootDescriptorTable(0, gbuffer->GetSRVGPUStart());

    cmdList->SetGraphicsRootConstantBufferView(1, m_lightCB->GetGPUVirtualAddress());

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->SetGraphicsRootDescriptorTable(4, m_prevGISRV); // t4

    cmdList->DrawInstanced(3, 1, 0, 0);
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_giTarget.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        m_prevGITarget.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
    
    cmdList->CopyResource(m_prevGITarget.Get(), m_giTarget.Get());
    
}


void LightingPass::SetLight(const LightConstants& lightData) {
    LightConstants* mapped = nullptr;
    m_lightCB->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
    *mapped = lightData;
    m_lightCB->Unmap(0, nullptr);
}


void LightingPass::SetCamera(const CameraConstants& camData) {
    CameraConstants* mapped = nullptr;
    m_cameraCB->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
    *mapped = camData;
    m_cameraCB->Unmap(0, nullptr);
}