#include "Mesh.h"
#include <d3dx12.h>

bool Mesh::Initialize(ID3D12Device* device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    m_indexCount = static_cast<UINT>(indices.size());

    const UINT vbSize = static_cast<UINT>(vertices.size() * sizeof(Vertex));
    const UINT ibSize = static_cast<UINT>(indices.size() * sizeof(uint32_t));

    // VB
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC vbDesc = CD3DX12_RESOURCE_DESC::Buffer(vbSize);
    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &vbDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer));
    
    void* mappedVB;
    m_vertexBuffer->Map(0, nullptr, &mappedVB);
    memcpy(mappedVB, vertices.data(), vbSize);
    m_vertexBuffer->Unmap(0, nullptr);

    m_vbView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vbView.StrideInBytes = sizeof(Vertex);
    m_vbView.SizeInBytes = vbSize;

    // IB
    CD3DX12_RESOURCE_DESC ibDesc = CD3DX12_RESOURCE_DESC::Buffer(ibSize);
    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &ibDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_indexBuffer));
    
    void* mappedIB;
    m_indexBuffer->Map(0, nullptr, &mappedIB);
    memcpy(mappedIB, indices.data(), ibSize);
    m_indexBuffer->Unmap(0, nullptr);

    m_ibView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_ibView.Format = DXGI_FORMAT_R32_UINT;
    m_ibView.SizeInBytes = ibSize;

    return true;
}

void Mesh::Draw(ID3D12GraphicsCommandList* cmdList) const {
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_vbView);
    cmdList->IASetIndexBuffer(&m_ibView);
    cmdList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}
