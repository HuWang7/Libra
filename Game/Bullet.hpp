#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"


//---------------------------------------------------------------------------------------
constexpr int NUM_BULLET_TRIS = 2;
constexpr int NUM_BULLET_VERTS = 3 * NUM_BULLET_TRIS;

//---------------------------------------------------------------------------------------
class Bullet : public Entity
{
public:

	Bullet(EntityType type, EntityFaction faction, Map* owner, const Vec2& startPos, float orientationDegrees);
	~Bullet();

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
	float m_physicsRadiusForBullet;
	float m_cosmeticRadiusForBullet;
	float m_physicsRadiusForFire;
	float m_cosmeticRadiusForFire;
	Vertex_PCU m_localVertsBullet[NUM_BULLET_VERTS];
	Texture* m_bulletTexture;
	Rgba8 m_bulletcolor;
	SpriteAnimDefinition* m_fireAnimation;
	SpriteSheet*	  m_fireSpriteSheet		= nullptr;
	const SpriteDefinition* m_fireDefinition;
	std::vector<Vertex_PCU> m_fireVerts;
	float m_lifeTime = 0.f;
	float m_rotationSpeed = 0.f;
};
