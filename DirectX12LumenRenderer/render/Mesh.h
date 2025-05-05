#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <vector>
#include <DirectXMath.h>

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 texcoord;
};

class Mesh {
public:
    bool Initialize(ID3D12Device* device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    void Draw(ID3D12GraphicsCommandList* cmdList) const;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;

    D3D12_VERTEX_BUFFER_VIEW m_vbView = {};
    D3D12_INDEX_BUFFER_VIEW m_ibView = {};
    UINT m_indexCount = 0;
};
