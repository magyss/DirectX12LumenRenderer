#include "DeviceManager.h"
#include <stdexcept>
#include <iostream>

bool DeviceManager::Initialize() {
    if (!CreateFactory()) return false;
    if (!CreateDevice()) return false;
    if (!CreateCommandQueue()) return false;
    return true;
}

bool DeviceManager::CreateFactory() {
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory));
    if (FAILED(hr)) {
        std::cerr << "Failed to create DXGI Factory.\n";
        return false;
    }

    return true;
}

bool DeviceManager::CreateDevice() {
    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

    for (UINT adapterIndex = 0;
         DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapters1(adapterIndex, &adapter);
         ++adapterIndex) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue; 
        }

        if (SUCCEEDED(D3D12CreateDevice(
                adapter.Get(),
                D3D_FEATURE_LEVEL_12_1,
                IID_PPV_ARGS(&m_device)))) {
            return true;
        }
    }

    std::cerr << "Failed to create D3D12 Device.\n";
    return false;
}

bool DeviceManager::CreateCommandQueue() {
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    HRESULT hr = m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue));
    if (FAILED(hr)) {
        std::cerr << "Failed to create Command Queue.\n";
        return false;
    }

    return true;
}
