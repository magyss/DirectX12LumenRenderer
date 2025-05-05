#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include "DeviceManager.h"
#include "SwapChain.h"
#include "FrameResources.h"

class DX12App {
public:
    DX12App(HINSTANCE hInstance);
    virtual ~DX12App();

    bool Initialize();
    int Run();

protected:
    virtual void Update();
    virtual void Render();

    bool InitWindow();
    bool InitDX12();

protected:
    HINSTANCE m_hInstance;
    HWND m_hWnd;
    int m_width = 1280;
    int m_height = 720;
    std::wstring m_appName = L"DX12 Lumen Renderer";

    std::unique_ptr<DeviceManager> m_deviceManager;
    std::unique_ptr<SwapChain> m_swapChain;
    std::unique_ptr<FrameResources> m_frameResources;

    static const UINT FrameCount = 2;
};
