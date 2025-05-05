#include "GBuffer.h"
#include <vector>
#include <stdexcept>
#include <dxgi1_4.h>
#include <d3dx12.h>

bool GBuffer::Initialize(ID3D12Device* device, UINT width, UINT height) {
    m_width = width;
    m_height = height;
    CreateResources(device, width, height);
    CreateDescriptors(device);
    return true;
}

void GBuffer::Resize(ID3D12Device* device, UINT width, UINT height) {
    Cleanup();
    Initialize(device, width, height);
}

void GBuffer::Cleanup() {
    m_renderTargets.clear();
    m_depthBuffer.Reset();
    m_rtvHeap.Reset();
    m_dsvHeap.Reset();
}

void GBuffer::CreateResources(ID3D12Device* device, UINT width, UINT height) {
    m_renderTargets.resize(GBufferCount);

    DXGI_FORMAT formats[GBufferCount] = {
        DXGI_FORMAT_R16G16B16A16_FLOAT, // Normals
        DXGI_FORMAT_R8G8B8A8_UNORM,     // Albedo
        DXGI_FORMAT_R16G16_FLOAT        // Roughness/Metallic
    };

    for (UINT i = 0; i < GBufferCount; ++i) {
        D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
            formats[i], width, height, 1, 1);
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_CLEAR_VALUE clear = {};
        clear.Format = formats[i];
        clear.Color[0] = 0.0f;

        device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clear,
            IID_PPV_ARGS(&m_renderTargets[i]));
    }

    // Depth
    D3D12_RESOURCE_DESC depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_D32_FLOAT, width, height, 1, 1);
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthClear = {};
    depthClear.Format = DXGI_FORMAT_D32_FLOAT;
    depthClear.DepthStencil.Depth = 1.0f;

    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClear,
        IID_PPV_ARGS(&m_depthBuffer));
}

void GBuffer::CreateDescriptors(ID3D12Device* device) {
    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
    rtvDesc.NumDescriptors = GBufferCount;
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&m_rtvHeap));
    m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < GBufferCount; ++i) {
        device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_rtvDescriptorSize;
    }

    D3D12_DESCRIPTOR_HEAP_DESC dsvDesc = {};
    dsvDesc.NumDescriptors = 1;
    dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    device->CreateDescriptorHeap(&dsvDesc, IID_PPV_ARGS(&m_dsvHeap));
    m_dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    device->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

const std::vector<ID3D12Resource*>& GBuffer::GetRenderTargets() const {
    return m_renderTargets;
}

ID3D12Resource* GBuffer::GetDepthBuffer() const {
    return m_depthBuffer.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE GBuffer::GetRTV(int index) const {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += index * m_rtvDescriptorSize;
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE GBuffer::GetDSV() const {
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}
