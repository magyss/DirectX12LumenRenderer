
Texture2D GBufferA : register(t0); // Albedo + Roughness + Metallic
Texture2D GBufferB : register(t1); // Normal
Texture2D GBufferC : register(t2); // World Position

Texture2D SceneDepth : register(t3);
Texture2D SceneColor : register(t4); // For SSR
TextureCube Skybox : register(t5);

StructuredBuffer<Light> Lights : register(t6);
Texture3DArray VoxelMip : register(t7);        // voxel GI/Reflections

SamplerState samLinear : register(s0);


struct GBufferData
{
    float3 Albedo;
    float Roughness;
    float Metallic;
    float3 NormalWS;
    float3 PositionWS;
};


float3 ComputeLighting(GBufferData g, float3 viewDir)
{
    float3 color = float3(0, 0, 0);

    float3 N = normalize(g.NormalWS);
    float3 V = normalize(viewDir);

    [loop]
    for (int i = 0; i < LightCount; ++i)
    {
        Light l = Lights[i];
        float3 L = normalize(l.Position - g.PositionWS);
        float3 H = normalize(L + V);

        float NdotL = saturate(dot(N, L));
        float NdotV = saturate(dot(N, V));
        float NdotH = saturate(dot(N, H));
        float VdotH = saturate(dot(V, H));

        float3 F0 = lerp(0.04, g.Albedo, g.Metallic);

        float D = D_GGX(NdotH, g.Roughness);
        float G = G_Smith(NdotL, NdotV, g.Roughness);
        float3 F = FresnelSchlick(F0, VdotH);

        float3 spec = D * G * F / (4.0 * NdotL * NdotV + 0.001);
        float3 diff = (1.0 - F) * g.Albedo / PI;

        float attenuation = l.Intensity / length_squared(l.Position - g.PositionWS);
        color += (diff + spec) * l.Color * attenuation * NdotL;
    }

    float coneAngle = g.Roughness * 0.4 + 0.05;
    float3 gi = SampleVoxelCone(g.PositionWS, N, coneAngle);
    color += gi * g.Albedo;
    float3 R = reflect(-V, N);
    bool SSRHit;
    float3 ssr = ComputeSSR(g.PositionWS, V, N, SSRHit);

    float3 refl = SSRHit ? ssr : VoxelConeReflections(g.PositionWS, R, g.Roughness);

    float3 F_refl = FresnelSchlick(lerp(0.04, g.Albedo, g.Metallic), saturate(dot(V, N)));
    color = lerp(color, refl, F_refl);

    return color;
}


float4 PSMain(float4 pos : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET
{
    GBufferData g;
    float4 gA = GBufferA.Sample(samLinear, uv);
    float4 gB = GBufferB.Sample(samLinear, uv);
    float4 gC = GBufferC.Sample(samLinear, uv);

    g.Albedo = gA.rgb;
    g.Roughness = gA.a;
    g.Metallic = gB.a;
    g.NormalWS = normalize(gB.rgb * 2.0 - 1.0);
    g.PositionWS = gC.rgb;

    float3 viewDir = normalize(CameraPos - g.PositionWS);
    float3 color = ComputeLighting(g, viewDir);

    if (DebugViewMode == 1) // GI
    {
        float coneAngle = 0.3;
        float3 gi = SampleVoxelCone(g.PositionWS, g.NormalWS, coneAngle);
        return float4(gi, 1.0);
    }
    else if (DebugViewMode == 2) // Reflections
    {
        float3 R = reflect(-viewDir, g.NormalWS);
        bool hit;
        float3 ssr = ComputeSSR(g.PositionWS, viewDir, g.NormalWS, hit);
        float3 refl = hit ? ssr : VoxelConeReflections(g.PositionWS, R, g.Roughness);
        return float4(refl, 1.0);
    }
    else if (DebugViewMode == 3) // Clipmap Level
    {
        float dist = distance(g.PositionWS, CameraPosition);
        float level = log2(dist / baseVoxelSize);
        return float4(level / NumClipmapLevels.xxx, 1.0); 
    }
    else if (DebugViewMode == 4) // Voxel Albedo
    {
        float3 voxelColor = SampleVoxel(g.PositionWS);
        return float4(voxelColor, 1.0);
    }
    else if (DebugViewMode == 5) // Emissive
    {
        float emissive = SampleVoxelEmissive(g.PositionWS);
        return float4(emissive.xxx, 1.0);
    }


    return float4(color, 1.0);
}
