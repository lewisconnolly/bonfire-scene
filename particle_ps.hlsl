Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 color : COLOR;
};

float4 main(InputType input) : SV_TARGET
{
    float4 textureColor;
    float4 finalColor;
    
    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    // Combine the texture color and the particle color to get the final color result.
    finalColor = textureColor * input.color;
    
    return finalColor;
}