#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <vector>

class GBuffer {
    public:
        bool Initialize(...);
        void Resize(...);
        void Begin(ID3D12GraphicsCommandList* cmdList);
        void End(ID3D12GraphicsCommandList* cmdList);
    
        D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(int index) const;
        ID3D12Resource* GetRTResource(int index) const;
    
        ID3D12DescriptorHeap* GetSRVHeap() const { return m_srvHeap.Get(); }
        D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUStart() const { return m_srvHeap->GetGPUDescriptorHandleForHeapStart(); }
    
        void CreateSRVs(ID3D12Device* device);
    
    private:
        static const int kNumRenderTargets = 3;
    
        Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[kNumRenderTargets];
        D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandles[kNumRenderTargets];
    
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    };
    