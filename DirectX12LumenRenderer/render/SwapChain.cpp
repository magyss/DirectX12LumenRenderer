#include "SwapChain.h"
#include <stdexcept>

bool SwapChain::Initialize(HWND hwnd, UINT width, UINT height, ID3D12Device* device, IDXGIFactory6* factory, ID3D12CommandQueue* commandQueue) {
    if (!CreateSwapChain(hwnd, width, height, factory, commandQueue)) return false;
    if (!CreateRTVs(device)) return false;
    return true;
}

bool SwapChain::CreateSwapChain(HWND hwnd, UINT width, UINT height, IDXGIFactory6* factory, ID3D12CommandQueue* commandQueue) {
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    HRESULT hr = factory->CreateSwapChainForHwnd(
        commandQueue,
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    );

    if (FAILED(hr)) return false;

    hr = swapChain1.As(&m_swapChain);
    if (FAILED(hr)) return false;

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    return true;
}

bool SwapChain::CreateRTVs(ID3D12Device* device) {
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)))) {
        return false;
    }

    m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT i = 0; i < FrameCount; ++i) {
        if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])))) {
            return false;
        }
        device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }

    return true;
}

void SwapChain::Present() {
    m_swapChain->Present(1, 0);
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

ID3D12Resource* SwapChain::GetCurrentRenderTarget() const {
    return m_renderTargets[m_frameIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetCurrentRTV() const {
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(m_frameIndex, m_rtvDescriptorSize);
    return rtvHandle;
}

UINT SwapChain::GetCurrentBackBufferIndex() const {
    return m_frameIndex;
}

void SwapChain::Cleanup() {
    for (UINT i = 0; i < FrameCount; ++i) {
        m_renderTargets[i].Reset();
    }
    m_swapChain.Reset();
    m_rtvHeap.Reset();
}
