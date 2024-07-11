#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Texture.hpp"


//---------------------------------------------------------------------------------------
constexpr int NUM_MISSILE_TRIS = 2;
constexpr int NUM_MISSILE_VERTS = 3 * NUM_MISSILE_TRIS;

//---------------------------------------------------------------------------------------
class Missile : public Entity
{
public:

	Missile(EntityType type, EntityFaction faction, Map* owner, const Vec2& startPos, float orientationDegrees);
	~Missile();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	virtual void TakeDamage(int damage) override;

	void Die();
	void BounceOff(Vec2 bounceNormal);

	int m_damage = 1;
	float m_seconds;

private:
	void InitializeLocalVertsForTank();
	bool IsOffscreen() const;

private:
	Vertex_PCU m_localVertsBullet[NUM_MISSILE_VERTS];
	Texture* m_bulletTexture;
	Rgba8 m_bulletcolor;
	float m_Degrees;
};
