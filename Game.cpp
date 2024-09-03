//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include <fstream>

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;


Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
    m_ParticleSystem = 0;
    m_ParticleShader = 0;
    m_CameraMiniMapHeight = 7.0f;
    firePosX = -42.32f;
    firePosY = 2.0f;
    firePosZ = -18.12f;
}

Game::~Game()
{
#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	m_input.Initialise(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	m_fullscreenRect.left = 0;
	m_fullscreenRect.top = 0;
	m_fullscreenRect.right = 1600;
	m_fullscreenRect.bottom = 1200;

	m_CameraViewRect.left = 1620;
	m_CameraViewRect.top = 0;
	m_CameraViewRect.right = 1920;
	m_CameraViewRect.bottom = 240;

    m_shadowMapRect.left = 1420;
    m_shadowMapRect.top = 580;
    m_shadowMapRect.right = 1920;
    m_shadowMapRect.bottom = 1080;

	//set up directional light
	m_Light.setAmbientColour(0.025f, 0.025f, 0.05f, 1.0f);
	m_Light.setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light.setSpecularColour(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light.setSpecularPower(100.0f);    
    m_Light.setPosition(140, 135, -15);
    
    //set up light view and projection for shadow mapping
    m_LightForward = { -0.85, -0.55, 0 };
    m_LightOrientation = { -35, 2700, 0 };
    m_LightForward.Cross(DirectX::SimpleMath::Vector3::UnitY, m_LightRight);
    m_LightLookAt = m_Light.getPosition() + m_LightForward;
    m_Light.setView(DirectX::SimpleMath::Matrix::CreateLookAt(m_Light.getPosition(), m_LightLookAt, DirectX::SimpleMath::Vector3::UnitY));
    m_LightView = m_Light.getView();
    m_LightCenter = DirectX::SimpleMath::Vector3::Transform(m_LightLookAt, m_LightView);
    // Ortho frustum in light space encloses scene
    float l = m_LightCenter.x - 500;
    float b = m_LightCenter.y - 500;
    float n = m_LightCenter.z - 250;
    float r = m_LightCenter.x + 500;
    float t = m_LightCenter.y + 500;
    float f = m_LightCenter.z + 1000;
    m_Light.setProjection(DirectX::SimpleMath::Matrix::CreateOrthographicOffCenter(l, r, b, t, n, f));
    m_LightProjection = m_Light.getProjection();
    
    //set up point light
    m_numPLights = 1;
    m_pLights = new Light[m_numPLights];    
    //m_pLights[0].setDiffuseColour(0.51, 0.5, 0.5, 1.0);  
    m_pLights[0].setDiffuseColour(0.30, 0.20, 0.0, 1.0);
    m_pLights[0].setPosition(firePosX, 2.05f, firePosZ);

	//set up main camera
	m_Camera01.setPosition(Vector3(-45.12, 2.13, -18.05));
	m_Camera01.setRotation(Vector3(18.73, 2.33, 0));

	//set up mini map camera
	m_CameraMiniMap.setPosition(Vector3(-45.12, m_CameraMiniMapHeight, -18.05));
	m_CameraMiniMap.setRotation(Vector3(-90.0f, 0.0f, 0.0f));    

#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    //m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    //m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"firelinkshrine.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    //m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    //m_effect2->Play();
#endif
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	//take in input
	m_input.Update();								//update the hardware
	m_gameInputCommands = m_input.getGameInput();	//retrieve the input for our game
	
	//Update all game objects
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });    
	
    //Render all game content. 
    Render();

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

	
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    /* Frame time */
    float delta = float(timer.GetElapsedSeconds());
    float sprintModifier;
    
    /* Input */
    // If player holds shift, set speed modifier for camera movement
    m_gameInputCommands.sprint
        ? sprintModifier = 15.0f
        : sprintModifier = 1.0f;    

    // Mouse-based camera control
    Vector3 rotation = m_Camera01.getRotation();
    rotation.x -= m_Camera01.getRotationSpeed() * m_gameInputCommands.mouseDelta.y * delta;
    rotation.y += m_Camera01.getRotationSpeed() * m_gameInputCommands.mouseDelta.x * delta;
    m_Camera01.setRotation(rotation);
    
    if (m_gameInputCommands.left)
    {
        Vector3 position = m_Camera01.getPosition();
        position -= (m_Camera01.getRight() * m_Camera01.getMoveSpeed() * delta * sprintModifier); //add the right vector
        m_Camera01.setPosition(position);
    }
    if (m_gameInputCommands.right)
    {
        Vector3 position = m_Camera01.getPosition();
        position += (m_Camera01.getRight() * m_Camera01.getMoveSpeed() * delta * sprintModifier); //add the right vector
        m_Camera01.setPosition(position);
    }
    if (m_gameInputCommands.forward)
    {
        Vector3 position = m_Camera01.getPosition();
        position += (m_Camera01.getForward() * m_Camera01.getMoveSpeed() * delta * sprintModifier); //add the forward vector
        m_Camera01.setPosition(position);
    }
    if (m_gameInputCommands.back)
    {
        Vector3 position = m_Camera01.getPosition();
        position -= (m_Camera01.getForward() * m_Camera01.getMoveSpeed() * delta * sprintModifier); //add the forward vector
        m_Camera01.setPosition(position);
    }
    if (m_gameInputCommands.rotLeft)
    {
        Vector3 rotation = m_Camera01.getRotation();
        rotation.y -= m_Camera01.getRotationSpeed() * delta;
        m_Camera01.setRotation(rotation);
    }
    if (m_gameInputCommands.rotRight)
    {
        Vector3 rotation = m_Camera01.getRotation();
        rotation.y += m_Camera01.getRotationSpeed() * delta;
        m_Camera01.setRotation(rotation);
    }
    if (m_gameInputCommands.rotUp)
    {
        Vector3 rotation = m_Camera01.getRotation();
        rotation.x += m_Camera01.getRotationSpeed() * delta;
        m_Camera01.setRotation(rotation);
    }
    if (m_gameInputCommands.rotDown)
    {
        Vector3 rotation = m_Camera01.getRotation();
        rotation.x -= m_Camera01.getRotationSpeed() * delta;
        m_Camera01.setRotation(rotation);
    }

    m_ParticleSystem->Frame(delta, context);

	m_Camera01.Update();	

    //Follow main camera on mini map
    DirectX::SimpleMath::Vector3 camera01CurrentPosition = m_Camera01.getPosition();
    m_CameraMiniMap.setPosition(Vector3(camera01CurrentPosition.x, m_CameraMiniMapHeight, camera01CurrentPosition.z));
    // Rotate mini map based on forward direction of main camera
    DirectX::SimpleMath::Vector3 cameraMiniMapCurrentRotation = m_CameraMiniMap.getRotation();
    DirectX::SimpleMath::Vector3 camera01CurrentRotation = m_Camera01.getRotation();
    m_CameraMiniMap.setRotation(Vector3(cameraMiniMapCurrentRotation.x, camera01CurrentRotation.y, cameraMiniMapCurrentRotation.z));

    m_CameraMiniMap.Update();

	m_view = m_Camera01.getCameraMatrix();
	m_map_view = m_CameraMiniMap.getCameraMatrix();
	m_world = Matrix::Identity;
    
    /*AllocConsole();
    freopen("CONOUT$", "w", stdout);
    std::cout << "cam pos(" << m_Camera01.getPosition().x << "," << m_Camera01.getPosition().y << "," << m_Camera01.getPosition().z << ")" <<  "\n";
    std::cout << "cam fwd(" << m_Camera01.getForward().x << "," << m_Camera01.getForward().y << "," << m_Camera01.getForward().z << ")" << "\n";
    std::cout << "cam rot(" << m_Camera01.getRotation().x << "," << m_Camera01.getRotation().y << "," << m_Camera01.getRotation().z << ")" << "\n";
    std::cout << delta << "\n";*/

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

           // m_waveBank->Play(m_audioEvent++);

           //if (m_audioEvent >= 11)
           //     m_audioEvent = 0;
        }
    }
