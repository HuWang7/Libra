#include "Game/Bullet.hpp"
#include "Game/Map.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"

extern Renderer* g_theRenderer;
extern Game* m_theGame;
extern AudioSystem* g_theAudio;
extern RandomNumberGenerator* g_rng;
extern App* g_theApp;


Bullet::Bullet(EntityType type, EntityFaction faction, Map* owner, const Vec2& startPos, float orientationDegrees)
	: Entity(type, faction, owner, startPos, orientationDegrees)
{
	m_health = 3;
	m_doesPushEntities = false;
	m_isPushedEntities = false;
	float defaultBulletSpeed =g_gameConfigBlackboard.GetValue("defaultBulletSpeed", 6.f);
	m_velocity = GetForwardNormal() * defaultBulletSpeed;
	m_bulletcolor = Rgba8(255, 255, 255, 255);
	m_physicsRadiusForBullet = g_gameConfigBlackboard.GetValue("BULLET_PHYSICS_RADIUS", 0.1f);
	m_cosmeticRadiusForBullet = g_gameConfigBlackboard.GetValue("BULLET_COSMETIC_RADIUS", 0.2f);
	m_physicsRadiusForFire = g_gameConfigBlackboard.GetValue("FIRE_PHYSICS_RADIUS", 0.2f);
	m_cosmeticRadiusForFire = g_gameConfigBlackboard.GetValue("FIRE_COSMETIC_RADIUS", 0.4f);
	Texture* Explosion_5x5 = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Explosion_5x5.png");
	m_fireSpriteSheet = new SpriteSheet(*Explosion_5x5, IntVec2(5, 5));
	m_rotationSpeed = g_rng->RollRandomFloatInRange(-500.f, 500.f); // Random rotation rate

	switch (type)
	{
	case ENTITY_TYPE_EVIL_BULLET: 	m_bulletTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyBolt.png");
		m_physicsRadius = m_physicsRadiusForBullet;
		m_cosmeticRadius = m_cosmeticRadiusForBullet;
		break;
	case ENTITY_TYPE_GOOD_BULLET:	m_bulletTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/FriendlyBolt.png");
		m_physicsRadius = m_physicsRadiusForBullet;
		m_cosmeticRadius = m_cosmeticRadiusForBullet;
		break;
	case ENTITY_TYPE_EVIL_FIRES:
		m_fireAnimation = new SpriteAnimDefinition(*m_fireSpriteSheet, 0, 10, 1.f, SpriteAnimPlaybackType::ONCE);
		m_physicsRadius = m_physicsRadiusForFire;
		m_cosmeticRadius = m_cosmeticRadiusForFire;
		break;
	case ENTITY_TYPE_GOOD_FIRES:
		m_fireAnimation = new SpriteAnimDefinition(*m_fireSpriteSheet, 0, 10, 1.f, SpriteAnimPlaybackType::ONCE);
		m_physicsRadius = m_physicsRadiusForFire;
		m_cosmeticRadius = m_cosmeticRadiusForFire;
		break;
	default:
		break;
	}
	InitializeLocalVertsForTank();

}

Bullet::~Bullet()
{

}

void Bullet::Update(float deltaSeconds)
{
	m_seconds = deltaSeconds;
	if (m_entityType == ENTITY_TYPE_EVIL_BULLET || m_entityType == ENTITY_TYPE_GOOD_BULLET)
	{
		Vec2 prePosition = m_position;
		m_position += m_velocity * deltaSeconds;
		if (IsOffscreen())
		{
			Die();
		}
		else
		{
			IntVec2 preTileCoords = m_map->GetTileCoordsForWorldPos(prePosition);
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
				m_position = prePosition;
				Vec2 bounceNormal = (Vec2((float)preTileCoords.x, (float)preTileCoords.y) - Vec2((float)nextTileCoords.x, (float)nextTileCoords.y)).GetNormalized();
				BounceOff(bounceNormal);
			}
		}
	}
	else if (m_entityType == ENTITY_TYPE_EVIL_FIRES || m_entityType == ENTITY_TYPE_GOOD_FIRES)
	{
		Vec2 prePosition = m_position;
		m_lifeTime += deltaSeconds;
		if (m_lifeTime >= 0.5f || IsOffscreen())
		{
			Die();
		}
		if (g_theApp->WasKeyJustPressed('M'))
		{
			static_cast<Player*>(m_map->m_player)->m_fireBulletTime = 0.f;
		}
		m_position += m_velocity * deltaSeconds;
		IntVec2 nextTileCoords = m_map->GetTileCoordsForWorldPos(m_position+Vec2(m_physicsRadiusForFire, m_physicsRadiusForFire));
		int tileIndex = m_map->GetTileIndexForTileCoords(nextTileCoords.x, nextTileCoords.y);
		Vec2 nearestPointOnTile = m_map->m_tiles[tileIndex].GetBounds().GetNearestPoint(m_position);
		float distanceFromCenterToNearestPoint = GetDistance2D(nearestPointOnTile, prePosition);
		if (distanceFromCenterToNearestPoint <= m_physicsRadiusForFire && m_map->m_tiles[tileIndex].IsSolid())
		{
			m_position = prePosition;
			Die();
		}
		m_orientationDegreesTank += deltaSeconds * m_rotationSpeed;
	}
	
}

