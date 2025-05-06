Texture3D<float3> VoxelMip0 : register(t0);
RWTexture3D<float3> VoxelMip1 : register(u0);
RWTexture3D<float3> VoxelMip2 : register(u1);
RWTexture3D<float3> VoxelMip3 : register(u2);

[numthreads(4, 4, 4)]
void GenMipLevel(uint3 id : SV_DispatchThreadID)
{
    uint3 baseID = id * 2;

    float3 sum = float3(0, 0, 0);
    for (uint z = 0; z < 2; ++z)
    for (uint y = 0; y < 2; ++y)
    for (uint x = 0; x < 2; ++x)
    {
        uint3 offset = baseID + uint3(x, y, z);
        sum += VoxelMip0[offset];
    }

    VoxelMip1[id] = sum / 8.0;
}