#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#define NUM_LIGHTS 6

Texture2D shaderTexture : register(t0);
Texture2D pointDepthMapTexture[NUM_LIGHTS] : register(t1);

SamplerState diffuseSampler : register(s0);
SamplerState shadowSampler : register(s1);

struct Light
{
    float4 diffuse;
    float3 position;
    float3 direction;
    int type;
};

cbuffer LightBuffer : register(b0)
{
    float4 ambient;
    
    Light lights[NUM_LIGHTS];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    
    float4 lightViewPos[NUM_LIGHTS] : TEXCOORD1;
    float3 worldPosition : WORLD;
};

float4 calculateDirectional(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = diffuse * intensity;
    return colour;
}

float4 calculatePoint(float3 lightPosition, float3 normal, float4 diffuse, float3 worldPosition, float3 lightDirection)
{
    float3 direction = normalize(lightPosition - worldPosition);
    //float intensity = saturate(dot(normal, direction));
    float intensity = 0.0f;
    
    //float spotEffect = dot(direction, lightDirection);
    
    float spotEffect[3] = { dot(normalize(direction.xy), lightDirection.xy),
                            dot(normalize(direction.zy), lightDirection.zy),
                            dot(normalize(direction.xz), lightDirection.xz)
    };
    
    float coneEffect = cos(radians(45));
    
    if (lightDirection.x == 0.0f && lightDirection.z == 0.0f)
    {
        if (spotEffect[0] >= coneEffect && spotEffect[1] >= coneEffect)
        {
            intensity = saturate(dot(normal, direction));
        }
    }
    else if (lightDirection.x == 0.0f && lightDirection.y == 0.0f)
    {
        if (spotEffect[1] >= coneEffect && spotEffect[2] >= coneEffect)
        {
            intensity = saturate(dot(normal, direction));
        }
    }
    else if (lightDirection.z == 0.0f && lightDirection.y == 0.0f)
    {
        if (spotEffect[0] >= coneEffect && spotEffect[2] >= coneEffect)
        {
            intensity = saturate(dot(normal, direction));
        }
    }
    
    //if (spotEffect >= coneEffect)
    //{
    //    intensity = saturate(dot(normal, direction));
    //}
    
    float4 colour = diffuse * intensity;
    return colour;
}

float4 calculateSpot(
float3 position, float3 normal, float4 diffuse, float3 direction, float coneAngle, float3 attenuationVector, float3 worldPosition)
{
    float3 lightVector = normalize(position - worldPosition);
    float distance = length(lightVector);
    float spotEffect = saturate(dot(lightVector, direction));
    float coneEffect = cos(coneAngle);
    
    float intensity = 0.0f;
    if (spotEffect >= coneEffect)
    {
        float intensity = saturate(dot(normal, lightVector));
        
        float attenuation = 1.0 / (attenuationVector.x + attenuationVector.y * distance + attenuationVector.z * distance * distance);
        
        intensity = intensity * attenuation * spotEffect;
    }
    
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float4 calculateMultipleLighting(Light light, float3 normal, float3 worldPosition)
{
    float4 result = float4(0.0, 0.0, 0.0, 0.0);
    
    switch (light.type)
    {
        case LIGHT_TYPE_DIRECTIONAL:
            result = calculateDirectional(-light.direction, normal, light.diffuse);
            break;
        case LIGHT_TYPE_POINT:
            result = calculatePoint(light.position, normal, light.diffuse, worldPosition, -light.direction);
            //result = calculateDirectional(-light.direction, normal, light.diffuse);
            break;
        case LIGHT_TYPE_SPOT:
            //result = calculateSpot(light.position, normal, light.diffuse, light.direction, light.cone, light.attenuation, worldPosition);
            break;
    }
    return result;
}

bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false;
    }
    return true;
}

bool isInShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    float depthValue = sMap.Sample(shadowSampler, uv).r;

    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;

    if (lightDepthValue < depthValue)
    {
        return false;
    }
    return true;
}

float2 getProjectiveCoords(float4 lightViewPosition)
{
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}

float4 main(InputType input) : SV_TARGET
{
    float shadowMapBias = 0.0001f;
    float4 colour = float4(0.f, 0.f, 0.f, 0.f);
    float4 textureColour = shaderTexture.Sample(diffuseSampler, input.tex);
	
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        float2 pTexCoord = getProjectiveCoords(input.lightViewPos[i]);

        if (!isInShadow(pointDepthMapTexture[i], pTexCoord, input.lightViewPos[i], shadowMapBias))
        {
            colour += calculateMultipleLighting(lights[i], input.normal, input.worldPosition);
        }
    }
   
    colour += ambient;
    return saturate(colour * textureColour);
}