#include <windows.h>
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/XmlUtils.hpp"
using namespace std;

App*		 g_theApp = nullptr;				// Created and owned by Main_Windows.cpp
Game*		 m_theGame = nullptr;				// Created and owned by the App
Renderer*	 g_theRenderer = nullptr;
Window*		 g_theWindow = nullptr;
InputSystem* g_theInput = nullptr;
BitmapFont*	 g_theFont = nullptr;
AudioSystem* g_theAudio = nullptr;
extern DevConsole* g_theConsole;
extern EventSystem* g_theEventSystem;
double       g_previousFrameTime = 0.0;




App::App(){}		
App::~App(){}

void App::Startup()
{
	Clock::TickSystemClock();

	XmlDocument xmlDocument;
	XmlResult loadResult = xmlDocument.LoadFile("Data/GameConfig.xml");
	if (loadResult == tinyxml2::XML_SUCCESS) 
	{
		XmlElement* root = xmlDocument.RootElement();
		g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*root);
	}

	// Create engine Subsystems and game
	InputConfig inputConfig;
	g_theInput = new InputSystem( inputConfig );

	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "SD-A8: Libra";
	windowConfig.m_clientAspect = 2;
	g_theWindow = new Window( windowConfig );

	RenderConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer( renderConfig );

	AudioConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );

	m_theGame = new Game(this);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_theRenderer = g_theRenderer;
	devConsoleConfig.m_camera = &m_consoleCamera;
	g_theConsole = new DevConsole(devConsoleConfig);

	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	// Start up engine subsystems and game
	
	g_theConsole->Startup();
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theAudio->Startup();
	g_theEventSystem->Startup();
	m_theGame -> Startup();
	g_theFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont"); // DO NOT SPECIFY FILE .EXTENSION!!  (Important later on.)
	
	LoadAudio();
	if (m_theGame->m_isAttractMode)
	{
		m_attractMusic = g_theAudio->StartSound(g_theApp->m_attractMusicID, true, 1.0f, 0.0f, 1.0f, false);
	}
	g_previousFrameTime = GetCurrentTimeSeconds();

	g_theEventSystem->SubscribeEventCallbackFunction("quit", QuitRequested);

}

void App::Shutdown() 
{
	g_theInput->Shutdown();
	delete g_theInput;
	g_theInput = nullptr;

	g_theWindow->Shutdown();
	delete g_theWindow;
	g_theWindow = nullptr;

	g_theRenderer->Shutdown();
	delete g_theRenderer;
	g_theRenderer = nullptr;

	g_theAudio->Shutdown();
	delete g_theAudio;
	g_theAudio = nullptr;

	g_theConsole->Shutdown();
	delete g_theConsole;
	g_theConsole = nullptr;

	g_theEventSystem->Shutdown();
	delete g_theEventSystem;
	g_theEventSystem = nullptr;

	m_theGame->Shutdown();
	delete m_theGame;
	m_theGame = nullptr;
}

void App::Run() 
{
	while (!IsQuitting())
	{
		double currentFrameTime = GetCurrentTimeSeconds();
		double deltaSeconds = currentFrameTime - g_previousFrameTime;

		// Clamp the deltaSeconds to a maximum of 0.1 seconds.
		if (deltaSeconds > 0.1)
		{
			deltaSeconds = 0.1;
		}
		BeginFrame();
		Update((float)deltaSeconds);
		Render();
		EndFrame();
		g_previousFrameTime = currentFrameTime;
	}
}

void App::BeginFrame()
{
	Clock::TickSystemClock();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();
	g_theConsole->BeginFrame();
	g_theEventSystem->BeginFrame();
}

