#pragma once

#include "DeviceResources.h"
#include "Light.h"
#include "Camera.h"

/////////////
// GLOBALS //
/////////////

class ShaderFire
{
public:
	ShaderFire();
	~ShaderFire();

	//we could extend this to load in only a vertex shader, only a pixel shader etc.  or specialised init for Geometry or domain shader. 
	//All the methods here simply create new versions corresponding to your needs
	bool InitStandard(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename);		//Loads the Vert / pixel Shader pair
	
	bool SetShaderParameters(ID3D11DeviceContext*, DirectX::SimpleMath::Matrix*, DirectX::SimpleMath::Matrix*, DirectX::SimpleMath::Matrix*, ID3D11ShaderResourceView*,
		ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, float, DirectX::SimpleMath::Vector3, DirectX::SimpleMath::Vector3, DirectX::SimpleMath::Vector2,
		DirectX::SimpleMath::Vector2, DirectX::SimpleMath::Vector2, float, float);
	void EnableShader(ID3D11DeviceContext* context);
	
private:
	//standard matrix buffer supplied to all shaders
	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

	struct NoiseBufferType
	{
		float frameTime;
		DirectX::SimpleMath::Vector3 scrollSpeeds;
		DirectX::SimpleMath::Vector3 scales;
		float padding;
	};

	struct DistortionBufferType
	{
		DirectX::SimpleMath::Vector2 distortion1;
		DirectX::SimpleMath::Vector2 distortion2;
		DirectX::SimpleMath::Vector2 distortion3;
		float distortionScale;
		float distortionBias;
	};

	//Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader>								m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>								m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_noiseBuffer;
	ID3D11SamplerState* m_sampleState;
	ID3D11SamplerState* m_sampleState2;
	ID3D11Buffer* m_distortionBuffer;	
};

