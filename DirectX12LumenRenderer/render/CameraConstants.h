#pragma once
#include <DirectXMath.h>

struct CameraConstants {
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Proj;
    DirectX::XMMATRIX InvViewProj;
    DirectX::XMFLOAT3 CameraPosition;
    float Padding;
};