#endif

  
	if (m_input.Quit())
	{
		ExitGame();
	}
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{	
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }    

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();
        
    // Draw Text to the screen
    //m_sprites->Begin();
	//m_font->DrawString(m_sprites.get(), L"", XMFLOAT2(10, 10), Colors::Yellow);
    //m_sprites->End();

    /* Point light */
    DirectX::SimpleMath::Vector4 pLightPosition[1], pLightColor[1];
    int i;
    // Get the light properties.
    for (i = 0; i < m_numPLights; i++)
    {
        // Create the diffuse color array from the light colors.
        pLightColor[i] = m_pLights[i].getDiffuseColour();

        // Create the light position array from the light positions.
        pLightPosition[i] = DirectX::SimpleMath::Vector4(m_pLights[i].getPosition().x, m_pLights[i].getPosition().y, m_pLights[i].getPosition().z, 1.0f);
    }

    /* Sky Box */
    m_effect->SetView(m_view);
    m_sky->Draw(m_effect.get(), m_skyInputLayout.Get());

	//Set Rendering states. 
	//context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	//context->OMSetBlendState(m_states->AlphaBlend(), nullptr, 0xFFFFFFFF);
	context->OMSetBlendState(m_states->NonPremultiplied(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullClockwise());    
    //context->RSSetState(m_states->Wireframe());

	//create our render to textures	
      
    RenderShadowMap();
    RenderMiniMap(pLightPosition, pLightColor);

	/////////////////////////////////////////////////////////////draw our scene normally. 
	
    m_world = SimpleMath::Matrix::Identity;
	
    /* Ground */
    m_BasicShaderPairTiling.EnableShader(context);
    m_BasicShaderPairTiling.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texSnow.Get(), m_texSnowNormal.Get(), m_shadowResourceView.Get());
    m_GroundBoxNM.Render(context);

    // Turn shaders with no specular highlight on
    m_BasicShaderPairNoSpec.EnableShader(context);
   
    /* Mountains */
	// mountain1
	m_BasicShaderPairNoSpec.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texSnowMountain.Get(), m_texSnowMountainNormalMap.Get(), m_shadowResourceView.Get());
	m_Mountain1NM.Render(context);    
    // mountain2
    m_Mountain2NM.Render(context);
    // glacier1
    m_BasicShaderPairNoSpec.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texGlacier.Get(), m_texGlacierNormalMap.Get(), m_shadowResourceView.Get());
    m_Glacier1NM.Render(context);
    // glacier2
    m_Glacier2NM.Render(context);

    /* Deadwoods */
    // deadwood1
    m_BasicShaderPairNoSpec.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texDeadwood.Get(), m_texDeadwoodNormal.Get(), m_shadowResourceView.Get());
    m_Deadwood1NM.Render(context);
    // deadwood2
    m_Deadwood2NM.Render(context);
    // deadwood3
    m_Deadwood3NM.Render(context);
    // deadwood4
    m_Deadwood4NM.Render(context);
    // deadwood5
    m_Deadwood5NM.Render(context);
    // deadwood6
    m_Deadwood6NM.Render(context);
    // deadwood7
    m_Deadwood7NM.Render(context);
    // deadwood8
    m_Deadwood8NM.Render(context);
    // deadwood9
    m_Deadwood9NM.Render(context);

    //* Camp */
    // igloo
    m_BasicShaderPairNoSpecNoNormalMap.EnableShader(context);
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texIgloo.Get(), m_shadowResourceView.Get());
    m_CampIgloo.Render(context);
    // crow
    m_BasicShaderPairNoSpec.EnableShader(context);
    m_BasicShaderPairNoSpec.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texCrow.Get(), m_texCrowNormal.Get(), m_shadowResourceView.Get());
    m_CampCrowNM.Render(context);
    // dirty snow patch
    m_BasicShaderPairNoSpecNoNormalMap.EnableShader(context);
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texTerrain.Get(), m_shadowResourceView.Get());
    m_CampSnow.Render(context);
    // stone
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texCampStones.Get(), m_shadowResourceView.Get());
    m_CampStones.Render(context);
    // tree stones
    m_BasicShaderPairNoSpec.EnableShader(context);
    m_BasicShaderPairNoSpec.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texCampTreeStones.Get(), m_texCampTreeStonesNormal.Get(), m_shadowResourceView.Get());
    m_CampTreeStonesNM.Render(context);
    // tree
    m_BasicShaderPairNoSpec.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texDeadwood.Get(), m_texDeadwoodNormal.Get(), m_shadowResourceView.Get());
    m_CampDeadwoodNM.Render(context);
    // ice border
    m_BasicShaderPair.EnableShader(context);
    m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texSnow.Get(), m_texSnowNormal.Get(), m_shadowResourceView.Get());
    m_CampIceBorderNM.Render(context);    
    // ice
    m_BasicShaderPairIce.EnableShader(context);
    m_BasicShaderPairIce.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texIce.Get(), m_texIceNormal.Get(), m_shadowResourceView.Get(), m_texFog.Get());    
    m_CampIceNM.Render(context);
    // estus flask
    m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texEF.Get(), m_texEFNormal.Get(), m_shadowResourceView.Get());
    m_CampEstusNM.Render(context);    

    /* Bonfire */
    // stones
    m_BasicShaderPairNoSpec.EnableShader(context);
    m_BasicShaderPairNoSpec.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texBfStones.Get(), m_texBfStonesNormal.Get(), m_shadowResourceView.Get());
    m_BfStonesNM.Render(context);
    // ash
    m_BasicShaderPairNoSpec.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texBfAsh.Get(), m_texBfAshNormal.Get(), m_shadowResourceView.Get());
    m_BfAshNM.Render(context);
    // skulls
    m_BasicShaderPair.EnableShader(context);
    m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texBfSkulls.Get(), m_texBfSkullsNormal.Get(), m_shadowResourceView.Get());
    m_BfSkullsNM.Render(context);
    // bones
    m_BasicShaderPairNoNormalMap.EnableShader(context);
    m_BasicShaderPairNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texBfBones.Get(), m_shadowResourceView.Get());
    m_BfBones.Render(context);
    // blade
    m_BasicShaderPairNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texBfBlade.Get(), m_shadowResourceView.Get());
    m_BfBlade.Render(context);
    // hilt
    m_BasicShaderPairNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texBfHilt.Get(), m_shadowResourceView.Get());
    m_BfHilt.Render(context);
    
    /*Foliage*/
    // deadbush1
    m_BasicShaderPairNoSpecNoNormalMap.EnableShader(context);
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageDeadBush.Get(), m_shadowResourceView.Get());
    m_FoliageDeadBush1.Render(context);
    // deadbush2
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageDeadBush.Get(), m_shadowResourceView.Get());
    m_FoliageDeadBush2.Render(context);
    // deadbush3
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageDeadBush.Get(), m_shadowResourceView.Get());
    m_FoliageDeadBush3.Render(context);
    // fern
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageFern.Get(), m_shadowResourceView.Get());
    m_FoliageFern.Render(context);
    // grass1
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass1.Render(context);
    // grass2
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass2.Render(context);
    // grass3
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass3.Render(context);
    // grass4
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass4.Render(context);
    // grass5
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass5.Render(context);
    // grass6
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass6.Render(context);
    // grass7
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass7.Render(context);
    // grass8
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass8.Render(context);
    // grass9
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass9.Render(context);

    /* Particle System */
    //context->OMSetBlendState(m_states->Additive(), nullptr, 0xFFFFFFFF);
    //// Put the particle system vertex and index buffers on the graphics pipeline to prepare them for drawing.
    //m_ParticleSystem->Render(context);
    //// Render the model using the texture shader.
    //m_ParticleShader->Render(context, m_ParticleSystem->GetIndexCount(), &m_world, &m_view, &m_projection, m_texStar.Get());

    /* Fire */
    DirectX::SimpleMath::Vector3 scrollSpeeds, scales;
    DirectX::SimpleMath::Vector2 distortion1, distortion2, distortion3;
    float distortionScale, distortionBias;
    float time = float(m_timer.GetTotalSeconds());
    // Set the three scrolling speeds for the three different noise textures.
    scrollSpeeds = DirectX::SimpleMath::Vector3(1.3f, 2.1f, 2.3f);

    // Set the three scales which will be used to create the three different noise octave textures.
    scales = DirectX::SimpleMath::Vector3(1.0f, 2.0f, 3.0f);

    // Set the three different x and y distortion factors for the three different noise textures.
    distortion1 = DirectX::SimpleMath::Vector2(0.1f, 0.2f);
    distortion2 = DirectX::SimpleMath::Vector2(0.1f, 0.3f);
    distortion3 = DirectX::SimpleMath::Vector2(0.1f, 0.1f);
 
    // The scale and bias of the texture coordinate sampling perturbation.
    distortionScale = 0.8f;
    distortionBias = 0.5f;

    context->OMSetBlendState(m_states->Additive(), nullptr, 0xFFFFFFFF);
    SimpleMath::Matrix newPosition1 = SimpleMath::Matrix::CreateTranslation(firePosX, firePosY, firePosZ);
    SimpleMath::Matrix newScaling = SimpleMath::Matrix::CreateScale(0.15);
    // Calculate the rotation that needs to be applied to the fire model to face the current camera position using the arc tangent function
    billboardAngle = atan2(firePosX - m_Camera01.getPosition().x, firePosZ - m_Camera01.getPosition().z) * (180.0 / DirectX::XM_PI);
    // Convert rotation into radians.
    billboardRotation = billboardAngle * 0.0174532925f;
    SimpleMath::Matrix newRotation = SimpleMath::Matrix::CreateRotationY(billboardRotation);

    m_world = m_world * newScaling * newRotation * newPosition1;
    
    m_BasicShaderPairFire.EnableShader(context);
    m_BasicShaderPairFire.SetShaderParameters(context, &m_world, &m_view, &m_projection,
           m_texFire.Get(), m_texFireNoise.Get(), m_texFireAlpha.Get(), time, scrollSpeeds,
           scales, distortion1, distortion2, distortion3, distortionScale, distortionBias);
    m_Fire.Render(context);

    m_world = SimpleMath::Matrix::Identity;

	///////////////////////////////////////draw our sprite with the render texture displayed on it. 

    m_sprites->Begin();
    //m_sprites->Draw(m_shadowResourceView.Get(), m_shadowMapRect);
    m_sprites->Draw(m_MiniMapTexture->getShaderResourceView(), m_CameraViewRect);
    m_sprites->End();

    // Show the new frame.
    m_deviceResources->Present();
}

