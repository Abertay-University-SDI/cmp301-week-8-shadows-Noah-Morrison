// Define number of clumps - don't like having this defined in two different places
#define NUM_WHEAT_CLUMPS 3

cbuffer MatrixBuffer : register(b0)
{
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
    InstanceData instances[NUM_WHEAT_CLUMPS];
}

StructuredBuffer<InstanceData> InstanceBuffer : register(t0);

struct VSInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
    uint instanceID : SV_InstanceID;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    InstanceData instance = InstanceBuffer[input.instanceID];
    //InstanceData instance = instances[input.instanceID];

    float3 worldPosition = (input.position * instance.scale) + instance.position;

    float4 viewPosition = mul(float4(worldPosition, 1.0f), viewMatrix);
    output.position = mul(viewPosition, projectionMatrix);

    output.texcoord = input.texcoord;

    return output;
}