void Bullet::Render() const
{
	if (m_entityType == ENTITY_TYPE_EVIL_BULLET || m_entityType == ENTITY_TYPE_GOOD_BULLET)
	{
		Vertex_PCU tempTankWorldVerts[NUM_BULLET_VERTS];
		for (int vertIndex = 0; vertIndex < NUM_BULLET_VERTS; ++vertIndex)
		{
			tempTankWorldVerts[vertIndex] = m_localVertsBullet[vertIndex];
		}

		TransfromVertexArrayXY3D(NUM_BULLET_VERTS, tempTankWorldVerts, 1.f, m_orientationDegreesTank, m_position);
		g_theRenderer->BindTexture(m_bulletTexture);
		g_theRenderer->DrawVertexArray(NUM_BULLET_VERTS, tempTankWorldVerts);
		g_theRenderer->BindTexture(nullptr);
	}
	else if (m_entityType == ENTITY_TYPE_EVIL_FIRES || m_entityType == ENTITY_TYPE_GOOD_FIRES)
	{
		Vertex_PCU tempTankWorldVerts[NUM_BULLET_VERTS];
		for (int vertIndex = 0; vertIndex < NUM_BULLET_VERTS; ++vertIndex)
		{
			tempTankWorldVerts[vertIndex] = m_fireVerts[vertIndex];
		}

		TransfromVertexArrayXY3D(NUM_BULLET_VERTS, tempTankWorldVerts, 1.f, m_orientationDegreesTank, m_position);
		g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
		g_theRenderer->BindTexture(&m_fireDefinition->GetTexture());
		g_theRenderer->DrawVertexArray(NUM_BULLET_VERTS, tempTankWorldVerts);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	}
}

void Bullet::DebugRender() const
{
	if (m_entityType == ENTITY_TYPE_EVIL_BULLET || m_entityType == ENTITY_TYPE_GOOD_BULLET)
	{
		std::vector<Vertex_PCU> verts;
		AddVertsForDisc2D(verts, m_position, m_physicsRadiusForBullet, Rgba8::WHITE);
		g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
		g_theRenderer->BindTexture(nullptr);
	}
	else if (m_entityType == ENTITY_TYPE_EVIL_FIRES || m_entityType == ENTITY_TYPE_GOOD_FIRES)
	{
		DebugDrawRing(m_position, m_physicsRadiusForFire, 0.01f, Rgba8(0, 255, 255, 255));
		DebugDrawRing(m_position, m_cosmeticRadiusForFire, 0.01f, Rgba8(255, 0, 255, 255));
	}
}

void Bullet::InitializeLocalVertsForTank()
{
	if (m_entityType == ENTITY_TYPE_EVIL_BULLET || m_entityType == ENTITY_TYPE_GOOD_BULLET)
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

		for (int vertIndex = 0; vertIndex < NUM_BULLET_VERTS; ++vertIndex)
		{
			m_localVertsBullet[vertIndex].m_color = m_tankcolor;
		}
	}
	else if (m_entityType == ENTITY_TYPE_EVIL_FIRES || m_entityType == ENTITY_TYPE_GOOD_FIRES)
	{
		m_fireDefinition = &m_fireAnimation->GetSpriteDefAtTime(static_cast<Player*>(m_map->m_player)->m_fireBulletTime);
		Vec2 uvAtMins, uvAtMaxs;
		m_fireDefinition->GetUVs(uvAtMins, uvAtMaxs);
		AddVertsForAABB2D(m_fireVerts, AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f)), Rgba8::WHITE, uvAtMins, uvAtMaxs);
	}
}

void Bullet::BounceOff(Vec2 bounceNormal)
{
	float bulletBounceVarianceDegrees = g_gameConfigBlackboard.GetValue("bulletBounceVarianceDegrees", 10.f);
	TakeDamage(1);
	if (m_isDead)
	{
		return;
	}
	m_velocity.Reflect(bounceNormal);
	m_orientationDegreesTank = m_velocity.GetOrientationDegrees();
	m_orientationDegreesTank += g_rng->RollRandomFloatInRange(-bulletBounceVarianceDegrees, bulletBounceVarianceDegrees);
	m_velocity.SetOrientationDegrees(m_orientationDegreesTank);
}

void Bullet::TakeDamage(int damage)
{
	m_health -= damage;
	if (m_health <= 0)
	{
		Die();
	}
}

void Bullet::Die()
{
	if (m_entityType == ENTITY_TYPE_GOOD_BULLET || m_entityType == ENTITY_TYPE_EVIL_BULLET)
	{
		m_theGame->m_currentMap->BulletDiedExplosions(m_seconds, m_position);
	}
	m_isDead = true;
	m_isGarbage = true;
}

bool Bullet::IsOffscreen() const
{
	if (m_position.x > (float)m_map->m_dimensions.x - m_cosmeticRadius) { return true; }
	if (m_position.y > (float)m_map->m_dimensions.y - m_cosmeticRadius) { return true; }
	if (m_position.x < 0.f + m_cosmeticRadius) { return true; }
	if (m_position.y < 0.f + m_cosmeticRadius) { return true; }
	return false;
}

