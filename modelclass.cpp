////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "modelclass.h"

using namespace DirectX;

ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	normalMapping = false;
	shadowMapping = false;

}
ModelClass::~ModelClass()
{
}

bool ModelClass::InitializeBox(ID3D11Device* device, float xwidth, float yheight, float zdepth)
{
	GeometricPrimitive::CreateBox(preFabVertices, preFabIndices, DirectX::SimpleMath::Vector3(xwidth, yheight, zdepth), false);
	m_vertexCount = preFabVertices.size();
	m_indexCount = preFabIndices.size();

	bool result;
	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
	return true;
}

bool ModelClass::InitializeModel(ID3D11Device* device, char* filename)
{
	LoadModel(filename);
	if (normalMapping)
	{
		CalculateModelVectors(); // Calculate the tangent and binormal vectors for the model.
	}
	InitializeBuffers(device);
	return false;
}

void ModelClass::Shutdown()
{

	// Shutdown the vertex and index buffers.
	ShutdownBuffers();

	// Release the model data.
	ReleaseModel();

	return;
}


void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);
	deviceContext->DrawIndexed(m_indexCount, 0, 0);

	return;
}


int ModelClass::GetIndexCount()
{
	return m_indexCount;
}


bool ModelClass::InitializeBuffers(ID3D11Device* device)
{	
	VertexType* vertices;
	VertexTypeNM* verticesNM;
	VertexTypeSM* verticesSM;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;

	// Create the vertex arrays (only one will be used)
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	verticesNM = new VertexTypeNM[m_vertexCount];
	if (!verticesNM)
	{
		return false;
	}

	verticesSM = new VertexTypeSM[m_vertexCount];
	if (!verticesSM)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if(!indices)
	{
		return false;
	}
	
	// Load the vertex array and index array with data from the pre-fab
	if (!normalMapping)
	{				
		if (!shadowMapping)
		{
			for (i = 0; i < m_vertexCount; i++)
			{
				vertices[i].position = DirectX::SimpleMath::Vector3(preFabVertices[i].position.x, preFabVertices[i].position.y, preFabVertices[i].position.z);
				vertices[i].texture = DirectX::SimpleMath::Vector2(preFabVertices[i].textureCoordinate.x, preFabVertices[i].textureCoordinate.y);
				vertices[i].normal = DirectX::SimpleMath::Vector3(preFabVertices[i].normal.x, preFabVertices[i].normal.y, preFabVertices[i].normal.z);
			}
		}
		else
		{
			for (i = 0; i < m_vertexCount; i++)
			{
				verticesSM[i].position = DirectX::SimpleMath::Vector3(preFabVerticesSM[i].position.x, preFabVerticesSM[i].position.y, preFabVerticesSM[i].position.z);				
			}
		}
	}
	else
	{
		for (i = 0; i < m_vertexCount; i++)
		{
			verticesNM[i].position = DirectX::SimpleMath::Vector3(preFabVerticesNM[i].position.x, preFabVerticesNM[i].position.y, preFabVerticesNM[i].position.z);
			verticesNM[i].texture = DirectX::SimpleMath::Vector2(preFabVerticesNM[i].textureCoordinate.x, preFabVerticesNM[i].textureCoordinate.y);
			verticesNM[i].normal = DirectX::SimpleMath::Vector3(preFabVerticesNM[i].normal.x, preFabVerticesNM[i].normal.y, preFabVerticesNM[i].normal.z);
			verticesNM[i].tangent = DirectX::SimpleMath::Vector3(preFabVerticesNM[i].tangent.x, preFabVerticesNM[i].tangent.y, preFabVerticesNM[i].tangent.z);
			verticesNM[i].binormal = DirectX::SimpleMath::Vector3(preFabVerticesNM[i].binormal.x, preFabVerticesNM[i].binormal.y, preFabVerticesNM[i].binormal.z);
		}
	}

	for (i = 0; i < m_indexCount; i++)
	{
		indices[i] = preFabIndices[i];
	}

	// Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	if (!normalMapping)
	{
		if (!shadowMapping)
		{
			vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
		}
		else
		{
			vertexBufferDesc.ByteWidth = sizeof(VertexTypeSM) * m_vertexCount;
		}
	}
	else
	{
		vertexBufferDesc.ByteWidth = sizeof(VertexTypeNM) * m_vertexCount;
	}

    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	if (!normalMapping)
	{
		if (!shadowMapping)
		{
			vertexData.pSysMem = vertices;
		}
		else
		{
			vertexData.pSysMem = verticesSM;
		}
	}
	else
	{
		vertexData.pSysMem = verticesNM;
	}

	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete [] vertices;
	delete [] verticesNM;
	delete [] verticesSM;
	vertices = 0;
	verticesNM = 0;
	verticesSM = 0;

	delete [] indices;
	indices = 0;

	return true;
}


