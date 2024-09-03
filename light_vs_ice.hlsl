// Light vertex shader
// - Normal mapping
// Standard issue vertex shader, apply matrices, pass info to pixel shader

/////////////
// DEFINES //
/////////////
#define NUM_PLIGHTS 1

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CameraBuffer
{
    float3 cameraPosition;
    float padding;
};

cbuffer pLightPositionBuffer
{
    float4 pLightPosition[NUM_PLIGHTS];
};

cbuffer LightViewBuffer
{
    matrix lightView;
    matrix lightProjection;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 viewDirection : TEXCOORD1;
    float3 position3D : TEXCOORD2;
    float3 pLightPos[NUM_PLIGHTS] : TEXCOORD3;
    float4 lightSpacePos : TEXCOORD4;
    float4 refractionPosition : TEXCOORD5;
};

OutputType main(InputType input)
{
    OutputType output;
    matrix viewProjectWorld;
    int i;
    
    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    //Transform the vertex position into projected space from the POV of the light.
    float4 modelPos = mul(input.position, worldMatrix);
    float4 lightSpacePos = mul(modelPos, lightView);
    lightSpacePos = mul(lightSpacePos, lightProjection);
    output.lightSpacePos = lightSpacePos;
    
    // Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

	 // Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, (float3x3) worldMatrix);
	
    // Normalize the normal vector.
    output.normal = normalize(output.normal);

    // Calculate the tangent vector against the world matrix only and then normalize the final value.
    output.tangent = mul(input.tangent, (float3x3) worldMatrix);
    output.tangent = normalize(output.tangent);

    // Calculate the binormal vector against the world matrix only and then normalize the final value.
    output.binormal = mul(input.binormal, (float3x3) worldMatrix);
    output.binormal = normalize(output.binormal);
    
	// world position of vertex (for point light)
    output.position3D = (float3) mul(input.position, worldMatrix);
    
    // Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
    output.viewDirection = cameraPosition.xyz - output.position3D.xyz;
	
    // Normalize the viewing direction vector.
    output.viewDirection = normalize(output.viewDirection);

    for (i = 0; i < NUM_PLIGHTS; i++)
    {
        // Determine the point light positions based on the position of the lights and the position of the vertex in the world.
        output.pLightPos[i] = pLightPosition[i].xyz - output.position3D.xyz;

        // Normalize the light position vectors.
        output.pLightPos[i] = normalize(output.pLightPos[i]);
    }
    
    // Create the view projection world matrix for refraction.
    viewProjectWorld = mul(viewMatrix, projectionMatrix);
    viewProjectWorld = mul(worldMatrix, viewProjectWorld);

    // Calculate the input position against the viewProjectWorld matrix.
    output.refractionPosition = mul(input.position, viewProjectWorld);
    
    return output;
}