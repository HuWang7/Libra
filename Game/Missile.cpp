#include "Game/Missile.hpp"
#include "Game/Map.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Game.hpp"

extern Renderer* g_theRenderer;
extern Game* m_theGame;
extern AudioSystem* g_theAudio;
extern RandomNumberGenerator* g_rng;


Missile::Missile(EntityType type, EntityFaction faction, Map* owner, const Vec2& startPos, float orientationDegrees)
	: Entity(type, faction, owner, startPos, orientationDegrees)
{
	m_health = 1;
	m_doesPushEntities = false;
	m_isPushedEntities = false;
	m_bulletTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyBullet.png");
	m_bulletcolor = Rgba8(255, 255, 255, 255);
	m_physicsRadius = g_gameConfigBlackboard.GetValue("BULLET_PHYSICS_RADIUS", 0.1f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("BULLET_COSMETIC_RADIUS", 0.2f);
	InitializeLocalVertsForTank();
}

Missile::~Missile()
{

}

void Missile::Update(float deltaSeconds)
{
	m_seconds = deltaSeconds;
	float defaultMissileSpeed = g_gameConfigBlackboard.GetValue("defaultMissileSpeed", 3.f);
	float missileTurnRate = g_gameConfigBlackboard.GetValue("defaultMissileTurnRate", 180.f);
	Vec2 directionToPlayerShip = m_map->GetPlayerPos() - m_position;
	m_Degrees = directionToPlayerShip.GetOrientationDegrees();
	m_orientationDegreesTank = GetTurnedTowardDegrees(m_orientationDegreesTank, m_Degrees, missileTurnRate * deltaSeconds);
	m_velocity = GetForwardNormal() * defaultMissileSpeed;
	m_position += m_velocity * deltaSeconds;
	if (IsOffscreen())
	{
		Die();
	}
	else
	{
		IntVec2 nextTileCoords = m_map->GetTileCoordsForWorldPos(m_position);
		int tileIndex = m_map->GetTileIndexForTileCoords(nextTileCoords.x, nextTileCoords.y);
		if (m_map->m_tiles[tileIndex].IsSolid())
		{
			if (m_map->m_tiles[tileIndex].IsDestructible())
			{
				m_map->m_tiles[tileIndex].m_health -= 1;
				if (m_map->m_tiles[tileIndex].m_health <= 0)
				{
					m_map->m_tiles[tileIndex].SetType(TileDefinition::GetTileDefinition(m_map->m_tiles[tileIndex].GetAlternateTileName()));
					m_map->PopulateDistanceField(m_map->m_heatMap, IntVec2(1, 1), 999.f, true);
				}
			}
			g_theAudio->StartSound(m_theGame->m_bulletBounceAudio);
			TakeDamage(1);
		}
	}
}

void Missile::Render() const
{
	Vertex_PCU tempTankWorldVerts[NUM_MISSILE_VERTS];
	for (int vertIndex = 0; vertIndex < NUM_MISSILE_VERTS; ++vertIndex)
	{
		tempTankWorldVerts[vertIndex] = m_localVertsBullet[vertIndex];
	}

	TransfromVertexArrayXY3D(NUM_MISSILE_VERTS, tempTankWorldVerts, 1.f, m_orientationDegreesTank, m_position);
	g_theRenderer->BindTexture(m_bulletTexture);
	g_theRenderer->DrawVertexArray(NUM_MISSILE_VERTS, tempTankWorldVerts);
	g_theRenderer->BindTexture(nullptr);
}

void Missile::DebugRender() const
{

}

void Missile::InitializeLocalVertsForTank()
{
	m_localVertsBullet[0].m_position = Vec3(0.12f, 0.05f, 0.0f);
	m_localVertsBullet[1].m_position = Vec3(0.12f, -0.05f, 0.0f);
	m_localVertsBullet[2].m_position = Vec3(-0.12f, -0.05f, 0.0f);

	m_localVertsBullet[3].m_position = Vec3(0.12f, 0.05f, 0.0f);
	m_localVertsBullet[4].m_position = Vec3(-0.12f, 0.05f, 0.0f);
	m_localVertsBullet[5].m_position = Vec3(-0.12f, -0.05f, 0.0f);

	m_localVertsBullet[0].m_uvTexCoords = Vec2(1.f, 1.f);
	m_localVertsBullet[1].m_uvTexCoords = Vec2(1.f, 0.f);
	m_localVertsBullet[2].m_uvTexCoords = Vec2(0.f, 0.f);

	m_localVertsBullet[3].m_uvTexCoords = Vec2(1.f, 1.f);
	m_localVertsBullet[4].m_uvTexCoords = Vec2(0.f, 1.f);
	m_localVertsBullet[5].m_uvTexCoords = Vec2(0.f, 0.f);

	for (int vertIndex = 0; vertIndex < NUM_MISSILE_VERTS; ++vertIndex)
	{
		m_localVertsBullet[vertIndex].m_color = m_tankcolor;
	}
}

void Missile::TakeDamage(int damage)
{
	m_health -= damage;
	if (m_health <= 0)
	{
		Die();
	}
}

void Missile::Die()
{
	m_theGame->m_currentMap->BulletDiedExplosions(m_seconds, m_position);
	m_isDead = true;
	m_isGarbage = true;
}

bool Missile::IsOffscreen() const
{
	if (m_position.x > (float)m_map->m_dimensions.x - m_cosmeticRadius) { return true; }
	if (m_position.y > (float)m_map->m_dimensions.y - m_cosmeticRadius) { return true; }
	if (m_position.x < 0.f + m_cosmeticRadius) { return true; }
	if (m_position.y < 0.f + m_cosmeticRadius) { return true; }
	return false;
}

