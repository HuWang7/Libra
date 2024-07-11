#include "Game/Entity.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Bullet.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"

extern Game* m_theGame;
extern AudioSystem* g_theAudio;

Entity::Entity(EntityType type, EntityFaction faction, Map* owner, const Vec2& StartPos, float orientationDegrees)
	:m_entityType(type)
	,m_faction(faction)
	,m_map(owner)
	,m_position(StartPos)
	,m_orientationDegreesTank(orientationDegrees)
{
	m_heatMap = TileHeatMap(IntVec2(m_map->m_dimensions.x, m_map->m_dimensions.y));
}

Entity::~Entity()
{

}

void Entity::DebugRender() const
{

}

void Entity::TakeDamage(int damage)
{
	if (m_isHitByBullets)
	{
		m_health -= damage;
	}
	if (m_health <= 0)
	{
		g_theAudio->StartSound(m_theGame->m_DieAudio);
	}
}

Vec2 Entity::GetForwardNormal() const
{
	return Vec2( CosDegrees(m_orientationDegreesTank), SinDegrees(m_orientationDegreesTank));
}

EntityFaction Entity::GetFaction() const
{
	return m_faction;
}

void Entity::SetFaction(EntityFaction faction)
{
	m_faction = faction;
}

EntityType Entity::GetEntityType() const
{
	return m_entityType;
}

void Entity::SetEntityType(EntityType entityType)
{
	m_entityType = entityType;
}

void Entity::ReactToBulletHit(Bullet& bullet)
{
	if (m_entityType == ENTITY_TYPE_GOOD_PLAYER)
	{
		g_theAudio->StartSound(m_theGame->m_playerHitAudio);
	}
	else 
	{
		g_theAudio->StartSound(m_theGame->m_NPCHitAudio);
	}
	TakeDamage(bullet.m_damage);
	bullet.Die();
}

void Entity::ReactToBulletHit(Missile& bullet)
{
	if (m_entityType == ENTITY_TYPE_GOOD_PLAYER)
	{
		g_theAudio->StartSound(m_theGame->m_playerHitAudio);
	}
	else
	{
		g_theAudio->StartSound(m_theGame->m_NPCHitAudio);
	}
	TakeDamage(bullet.m_damage);
	bullet.Die();
}

void Entity::RenderHeatMap()
{

}

