#include "DX12App.h"
#include <stdexcept>

DX12App::DX12App(HINSTANCE hInstance) : m_hInstance(hInstance) {}
DX12App::~DX12App() {}

bool DX12App::Initialize() {
    if (!InitWindow()) return false;
    if (!InitDX12()) return false;
    return true;
}

bool DX12App::InitDX12() {
    m_deviceManager = std::make_unique<DeviceManager>();
    if (!m_deviceManager->Initialize()) return false;

    m_swapChain = std::make_unique<SwapChain>();
    if (!m_swapChain->Initialize(
        m_hWnd, m_width, m_height,
        m_deviceManager->GetDevice(),
        m_deviceManager->GetFactory(),
        m_deviceManager->GetCommandQueue())) {
        return false;
    }

    m_frameResources = std::make_unique<FrameResources>();
    if (!m_frameResources->Initialize(m_deviceManager->GetDevice(), FrameCount)) {
        return false;
    }

    return true;
}


void DX12App::Render() {
    UINT frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    m_frameResources->WaitForGPU(frameIndex);
    m_frameResources->BeginFrame(frameIndex);
    auto* commandList = m_frameResources->GetCommandList();

    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_swapChain->GetCurrentRenderTarget(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier);

    FLOAT clearColor[] = { 0.1f, 0.1f, 0.5f, 1.0f }; 
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapChain->GetCurrentRTV();
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_swapChain->GetCurrentRenderTarget(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier);
    m_frameResources->EndFrame(m_deviceManager->GetCommandQueue(), frameIndex);
    m_swapChain->Present();
}


void DX12App::Update() {
    
}


bool DX12App::InitWindow() {
    WNDCLASS wc = {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = L"DX12WindowClass";

    RegisterClass(&wc);

    RECT rect = { 0, 0, m_width, m_height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    m_hWnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        m_appName.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, m_hInstance, nullptr
    );

    if (!m_hWnd) return false;

    ShowWindow(m_hWnd, SW_SHOW);
    return true;
}