void App::Update(float deltaSeconds)
{	
	m_theGame -> Update(deltaSeconds);
	XboxController const& controller = g_theInput->GetController(0);
	if (WasKeyJustPressed('P') || controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
	{
		if (m_theGame->m_currentMap->m_player->m_isDead)
		{
			g_theAudio->StartSound(m_theGame->m_clickAudio);
			m_theGame->m_currentMap->m_player->m_health = 10;
			m_theGame->m_currentMap->m_player->m_isDead = false;
			g_theApp->m_isPaused = false;
			m_theGame->m_weatherContinue = false;
			m_theGame->m_currentMap->m_player->m_needExplosions = true;
		}
		else
		{
			if (!m_isPaused)
			{
				if (m_theGame->m_isAttractMode)
				{
					g_theAudio->SetSoundPlaybackSpeed(m_attractMusic, 0.f);
					g_theAudio->StartSound(m_theGame->m_clickAudio);
					m_playMusic = g_theAudio->StartSound(m_playMusicID, true);
					m_theGame->m_isAttractMode = false;
				}
				else
				{
					g_theAudio->SetSoundPlaybackSpeed(m_playMusic, 0.f);
					g_theAudio->StartSound(m_pauseMusicID);
					m_isPaused = true;
				}
			}
			else
			{
				if (m_theGame->m_isAttractMode)
				{
					g_theAudio->SetSoundPlaybackSpeed(m_attractMusic, 0.f);
					g_theAudio->StartSound(m_theGame->m_clickAudio);
					m_playMusic = g_theAudio->StartSound(m_playMusicID, true);
					m_theGame->m_isAttractMode = false;
					m_isPaused = false;
				}
				else if (m_theGame->m_victoryMode)
				{
					g_theAudio->StartSound(m_theGame->m_clickAudio);
					g_theAudio->SetSoundPlaybackSpeed(m_attractMusic, 1.f);
					m_theGame->Shutdown();
					delete m_theGame;
					m_theGame = nullptr;
					m_theGame = new Game(this);
					m_theGame->Startup();
					m_isPaused = false;
				}
				else
				{
					g_theAudio->StartSound(m_unPauseMusicID);
					g_theAudio->SetSoundPlaybackSpeed(m_playMusic, 1.f);
					m_isPaused = false;
				}
			}
		}
	}
	if (!m_theGame->m_isAttractMode && !IsKeyDown('T'))
	{
		if (IsKeyDown('Y'))
		{
			m_isFastMo = true;
			g_theAudio->SetSoundPlaybackSpeed(m_playMusic, 4.0f);		
		}

		else 
		{
			m_isFastMo = false;
			g_theAudio->SetSoundPlaybackSpeed(m_playMusic, 1.f);
		}
	}
	if (!m_theGame->m_isAttractMode && !IsKeyDown('Y'))
	{
		if (IsKeyDown('T'))
		{
			m_isSlowMo = true;
			g_theAudio->SetSoundPlaybackSpeed(m_playMusic, 0.5f);
		}
		else
		{
			m_isSlowMo = false;
			g_theAudio->SetSoundPlaybackSpeed(m_playMusic, 1.f);
		}
	}

	if ( WasKeyJustPressed(VK_ESCAPE)|| controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_BACK))
	{
		if (m_theGame->m_isAttractMode)
		{
			m_isQuitting = true;
		}
		else if (m_theGame->m_isAttractMode == false && !m_isPaused)
		{
			g_theAudio->SetSoundPlaybackSpeed(m_playMusic, 0.f);;
			g_theAudio->StartSound(m_pauseMusicID);
			m_isPaused = true;
		}
		else if (m_theGame->m_isAttractMode == false && m_isPaused)
		{
			g_theAudio->StartSound(m_theGame->m_clickAudio);
			m_worldCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(16.f, 8.f));
			m_consoleCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(16.f, 8.f));
			m_theGame->Shutdown();
			delete m_theGame;
			m_theGame = nullptr;

			m_theGame = new Game(this);
			m_theGame->Startup();
		}

	}
	if (WasKeyJustPressed(VK_F6))
	{
		if (m_heatIndex >= 4)
		{
			m_heatIndex = 0;
			m_heatMode = false;
		}
		else
		{	
			m_heatMode = true;
			m_heatIndex += 1;
		}
	}

	if (WasKeyJustPressed(VK_F8))
	{
		g_theAudio->StopSound(m_playMusic);
		m_theGame->Shutdown();
		delete m_theGame;
		m_theGame = nullptr;

		m_theGame = new Game(this);
		m_theGame -> Startup();
	}
	if (WasKeyJustPressed(VK_F9))
	{
		g_theAudio->StartSound(m_theGame->m_DieAudio);
		m_theGame->CheckGameOver();
	}

	if (WasKeyJustPressed(VK_F1))
	{
		m_debugDraw = !m_debugDraw;
	}

	if (WasKeyJustPressed(VK_F3))
	{
		m_developerCheat = !m_developerCheat;
	}

	if (WasKeyJustPressed(VK_F4))
	{
		m_debugCamera = !m_debugCamera;
	}

	if (WasKeyJustPressed('H'))
	{
		m_showHP = !m_showHP;
	}

	if (WasKeyJustPressed(192))// KeyBoard "~"
	{
		g_theConsole->ToggleMode();
	}

	if (m_isPaused)
	{
		g_theAudio->SetSoundPlaybackSpeed(m_playMusic, 0.f);
	}
}

void App::Render() const
{
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
	g_theRenderer->BeginCamera(m_worldCamera);
	m_theGame->Render();
	if (g_theConsole->IsOpen())
	{
		g_theRenderer->BeginCamera(m_consoleCamera);
		//g_theConsole->Render(AABB2(g_theApp->m_worldCamera.m_bottomLeft, g_theApp->m_worldCamera.m_topRight), g_theRenderer);
		g_theConsole->Render_OpenFull(AABB2(m_consoleCamera.GetOrthographicBottomLeft(), m_consoleCamera.GetOrthographicTopRight()), *g_theRenderer, *g_theFont);
		g_theRenderer->EndCamera(m_consoleCamera);

	}
	g_theRenderer->EndCamera(m_worldCamera);
}

void App::EndFrame()
{
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	g_theAudio->EndFrame();
	g_theConsole->EndFrame();
	g_theEventSystem->EndFrame();
}

void App::HandleKeyPressed(unsigned char keyCode)
{
	g_theInput->HandleKeyPressed(keyCode);
}

void App::HandleKeyReleased(unsigned char keyCode)
{
	g_theInput->HandleKeyReleased(keyCode);
}

bool App::HandleQuitRequested()
{
	return false;
}

bool App::IsKeyDown(unsigned char keyCode)
{
	return g_theInput->IsKeyDown(keyCode);
}

bool App::WasKeyJustPressed(unsigned char keyCode)
{
	return g_theInput->WasKeyJustPressed(keyCode);
}

void App::LoadAudio()
{
	m_attractMusicID = g_theAudio->CreateOrGetSound("Data/Audio/AttractMusic.mp3");
	m_playMusicID = g_theAudio->CreateOrGetSound("Data/Audio/GameplayMusic.mp3");
	m_pauseMusicID = g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3");
	m_unPauseMusicID = g_theAudio->CreateOrGetSound("Data/Audio/Unpause.mp3");
}

bool App::QuitRequested(EventArgs& args)
{
	UNUSED(args);
	g_theApp->m_isQuitting = true;
	return true;
}


