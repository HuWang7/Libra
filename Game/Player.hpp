#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Texture.hpp"


//---------------------------------------------------------------------------------------
constexpr int NUM_PLAYER_TRIS = 2;
constexpr int NUM_PLAYER_VERTS = 3 * NUM_PLAYER_TRIS;

//---------------------------------------------------------------------------------------
class Player : public Entity
{
public:

	Player(EntityType type, EntityFaction faction, Map* owner, const Vec2& StartPos, float orientationDegrees);
	~Player();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;

	float m_fireBulletTime = 0.f;

private:
	void InitializeLocalVertsForTank();
	void InitializeLocalVertsForTurret();
	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateFromController( float deltaSeconds );
	void UpdateTankFromKeyboard( float deltaSeconds, float degrees );
	void UpdateTurretFromKeyboard(float deltaSeconds, float degrees, float turnSpeed);
	

private:
	Vertex_PCU m_localVertsTank[NUM_PLAYER_VERTS];
	Vertex_PCU m_localVertsTurret[NUM_PLAYER_VERTS];
	float m_tempOrientation;
	Vec2 direction_tank;
	Vec2 direction_turret;
	Texture* m_tankTexture;
	Texture* m_turretTeture;
	float m_thrustFraction;
	float m_fireTime = 0;
};
