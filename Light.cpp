// Light class
// Holds data that represents a single light source
#include "pch.h"
#include "light.h"


Light::Light()
{
	m_ambientColour	=	DirectX::SimpleMath::Vector4(0.0f,0.0f,0.0f, 0.0f);
	m_diffuseColour	=	DirectX::SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	m_direction	=		DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	m_specularColour=	DirectX::SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	m_specularPower	=	0.0f;
	m_position	=		DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	m_lookAt	=		DirectX::SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	m_lightProjection = DirectX::SimpleMath::Matrix::Identity;
	m_lightView =		DirectX::SimpleMath::Matrix::Identity;
};


Light::~Light()
{
}

void Light::setAmbientColour(float red, float green, float blue, float alpha)
{
	m_ambientColour = DirectX::SimpleMath::Vector4(red, green, blue, alpha);
}

void Light::setDiffuseColour(float red, float green, float blue, float alpha)
{
	m_diffuseColour = DirectX::SimpleMath::Vector4(red, green, blue, alpha);
}

void Light::setDirection(float x, float y, float z)
{
	m_direction = DirectX::SimpleMath::Vector3(x, y, z);
}

void Light::setSpecularColour(float red, float green, float blue, float alpha)
{
	m_specularColour = DirectX::SimpleMath::Vector4(red, green, blue, alpha);
}

void Light::setSpecularPower(float power)
{
	m_specularPower = power;
}

void Light::setPosition(float x, float y, float z)
{
	m_position = XMVectorSet(x, y, z, 1.0f);
}

DirectX::SimpleMath::Vector4 Light::getAmbientColour()
{
	return m_ambientColour;
}

DirectX::SimpleMath::Vector4 Light::getDiffuseColour()
{
	return m_diffuseColour;
}


DirectX::SimpleMath::Vector3 Light::getDirection()
{
	return m_direction;
}

DirectX::SimpleMath::Vector4 Light::getSpecularColour()
{
	return m_specularColour;
}


float Light::getSpecularPower()
{
	return m_specularPower;
}

DirectX::SimpleMath::Vector3 Light::getPosition()
{
	return m_position;
}

DirectX::SimpleMath::Vector4 Light::getLookAt()
{
	return m_lookAt;
}

void Light::setLookAt(float x, float y, float z)
{
	m_lookAt = DirectX::SimpleMath::Vector4(x, y, z, 1.0f);
}

DirectX::SimpleMath::Matrix Light::getProjection()
{
	return m_lightProjection;
}

DirectX::SimpleMath::Matrix Light::getView()
{
	return m_lightView;
}

void Light::setProjection(DirectX::SimpleMath::Matrix projectionMatrix)
{
	m_lightProjection = projectionMatrix;
}

void Light::setView(DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Vector4 lookat)
{	
	m_lightView = DirectX::SimpleMath::Matrix::CreateLookAt(position, DirectX::SimpleMath::Vector3(lookat.x, lookat.y, lookat.z), DirectX::SimpleMath::Vector3::UnitY);
}

void Light::setView(DirectX::SimpleMath::Matrix viewMatrix)
{
	m_lightView = viewMatrix;
}
