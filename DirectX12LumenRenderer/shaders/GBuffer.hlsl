struct VSInput {
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texcoord : TEXCOORD;
};

struct PSInput {
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
    float2 texcoord : TEXCOORD;
};

PSInput VSMain(VSInput input) {
    PSInput output;
    float4 worldPos = mul(float4(input.position, 1.0), World);
    output.position = mul(worldPos, mul(View, Proj));
    output.worldPos = worldPos.xyz;
    output.normal = mul(float4(input.normal, 0.0), World).xyz;
    output.texcoord = input.texcoord;
    return output;
}


struct PSOutput {
    float4 RT0 : SV_Target0; // world normal
    float4 RT1 : SV_Target1; // albedo
    float4 RT2 : SV_Target2; // world pos (для lighting pass)
};


PSOutput PSMain(PSInput input) {
    PSOutput out;
    out.RT0 = float4(normalize(input.normal), 0.0);
    out.RT1 = float4(1.0, 0.5, 0.2, 1.0);
    out.RT2 = float4(input.worldPos, 1.0);
    return out;
}


cbuffer CameraCB : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
};



