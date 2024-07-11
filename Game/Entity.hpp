#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/HeatMaps.hpp"

enum EntityType
{
	ENTITY_TYPE_UNKNOWN = -1,
	ENTITY_TYPE_EVIL_BULLET,
	ENTITY_TYPE_GOOD_BULLET,
	ENTITY_TYPE_EVIL_MISSILE,
	ENTITY_TYPE_GOOD_MISSILE,
	ENTITY_TYPE_EVIL_FIRES,
	ENTITY_TYPE_GOOD_FIRES,
	ENTITY_TYPE_GOOD_PLAYER,
	ENTITY_TYPE_EVIL_SCORPIO,
	ENTITY_TYPE_EVIL_LEO,
	ENTITY_TYPE_EVIL_ARIES,
	ENTITY_TYPE_EVIL_CAPRICORN,
	ENTITY_TYPE_NETURAL_EXPLOSION,
	NUM_ENTITY_TYPES
};

enum EntityFaction 
{
	FACTION_GOOD, 
	FACTION_NEUTRAL, 
	FACTION_EVIL,
	NUM_FACTIONS
};

class Map; // Forward declaration
class Bullet;
class Missile;

class Entity {
public:
	Entity(EntityType type, EntityFaction faction, Map* owner, const Vec2& StartPos, float orientationDegrees); 
	virtual ~Entity(); 

	// Base Virtual Functions for actions.
	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;
	virtual void DebugRender() const;
	virtual void TakeDamage(int damage);
	virtual void ReactToBulletHit(Bullet& bullet);
	virtual void ReactToBulletHit(Missile& bullet);
	virtual void RenderHeatMap();


	Vec2 GetForwardNormal() const;
	EntityFaction GetFaction() const;
	void SetFaction(EntityFaction faction);
	EntityType GetEntityType() const;
	void SetEntityType(EntityType entityType);
	
public:
	Map*			m_map						= nullptr;
	Vec2			m_position;
	Vec2			m_velocity;
	float			m_orientationDegreesTank	= 0.f;
	float			m_orientationDegreesTurret	= 0.f;
	float			m_physicsRadius				= 5.f;
	float			m_cosmeticRadius			= 10.f;
	int				m_health					= 1;
	int				m_maxHealth;
	bool			m_isDead					= false;
	bool			m_isGarbage					= false;
	Rgba8           m_tankcolor;
	Rgba8			m_turretcolor;
	EntityFaction	m_faction					= NUM_FACTIONS;
	EntityType      m_entityType				= NUM_ENTITY_TYPES;
	bool			m_doesPushEntities;
	bool			m_isPushedEntities;
	bool			m_isHitByBullets = true;
	bool			m_canSwim = false;
	bool			m_needExplosions = true;
	mutable TileHeatMap		m_heatMap = TileHeatMap(IntVec2(0, 0));
	bool m_playFindPlayerAudio = true;
};

