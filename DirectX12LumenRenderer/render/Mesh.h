#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <vector>
#include <DirectXMath.h>

DirectX::XMMATRIX WorldMatrix = DirectX::XMMatrixIdentity();

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 texcoord;
};

class Mesh {
    public:
        bool Initialize(...);
        void Draw(ID3D12GraphicsCommandList* cmdList) const;
    
        void SetWorldMatrix(const DirectX::XMMATRIX& world) { m_world = world; }
        const DirectX::XMMATRIX& GetWorldMatrix() const { return m_world; }
    
    private:
        ...
        DirectX::XMMATRIX m_world = DirectX::XMMatrixIdentity();
    };