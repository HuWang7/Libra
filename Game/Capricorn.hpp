#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Texture.hpp"


//---------------------------------------------------------------------------------------
constexpr int NUM_CAPRICORN_TRIS = 2;
constexpr int NUM_CAPRICORN_VERTS = 3 * NUM_CAPRICORN_TRIS;

//---------------------------------------------------------------------------------------
class Capricorn : public Entity
{
public:

	Capricorn(EntityType type, EntityFaction faction, Map* owner, const Vec2& startPos, float orientationDegrees);
	~Capricorn();

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
	Vertex_PCU m_localVertsTank[NUM_CAPRICORN_VERTS];
	Texture* m_tankTexture;
	float m_randomTime = 0.f;
	float m_Degrees;
	Vec2 m_playerPrePos;
	bool m_knowPlayerBefore = false;
	float m_fireTime = 0.f;
	Vec2 targetPos = Vec2(0.f, 0.f);
};
