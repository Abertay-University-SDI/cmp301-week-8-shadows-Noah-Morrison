// Define number of clumps - don't like having this defined in two different places
#define MAX_WHEAT_CLUMPS 1365 // Maximum constant buffer size is 4096 16 byte entries - each case of instance data is 3 entires. 4096 / 3 = 1365.333... => 1365

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct InstanceData
{
    float3 position;
    float3 scale;
    float4 rotation;
};

cbuffer InstanceBuffer : register(b1)
{
    InstanceData instances[MAX_WHEAT_CLUMPS];
}

cbuffer TimeBuffer : register(b2)
{
    float time;
}

StructuredBuffer<InstanceData> InstanceBuffer : register(t0);

struct VSInput
{
    float3 position : POSITION;
    float2 tex : TEXCOORD;
    float3 normal : NORMAL;
    uint instanceID : SV_InstanceID;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    //InstanceData instance = InstanceBuffer[input.instanceID];
    InstanceData instance = instances[input.instanceID];

    float3 worldPosition = (input.position * instance.scale) + instance.position;
    
    // Add wheat sway
    worldPosition.y += sin(time + input.position.x * 0.1f) * (worldPosition.y * 0.1f);
    
    // TODO - add normals

    output.position = mul(float4(worldPosition, 1.0f), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.tex = input.tex;

    return output;
}
