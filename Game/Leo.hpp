#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Texture.hpp"


//---------------------------------------------------------------------------------------
constexpr int NUM_LEO_TRIS = 2;
constexpr int NUM_LEO_VERTS = 3 * NUM_LEO_TRIS;

//---------------------------------------------------------------------------------------
class Leo : public Entity
{
public:

	Leo(EntityType type, EntityFaction faction, Map* owner, const Vec2& startPos, float orientationDegrees);
	~Leo();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	virtual void RenderHeatMap();

private:
	void InitializeLocalVertsForTank();
	bool CheckPlayerInSight();
	void FindPathToTarget(Vec2 startPos);
	void FindPathToRandomTarget();

private:
	Vertex_PCU m_localVertsTank[NUM_LEO_VERTS];
	Texture* m_tankTexture;
	float m_randomTime = 0.f;
	float m_Degrees;
	Vec2 m_playerPrePos;
	bool m_knowPlayerBefore = false;
	float m_fireTime = 0.f;
	Vec2 targetPos = Vec2(0.f, 0.f);
};