void Game::RenderShadowMap()
{    
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTargetView = m_deviceResources->GetRenderTargetView();
    auto depthTargetView = m_deviceResources->GetDepthStencilView();
    auto viewport = m_deviceResources->GetScreenViewport();
    
    // Only bind the ID3D11DepthStencilView for output.    
    context->OMSetRenderTargets(0, nullptr, m_shadowDepthView.Get());
    
    // Clear the render to texture.
    context->ClearDepthStencilView(m_shadowDepthView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Turn on front-face culling
    context->RSSetState(m_shadowRenderState.Get());

    // Set rendering viewport.
    context->RSSetViewports(1, &m_shadowViewport);

    /* Render all the objects in the scene that can cast shadows onto themselves or onto other objects */

    m_world = SimpleMath::Matrix::Identity;

    m_BasicShaderPairShadowMap.EnableShader(context);
    m_BasicShaderPairShadowMap.SetShaderParameters(context, &m_world, &m_LightView, &m_LightProjection);

    /* Ground */
    //m_GroundBoxSM.Render(context);

    /* Mountains */
    // mountain1
    m_Mountain1SM.Render(context);
    // mountain2
    m_Mountain2SM.Render(context);
    // glacier1
    m_Glacier1SM.Render(context);
    // glacier2
    m_Glacier2SM.Render(context);

    /* Deadwoods */
    // deadwood1
    m_Deadwood1SM.Render(context);
    // deadwood2
    m_Deadwood2SM.Render(context);
    // deadwood3
    m_Deadwood3SM.Render(context);
    // deadwood4
    m_Deadwood4SM.Render(context);
    // deadwood5
    m_Deadwood5SM.Render(context);
    // deadwood6
    m_Deadwood6SM.Render(context);
    // deadwood7
    m_Deadwood7SM.Render(context);
    // deadwood8
    m_Deadwood8SM.Render(context);
    // deadwood9
    m_Deadwood9SM.Render(context);

    //* Camp */
    // igloo
    m_CampIglooSM.Render(context);
    // crow
    m_CampCrowSM.Render(context);
    // dirty snow patch
    m_CampSnowSM.Render(context);
    // stone
    m_CampStonesSM.Render(context);
    // tree stones
    m_CampTreeStonesSM.Render(context);
    // tree
    m_CampDeadwoodSM.Render(context);
    // estus flask
    m_CampEstusSM.Render(context);

    /* Bonfire */
    // stones
    m_BfStonesSM.Render(context);
    // ash
    m_BfAshSM.Render(context);
    // skulls
    m_BfSkullsSM.Render(context);
    // bones
    m_BfBonesSM.Render(context);
    // blade
    m_BfBladeSM.Render(context);
    // hilt
    m_BfHiltSM.Render(context);
    
    // Reset the render target back to the original back buffer and not the render to texture anymore	
    context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);

    // Turn on back-face culling
    context->RSSetState(m_states->CullClockwise());

    // Reset viewport
    context->RSSetViewports(1, &viewport);
}

