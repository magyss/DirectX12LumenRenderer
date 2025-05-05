#pragma once
#include <DirectXMath.h>

class Camera {
public:
    void SetLens(float fovY, float aspect, float zn, float zf);
    void SetPosition(DirectX::XMFLOAT3 pos);
    void LookAt(DirectX::XMFLOAT3 eye, DirectX::XMFLOAT3 target, DirectX::XMFLOAT3 up);

    DirectX::XMMATRIX GetView() const;
    DirectX::XMMATRIX GetProj() const;
    DirectX::XMMATRIX GetViewProj() const;

private:
    DirectX::XMFLOAT3 m_pos;
    DirectX::XMFLOAT3 m_target;
    DirectX::XMFLOAT3 m_up;

    float m_fovY, m_aspect, m_zNear, m_zFar;
};
