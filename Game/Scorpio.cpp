#include "Game/Scorpio.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"

extern Renderer* g_theRenderer;
extern Game* m_theGame;
extern AudioSystem* g_theAudio;
extern App* g_theApp;

Scorpio::Scorpio(EntityType type, EntityFaction faction, Map* owner, const Vec2& startPos, float orientationDegrees)
	: Entity(type, faction, owner, startPos, orientationDegrees)
{
	m_health = 3;
	m_maxHealth = m_health;
	m_orientationDegreesTurret = orientationDegrees;
	m_faction = FACTION_EVIL;
	m_entityType = ENTITY_TYPE_EVIL_SCORPIO;
	m_doesPushEntities = true;
	m_isPushedEntities = false;
	m_tankTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTurretBase.png");
	m_turretTeture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyCannon.png");
	m_tankcolor = Rgba8(255, 255, 255, 255);
	m_turretcolor = Rgba8(255, 255, 255, 255);
	m_physicsRadius = g_gameConfigBlackboard.GetValue("ENTITY_PHYSICS_RADIUS", 0.3f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("ENTITY_COSMETIC_RADIUS", 0.6f);
	InitializeLocalVertsForTank();
	InitializeLocalVertsForTurret();

}

Scorpio::~Scorpio()
{

}

void Scorpio::Update(float deltaSeconds)
{
	float scorpioTurnRate = g_gameConfigBlackboard.GetValue("scorpioTurnRate", 10.f);
	//float scorpioTurnAperture = g_gameConfigBlackboard.GetValue("scorpioTurnAperture", 10.f);
	float scorpioShootCooldownSeconds = g_gameConfigBlackboard.GetValue("scorpioShootCooldownSeconds", 10.f);
	if (m_health <= 0)
	{
		m_theGame->m_currentMap->EntityDiedExplosions(deltaSeconds, m_position);
		m_isDead = true;
		m_isGarbage = true;
	}
	RayCastOnTile();
	if (CheckPlayerInSight())
	{
		if (m_playFindPlayerAudio && m_theGame->m_isPlayFoundedAudio > 0.1f)
		{
			g_theAudio->StartSound(m_theGame->m_foundPlayer);
			m_playFindPlayerAudio = false;
			m_theGame->m_isPlayFoundedAudio = 0.f;
		}
		m_orientationDegreesTurret = GetTurnedTowardDegrees(m_orientationDegreesTurret, m_Degrees, scorpioTurnRate * deltaSeconds);
		
		float angleBetweenPlrAndEnm  = GetAngleDegreesBetweenVectors2D(m_map->GetPlayerPos()-m_position, m_position.MakeFromPolarDegrees(m_orientationDegreesTurret,1.f));
		if ( angleBetweenPlrAndEnm < 5.f)
		{
			m_fireTime += deltaSeconds;
			if (m_fireTime > scorpioShootCooldownSeconds)
			{
				g_theAudio->StartSound(m_theGame->m_NPCShootAudio);
				m_fireTime = 0.f;
				Vec2 gunpoint = m_position + Vec2::MakeFromPolarDegrees(m_orientationDegreesTurret, 0.4f);
				m_theGame->m_currentMap->SpawnNewEntity(ENTITY_TYPE_EVIL_BULLET, gunpoint, m_orientationDegreesTurret);
				m_theGame->m_currentMap->EntityFireExplosions(deltaSeconds, gunpoint, m_orientationDegreesTurret);
			}
		}
	}
	else
	{
		m_playFindPlayerAudio = true;
		m_orientationDegreesTurret -= deltaSeconds * scorpioTurnRate;
	}
}

void Scorpio::Render() const
{
	Vertex_PCU tempTankWorldVerts[NUM_SCORPIO_VERTS];
	for (int vertIndex = 0; vertIndex < NUM_SCORPIO_VERTS; ++vertIndex)
	{
		tempTankWorldVerts[vertIndex] = m_localVertsTank[vertIndex];
	}

	TransfromVertexArrayXY3D(NUM_SCORPIO_VERTS, tempTankWorldVerts, 1.f, m_orientationDegreesTank, m_position);
	g_theRenderer->BindTexture(m_tankTexture);
	g_theRenderer->DrawVertexArray(NUM_SCORPIO_VERTS, tempTankWorldVerts);
	g_theRenderer->BindTexture(nullptr);

	Vertex_PCU tempTurretWorldVerts[NUM_SCORPIO_VERTS];
	for (int vertIndex = 0; vertIndex < NUM_SCORPIO_VERTS; ++vertIndex)
	{
		tempTurretWorldVerts[vertIndex] = m_localVertsTurret[vertIndex];
	}

	TransfromVertexArrayXY3D(NUM_SCORPIO_VERTS, tempTurretWorldVerts, 1.f, m_orientationDegreesTurret, m_position);
	g_theRenderer->BindTexture(m_turretTeture);
	g_theRenderer->DrawVertexArray(NUM_SCORPIO_VERTS, tempTurretWorldVerts);
	g_theRenderer->BindTexture(nullptr);

	Vec2 end = m_position + m_position.MakeFromPolarDegrees(m_orientationDegreesTurret, m_raycastLength);
	LineSegment2 Line = LineSegment2(m_position, end);
	std::vector<Vertex_PCU> verts;
	AddVertsForLineSegment2D(verts, Line, 0.02f, Rgba8(255, 0, 0, 128));
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
	g_theRenderer->BindTexture(nullptr);

	if (g_theApp->m_showHP)
	{
		std::vector<Vertex_PCU> verts_red;
		Vec2 hpBarRedMins = m_position + Vec2(-0.4f, 0.4f);
		Vec2 hpBarRedMaxs = m_position + Vec2(0.4f, 0.5f);
		AABB2 hpBarRed = AABB2(hpBarRedMins, hpBarRedMaxs);
		AddVertsForAABB2D(verts_red, hpBarRed, Rgba8::RED);
		g_theRenderer->DrawVertexArray((int)verts_red.size(), verts_red.data());
		g_theRenderer->BindTexture(nullptr);

		std::vector<Vertex_PCU> verts_green;
		Vec2 hpBarGreenMaxs = Vec2(hpBarRedMins.x + ((hpBarRedMaxs.x - hpBarRedMins.x) * m_health / m_maxHealth), hpBarRedMaxs.y);
		AABB2 hpBarGreen = AABB2(hpBarRedMins, hpBarGreenMaxs);
		AddVertsForAABB2D(verts_green, hpBarGreen, Rgba8::GREEN);
		g_theRenderer->DrawVertexArray((int)verts_green.size(), verts_green.data());
		g_theRenderer->BindTexture(nullptr);
	}
}

void Scorpio::DebugRender() const
{
	if (m_knowPlayerBefore && !m_map->CheckPlayerDead())
	{
		std::vector<Vertex_PCU> verts;
		AddVertsForDisc2D(verts, m_playerPrePos, 0.03f, Rgba8(0, 0, 0, 255));
		g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
		g_theRenderer->BindTexture(nullptr);

		DebugDrawLine(m_position, m_playerPrePos, Rgba8(0, 0, 0, 255), 0.01f);
	}

	float thickness = 0.04f;
	Vec2 playerStartPos = m_position;
	Vec2 playerEndPosForward = Vec2(
		m_position.x + m_cosmeticRadius * CosDegrees(m_orientationDegreesTurret),
		m_position.y + m_cosmeticRadius * SinDegrees(m_orientationDegreesTurret)
	);
	Vec2 playerEndPosLeft = Vec2(
		m_position.x + m_cosmeticRadius * CosDegrees(m_orientationDegreesTurret + 90.f),
		m_position.y + m_cosmeticRadius * SinDegrees(m_orientationDegreesTurret + 90.f)
	);

	DebugDrawLine(playerStartPos, playerEndPosForward, Rgba8(255, 0, 0, 255), thickness);
	DebugDrawLine(playerStartPos, playerEndPosLeft, Rgba8(0, 255, 0, 255), thickness);
	DebugDrawRing(playerStartPos, m_cosmeticRadius, thickness, Rgba8(255, 0, 255, 255));
	DebugDrawRing(playerStartPos, m_physicsRadius, thickness, Rgba8(0, 255, 255, 255));
}

void Scorpio::RenderHeatMap()
{
	std::vector<Vertex_PCU> heatVerts;
	heatVerts.reserve(3 * 2 * m_map->m_dimensions.x * m_map->m_dimensions.y);
	m_heatMap.AddVertsForDebugDraw(heatVerts, AABB2(0.0, 0.0, (float)m_map->m_dimensions.x, (float)m_map->m_dimensions.y), FloatRange(0.f, float(m_map->m_dimensions.x + m_map->m_dimensions.y)), Rgba8(0, 0, 0, 255), Rgba8(255, 255, 255, 255), 999, Rgba8(0, 0, 255, 255));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)heatVerts.size(), heatVerts.data());
}

