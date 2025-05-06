#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "GBuffer.h"

class LightingPass {
    public:
        bool Initialize(ID3D12Device* device, DXGI_FORMAT backBufferFormat);
        void Render(ID3D12GraphicsCommandList* cmdList, GBuffer* gbuffer);
        void SetLight(const LightConstants& lightData); 
        void SetCamera(const CameraConstants& camData);
    
    private:
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_voxelGrid;
        D3D12_GPU_DESCRIPTOR_HANDLE m_voxelUAV;
        D3D12_GPU_DESCRIPTOR_HANDLE m_voxelSRV;
    
        Microsoft::WRL::ComPtr<ID3D12Resource> m_lightCB;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_cameraCB;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_giTarget;
        D3D12_CPU_DESCRIPTOR_HANDLE m_giRTV;
        D3D12_GPU_DESCRIPTOR_HANDLE m_giSRV;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_prevGITarget;
        D3D12_GPU_DESCRIPTOR_HANDLE m_prevGISRV;


    };