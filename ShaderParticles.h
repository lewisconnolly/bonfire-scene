#pragma once

#include "DeviceResources.h"
#include "Light.h"
#include "Camera.h"


/////////////
// GLOBALS //
/////////////

class ShaderParticles
{
public:
	ShaderParticles();
	~ShaderParticles();

	bool InitStandard(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename);

	bool SetShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, ID3D11ShaderResourceView* texture);
	bool Render(ID3D11DeviceContext*, int, DirectX::SimpleMath::Matrix*, DirectX::SimpleMath::Matrix*, DirectX::SimpleMath::Matrix*, ID3D11ShaderResourceView*);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	//standard matrix buffer supplied to all shaders
	struct MatrixBufferType
	{
		DirectX::SimpleMath::Matrix world;
		DirectX::SimpleMath::Matrix view;
		DirectX::SimpleMath::Matrix projection;
	};

	//Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader>								m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>								m_pixelShader;
	ID3D11InputLayout*														m_layout;
	ID3D11Buffer*															m_matrixBuffer;
	ID3D11SamplerState*														m_sampleState;	
};

