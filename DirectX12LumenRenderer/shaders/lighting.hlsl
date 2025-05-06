Texture2D<float4> gNormal : register(t0);
Texture2D<float4> gAlbedo : register(t1);
Texture2D<float4> gPosition : register(t2);

SamplerState samLinear : register(s0);
Texture2D GITexture : register(t3);
Texture2D PrevGITexture : register(t4);


Texture3D<float3> VoxelGI : register(t5);
RWTexture2D<float3> GI_Cache : register(u0);
Texture2D<float> Depth : register(t0);
SamplerState samPoint : register(s0);

cbuffer FrameParams : register(b0)
{
    float4x4 invProj;
    float4x4 prevViewProj;
    float4x4 currViewProj;
    float2 screenSize;
    float blendingFactor;
};

float3 ReconstructWorldPos(float2 uv, float depth)
{
    float4 clip = float4(uv * 2.0f - 1.0f, depth, 1.0f);
    float4 view = mul(invProj, clip);
    view /= view.w;
    return view.xyz;
}

[numthreads(8, 8, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    float2 uv = id.xy / screenSize;
    float depth = Depth.Sample(samPoint, uv);
    float3 worldPos = ReconstructWorldPos(uv, depth);

    float3 normal = ...; // из GBuffer
    float3 gi = ConeTraceGI(worldPos, normal);

    GI_Cache[id.xy] = gi;
}

float3 WorldToVoxelUV(float3 worldPos)
{
    return (worldPos - voxelGridOrigin) / (voxelSize * 128.0);
}

float3 SampleVoxelGI(float3 worldPos)
{
    float3 uvw = WorldToVoxelUV(worldPos);
    return VoxelGI.SampleLevel(samLinear, uvw, 0);
}

struct VSOutput {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
};

cbuffer CameraCB : register(b0)
{
    matrix View;
    matrix Proj;
    matrix InvViewProj;
    float3 CameraPosition;
    float Padding;
}


float3 TangentSpaceHemisphereSample(float3 N, float2 rand) {
    // Uniform sampling over hemisphere
    float phi = 2.0 * 3.141592 * rand.x;
    float cosTheta = pow(1.0 - rand.y, 0.5);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 H = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    // Build orthonormal basis
    float3 up = abs(N.z) < 0.999 ? float3(0,0,1) : float3(1,0,0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);

    float3 sampleDir = H.x * tangent + H.y * bitangent + H.z * N;
    return normalize(sampleDir);
}


float2 WorldToUV(float3 worldPos) {
    float4 clip = mul(float4(worldPos, 1.0), InvViewProj); // inv(ViewProj)^-1 = ViewProj
    float2 ndc = clip.xy / clip.w;
    return ndc * 0.5 + 0.5;
}

VSOutput VSMain(uint vid : SV_VertexID) {
    float2 pos[] = {
        float2(-1, -1),
        float2(-1, +3),
        float2(+3, -1)
    };

    float2 uv[] = {
        float2(0, 0),
        float2(0, 2),
        float2(2, 0)
    };

    VSOutput output;
    output.pos = float4(pos[vid], 0, 1);
    output.uv = uv[vid];
    return output;
}

float4 PSMain(VSOutput input) : SV_Target {

    for (int mip = 0; mip < maxMip - 1; ++mip)
    {
        cmdList->SetComputeRootDescriptorTable(0, uavMip[mip + 1]); 
        cmdList->SetComputeRootDescriptorTable(1, srvMip[mip]);     

        int size = 128 >> (mip + 1);
        cmdList->Dispatch((size + 3) / 4, (size + 3) / 4, (size + 3) / 4);
    }


    float3 curGI = GITexture.Sample(samLinear, input.uv).rgb;
    float3 prevGI = PrevGITexture.Sample(samLinear, input.uv).rgb;

    float blendFactor = 0.9;
    float3 accumulatedGI = lerp(curGI, prevGI, blendFactor);

    float3 finalColor = directLight + accumulatedGI;
    return float4(finalColor, 1.0);


    float3 giColor = GITexture.Sample(samLinear, input.uv).rgb;
    float3 finalColor = directLight + giColor;
    return float4(finalColor, 1.0);

    float2 uv = WorldToUV(samplePos);
    if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1) continue;


    float3 normal = normalize(gNormal.Sample(samLinear, input.uv).xyz * 2.0 - 1.0);
    float3 albedo = gAlbedo.Sample(samLinear, input.uv).rgb;
    float3 worldPos = gPosition.Sample(samLinear, input.uv).xyz;

    float3 L = normalize(LightPosition - worldPos);
    float dist = length(LightPosition - worldPos);
    float attenuation = 1.0 / (dist * dist + 1.0);
    float NdotL = saturate(dot(normal, L));
    float3 directLight = albedo * LightColor * NdotL * LightIntensity * attenuation;

    float3 giColor = float3(0,0,0);
    float3 origin = worldPos + normal * 0.1;

    const int numSamples = 8;
    const float maxDistance = 5.0;

    for (int i = 0; i < numSamples; ++i) {
        float2 rand = float2(i / float(numSamples), frac(sin(i * 12.9898) * 43758.5453)); // псевдо-рандом
        float3 rayDir = TangentSpaceHemisphereSample(normal, rand);

        for (float t = 0.2; t < maxDistance; t += 0.3) {
            float3 samplePos = origin + rayDir * t;
            float2 uv = WorldToUV(samplePos);

            if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1) break;

            float3 hitNormal = gNormal.Sample(samLinear, uv).xyz * 2.0 - 1.0;
            float3 hitAlbedo = gAlbedo.Sample(samLinear, uv).rgb;
            float3 hitPos = gPosition.Sample(samLinear, uv).xyz;

            float dist = length(hitPos - samplePos);
            if (dist < 0.2) {
                float weight = saturate(dot(normal, normalize(hitPos - origin)));
                giColor += hitAlbedo * weight;
                break;
            }
        }
    }
    giColor /= numSamples;
    giColor *= 0.5;
    giColor *= 0.3;

    float3 finalColor = directLight + giColor;


    float3 bouncedGI = ConeTraceGI(worldPos, normal);
    float3 finalColor = directLight + bouncedGI;


    return float4(finalColor, 1.0);
}


