RWTexture3D<float3> VoxelOut : register(u0);
Texture3D<float3> VoxelIn : register(t0);
Texture2D<float> SceneDepth : register(t0);
Texture2D<float3> SceneColor : register(t1);
SamplerState samPoint : register(s0);

cbuffer SSRParams : register(b1)
{
    float2 screenSize;
    float4x4 projMatrix;
    float4x4 invProjMatrix;
    float maxRayDist;
};


cbuffer VoxelParams : register(b0)
{
    float3 voxelGridOrigin;
    float voxelSize;
};

struct VoxelClipmapLevel
{
    ID3D12Resource* voxelTexture; 
    XMFLOAT3 centerWS;
    float voxelSize; 
    int resolution;  
};


[numthreads(8, 8, 8)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    float3 pos = voxelGridOrigin + id * voxelSize;
    float3 normal = float3(0, 1, 0); // Можно генерировать шумно
    float3 gi = float3(0, 0, 0);

    const int NUM_CONES = 4;
    float3 dirs[NUM_CONES] = {
        normalize(float3(1, 1, 0.3)),
        normalize(float3(-1, 1, -0.2)),
        normalize(float3(0.3, 1, 1)),
        normalize(float3(-0.4, 1, 0.8))
    };

    for (int c = 0; c < NUM_CONES; ++c)
    {
        float3 ray = pos + dirs[c] * voxelSize;
        float3 acc = float3(0, 0, 0);
        float total = 0;

        for (int i = 0; i < 10; ++i)
        {
            float3 uvw = (ray - voxelGridOrigin) / (voxelSize * 128.0);
            float3 sample = VoxelIn.SampleLevel(samLinear, uvw, 0);
            float w = 1.0 / (1.0 + i * 0.4);
            acc += sample * w;
            total += w;
            ray += dirs[c] * voxelSize;
        }

        gi += acc / total;
    }

    for (int i = 0; i < NumClipmapLevels; ++i)
    {
        float voxelSize = baseVoxelSize * powf(2.0f, i);
        int resolution = 128;

        XMFLOAT3 cameraPos = camera.GetPosition();

        XMFLOAT3 snappedPos = {
            floorf(cameraPos.x / voxelSize) * voxelSize,
            floorf(cameraPos.y / voxelSize) * voxelSize,
            floorf(cameraPos.z / voxelSize) * voxelSize,
        };

        clipmaps[i].centerWS = snappedPos;
        clipmaps[i].voxelSize = voxelSize;
    }

    if (SSRSuccess)
    {
        reflColor = ssr;
    }
    else
    {
        reflColor = VoxelConeReflections(worldPos, reflect(-viewDir, normal), roughness);
    }

    float fresnel = pow(1.0 - saturate(dot(viewDir, normal)), 5.0);
    float reflectance = lerp(0.04, 1.0, metallic);

    float3 color = directLight + bouncedGI;
    color = lerp(color, reflColor, reflectance * fresnel);


    VoxelOut[id] = gi / NUM_CONES;
}


float3 ComputeSSR(float3 worldPos, float3 viewDir, float3 normal, float2 uv, out bool hit)
{
    hit = false;
    float3 reflectDir = reflect(-viewDir, normal);
    float3 rayOrigin = worldPos;

    float rayStep = 0.1;
    int maxSteps = 64;

    for (int i = 0; i < maxSteps; ++i)
    {
        float3 samplePos = rayOrigin + reflectDir * (i * rayStep);
        float4 clip = mul(projMatrix, float4(samplePos, 1.0));
        float2 screenUV = clip.xy / clip.w * 0.5 + 0.5;

        if (screenUV.x < 0 || screenUV.x > 1 || screenUV.y < 0 || screenUV.y > 1)
            break;

        float sceneDepth = SceneDepth.Sample(samPoint, screenUV);
        float viewZ = clip.z / clip.w;

        if (abs(viewZ - sceneDepth) < 0.01)
        {
            hit = true;
            return SceneColor.Sample(samPoint, screenUV);
        }
    }

    return float3(0, 0, 0);
}

float3 VoxelConeReflections(float3 worldPos, float3 reflectDir, float roughness)
{
    float coneAngle = lerp(0.05, 0.5, roughness);
    float3 reflColor;
    
    return SampleVoxelCone(worldPos, reflectDir, coneAngle);
}