void ModelClass::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	if (!normalMapping)
	{
		if (!shadowMapping)
		{
			stride = sizeof(VertexType);
		}
		else
		{
			stride = sizeof(VertexTypeSM);
		}
	}
	else
	{
		stride = sizeof(VertexTypeNM);
	}
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


bool ModelClass::LoadModel(char* filename)
{
	std::vector<XMFLOAT3> verts;
	std::vector<XMFLOAT3> norms;
	std::vector<XMFLOAT2> texCs;
	std::vector<unsigned int> faces;

	FILE* file;// = fopen(filename, "r");
	errno_t err;
	err = fopen_s(&file, filename, "r");
	if (err != 0)
		//if (file == NULL)
	{
		return false;
	}

	while (true)
	{
		char lineHeader[128];

		// Read first word of the line
		int res = fscanf_s(file, "%s", lineHeader, sizeof(lineHeader));
		if (res == EOF)
		{
			break; // exit loop
		}
		else // Parse
		{
			if (strcmp(lineHeader, "v") == 0) // Vertex
			{
				XMFLOAT3 vertex;
				fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				verts.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0) // Tex Coord
			{
				XMFLOAT2 uv;
				fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
				texCs.push_back(uv);
			}
			else if (strcmp(lineHeader, "vn") == 0) // Normal
			{
				XMFLOAT3 normal;
				fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				norms.push_back(normal);
			}
			else if (strcmp(lineHeader, "f") == 0) // Face
			{
				unsigned int face[9];
				int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &face[0], &face[1], &face[2],
					&face[3], &face[4], &face[5],
					&face[6], &face[7], &face[8]);
				if (matches != 9)
				{
					// Parser error, or not triangle faces
					return false;
				}

				for (int i = 0; i < 9; i++)
				{
					faces.push_back(face[i]);
				}


			}
		}
	}

	int vIndex = 0, nIndex = 0, tIndex = 0;
	int numFaces = (int)faces.size() / 9;

	//// Create the model using the vertex count that was read in.
	m_vertexCount = numFaces * 3;
//	model = new ModelType[vertexCount];

	// "Unroll" the loaded obj information into a list of triangles.
	
	if (!normalMapping)
	{
		if (!shadowMapping)
		{
			for (int f = 0; f < (int)faces.size(); f += 3)
			{
				VertexPositionNormalTexture tempVertex;
				tempVertex.position.x = verts[(faces[f + 0] - 1)].x;
				tempVertex.position.y = verts[(faces[f + 0] - 1)].y;
				tempVertex.position.z = verts[(faces[f + 0] - 1)].z;

				tempVertex.textureCoordinate.x = texCs[(faces[f + 1] - 1)].x;
				tempVertex.textureCoordinate.y = texCs[(faces[f + 1] - 1)].y;

				tempVertex.normal.x = norms[(faces[f + 2] - 1)].x;
				tempVertex.normal.y = norms[(faces[f + 2] - 1)].y;
				tempVertex.normal.z = norms[(faces[f + 2] - 1)].z;

				//increase index count
				preFabVertices.push_back(tempVertex);

				int tempIndex;
				tempIndex = vIndex;
				preFabIndices.push_back(tempIndex);
				vIndex++;
			}
		}
		else
		{
			for (int f = 0; f < (int)faces.size(); f += 3)
			{
				VertexPosition tempVertex;
				tempVertex.position.x = verts[(faces[f + 0] - 1)].x;
				tempVertex.position.y = verts[(faces[f + 0] - 1)].y;
				tempVertex.position.z = verts[(faces[f + 0] - 1)].z;
				
				//increase index count
				preFabVerticesSM.push_back(tempVertex);

				int tempIndex;
				tempIndex = vIndex;
				preFabIndices.push_back(tempIndex);
				vIndex++;
			}
		}
	}
	else
	{
		for (int f = 0; f < (int)faces.size(); f += 3)
		{
			VertexPositionNormalTextureTangentBinormal tempVertex;
			tempVertex.position.x = verts[(faces[f + 0] - 1)].x;
			tempVertex.position.y = verts[(faces[f + 0] - 1)].y;
			tempVertex.position.z = verts[(faces[f + 0] - 1)].z;

			tempVertex.textureCoordinate.x = texCs[(faces[f + 1] - 1)].x;
			tempVertex.textureCoordinate.y = texCs[(faces[f + 1] - 1)].y;

			tempVertex.normal.x = norms[(faces[f + 2] - 1)].x;
			tempVertex.normal.y = norms[(faces[f + 2] - 1)].y;
			tempVertex.normal.z = norms[(faces[f + 2] - 1)].z;

			//tangent and binormals will be calculated later
			tempVertex.tangent.x = 0;
			tempVertex.tangent.y = 0;
			tempVertex.tangent.z = 0;

			tempVertex.binormal.x = 0;
			tempVertex.binormal.y = 0;
			tempVertex.binormal.z = 0;

			//increase index count
			preFabVerticesNM.push_back(tempVertex);

			int tempIndex;
			tempIndex = vIndex;
			preFabIndices.push_back(tempIndex);
			vIndex++;
		}
	}
	m_indexCount = vIndex;

	verts.clear();
	norms.clear();
	texCs.clear();
	faces.clear();
	return true;
}


