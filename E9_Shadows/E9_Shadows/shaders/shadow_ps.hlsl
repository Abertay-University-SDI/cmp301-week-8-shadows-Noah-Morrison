// Define light types
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

// Define number of lights
#define NUM_LIGHTS 3

Texture2D shaderTexture : register(t0);
Texture2D directionalDepthMapTexture[NUM_LIGHTS] : register(t1);
//Texture2D pointDepthMapTexture[NUM_LIGHTS] : register(t2);

SamplerState diffuseSampler  : register(s0);
SamplerState shadowSampler : register(s1);

// New
struct Light
{
    float4 diffuse;
    float3 position;
    float3 direction;
    int type;
};

// Old
//cbuffer LightBuffer : register(b0)
//{
//	float4 ambient;
//	float4 diffuse;
//	float3 direction;
//    float3 position;
//};

// New
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

float4 calculatePoint(float3 lightPosition, float3 normal, float4 diffuse, float3 worldPosition)
{
    float3 direction = normalize(lightPosition - worldPosition);
    float intensity = saturate(dot(normal, direction));
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

// Calculate lighting intensity based on direction and normal. Combine with light colour.
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
            result = calculatePoint(light.position, normal, light.diffuse, worldPosition);
            break;
        case LIGHT_TYPE_SPOT:
            //result = calculateSpot(light.position, normal, light.diffuse, light.direction, light.cone, light.attenuation, worldPosition);
            break;
    }
    return result;
}

// Is the gemoetry in our shadow map
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
    // Sample the shadow map (get depth of geometry)
    float depthValue = sMap.Sample(shadowSampler, uv).r;
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;

	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    if (lightDepthValue < depthValue)
    {
        return false;
    }
    return true;
}

float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}

float4 main(InputType input) : SV_TARGET
{
    float shadowMapBias = 0.005f;
    float4 colour = float4(0.f, 0.f, 0.f, 0.f);
    float4 textureColour = shaderTexture.Sample(diffuseSampler, input.tex);
	
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        // Calculate the projected texture coordinates.
        float2 pTexCoord = getProjectiveCoords(input.lightViewPos[i]);
        
        // Shadow test. Is or isn't in shadow
        //if (hasDepthData(pTexCoord))
        //{
        // Has depth map data
            if (!isInShadow(directionalDepthMapTexture[i], pTexCoord, input.lightViewPos[i], shadowMapBias))
            {
        // is NOT in shadow, therefore light
                //colour += calculateLighting(-lights[i].direction, input.normal, lights[i].diffuse);
                colour += calculateMultipleLighting(lights[i], input.normal, input.worldPosition);
        }
        //}
    }
    
    // TODO - TEMP - Test Directional Lights are working properly
    //float4 totalLightContribution = float4(0.0, 0.0, 0.0, 0.0);
        
    //for (int i = 0; i < NUM_LIGHTS; i++)
    //{
    //    totalLightContribution += calculateLighting(-lights[i].direction, input.normal, lights[i].diffuse);
    //}
    
    //totalLightContribution += ambient;

    //return saturate(totalLightContribution * textureColour);
   
    colour += ambient;
    colour = saturate(colour);
    //return float4(1.0f, 1.0f, 1.0f, 1.0f);
    return colour * textureColour;
}