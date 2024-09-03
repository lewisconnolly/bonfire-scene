// Light pixel shader
// Calculate diffuse lighting for a single directional light(also texturing)

/////////////
// DEFINES //
/////////////
#define NUM_PLIGHTS 1

Texture2D shaderTexture : register(t0);
Texture2D shadowMap : register(t1);
SamplerState SampleType : register(s0);

SamplerComparisonState cmpSampler
{
   // sampler state
    Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
    AddressU = MIRROR;
    AddressV = MIRROR;
 
   // sampler comparison state
    ComparisonFunc = LESS_EQUAL;
};

float2 texOffset(int u, int v)
{
    return float2(u * 1.0f / 2048.0, v * 1.0f / 2048.0);
}

cbuffer LightBuffer : register(b0)
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightPosition;
    float specularPower;
    float4 specularColor;
};

cbuffer pLightColorBuffer
{
    float4 pLightDiffuseColor[NUM_PLIGHTS];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;   
    float3 viewDirection : TEXCOORD1;
    float3 position3D : TEXCOORD2;
    float3 pLightPos[NUM_PLIGHTS] : TEXCOORD3;
    float4 lightSpacePos : TEXCOORD4;
};

float4 main(InputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;

    // point light properties
    float pLightIntensity[NUM_PLIGHTS];
    float4 pLightColorArray[NUM_PLIGHTS];
    float4 pLightColorSum;
    float4 pLightColor;
    int i;
    
    float shadowMapBias = 0.001;
    
    //re-homogenize position after interpolation
    input.lightSpacePos.xyz /= input.lightSpacePos.w;
 
    //if position is not visible to the light - dont illuminate it
    //results in hard light frustum
    if (input.lightSpacePos.x < -1.0f || input.lightSpacePos.x > 1.0f || input.lightSpacePos.y < -1.0f || input.lightSpacePos.y > 1.0f || input.lightSpacePos.z < 0.0f || input.lightSpacePos.z > 1.0f)
        return ambientColor;
 
    //transform clip space coords to texture space coords (-1:1 to 0:1)
    input.lightSpacePos.x = input.lightSpacePos.x / 2 + 0.5;
    input.lightSpacePos.y = input.lightSpacePos.y / -2 + 0.5;
 
    //apply shadow map bias
    input.lightSpacePos.z -= shadowMapBias;
 
    //PCF sampling for shadow map
    float sum = 0;
    float x, y;
 
    //perform PCF filtering on a 4 x 4 texel neighborhood
    for (y = -1.5; y <= 1.5; y += 1.0)
    {
        for (x = -1.5; x <= 1.5; x += 1.0)
        {
            sum += shadowMap.SampleCmpLevelZero(cmpSampler, input.lightSpacePos.xy + texOffset(x, y), input.lightSpacePos.z);
        }
    }
    
    float shadowFactor = sum / 16.0;
    
    for (i = 0; i < NUM_PLIGHTS; i++)
    {
        // Calculate the different amounts of light on this pixel based on the positions of the lights.
        pLightIntensity[i] = saturate(dot(input.normal, input.pLightPos[i]));

        // Determine the diffuse color amount of each of the four lights.
        pLightColorArray[i] = pLightDiffuseColor[i] * pLightIntensity[i];
    }
    
    // Initialize the sum of colors.
    pLightColorSum = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Add all of the light colors up.
    for (i = 0; i < NUM_PLIGHTS; i++)
    {
        pLightColorSum.r += pLightColorArray[i].r;
        pLightColorSum.g += pLightColorArray[i].g;
        pLightColorSum.b += pLightColorArray[i].b;
    }
    
    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    // Default colour is ambient light value for all pixels
    color = ambientColor;
    
    // Invert the light direction for calculations.
    lightDir = normalize(input.position3D - lightPosition);

    // Calculate the amount of light on this pixel.
    lightIntensity = saturate(dot(input.normal, -lightDir));
    
    if (lightIntensity > 0.0f)
    {
    // Determine the final diffuse color based on the diffuse color and the amount of light intensity.
        color += (diffuseColor * lightIntensity);

    // Saturate the ambient and diffuse color.
        color = saturate(color);
    }
    
    // Multiply the texture pixel, the final diffuse color and the point light colour to get the final pixel color result
    color.r += pLightColorSum.r;
    color.g += pLightColorSum.g;
    color.b += pLightColorSum.b;
    
    color = saturate(color) * shadowFactor * textureColor;
    color.a = textureColor.a;
        
    return color;
}

