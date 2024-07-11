#include "Game.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/HeatMaps.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/DevConsole.hpp"

extern App* g_theApp;
extern RandomNumberGenerator* g_rng;
extern Renderer* g_theRenderer;
extern AudioSystem* g_theAudio;
extern BitmapFont* g_theFont;
extern DevConsole* g_theConsole;


Game::Game(App* owner)
	: m_App(owner)
{
}

Game::~Game()
{

}

void Game::Startup()
{
	LoadAssets(); 
	TileDefinition::InitializeTileDefs();
	m_attractTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/AttractScreen.png");
	m_failTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/YouDiedScreen.png");
	m_victoryTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/VictoryScreen.jpg");
	m_currentMap->Startup();
	if (m_isAttractMode)
	{
		g_theAudio->SetSoundPlaybackSpeed(g_theApp->m_attractMusic, 1.f);
	}
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
}

void Game::Update(float deltaSeconds)
{
	m_isPlayFoundedAudio += deltaSeconds;
	float perishFadeoutSeconds = g_gameConfigBlackboard.GetValue("perishFadeoutSeconds", 3.0f);
	if (m_isAttractMode)
	{
		UpdateAttract(deltaSeconds);
		return;
	}
	if (m_currentMap->CheckPlayerDead())
	{
		m_playerDeadTime += deltaSeconds;
		if (m_playerDeadTime >= perishFadeoutSeconds)
		{
			g_theApp->m_isPaused = true;
			m_weatherContinue = true;
			m_playerDeadTime = 0.f;
		}
	}
	if (IsPointInsideDisc2D(m_exitPos, m_currentMap->m_player->m_position, m_currentMap->m_player->m_physicsRadius))
	{
		CheckGameOver();
	}
	m_currentMap->Update(deltaSeconds);
}

void Game::Render() const
{
	if (m_isAttractMode)
	{
		RenderAttract();
		return;
	}
	else 
	{
		m_currentMap->Render();
		if (g_theApp->m_isPaused)
		{
			RenderPausedMode();
		}
		if (m_victoryMode)
		{
			RenderVictoryMode();
		}
		if (m_weatherContinue)
		{
			RenderFailMode();
		}
	}
}

void Game::Shutdown()
{
	m_currentMap->ShutDown();
	delete m_currentMap;
	m_currentMap = nullptr;
}




void Game::UpdateAttract(float deltaSeconds)
{
	attractModeTriangle[0].m_position = Vec3(1.0f, 0.0f, 0.0f);
	attractModeTriangle[1].m_position = Vec3(-1.0f, 1.0f, 0.0f);
	attractModeTriangle[2].m_position = Vec3(-1.0f, -1.0f, 0.0f);

	if (m_boolAlpha)
	{
		m_blinkTime -= deltaSeconds;
		if (m_blinkTime < 0.4f) {
			m_blinkTime = 0.4f;
			m_boolAlpha = false;
		}
	}
	else
	{
		m_blinkTime += deltaSeconds;
		if (m_blinkTime > 2.f) {
			m_blinkTime = 2.f;
			m_boolAlpha = true;
		}
	}
	m_alpha = (unsigned char)(255 * m_blinkTime / 2.f);

	for (int vertIndex = 0; vertIndex < NUM_TRIANGLE_VERTS; ++vertIndex)
	{
		attractModeTriangle[vertIndex].m_color = Rgba8(124, 254, 0, (unsigned char)m_alpha);
	}
}


void Game::RenderAttract() const
{
	AABB2 textureBox = AABB2(0.f, 0.f, 16.f, 8.f);
	std::vector<Vertex_PCU> textureVerts;
	AddVertsForAABB2D(textureVerts, textureBox, Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(m_attractTexture);
	g_theRenderer->DrawVertexArray((int)textureVerts.size(), textureVerts.data());
	g_theRenderer->BindTexture(nullptr);


	Vertex_PCU tempPlayVerts_3[NUM_TRIANGLE_VERTS];
	for (int vertIndex = 0; vertIndex < NUM_TRIANGLE_VERTS; ++vertIndex)
	{
		tempPlayVerts_3[vertIndex] = attractModeTriangle[vertIndex];
	}
	TransfromVertexArrayXY3D(NUM_TRIANGLE_VERTS, tempPlayVerts_3, 1.f, 0.f, Vec2(8.f, 4.f));
	g_theRenderer->DrawVertexArray(NUM_TRIANGLE_VERTS, tempPlayVerts_3);

	AABB2 textBox = AABB2(0.f, 0.f, 5.f, 5.f);
	std::vector<Vertex_PCU> textVerts;
	AddVertsForAABB2D(textVerts, textBox, Rgba8(0, 0, 0, 255));
	g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());
	Texture* fontTexture = &g_theFont->GetTexture();
	g_theRenderer->BindTexture(fontTexture);
	g_theFont->AddVertsForTextInBox2D(textVerts, textBox,  1.f, "test text \naaabbbcccdddeee", Rgba8(255, 255, 255, 255), 1.f, Vec2(1.f, 1.f), TextBoxMode::SHRINK, 12);
	g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());
	g_theRenderer->BindTexture(nullptr);
}