void Scorpio::InitializeLocalVertsForTank()
{
	m_localVertsTank[0].m_position = Vec3(0.5f, 0.5f, 0.0f);
	m_localVertsTank[1].m_position = Vec3(0.5f, -0.5f, 0.0f);
	m_localVertsTank[2].m_position = Vec3(-0.5f, -0.5f, 0.0f);

	m_localVertsTank[3].m_position = Vec3(0.5f, 0.5f, 0.0f);
	m_localVertsTank[4].m_position = Vec3(-0.5f, 0.5f, 0.0f);
	m_localVertsTank[5].m_position = Vec3(-0.5f, -0.5f, 0.0f);

	m_localVertsTank[0].m_uvTexCoords = Vec2(1.f, 1.f);
	m_localVertsTank[1].m_uvTexCoords = Vec2(1.f, 0.f);
	m_localVertsTank[2].m_uvTexCoords = Vec2(0.f, 0.f);

	m_localVertsTank[3].m_uvTexCoords = Vec2(1.f, 1.f);
	m_localVertsTank[4].m_uvTexCoords = Vec2(0.f, 1.f);
	m_localVertsTank[5].m_uvTexCoords = Vec2(0.f, 0.f);

	for (int vertIndex = 0; vertIndex < NUM_SCORPIO_VERTS; ++vertIndex)
	{
		m_localVertsTank[vertIndex].m_color = m_tankcolor;
	}
}

