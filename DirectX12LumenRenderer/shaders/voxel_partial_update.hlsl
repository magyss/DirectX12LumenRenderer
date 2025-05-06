RWTexture3D<float4> VoxelGrid : register(u0);

cbuffer VoxelUpdateParams : register(b0)
{
    float3 gridOrigin;
    float voxelSize;
    int   resolution;
};

[numthreads(4, 4, 4)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    float3 posWS = gridOrigin + voxelSize * id;

    float4 albedo = SampleScene(posWS);
    float occlusion = TraceShadows(posWS);
    float emissive = SampleEmissive(posWS);

    VoxelGrid[id] = float4(albedo.rgb * occlusion + emissive, 1.0);
}
