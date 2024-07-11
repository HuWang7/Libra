#include "Game/Explosions.hpp"
#include "Game/Map.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"

extern Renderer* g_theRenderer;
extern Game* m_theGame;
extern AudioSystem* g_theAudio;
extern RandomNumberGenerator* g_rng;
extern App* g_theApp;


Explosions::Explosions(EntityType type, EntityFaction faction, Map* owner, const Vec2& startPos, float orientationDegrees)
	: Entity(type, faction, owner, startPos, orientationDegrees)
{
	m_health = 1;
	m_doesPushEntities = false;
	m_isPushedEntities = false;
	float defaultBulletSpeed = g_gameConfigBlackboard.GetValue("explosionsMoveSpeed", 2.f);
	m_velocity = GetForwardNormal() * defaultBulletSpeed;
	m_physicsRadius = g_gameConfigBlackboard.GetValue("FIRE_PHYSICS_RADIUS", 0.2f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("FIRE_COSMETIC_RADIUS", 0.4f);
	Texture* Explosion_5x5 = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Explosion_5x5.png");
	m_fireSpriteSheet = new SpriteSheet(*Explosion_5x5, IntVec2(5, 5));
	m_rotationSpeed = g_rng->RollRandomFloatInRange(-500.f, 500.f); // Random rotation rate
	m_fireAnimation = new SpriteAnimDefinition(*m_fireSpriteSheet, 5, 20, 1.f, SpriteAnimPlaybackType::ONCE);

	InitializeLocalVertsForTank();
}

Explosions::~Explosions()
{

}

void Explosions::Update(float deltaSeconds)
{
	Vec2 prePosition = m_position;
	m_lifeTime += deltaSeconds;
	if (m_lifeTime >= 0.2f || IsOffscreen())
	{
		Die();
		return;
	}
	m_position += m_velocity * deltaSeconds;
	IntVec2 nextTileCoords = m_map->GetTileCoordsForWorldPos(m_position + Vec2(m_physicsRadius, m_physicsRadius));
	int tileIndex = m_map->GetTileIndexForTileCoords(nextTileCoords.x, nextTileCoords.y);
	Vec2 nearestPointOnTile = m_map->m_tiles[tileIndex].GetBounds().GetNearestPoint(m_position);
	float distanceFromCenterToNearestPoint = GetDistance2D(nearestPointOnTile, prePosition);
	if (distanceFromCenterToNearestPoint <= m_physicsRadius && m_map->m_tiles[tileIndex].IsSolid())
	{
		m_position = prePosition;
		Die();
		return;
	}
	m_orientationDegreesTank += deltaSeconds * m_rotationSpeed;
}

void Explosions::Render() const
{
	Vertex_PCU tempTankWorldVerts[NUM_BULLET_VERTS];
	for (int vertIndex = 0; vertIndex < NUM_BULLET_VERTS; ++vertIndex)
	{
		tempTankWorldVerts[vertIndex] = m_fireVerts[vertIndex];
	}
	TransfromVertexArrayXY3D(NUM_BULLET_VERTS, tempTankWorldVerts, m_scaleLocalToWorld, m_orientationDegreesTank, m_position);
	g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
	g_theRenderer->BindTexture(&m_fireDefinition->GetTexture());
	g_theRenderer->DrawVertexArray(NUM_BULLET_VERTS, tempTankWorldVerts);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
}

void Explosions::DebugRender() const
{
	DebugDrawRing(m_position, m_physicsRadius, 0.01f, Rgba8(0, 255, 255, 255));
	DebugDrawRing(m_position, m_cosmeticRadius, 0.01f, Rgba8(255, 0, 255, 255));
}

void Explosions::InitializeLocalVertsForTank()
{
	m_fireDefinition = &m_fireAnimation->GetSpriteDefAtTime(m_map->m_explosionsTime);
	Vec2 uvAtMins, uvAtMaxs;
	m_fireDefinition->GetUVs(uvAtMins, uvAtMaxs);
	AddVertsForAABB2D(m_fireVerts, AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f)), Rgba8::WHITE, uvAtMins, uvAtMaxs);

	if (m_map->m_explosionMode == 1) // Show explosion for entity dead
	{
		m_scaleLocalToWorld = 1.f;
	}
	else if (m_map->m_explosionMode == 2) // Show explosion for shoot bullets
	{
		m_scaleLocalToWorld = 0.2f;
	}
	else if (m_map->m_explosionMode == 3) // Show explosion for bullet dead
	{
		m_scaleLocalToWorld = 0.5f;
	}
}

void Explosions::TakeDamage(int damage)
{
	m_health -= damage;
	if (m_health <= 0)
	{
		Die();
	}
}

void Explosions::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}

bool Explosions::IsOffscreen() const
{
	if (m_position.x > (float)m_map->m_dimensions.x - m_cosmeticRadius) { return true; }
	if (m_position.y > (float)m_map->m_dimensions.y - m_cosmeticRadius) { return true; }
	if (m_position.x < 0.f + m_cosmeticRadius) { return true; }
	if (m_position.y < 0.f + m_cosmeticRadius) { return true; }
	return false;
}