void Scorpio::InitializeLocalVertsForTurret()
{
	m_localVertsTurret[0].m_position = Vec3(0.5f, 0.5f, 0.0f);
	m_localVertsTurret[1].m_position = Vec3(0.5f, -0.5f, 0.0f);
	m_localVertsTurret[2].m_position = Vec3(-0.5f, -0.5f, 0.0f);

	m_localVertsTurret[3].m_position = Vec3(0.5f, 0.5f, 0.0f);
	m_localVertsTurret[4].m_position = Vec3(-0.5f, 0.5f, 0.0f);
	m_localVertsTurret[5].m_position = Vec3(-0.5f, -0.5f, 0.0f);

	m_localVertsTurret[0].m_uvTexCoords = Vec2(1.f, 1.f);
	m_localVertsTurret[1].m_uvTexCoords = Vec2(1.f, 0.f);
	m_localVertsTurret[2].m_uvTexCoords = Vec2(0.f, 0.f);

	m_localVertsTurret[3].m_uvTexCoords = Vec2(1.f, 1.f);
	m_localVertsTurret[4].m_uvTexCoords = Vec2(0.f, 1.f);
	m_localVertsTurret[5].m_uvTexCoords = Vec2(0.f, 0.f);

	for (int vertIndex = 0; vertIndex < NUM_SCORPIO_VERTS; ++vertIndex)
	{
		m_localVertsTurret[vertIndex].m_color = m_turretcolor;
	}
}

bool Scorpio::CheckPlayerInSight()
{
	float enemyVisibleRange = g_gameConfigBlackboard.GetValue("enemyVisibleRange", 10.f);
	if (m_map->CheckPlayerDead())
	{
		return false;
	}
	Vec2 playerPos = m_map->GetPlayerPos();
	Vec2 directionToPlayer = playerPos - m_position;
	if (directionToPlayer.GetLength() <= enemyVisibleRange)
	{
		Vec2 tempPosForEnemy = m_position;
		Vec2 raycastPoint = 0.1f * directionToPlayer.GetNormalized();
		while (GetDistance2D(m_position, tempPosForEnemy) <= GetDistance2D(m_position, playerPos))
		{
			tempPosForEnemy += raycastPoint;
			IntVec2 tileCoords = m_map->GetTileCoordsForWorldPos(tempPosForEnemy);
			int tileIndex = m_map->GetTileIndexForTileCoords(tileCoords.x, tileCoords.y);
			if (m_map->m_tiles[tileIndex].IsSolid())
			{
				m_knowPlayerBefore = false;
				return false;
			}
		}
		m_Degrees = directionToPlayer.GetOrientationDegrees();
		m_playerPrePos = playerPos;
		m_knowPlayerBefore = true;
		return true;
	}
	else
	{
		m_knowPlayerBefore = false;
		return false;
	}
}

void Scorpio::RayCastOnTile()
{
	Vec2 endRayCast = m_position + m_position.MakeFromPolarDegrees(m_orientationDegreesTurret, 10.f);
	Vec2 directionToPlayer = endRayCast - m_position;
	Vec2 tempPosForEnemy = m_position;
	Vec2 raycastPoint = 0.01f * directionToPlayer.GetNormalized();
	IntVec2 tileCoords = m_map->GetTileCoordsForWorldPos(tempPosForEnemy);
	int tileIndex = m_map->GetTileIndexForTileCoords(tileCoords.x, tileCoords.y);
	while (!m_map->m_tiles[tileIndex].IsSolid())
	{
		tempPosForEnemy += raycastPoint;
		if (IsOffScreen(tempPosForEnemy))
		{
			tempPosForEnemy -= raycastPoint;
			tileCoords = m_map->GetTileCoordsForWorldPos(tempPosForEnemy);
			tileIndex = m_map->GetTileIndexForTileCoords(tileCoords.x, tileCoords.y);
			break;
		}
		else
		{
			tileCoords = m_map->GetTileCoordsForWorldPos(tempPosForEnemy);
			tileIndex = m_map->GetTileIndexForTileCoords(tileCoords.x, tileCoords.y);
		}
	}
	m_raycastLength = GetDistance2D(tempPosForEnemy, m_position);
	m_raycastLength = Clamp(m_raycastLength, 0.f, 10.f);
}

bool Scorpio::IsOffScreen(Vec2 position)
{
	if (position.x >= (float)m_map->m_dimensions.x) { return true; }
	if (position.y >= (float)m_map->m_dimensions.y) { return true; }
	if (position.x <= 0.f) { return true; }
	if (position.y <= 0.f) { return true; }
	return false;
}
