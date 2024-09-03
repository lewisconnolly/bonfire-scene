#pragma once

#include "DeviceResources.h"
#include "Light.h"
#include "Camera.h"

/////////////
// GLOBALS //
/////////////

//Class from which we create all shader objects used by the framework
//This single class can be expanded to accomodate shaders of all different types with different parameters
class ShaderIce
{
public:
	ShaderIce();
	~ShaderIce();

	//we could extend this to load in only a vertex shader, only a pixel shader etc.  or specialised init for Geometry or domain shader. 
	//All the methods here simply create new versions corresponding to your needs
	bool InitStandard(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename, D3D11_TEXTURE_ADDRESS_MODE textureAddressMode);		//Loads the Vert / pixel Shader pair

	bool SetShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, // context and matrices
		Light* sceneLight1, DirectX::SimpleMath::Vector4 pLightPositions[], DirectX::SimpleMath::Vector4 pLightColors[], Camera* camera1, // lighting
		ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* normalMap, ID3D11ShaderResourceView* shadowMap, ID3D11ShaderResourceView* refractionTexture); // textures
	void EnableShader(ID3D11DeviceContext* context);

private:
	//standard matrix buffer supplied to all shaders
	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

	//buffer for information of directional light for use in shadow mapping
	struct LightViewBufferType
	{
		DirectX::SimpleMath::Matrix lightView;
		DirectX::SimpleMath::Matrix lightProjection;
	};

	//buffer for information of a single light
	struct LightBufferType
	{
		DirectX::SimpleMath::Vector4 ambient;
		DirectX::SimpleMath::Vector4 diffuse;
		DirectX::SimpleMath::Vector3 position;
		float specularPower;
		DirectX::SimpleMath::Vector4 specularColor;
	};

	//buffer to pass in camera world Position
	struct CameraBufferType
	{
		DirectX::SimpleMath::Vector3 cameraPosition;
		float padding;
	};

	//point light buffers
	struct pLightColorBufferType
	{
		DirectX::SimpleMath::Vector4 pLightDiffuseColor[1];
	};

	struct pLightPositionBufferType
	{
		DirectX::SimpleMath::Vector4 pLightPosition[1];
	};

	//Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader>								m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>								m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;
	//ID3D11SamplerState*														m_comparisonSampler_point;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11Buffer* m_lightViewBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_pLightColorBuffer;
	ID3D11Buffer* m_pLightPositionBuffer;
};

