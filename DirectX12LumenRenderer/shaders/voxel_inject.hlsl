RWTexture3D<float3> VoxelGrid : register(u0);

cbuffer VoxelParams : register(b0)
{
    float3 voxelGridOrigin;
    float voxelSize;
};

[numthreads(8,8,8)]
void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    float3 worldPos = voxelGridOrigin + dispatchThreadId * voxelSize;


    float3 giColor = float3(0.1, 0.1, 0.1); 

    VoxelGrid[dispatchThreadId] = giColor;
}


Texture3D<float3> VoxelGI : register(t5);

float3 WorldToVoxelUV(float3 worldPos)
{
    return (worldPos - voxelGridOrigin) / (voxelSize * 128.0);
}

float3 SampleVoxelGI(float3 worldPos)
{
    float3 uvw = WorldToVoxelUV(worldPos);
    return VoxelGI.SampleLevel(samLinear, uvw, 0);
}