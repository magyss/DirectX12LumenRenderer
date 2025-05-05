#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <cstdint>
#include <vector>

class FrameResources {
public:
    bool Initialize(ID3D12Device* device, UINT frameCount);
    void BeginFrame(UINT frameIndex);
    void EndFrame(ID3D12CommandQueue* commandQueue, UINT frameIndex);
    void WaitForGPU(UINT frameIndex);

    ID3D12GraphicsCommandList* GetCommandList() const;
    ID3D12CommandAllocator* GetCommandAllocator(UINT frameIndex) const;

private:
    struct FrameData {
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
        UINT64 fenceValue = 0;
    };

    std::vector<FrameData> m_frames;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent = nullptr;

    UINT m_frameCount = 0;
    UINT64 m_currentFence = 1;
};
