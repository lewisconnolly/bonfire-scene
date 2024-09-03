///////////////////////////////////////////////////////////////////////////////
// Filename: particlesystemclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _PARTICLESYSTEMCLASS_H_
#define _PARTICLESYSTEMCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11_1.h>
#include <DirectXMath.h>
#include "SimpleMath.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: ParticleSystemClass
////////////////////////////////////////////////////////////////////////////////
class ParticleSystemClass
{
private:

	struct ParticleType
	{
		float positionX, positionY, positionZ;
		float red, green, blue;
		float velocity;
		bool active;
	};

	struct VertexType
	{
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector2 texture;
		DirectX::SimpleMath::Vector4 color;
	};

public:
	ParticleSystemClass();
	ParticleSystemClass(const ParticleSystemClass&);
	~ParticleSystemClass();

	bool Initialize(ID3D11Device*);
	void Shutdown();
	bool Frame(float, ID3D11DeviceContext*);
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

private:
	bool InitializeParticleSystem();
	void ShutdownParticleSystem();

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();

	void EmitParticles(float);
	void UpdateParticles(float);
	void KillParticles();

	bool UpdateBuffers(ID3D11DeviceContext*);

	void RenderBuffers(ID3D11DeviceContext*);

private:

	float m_particleDeviationX, m_particleDeviationY, m_particleDeviationZ;
	float m_particleVelocity, m_particleVelocityVariation;
	float m_particleSize, m_particlesPerSecond;
	int m_maxParticles;
	int m_currentParticleCount;
	float m_accumulatedTime;	
	ParticleType* m_particleList;

	int m_vertexCount, m_indexCount;
	VertexType* m_vertices;
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
};

#endif