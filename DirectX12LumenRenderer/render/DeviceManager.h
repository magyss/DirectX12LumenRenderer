#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <memory>

class DeviceManager {
public:
    bool Initialize();

    ID3D12Device* GetDevice() const { return m_device.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }
    IDXGIFactory6* GetFactory() const { return m_dxgiFactory.Get(); }

private:
    bool CreateFactory();
    bool CreateDevice();
    bool CreateCommandQueue();

private:
    Microsoft::WRL::ComPtr<IDXGIFactory6> m_dxgiFactory;
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
};
