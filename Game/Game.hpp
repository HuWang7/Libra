#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include <vector>


//------------------------------------------------------------------------------------------------------
class App;
class Entity;
class Map;

//------------------------------------------------------------------------------------------------------
class Game 
{

public:
	Game( App* owner);
	~Game();

	App* m_App = nullptr;

	Vertex_PCU attractModeTriangle[NUM_TRIANGLE_VERTS];

	bool			  m_isAttractMode	= true;
	bool			  m_boolAlpha		= true;
	int				  m_alpha			= 255;
	float			  m_endTime			= 3.f;
	float			  m_blinkTime		= 2.f;
	Map*			  m_currentMap		= nullptr; 
	Texture*		  m_attractTexture  = nullptr;
	Texture*		  m_failTexture		= nullptr;
	Texture*          m_victoryTexture  = nullptr;
	SpriteSheet*	  m_spriteSheet		= nullptr;
	std::vector<Map*> m_maps;
	bool			  m_weatherContinue = false;
	bool			  m_victoryMode     = false;

	void Startup();
	void Update(float deltaSeconds); 
	void Render() const;
	void Shutdown();
	void CheckGameOver();

public:
	SoundID m_clickAudio;
	SoundID m_playerShootAudio;
	SoundID m_NPCShootAudio;
	SoundID m_playerHitAudio;
	SoundID m_NPCHitAudio;
	SoundID m_DieAudio;
	SoundID m_victoryAudio;
	SoundID m_bulletBounceAudio;
	SoundID m_bulletRicochetAudio;
	SoundID m_foundPlayer;
	float	m_isPlayFoundedAudio = 0.f;

private:
	void UpdateAttract(float deltaSeconds);
	void RenderAttract() const;
	void RenderPausedMode() const;
	void RenderFailMode() const;
	void RenderVictoryMode() const;
	void LoadAssets();
	void CreateNewMap();
	void InitializeMapDefs();
	void TransportEntitiesToNewMap(Map* currentMap, Map* nextMap);
	float m_playerDeadTime = 0.f;
	Vec2 m_exitPos;
};

