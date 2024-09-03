#include "pch.h"
#include "ShaderFire.h"


ShaderFire::ShaderFire()
{
	m_vertexShader.Reset();
	m_pixelShader.Reset();
	m_layout = 0;
	m_matrixBuffer = 0;
	m_noiseBuffer = 0;
	m_sampleState = 0;
	m_sampleState2 = 0;
	m_distortionBuffer = 0;
}


ShaderFire::~ShaderFire()
{
}


bool ShaderFire::InitStandard(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC	matrixBufferDesc;
	D3D11_BUFFER_DESC noiseBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_SAMPLER_DESC samplerDesc2;
	D3D11_BUFFER_DESC distortionBufferDesc;

	//LOAD SHADER:	VERTEX
	auto vertexShaderBuffer = DX::ReadData(vsFilename);
	HRESULT result = device->CreateVertexShader(vertexShaderBuffer.data(), vertexShaderBuffer.size(), NULL, &m_vertexShader);
	if (result != S_OK)
	{
		//if loading failed.  
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the MeshClass and in the shader.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}		
	};

	// Get a count of the elements in the layout.
	unsigned int numElements;
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer.data(), vertexShaderBuffer.size(), &m_layout);


	//LOAD SHADER:	PIXEL
	auto pixelShaderBuffer = DX::ReadData(psFilename);
	result = device->CreatePixelShader(pixelShaderBuffer.data(), pixelShaderBuffer.size(), NULL, &m_pixelShader);
	if (result != S_OK)
	{
		//if loading failed. 
		return false;
	}

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);

	// Setup the description of the dynamic noise constant buffer that is in the vertex shader.
	noiseBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	noiseBufferDesc.ByteWidth = sizeof(NoiseBufferType);
	noiseBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	noiseBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	noiseBufferDesc.MiscFlags = 0;
	noiseBufferDesc.StructureByteStride = 0;

	// Create the noise buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&noiseBufferDesc, NULL, &m_noiseBuffer);
	
	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	device->CreateSamplerState(&samplerDesc, &m_sampleState);

	// Create a second texture sampler state description for a Clamp sampler.
	samplerDesc2.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc2.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc2.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc2.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc2.MipLODBias = 0.0f;
	samplerDesc2.MaxAnisotropy = 1;
	samplerDesc2.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc2.BorderColor[0] = 0;
	samplerDesc2.BorderColor[1] = 0;
	samplerDesc2.BorderColor[2] = 0;
	samplerDesc2.BorderColor[3] = 0;
	samplerDesc2.MinLOD = 0;
	samplerDesc2.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	device->CreateSamplerState(&samplerDesc2, &m_sampleState2);

	// Setup the description of the dynamic distortion constant buffer that is in the pixel shader.
	distortionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	distortionBufferDesc.ByteWidth = sizeof(DistortionBufferType);
	distortionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	distortionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	distortionBufferDesc.MiscFlags = 0;
	distortionBufferDesc.StructureByteStride = 0;

	// Create the distortion buffer pointer so we can access the pixel shader constant buffer from within this class.
	device->CreateBuffer(&distortionBufferDesc, NULL, &m_distortionBuffer);	
	

	return true;
}

bool ShaderFire::SetShaderParameters(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, //context and matrices
	ID3D11ShaderResourceView* fireTexture,ID3D11ShaderResourceView* noiseTexture, ID3D11ShaderResourceView* alphaTexture, // textures
	float frameTime, DirectX::SimpleMath::Vector3 scrollSpeeds, DirectX::SimpleMath::Vector3 scales, DirectX::SimpleMath::Vector2 distortion1,
	DirectX::SimpleMath::Vector2 distortion2, DirectX::SimpleMath::Vector2 distortion3, float distortionScale, float distortionBias) 	
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	NoiseBufferType* dataPtr2;
	DistortionBufferType* dataPtr3;
	DirectX::SimpleMath::Matrix  tworld, tview, tproj;
	DirectX::SimpleMath::Matrix  tLightView, tLightProj;

	// Transpose the matrices to prepare them for the shader.
	tworld = world->Transpose();
	tview = view->Transpose();
	tproj = projection->Transpose();
	context->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	context->Unmap(m_matrixBuffer, 0);
	context->VSSetConstantBuffers(0, 1, &m_matrixBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the VS

	// Lock the noise constant buffer so it can be written to.
	context->Map(m_noiseBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr2 = (NoiseBufferType*)mappedResource.pData;
	dataPtr2->frameTime = frameTime;
	dataPtr2->scrollSpeeds = scrollSpeeds;
	dataPtr2->scales = scales;
	dataPtr2->padding = 0.0f;
	context->Unmap(m_noiseBuffer, 0);
	context->VSSetConstantBuffers(1, 1, &m_noiseBuffer);

	context->Map(m_distortionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr3 = (DistortionBufferType*)mappedResource.pData;
	dataPtr3->distortion1 = distortion1;
	dataPtr3->distortion2 = distortion2;
	dataPtr3->distortion3 = distortion3;
	dataPtr3->distortionScale = distortionScale;
	dataPtr3->distortionBias = distortionBias;
	context->Unmap(m_distortionBuffer, 0);
	
	// Now set the distortion constant buffer in the pixel shader with the updated values.
	context->PSSetConstantBuffers(0, 1, &m_distortionBuffer);

	// Set the three shader texture resources in the pixel shader.
	context->PSSetShaderResources(0, 1, &fireTexture);
	context->PSSetShaderResources(1, 1, &noiseTexture);
	context->PSSetShaderResources(2, 1, &alphaTexture);

	return false;
}


void ShaderFire::EnableShader(ID3D11DeviceContext* context)
{
	context->IASetInputLayout(m_layout);							//set the input layout for the shader to match out geometry
	context->VSSetShader(m_vertexShader.Get(), 0, 0);				//turn on vertex shader
	context->PSSetShader(m_pixelShader.Get(), 0, 0);				//turn on pixel shader
	// Set the sampler state in the pixel shader.
	context->PSSetSamplers(0, 1, &m_sampleState);
	context->PSSetSamplers(1, 1, &m_sampleState2);
	//context->PSSetSamplers(1, 1, &m_comparisonSampler_point);

}