void Game::RenderPausedMode() const
{
	AABB2 pauseBox = AABB2(g_theApp->m_worldCamera.GetOrthographicBottomLeft(), g_theApp->m_worldCamera.GetOrthographicTopRight());
	std::vector<Vertex_PCU> pauseVerts;
	AddVertsForAABB2D(pauseVerts, pauseBox, Rgba8(0, 0, 0, 128));
	g_theRenderer->DrawVertexArray((int)pauseVerts.size(), pauseVerts.data());
}

void Game::RenderFailMode() const
{
	AABB2 failBox = AABB2(g_theApp->m_worldCamera.GetOrthographicBottomLeft(), g_theApp->m_worldCamera.GetOrthographicTopRight());
	std::vector<Vertex_PCU> failVerts;
	AddVertsForAABB2D(failVerts, failBox, Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(m_failTexture);
	g_theRenderer->DrawVertexArray((int)failVerts.size(), failVerts.data());
}

void Game::RenderVictoryMode() const
{
	AABB2 victoryBox = AABB2(g_theApp->m_worldCamera.GetOrthographicBottomLeft(), g_theApp->m_worldCamera.GetOrthographicTopRight());
	std::vector<Vertex_PCU> victoryVerts;
	AddVertsForAABB2D(victoryVerts, victoryBox, Rgba8(255, 255, 255, 255));
	g_theRenderer->BindTexture(m_victoryTexture);
	g_theRenderer->DrawVertexArray((int)victoryVerts.size(), victoryVerts.data());
}

void Game::LoadAssets()
{
	Texture* Texture_8x8 = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
	m_spriteSheet = new SpriteSheet(*Texture_8x8, IntVec2(8, 8));
	CreateNewMap();
	m_exitPos = Vec2((float)m_currentMap->m_dimensions.x-1.5f, (float)m_currentMap->m_dimensions.y-1.5f);

	m_clickAudio = g_theAudio->CreateOrGetSound("Data/Audio/Click.mp3");
	m_playerShootAudio = g_theAudio->CreateOrGetSound("Data/Audio/PlayerShootNormal.ogg");
	m_NPCShootAudio = g_theAudio->CreateOrGetSound("Data/Audio/EnemyShoot.wav");
	m_playerHitAudio = g_theAudio->CreateOrGetSound("Data/Audio/PlayerHit.wav");
	m_NPCHitAudio = g_theAudio->CreateOrGetSound("Data/Audio/EnemyHit.wav");
	m_DieAudio = g_theAudio->CreateOrGetSound("Data/Audio/EnemyDied.wav");
	m_victoryAudio = g_theAudio->CreateOrGetSound("Data/Audio/Victory.mp3");
	m_bulletBounceAudio = g_theAudio->CreateOrGetSound("Data/Audio/BulletBounce.wav");
	m_bulletRicochetAudio = g_theAudio->CreateOrGetSound("Data/Audio/BulletRicochet2.wav");
	m_foundPlayer = g_theAudio->CreateOrGetSound("Data/Audio/FindPkayer.wav");
}

void Game::CreateNewMap()
{
	for (int i = 0; i < (int)m_maps.size(); ++i)
	{
		delete m_maps[i];
		m_maps[i] = nullptr;
	}
	m_currentMap = nullptr;
	m_maps.clear();
	InitializeMapDefs();
	m_currentMap = m_maps[0];
	m_currentMap->SpawnNewEntity(ENTITY_TYPE_GOOD_PLAYER, Vec2(1.5f, 1.5f), 0.f);
}

void Game::InitializeMapDefs()
{
	XmlDocument mapDefsXml;
	char const* filePath = "Data/Definitions/MapDefinitions.xml";
	XmlResult result = mapDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required map defs file \"%s\"", filePath));

	XmlElement* rootElement = mapDefsXml.RootElement();
	GUARANTEE_OR_DIE( rootElement, "whut");
	
	XmlElement* mapDefElement = rootElement->FirstChildElement();
	while (mapDefElement)
	{
		MapDefinition mapDef;
		mapDef.m_name = ParseXmlAttribute(*mapDefElement, "name", "Approach");
		mapDef.m_dimensions = ParseXmlAttribute(*mapDefElement, "dimensions", IntVec2(48, 24));
		mapDef.m_fillTileType = ParseXmlAttribute(*mapDefElement, "fillTileType", "LongGrass");
		mapDef.m_edgeTileType = ParseXmlAttribute(*mapDefElement, "edgeTileType", "RockWall");

		mapDef.m_worm1Count = ParseXmlAttribute(*mapDefElement, "worm1Count", 15);
		mapDef.m_worm1TileType = ParseXmlAttribute(*mapDefElement, "worm1TileType", "DarkGrass");
		mapDef.m_worm1MaxLength = ParseXmlAttribute(*mapDefElement, "worm1MaxLength", 12);
		mapDef.m_worm2Count = ParseXmlAttribute(*mapDefElement, "worm2Count", 60);
		mapDef.m_worm2TileType = ParseXmlAttribute(*mapDefElement, "worm2TileType", "RockWall");
		mapDef.m_worm2MaxLength = ParseXmlAttribute(*mapDefElement, "worm2MaxLength", 8);

		mapDef.m_endFloorTileType = ParseXmlAttribute(*mapDefElement, "endFloorTileType", "Concrete");
		mapDef.m_endBunkerTileType = ParseXmlAttribute(*mapDefElement, "endBunkerTileType", "StoneWall");
		mapDef.m_startFloorTileType = ParseXmlAttribute(*mapDefElement, "startFloorTileType", "Concrete");
		mapDef.m_startBunkerTileType = ParseXmlAttribute(*mapDefElement, "startBunkerTileType", "RockWall");
		mapDef.m_mapEntry = ParseXmlAttribute(*mapDefElement, "mapEntry", "MapEntry");
		mapDef.m_mapExit = ParseXmlAttribute(*mapDefElement, "mapExit", "MapExit");

		mapDef.m_entitySpawnCounts[ENTITY_TYPE_EVIL_SCORPIO] = ParseXmlAttribute(*mapDefElement, "scorpioCount", 1);
		mapDef.m_entitySpawnCounts[ENTITY_TYPE_EVIL_LEO] = ParseXmlAttribute(*mapDefElement, "leoCount", 1);
		mapDef.m_entitySpawnCounts[ENTITY_TYPE_EVIL_ARIES] = ParseXmlAttribute(*mapDefElement, "ariesCount", 1);
		mapDef.m_entitySpawnCounts[ENTITY_TYPE_EVIL_CAPRICORN] = ParseXmlAttribute(*mapDefElement, "capricornCount", 1);
		m_maps.push_back(new Map(this, mapDef));
		mapDefElement = mapDefElement->NextSiblingElement();
	}


}

