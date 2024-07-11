#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"


//---------------------------------------------------------------------------------------
constexpr int NUM_EXPLOSIONS_TRIS = 2;
constexpr int NUM_EXPLOSIONS_VERTS = 3 * NUM_EXPLOSIONS_TRIS;

//---------------------------------------------------------------------------------------
class Explosions : public Entity
{
public:

	Explosions(EntityType type, EntityFaction faction, Map* owner, const Vec2& startPos, float orientationDegrees);
	~Explosions();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	virtual void TakeDamage(int damage) override;

	void Die();
	int m_damage = 1;

private:
	void InitializeLocalVertsForTank();
	bool IsOffscreen() const;

private:
	SpriteAnimDefinition* m_fireAnimation;
	SpriteSheet* m_fireSpriteSheet = nullptr;
	const SpriteDefinition* m_fireDefinition;
	std::vector<Vertex_PCU> m_fireVerts;
	float m_scaleLocalToWorld;
	float m_lifeTime = 0.f;
	float m_rotationSpeed = 0.f;
};
