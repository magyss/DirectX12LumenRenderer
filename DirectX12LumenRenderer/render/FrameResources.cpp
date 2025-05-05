#include "FrameResources.h"
#include <stdexcept>

bool FrameResources::Initialize(ID3D12Device* device, UINT frameCount) {
    m_frameCount = frameCount;
    m_frames.resize(frameCount);

    // Создание Command Allocators
    for (UINT i = 0; i < frameCount; ++i) {
        if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frames[i].commandAllocator)))) {
            return false;
        }
    }

    // Командный лист создаётся один и переиспользуется с разными allocators
    if (FAILED(device->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_frames[0].commandAllocator.Get(),
        nullptr, IID_PPV_ARGS(&m_commandList)))) {
        return false;
    }
    m_commandList->Close(); // Закрываем сразу, откроем вручную перед началом кадра

    // Создаём fence
    if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)))) {
        return false;
    }

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr) {
        return false;
    }

    return true;
}

void FrameResources::BeginFrame(UINT frameIndex) {
    auto& frame = m_frames[frameIndex];

    frame.commandAllocator->Reset();
    m_commandList->Reset(frame.commandAllocator.Get(), nullptr);
}

void FrameResources::EndFrame(ID3D12CommandQueue* commandQueue, UINT frameIndex) {
    m_commandList->Close();

    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    commandQueue->ExecuteCommandLists(1, ppCommandLists);

    auto& frame = m_frames[frameIndex];
    frame.fenceValue = m_currentFence;

    commandQueue->Signal(m_fence.Get(), m_currentFence);
    m_currentFence++;
}

void FrameResources::WaitForGPU(UINT frameIndex) {
    auto& frame = m_frames[frameIndex];
    if (m_fence->GetCompletedValue() < frame.fenceValue) {
        m_fence->SetEventOnCompletion(frame.fenceValue, m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

ID3D12GraphicsCommandList* FrameResources::GetCommandList() const {
    return m_commandList.Get();
}

ID3D12CommandAllocator* FrameResources::GetCommandAllocator(UINT frameIndex) const {
    return m_frames[frameIndex].commandAllocator.Get();
}
