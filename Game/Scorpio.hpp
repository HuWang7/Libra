#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Texture.hpp"


//---------------------------------------------------------------------------------------
constexpr int NUM_SCORPIO_TRIS = 2;
constexpr int NUM_SCORPIO_VERTS = 3 * NUM_SCORPIO_TRIS;

//---------------------------------------------------------------------------------------
class Scorpio : public Entity
{
public:

	Scorpio(EntityType type, EntityFaction faction, Map* owner, const Vec2& startPos, float orientationDegrees);
	~Scorpio();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	virtual void RenderHeatMap();

private:
	void InitializeLocalVertsForTank();
	void InitializeLocalVertsForTurret();
	bool CheckPlayerInSight();
	void RayCastOnTile();
	bool IsOffScreen(Vec2 position);

private:
	Vertex_PCU m_localVertsTank[NUM_SCORPIO_VERTS];
	Vertex_PCU m_localVertsTurret[NUM_SCORPIO_VERTS];
	Texture* m_tankTexture;
	Texture* m_turretTeture;
	float m_Degrees;
	Vec2 m_playerPrePos;
	bool m_knowPlayerBefore = false;
	float m_raycastLength;
	float m_fireTime = 0.f;
};
