#include "Game/Leo.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Map.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"


extern Renderer* g_theRenderer;
extern RandomNumberGenerator* g_rng;
extern Game* m_theGame;
extern AudioSystem* g_theAudio;
extern App* g_theApp;

Leo::Leo(EntityType type, EntityFaction faction, Map* owner, const Vec2& startPos, float orientationDegrees)
	: Entity(type, faction, owner, startPos, orientationDegrees)
{
	m_health = 3;
	m_maxHealth = m_health;
	m_doesPushEntities = true;
	m_isPushedEntities = true;
	m_tankTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTank4.png");
	m_tankcolor = Rgba8(255, 255, 255, 255);
	m_physicsRadius = g_gameConfigBlackboard.GetValue("ENTITY_PHYSICS_RADIUS", 0.3f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("ENTITY_COSMETIC_RADIUS", 0.6f);
	InitializeLocalVertsForTank();
}

Leo::~Leo()
{
	
}

void Leo::Update(float deltaSeconds)
{
	float leoDriveSpeed = g_gameConfigBlackboard.GetValue("leoDriveSpeed", 0.5f);
	float leoTurnRate = g_gameConfigBlackboard.GetValue("leoTurnRate", 90.f);
	float leoWanderRandomizePeriod = g_gameConfigBlackboard.GetValue("leoWanderRandomizePeriod", 1.0f);
	float leoShootCooldownSeconds = g_gameConfigBlackboard.GetValue("leoShootCooldownSeconds", 1.3f);
	if (m_health <= 0)
	{
		m_theGame->m_currentMap->EntityDiedExplosions(deltaSeconds, m_position);
		m_isDead = true;
		m_isGarbage = true;
	}
	m_velocity = Vec2(0.f, 0.f);
	m_randomTime += deltaSeconds;
	if (!CheckPlayerInSight())
	{
		m_playFindPlayerAudio = true;
		if (m_map->CheckPlayerDead())
		{
			if (m_randomTime >= leoWanderRandomizePeriod)
			{
				FindPathToRandomTarget();
				m_randomTime = 0.f;
			}
		}
		else
		{
			if (m_knowPlayerBefore)
			{
				targetPos = m_playerPrePos;
				FindPathToRandomTarget();
				if (GetDistance2D(m_playerPrePos, m_position) < m_physicsRadius)
				{
					m_knowPlayerBefore = false;
				}
			}
			else if (!m_knowPlayerBefore && m_randomTime >= leoWanderRandomizePeriod)
			{
				FindPathToRandomTarget();
				m_randomTime = 0.f;
			}
		}
	}
	else
	{
		if (m_playFindPlayerAudio && m_theGame->m_isPlayFoundedAudio > 0.1f)
		{
			g_theAudio->StartSound(m_theGame->m_foundPlayer);
			m_playFindPlayerAudio = false;
			m_theGame->m_isPlayFoundedAudio = 0.f;
		}
		float angleBetweenPlrAndEnm = GetAngleDegreesBetweenVectors2D(m_map->GetPlayerPos() - m_position, m_position.MakeFromPolarDegrees(m_orientationDegreesTank, 1.f));
		if (angleBetweenPlrAndEnm < 5.f)
		{
			m_fireTime += deltaSeconds;
			if (m_fireTime >= leoShootCooldownSeconds)
			{
				g_theAudio->StartSound(m_theGame->m_NPCShootAudio);
				m_fireTime = 0.f;
				Vec2 gunpoint = m_position + Vec2::MakeFromPolarDegrees(m_orientationDegreesTank, 0.4f);
				m_theGame->m_currentMap->SpawnNewEntity(ENTITY_TYPE_EVIL_BULLET, gunpoint, m_orientationDegreesTank);
				m_theGame->m_currentMap->EntityFireExplosions(deltaSeconds, gunpoint, m_orientationDegreesTank);
			}
		}
	}
	m_orientationDegreesTank = GetTurnedTowardDegrees(m_orientationDegreesTank, m_Degrees, leoTurnRate * deltaSeconds);
	Vec2 forwardDirection = GetForwardNormal();
	m_velocity += forwardDirection * leoDriveSpeed * deltaSeconds;
	m_position += m_velocity;
}

void Leo::Render() const
{
	Vertex_PCU tempTankWorldVerts[NUM_LEO_VERTS];
	for (int vertIndex = 0; vertIndex < NUM_LEO_VERTS; ++vertIndex)
	{
		tempTankWorldVerts[vertIndex] = m_localVertsTank[vertIndex];
	}

	TransfromVertexArrayXY3D(NUM_LEO_VERTS, tempTankWorldVerts, 1.f, m_orientationDegreesTank, m_position);
	g_theRenderer->BindTexture(m_tankTexture);
	g_theRenderer->DrawVertexArray(NUM_LEO_VERTS, tempTankWorldVerts);
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

void Leo::DebugRender() const
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
		m_position.x + m_cosmeticRadius * CosDegrees(m_orientationDegreesTank),
		m_position.y + m_cosmeticRadius * SinDegrees(m_orientationDegreesTank)
	);
	Vec2 playerEndPosLeft = Vec2(
		m_position.x + m_cosmeticRadius * CosDegrees(m_orientationDegreesTank + 90.f),
		m_position.y + m_cosmeticRadius * SinDegrees(m_orientationDegreesTank + 90.f)
	);

	Vec2 playerEndPosVelocity = m_position + m_velocity * 50.f;
	DebugDrawLine(playerStartPos, playerEndPosForward, Rgba8(255, 0, 0, 255), thickness);
	DebugDrawLine(playerStartPos, playerEndPosLeft, Rgba8(0, 255, 0, 255), thickness);
	DebugDrawRing(playerStartPos, m_cosmeticRadius, thickness, Rgba8(255, 0, 255, 255));
	DebugDrawRing(playerStartPos, m_physicsRadius, thickness, Rgba8(0, 255, 255, 255));
	DebugDrawLine(playerStartPos, playerEndPosVelocity, Rgba8(255, 255, 0, 255), thickness - 0.02f);
	
}