void ModelClass::ReleaseModel()
{
	return;
}


void ModelClass::CalculateModelVectors()
{
	int faceCount, i, index;
	TempVertexType vertex1, vertex2, vertex3;
	VectorType tangent, binormal;


	// Calculate the number of faces in the model.
	faceCount = m_vertexCount / 3;

	// Initialize the index to the model data.
	index = 0;

	// Go through all the faces and calculate the the tangent and binormal vectors.
	for (i = 0; i < faceCount; i++)
	{
		// Get the three vertices for this face from the model and add to our array with tangents and binormals
		vertex1.x = preFabVerticesNM[index].position.x;
		vertex1.y = preFabVerticesNM[index].position.y;
		vertex1.z = preFabVerticesNM[index].position.z;
		vertex1.tu = preFabVerticesNM[index].textureCoordinate.x;
		vertex1.tv = preFabVerticesNM[index].textureCoordinate.y;
		index++;

		vertex2.x = preFabVerticesNM[index].position.x;
		vertex2.y = preFabVerticesNM[index].position.y;
		vertex2.z = preFabVerticesNM[index].position.z;
		vertex2.tu = preFabVerticesNM[index].textureCoordinate.x;
		vertex2.tv = preFabVerticesNM[index].textureCoordinate.y;
		index++;

		vertex3.x = preFabVerticesNM[index].position.x;
		vertex3.y = preFabVerticesNM[index].position.y;
		vertex3.z = preFabVerticesNM[index].position.z;
		vertex3.tu = preFabVerticesNM[index].textureCoordinate.x;
		vertex3.tv = preFabVerticesNM[index].textureCoordinate.y;
		index++;

		// Calculate the tangent and binormal of that face.
		CalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);

		// Store the tangent and binormal for this face back in the model structure.
		preFabVerticesNM[index - 1].tangent.x = tangent.x;
		preFabVerticesNM[index - 1].tangent.y = tangent.y;
		preFabVerticesNM[index - 1].tangent.z = tangent.z;
		preFabVerticesNM[index - 1].binormal.x = binormal.x;
		preFabVerticesNM[index - 1].binormal.y = binormal.y;
		preFabVerticesNM[index - 1].binormal.z = binormal.z;

		preFabVerticesNM[index - 2].tangent.x = tangent.x;
		preFabVerticesNM[index - 2].tangent.y = tangent.y;
		preFabVerticesNM[index - 2].tangent.z = tangent.z;
		preFabVerticesNM[index - 2].binormal.x = binormal.x;
		preFabVerticesNM[index - 2].binormal.y = binormal.y;
		preFabVerticesNM[index - 2].binormal.z = binormal.z;

		preFabVerticesNM[index - 3].tangent.x = tangent.x;
		preFabVerticesNM[index - 3].tangent.y = tangent.y;
		preFabVerticesNM[index - 3].tangent.z = tangent.z;
		preFabVerticesNM[index - 3].binormal.x = binormal.x;
		preFabVerticesNM[index - 3].binormal.y = binormal.y;
		preFabVerticesNM[index - 3].binormal.z = binormal.z;
	}

	return;
}

void ModelClass::CalculateTangentBinormal(TempVertexType vertex1, TempVertexType vertex2, TempVertexType vertex3, VectorType& tangent, VectorType& binormal)
{
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];
	float den;
	float length;


	// Calculate the two vectors for this face.
	vector1[0] = vertex2.x - vertex1.x;
	vector1[1] = vertex2.y - vertex1.y;
	vector1[2] = vertex2.z - vertex1.z;

	vector2[0] = vertex3.x - vertex1.x;
	vector2[1] = vertex3.y - vertex1.y;
	vector2[2] = vertex3.z - vertex1.z;

	// Calculate the tu and tv texture space vectors.
	tuVector[0] = vertex2.tu - vertex1.tu;
	tvVector[0] = vertex2.tv - vertex1.tv;

	tuVector[1] = vertex3.tu - vertex1.tu;
	tvVector[1] = vertex3.tv - vertex1.tv;

	// Calculate the denominator of the tangent/binormal equation.
	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
	tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
	tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

	binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
	binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
	binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

	// Calculate the length of this normal.
	length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

	// Normalize the normal and then store it
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;

	// Calculate the length of this normal.
	length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

	// Normalize the normal and then store it
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;

	return;
}