Texture3D<float3> VoxelMip[NUM_MIPS];

float3 SampleVoxelCone(float3 origin, float3 dir, float coneAngle)
{
    float3 acc = 0;
    float total = 0;

    float t = 0.0;
    const float step = voxelSize;
    int numSteps = 16;

    for (int i = 0; i < numSteps; ++i)
    {
        float3 pos = origin + dir * t;
        float lod = log2(max(coneAngle * t / voxelSize, 1.0));

        int mip = clamp((int)lod, 0, NUM_MIPS - 1);

        float3 uvw = (pos - voxelGridOrigin) / (voxelSize * gridSize);
        float3 sample = VoxelMip[mip].SampleLevel(samLinear, uvw, mip);

        float w = 1.0 / (1.0 + i * 0.5);
        acc += sample * w;
        total += w;

        t += step;
    }

    return acc / total;
}



float3 ConeTraceGI(float3 worldPos, float3 normal)
{
    const int NUM_CONES = 4;
    float3 coneDirs[NUM_CONES] = {
        normal,
        normalize(reflect(normal, float3(0.8, 0.3, 0.1))),
        normalize(reflect(normal, float3(-0.7, 0.2, -0.6))),
        normalize(reflect(normal, float3(0.1, -0.9, 0.4)))
    };

    float coneAngle = 0.4; 
    float3 gi = float3(0, 0, 0);

    for (int c = 0; c < NUM_CONES; ++c)
    {
        float3 dir = coneDirs[c];
        float3 pos = worldPos + 0.1 * dir;
        float totalWeight = 0;
        float3 coneGI = float3(0, 0, 0);

        for (int i = 0; i < 16; ++i)
        {
            float radius = coneAngle * i * voxelSize; 
            float3 uvw = (pos - voxelGridOrigin) / (voxelSize * 128.0);
            float3 sample = VoxelGI.SampleLevel(samLinear, uvw, 0);

            float weight = 1.0 / (1.0 + i * 0.2); 
            coneGI += sample * weight;
            totalWeight += weight;

            pos += dir * voxelSize * 1.5; 
        }

        gi += coneGI / totalWeight;
    }

    return gi / NUM_CONES;
}