void Leo::RenderHeatMap()
{
	std::vector<Vertex_PCU> heatVerts;
	heatVerts.reserve(3 * 2 * m_map->m_dimensions.x * m_map->m_dimensions.y);
	m_heatMap.AddVertsForDebugDraw(heatVerts, AABB2(0.0, 0.0, (float)m_map->m_dimensions.x, (float)m_map->m_dimensions.y), FloatRange(0.f, float(m_map->m_dimensions.x + m_map->m_dimensions.y)), Rgba8(0, 0, 0, 255), Rgba8(255, 255, 255, 255), 999, Rgba8(0, 0, 255, 255));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)heatVerts.size(), heatVerts.data());


	std::vector<Vertex_PCU> verts5;
	AddVertsForArrow2D(verts5, Vec2(200.f, 100.f), m_position + m_physicsRadius * Vec2(1.f, 1.f), 0.3f, 0.05f, Rgba8(255, 0, 255, 255));
	g_theRenderer->DrawVertexArray((int)verts5.size(), verts5.data());
	g_theRenderer->BindTexture(nullptr);
}

void Leo::InitializeLocalVertsForTank()
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

	for (int vertIndex = 0; vertIndex < NUM_LEO_VERTS; ++vertIndex)
	{
		m_localVertsTank[vertIndex].m_color = m_tankcolor;
	}
}

bool Leo::CheckPlayerInSight()
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
				return false;
			}
		}
		FindPathToTarget(m_map->GetPlayerPos());
		m_playerPrePos = playerPos;
		m_knowPlayerBefore = true;
		return true;
	}
	else
	{
		return false;
	}
}