void Game::RenderMiniMap(DirectX::SimpleMath::Vector4 pLightPosition[], DirectX::SimpleMath::Vector4 pLightColor[])
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();
    auto viewport = m_deviceResources->GetScreenViewport();

    // Set the render target to be the render to texture.
	m_MiniMapTexture->setRenderTarget(context);
	
    // Clear the render to texture.
    m_MiniMapTexture->clearRenderTarget(context, 0.0f, 0.0f, 1.0f, 1.0f);

    /* Render the whole scene, minus sky box, without normal mapping*/
    
    m_world = SimpleMath::Matrix::Identity;

    /* Ground */
    m_BasicShaderPairTilingNoNormalMap.EnableShader(context);
    m_BasicShaderPairTilingNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texSnow.Get(), m_shadowResourceView.Get());
    m_GroundBox.Render(context);

    // Turn shaders with no specular highlight on
    m_BasicShaderPairNoSpecNoNormalMap.EnableShader(context);

    /* Mountains */
    // mountain1
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texSnowMountain.Get(), m_shadowResourceView.Get());
    m_Mountain1.Render(context);
    // mountain2
    m_Mountain2.Render(context);
    // glacier1
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texGlacier.Get(), m_shadowResourceView.Get());
    m_Glacier1.Render(context);
    // glacier2
    m_Glacier2.Render(context);

    /* Deadwoods */
    // deadwood1
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texDeadwood.Get(), m_shadowResourceView.Get());
    m_Deadwood1.Render(context);
    // deadwood2
    m_Deadwood2.Render(context);
    // deadwood3
    m_Deadwood3.Render(context);
    // deadwood4
    m_Deadwood4.Render(context);
    // deadwood5
    m_Deadwood5.Render(context);
    // deadwood6
    m_Deadwood6.Render(context);
    // deadwood7
    m_Deadwood7.Render(context);
    // deadwood8
    m_Deadwood8.Render(context);
    // deadwood9
    m_Deadwood9.Render(context);

    //* Camp */
    // igloo
    m_BasicShaderPairNoSpecNoNormalMap.EnableShader(context);
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texIgloo.Get(), m_shadowResourceView.Get());
    m_CampIgloo.Render(context);
    // crow
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texCrow.Get(), m_shadowResourceView.Get());
    m_CampCrow.Render(context);
    // dirty snow patch
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texTerrain.Get(), m_shadowResourceView.Get());
    m_CampSnow.Render(context);
    // stone
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texCampStones.Get(), m_shadowResourceView.Get());
    m_CampStones.Render(context);
    // tree stones
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texCampTreeStones.Get(), m_shadowResourceView.Get());
    m_CampTreeStones.Render(context);
    // tree
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texDeadwood.Get(), m_shadowResourceView.Get());
    m_CampDeadwood.Render(context);    
    // ice
    m_BasicShaderPairNoNormalMap.EnableShader(context);
    m_BasicShaderPairNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texIce.Get(), m_shadowResourceView.Get());
    m_CampIce.Render(context);
    // estus flask
    m_BasicShaderPairNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texEF.Get(), m_shadowResourceView.Get());
    m_CampEstus.Render(context);    

    /* Bonfire */
    // stones
    m_BasicShaderPairNoSpecNoNormalMap.EnableShader(context);
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texBfStones.Get(), m_shadowResourceView.Get());
    m_BfStones.Render(context);
    // ash
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texBfAsh.Get(), m_shadowResourceView.Get());
    m_BfAsh.Render(context);
    // skulls
    m_BasicShaderPairNoNormalMap.EnableShader(context);
    m_BasicShaderPairNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texBfSkulls.Get(), m_shadowResourceView.Get());
    m_BfSkulls.Render(context);
    // bones    
    m_BasicShaderPairNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texBfBones.Get(), m_shadowResourceView.Get());
    m_BfBones.Render(context);
    // blade
    m_BasicShaderPairNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texBfBlade.Get(), m_shadowResourceView.Get());
    m_BfBlade.Render(context);
    // hilt
    m_BasicShaderPairNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texBfHilt.Get(), m_shadowResourceView.Get());
    m_BfHilt.Render(context);

    /*Foliage*/
    // deadbush1
    m_BasicShaderPairNoSpecNoNormalMap.EnableShader(context);
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageDeadBush.Get(), m_shadowResourceView.Get());
    m_FoliageDeadBush1.Render(context);
    // deadbush2
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageDeadBush.Get(), m_shadowResourceView.Get());
    m_FoliageDeadBush2.Render(context);
    // deadbush3
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageDeadBush.Get(), m_shadowResourceView.Get());
    m_FoliageDeadBush3.Render(context);
    // fern
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageFern.Get(), m_shadowResourceView.Get());
    m_FoliageFern.Render(context);
    // grass1
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass1.Render(context);
    // grass2
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass2.Render(context);
    // grass3
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass3.Render(context);
    // grass4
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass4.Render(context);
    // grass5
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass5.Render(context);
    // grass6
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass6.Render(context);
    // grass7
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass7.Render(context);
    // grass8
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass8.Render(context);
    // grass9
    m_BasicShaderPairNoSpecNoNormalMap.SetShaderParameters(context, &m_world, &m_map_view, &m_projection, &m_Light, pLightPosition, pLightColor, &m_Camera01, m_texFoliageGrass.Get(), m_shadowResourceView.Get());
    m_FoliageGrass9.Render(context);

	// Reset the render target back to the original back buffer and not the render to texture anymore.	
	context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);

    // Reset viewport
    context->RSSetViewports(1, &viewport);
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();
    
    context->ClearRenderTargetView(renderTarget, Colors::Transparent);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    // Set the viewport for shadow rendering
    ZeroMemory(&m_shadowViewport, sizeof(D3D11_VIEWPORT));
    m_shadowViewport.Height = 2048.0;
    m_shadowViewport.Width = 2048.0;
    m_shadowViewport.MinDepth = 0.f;
    m_shadowViewport.MaxDepth = 1.f;

    m_deviceResources->PIXEndEvent();
}

