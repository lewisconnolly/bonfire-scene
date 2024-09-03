#include "pch.h"
#include "Shader.h"


Shader::Shader()
{
}


Shader::~Shader()
{
}

bool Shader::InitStandard(ID3D11Device * device, WCHAR * vsFilename, WCHAR * psFilename, D3D11_TEXTURE_ADDRESS_MODE textureAddressMode)
{
	D3D11_BUFFER_DESC	matrixBufferDesc;
	D3D11_SAMPLER_DESC	samplerDesc;
	//D3D11_SAMPLER_DESC	comparisonSamplerDesc;
	D3D11_BUFFER_DESC	lightBufferDesc;
	D3D11_BUFFER_DESC	cameraBufferDesc;
	D3D11_BUFFER_DESC	pLightColorBufferDesc;
	D3D11_BUFFER_DESC	pLightPositionBufferDesc;
	D3D11_BUFFER_DESC	lightViewBufferDesc;

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
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }		
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


	// Setup the description of the camera dynamic constant buffer that is in the vertex shader.
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	// Create the camera constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);

	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);

	// Setup the description of the dynamic constant buffer for point light colours that is in the pixel shader
	pLightColorBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pLightColorBufferDesc.ByteWidth = sizeof(pLightColorBufferType);
	pLightColorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pLightColorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pLightColorBufferDesc.MiscFlags = 0;
	pLightColorBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the pixel shader constant buffer from within this class.
	device->CreateBuffer(&pLightColorBufferDesc, NULL, &m_pLightColorBuffer);

	// Setup the description of the dynamic constant buffer for point light positions that is in the vertex shader.
	pLightPositionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pLightPositionBufferDesc.ByteWidth = sizeof(pLightPositionBufferType);
	pLightPositionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pLightPositionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pLightPositionBufferDesc.MiscFlags = 0;
	pLightPositionBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&pLightPositionBufferDesc, NULL, &m_pLightPositionBuffer);

	// Setup the description of the dynamic constant buffer for light view information that is in the vertex shader.
	lightViewBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightViewBufferDesc.ByteWidth = sizeof(LightViewBufferType);
	lightViewBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightViewBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightViewBufferDesc.MiscFlags = 0;
	lightViewBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightViewBufferDesc, NULL, &m_lightViewBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	//samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressU = textureAddressMode;
	samplerDesc.AddressV = textureAddressMode;
	samplerDesc.AddressW = textureAddressMode;
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

	//Create a comparison sampler state object for shadow mapping
	/*ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.BorderColor[0] = 1.0f;
	comparisonSamplerDesc.BorderColor[1] = 1.0f;
	comparisonSamplerDesc.BorderColor[2] = 1.0f;
	comparisonSamplerDesc.BorderColor[3] = 1.0f;
	comparisonSamplerDesc.MinLOD = 0.f;
	comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	comparisonSamplerDesc.MipLODBias = 0.f;
	comparisonSamplerDesc.MaxAnisotropy = 0;
	comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;*/

	//device->CreateSamplerState(&comparisonSamplerDesc, &m_comparisonSampler_point);

	return true;
}

bool Shader::SetShaderParameters(ID3D11DeviceContext * context, DirectX::SimpleMath::Matrix * world, DirectX::SimpleMath::Matrix * view, DirectX::SimpleMath::Matrix * projection, //context and matrices
								Light *sceneLight1, DirectX::SimpleMath::Vector4 pLightPositions[], DirectX::SimpleMath::Vector4 pLightColors[], Camera *camera1, // lighting
								ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* shadowMap) // textures
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* lightPtr;
	LightViewBufferType* lightViewPtr;
	CameraBufferType* cameraPtr;
	pLightPositionBufferType* pLightPosPtr;
	pLightColorBufferType* pLightColorPtr;
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
	
	context->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource); // Lock the camera constant buffer so it can be written to.	
	cameraPtr = (CameraBufferType*)mappedResource.pData; // Get a pointer to the data in the constant buffer.
	cameraPtr->cameraPosition = camera1->getPosition(); // Copy the camera position into the constant buffer.
	cameraPtr->padding = 0.0f;
	context->Unmap(m_cameraBuffer, 0); // Unlock the camera constant buffer.
	// Now set the camera constant buffer in the vertex shader with the updated values
	// (buffer number is 1 as it is the second buffer in the vertex shader)
	context->VSSetConstantBuffers(1, 1, &m_cameraBuffer);
	
	context->Map(m_pLightPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource); // Lock the point light position constant buffer so it can be written to.	
	pLightPosPtr = (pLightPositionBufferType*)mappedResource.pData; // Get a pointer to the data in the constant buffer.	
	pLightPosPtr->pLightPosition[0] = pLightPositions[0]; // Copy the light position variables into the constant buffer.		
	context->Unmap(m_pLightPositionBuffer, 0); // Unlock the constant buffer.		
	// Finally set the constant buffer in the vertex shader with the updated values.
	context->VSSetConstantBuffers(2, 1, &m_pLightPositionBuffer);

	tLightView = sceneLight1->getView().Transpose();
	tLightProj = sceneLight1->getProjection().Transpose();
	context->Map(m_lightViewBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightViewPtr = (LightViewBufferType*)mappedResource.pData;
	lightViewPtr->lightView = tLightView;
	lightViewPtr->lightProjection = tLightProj;
	context->Unmap(m_lightViewBuffer, 0);
	context->VSSetConstantBuffers(3, 1, &m_lightViewBuffer);

	context->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->ambient = sceneLight1->getAmbientColour();
	lightPtr->diffuse = sceneLight1->getDiffuseColour();	
	lightPtr->position = sceneLight1->getPosition();  
	lightPtr->specularColor = sceneLight1->getSpecularColour();
	lightPtr->specularPower = sceneLight1->getSpecularPower();
	context->Unmap(m_lightBuffer, 0);
	context->PSSetConstantBuffers(0, 1, &m_lightBuffer);	//note the first variable is the mapped buffer ID.  Corresponding to what you set in the PS	
		
	context->Map(m_pLightColorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource); // Lock the light color constant buffer so it can be written to.	
	pLightColorPtr = (pLightColorBufferType*)mappedResource.pData; // Get a pointer to the data in the constant buffer.
	pLightColorPtr->pLightDiffuseColor[0] = pLightColors[0]; // Copy the light color variables into the constant buffer.
	context->Unmap(m_pLightColorBuffer, 0);// Unlock the constant buffer.	
	context->PSSetConstantBuffers(1, 1, &m_pLightColorBuffer); // Finally set the constant buffer in the pixel shader with the updated values.

	//pass the desired texture to the pixel shader.
	context->PSSetShaderResources(0, 1, &texture1);	
	context->PSSetShaderResources(1, 1, &shadowMap);	

	return false;
}

void Shader::EnableShader(ID3D11DeviceContext * context)
{
	context->IASetInputLayout(m_layout);							//set the input layout for the shader to match out geometry
	context->VSSetShader(m_vertexShader.Get(), 0, 0);				//turn on vertex shader
	context->PSSetShader(m_pixelShader.Get(), 0, 0);				//turn on pixel shader
	// Set the sampler state in the pixel shader.
	context->PSSetSamplers(0, 1, &m_sampleState);
	//context->PSSetSamplers(1, 1, &m_comparisonSampler_point);

}
