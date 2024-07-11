#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Texture.hpp"


//---------------------------------------------------------------------------------------
constexpr int NUM_ARIES_TRIS = 2;
constexpr int NUM_ARIES_VERTS = 3 * NUM_ARIES_TRIS;

//---------------------------------------------------------------------------------------
class Aries : public Entity
{
public:

	Aries(EntityType type, EntityFaction faction, Map* owner, const Vec2& startPos, float orientationDegrees);
	~Aries();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	virtual void ReactToBulletHit(Bullet& bullet) override;
	virtual void RenderHeatMap();

private:
	void InitializeLocalVertsForTank();
	bool CheckPlayerInSight();
	void FindPathToTarget(Vec2 startPos);
	void FindPathToRandomTarget();

private:
	Vertex_PCU m_localVertsTank[NUM_ARIES_VERTS];
	Texture* m_tankTexture;
	float m_randomTime = 0.f;
	float m_Degrees;
	Vec2 m_playerPrePos;
	bool m_knowPlayerBefore = false;
	Vec2 targetPos = Vec2(0.f, 0.f);
};
