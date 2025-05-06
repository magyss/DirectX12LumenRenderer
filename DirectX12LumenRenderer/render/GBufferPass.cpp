#include "GBufferPass.h"
#include <d3dx12.h>
#include <d3dcompiler.h>
#include <stdexcept>
#include "CameraConstants.h"


bool GBufferPass::Initialize(ID3D12Device* device, DXGI_FORMAT* rtFormats, UINT rtCount, DXGI_FORMAT depthFormat) {
    CD3DX12_ROOT_PARAMETER rootParams[1];
    rootParams[0].InitAsConstantBufferView(0);
    
    CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
    rootDesc.Init(_countof(rootParams), rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    
    CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
    rootDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> sigBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &error);
    device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    D3DCompileFromFile(L"shaders/gbuffer.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, nullptr);
    D3DCompileFromFile(L"shaders/gbuffer.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, nullptr);

    D3D12_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { layout, _countof(layout) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = rtCount;
    for (UINT i = 0; i < rtCount; ++i)
        psoDesc.RTVFormats[i] = rtFormats[i];
    psoDesc.DSVFormat = depthFormat;
    psoDesc.SampleDesc.Count = 1;

    device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));

    return true;
}

void GBufferPass::Render(ID3D12GraphicsCommandList* cmdList, GBuffer* gbuffer, const std::vector<Mesh*>& meshes) {
    for (auto* mesh : meshes) {
        CameraConstants camData;
        camData.World = DirectX::XMMatrixTranspose(mesh->GetWorldMatrix());
        camData.View = DirectX::XMMatrixTranspose(camera->GetView());
        camData.Proj = DirectX::XMMatrixTranspose(camera->GetProj());
    
        Microsoft::WRL::ComPtr<ID3D12Resource> cbResource;
    
        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC bufDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(CameraConstants));
        device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&cbResource));
    
        CameraConstants* mappedData = nullptr;
        cbResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
        *mappedData = camData;
        cbResource->Unmap(0, nullptr);
    
        cmdList->SetGraphicsRootConstantBufferView(0, cbResource->GetGPUVirtualAddress());
        mesh->Draw(cmdList);
    }
    
    cmdList->SetPipelineState(m_pipelineState.Get());
    cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[3] = {
        gbuffer->GetRTV(0),
        gbuffer->GetRTV(1),
        gbuffer->GetRTV(2)
    };
    cmdList->OMSetRenderTargets(3, rtvs, FALSE, &gbuffer->GetDSV());

    FLOAT clear0[] = { 0, 0, 0, 0 };
    cmdList->ClearRenderTargetView(rtvs[0], clear0, 0, nullptr);
    cmdList->ClearRenderTargetView(rtvs[1], clear0, 0, nullptr);
    cmdList->ClearRenderTargetView(rtvs[2], clear0, 0, nullptr);
    cmdList->ClearDepthStencilView(gbuffer->GetDSV(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    for (auto* mesh : meshes)
        mesh->Draw(cmdList);
}