void Game::CheckGameOver()
{
	Map* nextMap = nullptr;
	for (int i = 0; i < m_maps.size(); ++i)
	{
		if (m_currentMap == m_maps[i])
		{
			if (i == m_maps.size() - 1)
			{
				g_theAudio->StartSound(m_victoryAudio);
				m_victoryMode = true;
				g_theApp->m_isPaused = true;
			}
			else
			{
				nextMap = m_maps[i+1];
				TransportEntitiesToNewMap(m_currentMap, nextMap);
				m_currentMap = nextMap;
				m_currentMap->Startup();
				m_currentMap->m_player->m_position = Vec2(1.5f, 1.5f);
				m_exitPos = Vec2((float)m_currentMap->m_dimensions.x-1.5f, (float)m_currentMap->m_dimensions.y-1.5f);
				return;
			}
		}
	}
}

void Game::TransportEntitiesToNewMap(Map* currentMap, Map* nextMap)
{
	for (int i = 0; i < currentMap->m_entityListsByType[ENTITY_TYPE_GOOD_PLAYER].size(); ++i)
	{
		Entity* entity = currentMap->m_entityListsByType[ENTITY_TYPE_GOOD_PLAYER][i];
		if (entity != nullptr)
		{
			nextMap->AddEntityToList(entity, nextMap->m_entityListsByType[ENTITY_TYPE_GOOD_PLAYER]);
			nextMap->AddEntityToList(entity, nextMap->m_actorsByFaction[FACTION_GOOD]);
			nextMap->AddEntityToList(entity, nextMap->m_allEntities);
		}
	}
}

