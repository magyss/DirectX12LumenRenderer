#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include "GBuffer.h"
#include "Mesh.h"

class GBufferPass {
public:
    bool Initialize(ID3D12Device* device, DXGI_FORMAT* rtFormats, UINT rtCount, DXGI_FORMAT depthFormat);
    void Render(ID3D12GraphicsCommandList* cmdList, GBuffer* gbuffer, const std::vector<Mesh*>& meshes);

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
};
