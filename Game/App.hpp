#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EventSystem.hpp"

class App
{
public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void Run();

	bool IsQuitting() const { return m_isQuitting; }
	void HandleKeyPressed( unsigned char keyCode );
	void HandleKeyReleased( unsigned char keyCode );
	bool HandleQuitRequested();

	bool IsKeyDown(unsigned char keyCode);
	bool WasKeyJustPressed(unsigned char keyCode);

	void LoadAudio();

	static bool QuitRequested(EventArgs& args);

	bool m_isQuitting = false;
	bool m_isPaused = false;
	bool m_isFastMo = false;
	bool m_isSlowMo = false;
	bool m_debugDraw = false;
	bool m_developerCheat = false;
	bool m_debugCamera = false;

	mutable Camera	m_worldCamera;
	mutable Camera	m_consoleCamera;

	SoundID m_attractMusicID;
	SoundID m_playMusicID;
	SoundID m_pauseMusicID;
	SoundID m_unPauseMusicID;

	SoundPlaybackID m_attractMusic;
	SoundPlaybackID m_playMusic;
	SoundPlaybackID m_pauseMusic;
	SoundPlaybackID m_unPauseMusic;
	bool m_isPlayMusicPlaying = false;
	bool m_isAttractMusicPlaying = false;
	bool m_isLoop = true;
	bool m_heatMode = false;
	bool m_showHP = true;
	int  m_heatIndex = 0;
	float m_audioSpeed = 1.f;

private:
	void BeginFrame();
	void Update( float deltaSeconds );
	void Render() const;
	void EndFrame();
};