void Leo::FindPathToTarget(Vec2 startPos)
{
	m_map->EnemyPopulateDistanceField(m_heatMap, IntVec2((int)startPos.x, (int)startPos.y), 999.f, true);
	IntVec2 HeatMapCoords = m_map->GetTileCoordsForWorldPos(m_position);
	IntVec2 northCoords = IntVec2(HeatMapCoords.x, HeatMapCoords.y + 1);
	IntVec2 southCoords = IntVec2(HeatMapCoords.x, HeatMapCoords.y - 1);
	IntVec2 eastCoords = IntVec2(HeatMapCoords.x + 1, HeatMapCoords.y);
	IntVec2 westCoords = IntVec2(HeatMapCoords.x - 1, HeatMapCoords.y);

	float currentHeatValue = m_heatMap.GetValue(HeatMapCoords);
	if (startPos.x > m_position.x && startPos.y > m_position.y)
	{
		if (m_heatMap.GetValue(northCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)northCoords.x + 0.5f, (float)northCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
		else if (m_heatMap.GetValue(eastCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)eastCoords.x + 0.5f, (float)eastCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
		else if (m_heatMap.GetValue(westCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)westCoords.x + 0.5f, (float)westCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
		else if (m_heatMap.GetValue(northCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)northCoords.x + 0.5f, (float)northCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
	}
	else if (startPos.x < m_position.x && startPos.y > m_position.y)
	{
		if (m_heatMap.GetValue(northCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)northCoords.x + 0.5f, (float)northCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
		else if (m_heatMap.GetValue(westCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)westCoords.x + 0.5f, (float)westCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
		else if (m_heatMap.GetValue(eastCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)eastCoords.x + 0.5f, (float)eastCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
		else if (m_heatMap.GetValue(southCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)southCoords.x + 0.5f, (float)southCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
	}
	else if (startPos.x > m_position.x && startPos.y < m_position.y)
	{
		if (m_heatMap.GetValue(southCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)southCoords.x + 0.5f, (float)southCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
		else if (m_heatMap.GetValue(eastCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)eastCoords.x + 0.5f, (float)eastCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
		else if (m_heatMap.GetValue(northCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)northCoords.x + 0.5f, (float)northCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
		else if (m_heatMap.GetValue(westCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)westCoords.x + 0.5f, (float)westCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
	}
	else if (startPos.x < m_position.x && startPos.y < m_position.y)
	{
		if (m_heatMap.GetValue(southCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)southCoords.x + 0.5f, (float)southCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
		else if (m_heatMap.GetValue(westCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)westCoords.x + 0.5f, (float)westCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
		else if (m_heatMap.GetValue(northCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)northCoords.x + 0.5f, (float)northCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
		else if (m_heatMap.GetValue(eastCoords) == currentHeatValue - 1)
		{
			Vec2 directionToNextTile = Vec2((float)eastCoords.x + 0.5f, (float)eastCoords.y + 0.5f) - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
	}
	IntVec2 playerHeatCoords = m_map->GetTileCoordsForWorldPos(m_map->m_player->m_position);
	if (playerHeatCoords == northCoords || playerHeatCoords == southCoords || playerHeatCoords == eastCoords || playerHeatCoords == westCoords)
	{
		Vec2 directionToNextTile = m_map->GetPlayerPos() - m_position;
		m_Degrees = directionToNextTile.GetOrientationDegrees();
	}
	if (m_knowPlayerBefore)
	{
		IntVec2 playerPreHeatCoords = m_map->GetTileCoordsForWorldPos(m_playerPrePos);
		if (playerPreHeatCoords == northCoords || playerPreHeatCoords == southCoords || playerPreHeatCoords == eastCoords || playerPreHeatCoords == westCoords)
		{
			Vec2 directionToNextTile = m_playerPrePos - m_position;
			m_Degrees = directionToNextTile.GetOrientationDegrees();
		}
	}

}

void Leo::FindPathToRandomTarget()
{
	if (IsPointInsideDisc2D(targetPos, m_position, m_physicsRadius) || targetPos == Vec2(0.f, 0.f))
	{
		bool loop = true;
		while (loop)
		{
			int x = g_rng->RollRandomIntInRange(1, m_map->m_dimensions.x - 2);
			int y = g_rng->RollRandomIntInRange(1, m_map->m_dimensions.y - 2);
			int tileIndex = m_map->GetTileIndexForTileCoords(x, y);
			if (!m_map->m_tiles[tileIndex].IsSolid() && !m_map->m_tiles[tileIndex].IsWater())
			{
				targetPos = Vec2((float)x + 0.5f, (float)y + 0.5f);
				if ((int)m_map->m_entityListsByType[ENTITY_TYPE_EVIL_SCORPIO].size() == 0)
				{
					loop = false;
				}
				else
				{
					bool needLoop = false;
					for (int i = 0; i < (int)m_map->m_entityListsByType[ENTITY_TYPE_EVIL_SCORPIO].size(); ++i)
					{
						Entity const* entity = m_map->m_entityListsByType[ENTITY_TYPE_EVIL_SCORPIO][i];
						if (entity && entity->m_position == targetPos)
						{
							needLoop = true;
							break;
						}
					}
					if (!needLoop)
					{
						loop = false;
						FindPathToTarget(targetPos);
					}
				}
			}
			IntVec2 currentPos = m_map->GetTileCoordsForWorldPos(m_position);
			if (m_heatMap.GetValue(currentPos) == 999)
			{
				loop = true;
			}
		}
	}
	FindPathToTarget(targetPos);
}



