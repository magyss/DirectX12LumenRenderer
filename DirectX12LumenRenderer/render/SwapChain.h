#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <cstdint>

class SwapChain {
public:
    bool Initialize(HWND hwnd, UINT width, UINT height, ID3D12Device* device, IDXGIFactory6* factory, ID3D12CommandQueue* commandQueue);
    void Present();
    void Resize(UINT width, UINT height);
    void Cleanup();

    ID3D12Resource* GetCurrentRenderTarget() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() const;
    UINT GetCurrentBackBufferIndex() const;

private:
    bool CreateSwapChain(HWND hwnd, UINT width, UINT height, IDXGIFactory6* factory, ID3D12CommandQueue* commandQueue);
    bool CreateRTVs(ID3D12Device* device);

private:
    static const UINT FrameCount = 2;

    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];

    UINT m_rtvDescriptorSize = 0;
    UINT m_frameIndex = 0;
};
