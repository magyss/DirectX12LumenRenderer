#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <vector>

class GBuffer {
public:
    bool Initialize(ID3D12Device* device, UINT width, UINT height);
    void Resize(ID3D12Device* device, UINT width, UINT height);
    void Cleanup();

    const std::vector<ID3D12Resource*>& GetRenderTargets() const;
    ID3D12Resource* GetDepthBuffer() const;

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(int index) const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const;

private:
    void CreateResources(ID3D12Device* device, UINT width, UINT height);
    void CreateDescriptors(ID3D12Device* device);

private:
    static const UINT GBufferCount = 3;

    UINT m_width = 0;
    UINT m_height = 0;

    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_renderTargets;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_depthBuffer;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

    UINT m_rtvDescriptorSize = 0;
    UINT m_dsvDescriptorSize = 0;
};
