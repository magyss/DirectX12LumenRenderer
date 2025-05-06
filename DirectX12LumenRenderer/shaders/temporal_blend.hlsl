Texture2D<float3> PrevGI : register(t0);
Texture2D<float3> CurrGI : register(t1);
Texture2D<float> Depth : register(t2);
RWTexture2D<float3> GI_Output : register(u0);

cbuffer TemporalParams : register(b0)
{
    float4x4 currViewProj;
    float4x4 prevViewProj;
    float4x4 invProj;
    float2 screenSize;
    float blendFactor;
};

[numthreads(8,8,1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    float2 uv = id.xy / screenSize;
    float depth = Depth.Load(int3(id.xy, 0));
    float3 worldPos = ReconstructWorldPos(uv, depth);

    float4 prevClip = mul(prevViewProj, float4(worldPos, 1.0));
    float2 prevUV = (prevClip.xy / prevClip.w) * 0.5 + 0.5;

    if (prevUV.x < 0 || prevUV.x > 1 || prevUV.y < 0 || prevUV.y > 1)
    {
        GI_Output[id.xy] = CurrGI[id.xy];
        return;
    }

    float3 curr = CurrGI[id.xy];
    float3 prev = PrevGI.SampleLevel(samPoint, prevUV, 0);
    GI_Output[id.xy] = lerp(curr, prev, blendFactor);
}
