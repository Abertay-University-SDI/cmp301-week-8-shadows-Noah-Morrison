cbuffer MatrixBuffer : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct InstanceData
{
    float3 Position;
    float3 Scale;
    float4 Rotation;
};

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

    float3 worldPosition = (input.position * instance.Scale) + instance.Position;

    float4 viewPosition = mul(float4(worldPosition, 1.0f), viewMatrix);
    output.position = mul(viewPosition, projectionMatrix);

    output.texcoord = input.texcoord;

    return output;
}
