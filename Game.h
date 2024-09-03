//
// Game.h
//
#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Shader.h"
#include "ShaderShadowMap.h"
#include "ShaderNormalMap.h"
#include "ShaderParticles.h"
#include "ShaderIce.h"
#include "modelclass.h"
#include "Light.h"
#include "Input.h"
#include "Camera.h"
#include "ParticleSystemClass.h"
#include "RenderTexture.h"
#include "SkyboxEffect.h"
#include "ShaderFire.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);
#ifdef DXTK_AUDIO
    void NewAudioDevice();
#endif

    // Properties
    void GetDefaultSize( int& width, int& height ) const;
	
private:

	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

    void Update(DX::StepTimer const& timer);
    void Render();
    void RenderShadowMap();
    void RenderMiniMap(DirectX::SimpleMath::Vector4 pLightPosition[], DirectX::SimpleMath::Vector4 pLightColor[]);
    void Clear();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();


    // Device resources.
    std::unique_ptr<DX::DeviceResources>                                    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                                                           m_timer;

	//input manager. 
	Input									                                m_input;
	InputCommands							                                m_gameInputCommands;

    // DirectXTK objects.
    std::unique_ptr<DirectX::CommonStates>                                  m_states;
    std::unique_ptr<DirectX::BasicEffect>                                   m_batchEffect;	
    std::unique_ptr<DirectX::EffectFactory>                                 m_fxFactory;
    std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
    std::unique_ptr<DirectX::SpriteFont>                                    m_font;

	// Scene Objects
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_batchInputLayout;

	//Lights
	Light																	m_Light;
    Light*                                                                  m_pLights;
    int                                                                     m_numPLights;

	//Cameras
	Camera																	m_Camera01;
	Camera																	m_CameraMiniMap;
    float                                                                   m_CameraMiniMapHeight;

    //Skybox
    std::unique_ptr<DirectX::GeometricPrimitive>                            m_sky;
    std::unique_ptr<SkyboxEffect>                                           m_effect;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_skyInputLayout;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_cubemap;

    //Shadow mapping
    Microsoft::WRL::ComPtr <ID3D11RasterizerState>                          m_shadowRenderState;
    DirectX::SimpleMath::Matrix                                             m_LightProjection;
    DirectX::SimpleMath::Matrix                                             m_LightView;
    DirectX::SimpleMath::Vector3                                            m_LightForward;
    DirectX::SimpleMath::Vector3                                            m_LightOrientation;
    DirectX::SimpleMath::Vector3                                            m_LightRight;
    DirectX::SimpleMath::Vector3                                            m_LightLookAt;
    DirectX::SimpleMath::Vector3                                            m_LightCenter;
    D3D11_VIEWPORT                                                          m_shadowViewport;
    Microsoft::WRL::ComPtr <ID3D11Texture2D>                                m_shadowMap;
    Microsoft::WRL::ComPtr <ID3D11DepthStencilView>                         m_shadowDepthView;
    Microsoft::WRL::ComPtr <ID3D11ShaderResourceView>                       m_shadowResourceView;
    ShaderShadowMap                                                         m_BasicShaderPairShadowMap;
    int                                                                     m_shadowMapHeight;
    int                                                                     m_shadowMapWidth;
    ModelClass                                                              m_GroundBoxSM;
    ModelClass                                                              m_Mountain1SM;
    ModelClass                                                              m_Mountain2SM;
    ModelClass                                                              m_Glacier1SM;
    ModelClass                                                              m_Glacier2SM;
    ModelClass                                                              m_Deadwood1SM;
    ModelClass                                                              m_Deadwood2SM;
    ModelClass                                                              m_Deadwood3SM;
    ModelClass                                                              m_Deadwood4SM;
    ModelClass                                                              m_Deadwood5SM;
    ModelClass                                                              m_Deadwood6SM;
    ModelClass                                                              m_Deadwood7SM;
    ModelClass                                                              m_Deadwood8SM;
    ModelClass                                                              m_Deadwood9SM;
    ModelClass                                                              m_CampIglooSM;
    ModelClass                                                              m_CampCrowSM;
    ModelClass                                                              m_CampDeadwoodSM;
    ModelClass                                                              m_CampTreeStonesSM;
    ModelClass                                                              m_CampStonesSM;
    ModelClass                                                              m_CampSnowSM;
    ModelClass                                                              m_CampIceBorderSM;
    ModelClass                                                              m_CampIceSM;
    ModelClass                                                              m_CampEstusSM;
    ModelClass                                                              m_BfStonesSM;
    ModelClass                                                              m_BfAshSM;
    ModelClass                                                              m_BfBonesSM;
    ModelClass                                                              m_BfSkullsSM;
    ModelClass                                                              m_BfBladeSM;
    ModelClass                                                              m_BfHiltSM;    

    //Particle system
    ShaderParticles*                                                        m_ParticleShader;
    ParticleSystemClass*                                                    m_ParticleSystem;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texStar;    

	//textures 
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture1;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texSnowMountain;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texSnowMountainNormalMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texSnow;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texSnowNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texGlacier;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texGlacierNormalMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texDeadwood;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texDeadwoodNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texIgloo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texCrow;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texCrowNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texTerrain;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texCampStones;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texCampTreeStones;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texCampTreeStonesNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texIce;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texIceNormal;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texBfStones;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texBfStonesNormal;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texBfAsh;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texBfAshNormal;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texBfSkulls;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texBfSkullsNormal;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texBfBones;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texBfBlade;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texBfHilt;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texFoliageDeadBush;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texFoliageFern;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texFoliageGrass;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texEF;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texEFNormal;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texFire;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texFireNoise;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texFireAlpha;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texFog;


	//Shaders
	ShaderNormalMap															m_BasicShaderPair;
	ShaderNormalMap															m_BasicShaderPairTiling;
    ShaderNormalMap															m_BasicShaderPairNoSpec;
    Shader																	m_BasicShaderPairTilingNoNormalMap;
	Shader																	m_BasicShaderPairNoSpecNoNormalMap;
	Shader																	m_BasicShaderPairNoNormalMap;
	ShaderIce																m_BasicShaderPairIce;
    ShaderFire                                                              m_BasicShaderPairFire;
	
    //Models    
    ModelClass                                                              m_Fire;
    ModelClass                                                              m_GroundBox;
    ModelClass                                                              m_GroundBoxNM;
    ModelClass                                                              m_Mountain1;
    ModelClass                                                              m_Mountain1NM;
    ModelClass                                                              m_Mountain2;
    ModelClass                                                              m_Mountain2NM;
    ModelClass                                                              m_Glacier1;
    ModelClass                                                              m_Glacier1NM;
    ModelClass                                                              m_Glacier2;
    ModelClass                                                              m_Glacier2NM;
    ModelClass                                                              m_Deadwood1;
    ModelClass                                                              m_Deadwood1NM;
    ModelClass                                                              m_Deadwood2;
    ModelClass                                                              m_Deadwood2NM;
    ModelClass                                                              m_Deadwood3;
    ModelClass                                                              m_Deadwood3NM;
    ModelClass                                                              m_Deadwood4;
    ModelClass                                                              m_Deadwood4NM;
    ModelClass                                                              m_Deadwood5;
    ModelClass                                                              m_Deadwood5NM;
    ModelClass                                                              m_Deadwood6;
    ModelClass                                                              m_Deadwood6NM;
    ModelClass                                                              m_Deadwood7;
    ModelClass                                                              m_Deadwood7NM;
    ModelClass                                                              m_Deadwood8;
    ModelClass                                                              m_Deadwood8NM;
    ModelClass                                                              m_Deadwood9;
    ModelClass                                                              m_Deadwood9NM;
    ModelClass                                                              m_CampIgloo;
    ModelClass                                                              m_CampCrow;
    ModelClass                                                              m_CampCrowNM;
    ModelClass                                                              m_CampDeadwood;
    ModelClass                                                              m_CampDeadwoodNM;
    ModelClass                                                              m_CampTreeStones;
    ModelClass                                                              m_CampTreeStonesNM;
    ModelClass                                                              m_CampStones;
    ModelClass                                                              m_CampSnow;
    ModelClass                                                              m_CampIceBorder;
    ModelClass                                                              m_CampIceBorderNM;
    ModelClass                                                              m_CampIce;
    ModelClass                                                              m_CampIceNM;
    ModelClass                                                              m_CampEstus;
    ModelClass                                                              m_CampEstusNM;
    ModelClass                                                              m_BfStones;
    ModelClass                                                              m_BfStonesNM;
    ModelClass                                                              m_BfAsh;
    ModelClass                                                              m_BfAshNM;
    ModelClass                                                              m_BfBones;
    ModelClass                                                              m_BfSkulls;
    ModelClass                                                              m_BfSkullsNM;
    ModelClass                                                              m_BfBlade;
    ModelClass                                                              m_BfHilt;
    ModelClass                                                              m_FoliageDeadBush1;
    ModelClass                                                              m_FoliageDeadBush2;
    ModelClass                                                              m_FoliageDeadBush3;
    ModelClass                                                              m_FoliageFern;
    ModelClass                                                              m_FoliageGrass1;
    ModelClass                                                              m_FoliageGrass2;
    ModelClass                                                              m_FoliageGrass3;
    ModelClass                                                              m_FoliageGrass4;
    ModelClass                                                              m_FoliageGrass5;
    ModelClass                                                              m_FoliageGrass6;
    ModelClass                                                              m_FoliageGrass7;
    ModelClass                                                              m_FoliageGrass8;
    ModelClass                                                              m_FoliageGrass9;

	//RenderTextures
	RenderTexture*															m_MiniMapTexture;
	RECT																	m_fullscreenRect;
	RECT																	m_CameraViewRect;    
	RECT																	m_shadowMapRect;    

    //Billboard
    float                                                                   billboardAngle;
    float                                                                   billboardRotation;
    float                                                                   firePosX;
    float                                                                   firePosY;
    float                                                                   firePosZ;

    //Audio    
#ifdef DXTK_AUDIO
    std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
    //std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
    std::unique_ptr<DirectX::SoundEffect>                                   m_soundEffect;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
    //std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;
    //uint32_t                                                                m_audioEvent;
    float                                                                   m_audioTimerAcc;
    bool                                                                    m_retryDefault;
#endif

    DirectX::SimpleMath::Matrix                                             m_world;
    DirectX::SimpleMath::Matrix                                             m_view;
    DirectX::SimpleMath::Matrix                                             m_map_view;
    DirectX::SimpleMath::Matrix                                             m_projection;
};