#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();
#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    width = 1920;
    height = 1080;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);
    m_fxFactory = std::make_unique<EffectFactory>(device);
    m_sprites = std::make_unique<SpriteBatch>(context);
    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");
	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

    /* Fire */
    m_Fire.InitializeBox(device, 1.0f, 1.0f, 0.0f);

    /* Sky box*/
    m_sky = GeometricPrimitive::CreateGeoSphere(context, 2.f, 3, false /*invert for being inside the shape*/);
    m_effect = std::make_unique<SkyboxEffect>(device);
    m_sky->CreateInputLayout(m_effect.get(), m_skyInputLayout.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"skybox.dds", nullptr, m_cubemap.ReleaseAndGetAddressOf());
    m_effect->SetTexture(m_cubemap.Get());

    /* Ground */
    m_GroundBoxNM.normalMapping = true;
    m_GroundBoxNM.InitializeModel(device, "ground_box.obj");    

    /* Surrounding mountains */        
    m_Mountain1NM.normalMapping = true;
    m_Mountain1NM.InitializeModel(device, "mountain1.obj");
    m_Mountain2NM.normalMapping = true;
    m_Mountain2NM.InitializeModel(device, "mountain2.obj");
    m_Glacier1NM.normalMapping = true;
    m_Glacier1NM.InitializeModel(device, "glacier1.obj");
    m_Glacier2NM.normalMapping = true;
    m_Glacier2NM.InitializeModel(device, "glacier2.obj");

    /* Deadwoods */
    m_Deadwood1NM.normalMapping = true;
    m_Deadwood1NM.InitializeModel(device, "deadwood1.obj");
    m_Deadwood2NM.normalMapping = true;
    m_Deadwood2NM.InitializeModel(device, "deadwood2.obj");
    m_Deadwood3NM.normalMapping = true;
    m_Deadwood3NM.InitializeModel(device, "deadwood3.obj");
    m_Deadwood4NM.normalMapping = true;
    m_Deadwood4NM.InitializeModel(device, "deadwood4.obj");
    m_Deadwood5NM.normalMapping = true;
    m_Deadwood5NM.InitializeModel(device, "deadwood5.obj");
    m_Deadwood6NM.normalMapping = true;
    m_Deadwood6NM.InitializeModel(device, "deadwood6.obj");
    m_Deadwood7NM.normalMapping = true;
    m_Deadwood7NM.InitializeModel(device, "deadwood7.obj");
    m_Deadwood8NM.normalMapping = true;
    m_Deadwood8NM.InitializeModel(device, "deadwood8.obj");
    m_Deadwood9NM.normalMapping = true;
    m_Deadwood9NM.InitializeModel(device, "deadwood9.obj");

    /* Camp */
    m_CampIgloo.InitializeModel(device, "igloo.obj");
    m_CampCrowNM.normalMapping = true;
    m_CampCrowNM.InitializeModel(device, "crow.obj");
    m_CampSnow.InitializeModel(device, "camp_snow.obj");
    m_CampStones.InitializeModel(device, "camp_stones.obj");  
    m_CampTreeStonesNM.normalMapping = true;
    m_CampTreeStonesNM.InitializeModel(device, "camp_tree_stones.obj");
    m_CampDeadwoodNM.normalMapping = true;
    m_CampDeadwoodNM.InitializeModel(device, "camp_deadwood.obj");
    m_CampIceBorderNM.normalMapping = true;
    m_CampIceBorderNM.InitializeModel(device, "ice_border.obj");
    m_CampIceNM.normalMapping = true;
    m_CampIceNM.InitializeModel(device, "ice.obj");
    m_CampEstusNM.normalMapping = true;
    m_CampEstusNM.InitializeModel(device, "estus.obj");
    
    /* Bonfire */
    m_BfStonesNM.normalMapping = true;
    m_BfStonesNM.InitializeModel(device, "bf_stones.obj");
    m_BfAshNM.normalMapping = true;
    m_BfAshNM.InitializeModel(device, "bf_ash.obj");
    m_BfSkullsNM.normalMapping = true;
    m_BfSkullsNM.InitializeModel(device, "bf_skulls.obj");
    m_BfBones.InitializeModel(device, "bf_bones.obj");
    m_BfBlade.InitializeModel(device, "bf_blade.obj");
    m_BfHilt.InitializeModel(device, "bf_hilt.obj");

    /* Foliage */
    m_FoliageDeadBush1.InitializeModel(device, "foliage_deadbush1.obj");
    m_FoliageDeadBush2.InitializeModel(device, "foliage_deadbush2.obj");
    m_FoliageDeadBush3.InitializeModel(device, "foliage_deadbush3.obj");
    m_FoliageFern.InitializeModel(device, "foliage_fern.obj");
    m_FoliageGrass1.InitializeModel(device, "foliage_grass1.obj");
    m_FoliageGrass2.InitializeModel(device, "foliage_grass2.obj");
    m_FoliageGrass3.InitializeModel(device, "foliage_grass3.obj");
    m_FoliageGrass4.InitializeModel(device, "foliage_grass4.obj");
    m_FoliageGrass5.InitializeModel(device, "foliage_grass5.obj");
    m_FoliageGrass6.InitializeModel(device, "foliage_grass6.obj");
    m_FoliageGrass7.InitializeModel(device, "foliage_grass7.obj");
    m_FoliageGrass8.InitializeModel(device, "foliage_grass8.obj");
    m_FoliageGrass9.InitializeModel(device, "foliage_grass9.obj");
	
    /* Shaders */
    m_BasicShaderPair.InitStandard(device, L"light_vs.cso", L"light_ps.cso", D3D11_TEXTURE_ADDRESS_WRAP);    
    m_BasicShaderPairTiling.InitStandard(device, L"light_vs_tiling.cso", L"light_ps.cso", D3D11_TEXTURE_ADDRESS_WRAP); // Shader pair that tiles textures
    m_BasicShaderPairTilingNoNormalMap.InitStandard(device, L"light_vs_tiling_nonormalmap.cso", L"light_ps_nonormalmap.cso", D3D11_TEXTURE_ADDRESS_WRAP); // Shader pair that tiles textures, no normal mapping
    m_BasicShaderPairNoSpec.InitStandard(device, L"light_vs.cso", L"light_ps_nospec.cso", D3D11_TEXTURE_ADDRESS_WRAP); // Shader pair that does not add specular highlights
    m_BasicShaderPairNoSpecNoNormalMap.InitStandard(device, L"light_vs_nonormalmap.cso", L"light_ps_nospec_nonormalmap.cso", D3D11_TEXTURE_ADDRESS_WRAP); // Shader pair that does not compute specular highlights or bump normals
    m_BasicShaderPairNoNormalMap.InitStandard(device, L"light_vs_nonormalmap.cso", L"light_ps_nonormalmap.cso", D3D11_TEXTURE_ADDRESS_WRAP); // Shader pair that does not compute bump normals    
    m_BasicShaderPairShadowMap.InitStandard(device, L"light_vs_shadowmap.cso", L"light_ps_shadowmap.cso"); // Shader pair that renders just the vertex position in light space
    m_BasicShaderPairIce.InitStandard(device, L"light_vs_ice.cso", L"light_ps_ice.cso", D3D11_TEXTURE_ADDRESS_WRAP); // Shader pair that uses normal map to offset sampling of diffuse texture
    m_BasicShaderPairFire.InitStandard(device, L"fire_vs.cso", L"fire_ps.cso"); // Shader pair that produce fire effect

	/* Textures */
    CreateDDSTextureFromFile(device, L"snow_diffuse.dds", nullptr, m_texSnow.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"snow_normal.dds", nullptr, m_texSnowNormal.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"snow_mountain_upscale.dds", nullptr, m_texSnowMountain.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"snow_mountain_normal.dds", nullptr, m_texSnowMountainNormalMap.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"glacier.dds", nullptr, m_texGlacier.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"glacier_normal.dds", nullptr, m_texGlacierNormalMap.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"deadwood.dds", nullptr, m_texDeadwood.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"deadwood_normal.dds", nullptr, m_texDeadwoodNormal.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"igloo.dds", nullptr, m_texIgloo.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"crow.dds", nullptr, m_texCrow.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"crow_normal.dds", nullptr, m_texCrowNormal.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"terrain.dds", nullptr, m_texTerrain.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"camp_stones.dds", nullptr, m_texCampStones.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"camp_tree_stones.dds", nullptr, m_texCampTreeStones.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"camp_tree_stones_normal.dds", nullptr, m_texCampTreeStonesNormal.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"ice_desat.dds", nullptr, m_texIce.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"ice_normal.dds", nullptr, m_texIceNormal.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"bf_stones.dds", nullptr, m_texBfStones.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"bf_stones_normal.dds", nullptr, m_texBfStonesNormal.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"bf_ash.dds", nullptr, m_texBfAsh.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"bf_ash_normal.dds", nullptr, m_texBfAshNormal.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"bf_skulls.dds", nullptr, m_texBfSkulls.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"bf_skulls_normal.dds", nullptr, m_texBfSkullsNormal.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"bf_bones.dds", nullptr, m_texBfBones.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"bf_blade.dds", nullptr, m_texBfBlade.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"bf_hilt.dds", nullptr, m_texBfHilt.ReleaseAndGetAddressOf()); 
	CreateDDSTextureFromFile(device, L"foliage_fern.dds", nullptr, m_texFoliageFern.ReleaseAndGetAddressOf()); 
	CreateDDSTextureFromFile(device, L"foliage_deadbush.dds", nullptr, m_texFoliageDeadBush.ReleaseAndGetAddressOf()); 
	CreateDDSTextureFromFile(device, L"foliage_grass.dds", nullptr, m_texFoliageGrass.ReleaseAndGetAddressOf()); 
	CreateDDSTextureFromFile(device, L"estus_diffuse.dds", nullptr, m_texEF.ReleaseAndGetAddressOf()); 
	CreateDDSTextureFromFile(device, L"estus_normal.dds", nullptr, m_texEFNormal.ReleaseAndGetAddressOf()); 
	CreateDDSTextureFromFile(device, L"ice.dds", nullptr, m_texIce.ReleaseAndGetAddressOf()); 
	CreateDDSTextureFromFile(device, L"star.dds", nullptr, m_texStar.ReleaseAndGetAddressOf()); 
	CreateDDSTextureFromFile(device, L"fire01.dds", nullptr, m_texFire.ReleaseAndGetAddressOf()); 
	CreateDDSTextureFromFile(device, L"noise01.dds", nullptr, m_texFireNoise.ReleaseAndGetAddressOf()); 
	CreateDDSTextureFromFile(device, L"alpha01.dds", nullptr, m_texFireAlpha.ReleaseAndGetAddressOf());         
	CreateDDSTextureFromFile(device, L"fog.dds", nullptr, m_texFog.ReleaseAndGetAddressOf());         
    m_MiniMapTexture = new RenderTexture(device, 300, 240, 1, 2);	//for render to texture mini-map view

    /* Shadow Map */    
    //Load models for shadow map render pass (vertices array with only position property)
    m_Mountain1SM.shadowMapping = true;    
    m_Mountain1SM.InitializeModel(device, "mountain1.obj");
    m_Mountain2SM.shadowMapping = true;
    m_Mountain2SM.InitializeModel(device, "mountain2.obj");
    m_Glacier1SM.shadowMapping = true;
    m_Glacier1SM.InitializeModel(device, "glacier1.obj");
    m_Glacier2SM.shadowMapping = true;
    m_Glacier2SM.InitializeModel(device, "glacier2.obj");
    m_Deadwood1SM.shadowMapping = true;
    m_Deadwood1SM.InitializeModel(device, "deadwood1.obj");
    m_Deadwood2SM.shadowMapping = true;
    m_Deadwood2SM.InitializeModel(device, "deadwood2.obj");
    m_Deadwood3SM.shadowMapping = true;
    m_Deadwood3SM.InitializeModel(device, "deadwood3.obj");
    m_Deadwood4SM.shadowMapping = true;
    m_Deadwood4SM.InitializeModel(device, "deadwood4.obj");
    m_Deadwood5SM.shadowMapping = true;
    m_Deadwood5SM.InitializeModel(device, "deadwood5.obj");
    m_Deadwood6SM.shadowMapping = true;
    m_Deadwood6SM.InitializeModel(device, "deadwood6.obj");
    m_Deadwood7SM.shadowMapping = true;
    m_Deadwood7SM.InitializeModel(device, "deadwood7.obj");
    m_Deadwood8SM.shadowMapping = true;
    m_Deadwood8SM.InitializeModel(device, "deadwood8.obj");
    m_Deadwood9SM.shadowMapping = true;
    m_Deadwood9SM.InitializeModel(device, "deadwood9.obj");
    m_CampIglooSM.InitializeModel(device, "igloo.obj");
    m_CampCrowSM.shadowMapping = true;
    m_CampCrowSM.InitializeModel(device, "crow.obj");
    m_CampSnowSM.InitializeModel(device, "camp_snow.obj");
    m_CampStonesSM.InitializeModel(device, "camp_stones.obj");
    m_CampTreeStonesSM.shadowMapping = true;
    m_CampTreeStonesSM.InitializeModel(device, "camp_tree_stones.obj");
    m_CampDeadwoodSM.shadowMapping = true;
    m_CampDeadwoodSM.InitializeModel(device, "camp_deadwood.obj");
    m_CampIceBorderSM.shadowMapping = true;
    m_CampIceBorderSM.InitializeModel(device, "ice_border.obj");
    m_CampIceSM.shadowMapping = true;
    m_CampIceSM.InitializeModel(device, "ice.obj");
    m_CampEstusSM.shadowMapping = true;
    m_CampEstusSM.InitializeModel(device, "estus.obj");
    m_BfStonesSM.shadowMapping = true;
    m_BfStonesSM.InitializeModel(device, "bf_stones.obj");
    m_BfAshSM.shadowMapping = true;
    m_BfAshSM.InitializeModel(device, "bf_ash.obj");
    m_BfSkullsSM.shadowMapping = true;
    m_BfSkullsSM.InitializeModel(device, "bf_skulls.obj");
    m_BfBonesSM.InitializeModel(device, "bf_bones.obj");
    m_BfBladeSM.InitializeModel(device, "bf_blade.obj");
    m_BfHiltSM.InitializeModel(device, "bf_hilt.obj");

    D3D11_TEXTURE2D_DESC shadowMapDesc;
    ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
    shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    shadowMapDesc.MipLevels = 1;
    shadowMapDesc.ArraySize = 1;
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    shadowMapDesc.Height = static_cast<UINT>(2048);
    shadowMapDesc.Width = static_cast<UINT>(2048);

    device->CreateTexture2D(
        &shadowMapDesc,
        nullptr,
        &m_shadowMap
    );

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;


    device->CreateDepthStencilView(
        m_shadowMap.Get(),
        &depthStencilViewDesc,
        &m_shadowDepthView
    );

    device->CreateShaderResourceView(
        m_shadowMap.Get(),
        &shaderResourceViewDesc,
        &m_shadowResourceView
    );

    //Create render state than can be used to enable front face culling
    D3D11_RASTERIZER_DESC shadowRenderStateDesc;
    ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
    shadowRenderStateDesc.CullMode = D3D11_CULL_FRONT;
    shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
    shadowRenderStateDesc.DepthClipEnable = true;

    device->CreateRasterizerState(&shadowRenderStateDesc, &m_shadowRenderState);

    /* Load non normal mapping versions (no tangent or binormals in vertices array) of models for minimap render pass */
    m_GroundBox.InitializeModel(device, "ground_box.obj");
    m_Mountain1.InitializeModel(device, "mountain1.obj");
    m_Mountain2.InitializeModel(device, "mountain2.obj");
    m_Glacier1.InitializeModel(device, "glacier1.obj");
    m_Glacier2.InitializeModel(device, "glacier2.obj");
    m_Deadwood1.InitializeModel(device, "deadwood1.obj");
    m_Deadwood2.InitializeModel(device, "deadwood2.obj");
    m_Deadwood3.InitializeModel(device, "deadwood3.obj");
    m_Deadwood4.InitializeModel(device, "deadwood4.obj");
    m_Deadwood5.InitializeModel(device, "deadwood5.obj");
    m_Deadwood6.InitializeModel(device, "deadwood6.obj");
    m_Deadwood7.InitializeModel(device, "deadwood7.obj");
    m_Deadwood8.InitializeModel(device, "deadwood8.obj");
    m_Deadwood9.InitializeModel(device, "deadwood9.obj");
    m_CampCrow.InitializeModel(device, "crow.obj");
    m_CampTreeStones.InitializeModel(device, "camp_tree_stones.obj");
    m_CampDeadwood.InitializeModel(device, "camp_deadwood.obj");
    m_CampIceBorder.InitializeModel(device, "ice_border.obj");
    m_CampIce.InitializeModel(device, "ice.obj");
    m_CampEstus.InitializeModel(device, "estus.obj");
    m_BfStones.InitializeModel(device, "bf_ash.obj");
    m_BfAsh.InitializeModel(device, "bf_stones.obj");
    m_BfSkulls.InitializeModel(device, "bf_skulls.obj");
    
    /* Particle System */
    m_ParticleShader = new ShaderParticles;
    m_ParticleShader->InitStandard(device, L"particle_vs.cso", L"particle_ps.cso");
    m_ParticleSystem = new ParticleSystemClass;
    m_ParticleSystem->Initialize(device);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        1000.0f
    );

    m_effect->SetProjection(m_projection);
}


void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_font.reset();
	m_batch.reset();
    m_batchInputLayout.Reset();
    m_sky.reset();
    m_effect.reset();
    m_skyInputLayout.Reset();
    m_cubemap.Reset();
    m_ParticleSystem->Shutdown();
    delete m_ParticleSystem;  
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}

#pragma endregion
