#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"

extern Renderer* g_theRenderer;

void DebugDrawLine(Vec2 start, Vec2 end, Rgba8 color, float thickness)
{
	Vec2 forwardDisplacement = end - start;
	Vec2 forwardNormal = forwardDisplacement.GetNormalized();
	Vec2 forwardStep = forwardNormal * (thickness/2.f);
	Vec2 LeftStep = forwardStep.GetRotated90Degrees();
	Vec2 Vertex_A = end + forwardStep + LeftStep;
	Vec2 Vertex_B = start - forwardStep + LeftStep;
	Vec2 Vertex_C = start - forwardStep - LeftStep;
	Vec2 Vertex_D = end + forwardStep - LeftStep;

	Vertex_PCU vertexes[6] = {
		Vertex_PCU(Vec3(Vertex_A.x, Vertex_A.y, 0.f),color,Vec2(0.f, 0.f)),
		Vertex_PCU(Vec3(Vertex_B.x, Vertex_B.y, 0.f),color,Vec2(0.f, 0.f)),
		Vertex_PCU(Vec3(Vertex_C.x, Vertex_C.y, 0.f),color,Vec2(0.f, 0.f)),
		Vertex_PCU(Vec3(Vertex_A.x, Vertex_A.y, 0.f),color,Vec2(0.f, 0.f)),
		Vertex_PCU(Vec3(Vertex_C.x, Vertex_C.y, 0.f),color,Vec2(0.f, 0.f)),
		Vertex_PCU(Vec3(Vertex_D.x, Vertex_D.y, 0.f),color,Vec2(0.f, 0.f)),
	};

	g_theRenderer->DrawVertexArray(6, vertexes);
}

void DebugDrawRing(Vec2 const& center, float radius, float thinkness, Rgba8 const& color)
{
	float halfThickness = thinkness * 0.5f;
	float innerRadius = radius - halfThickness;
	float outerRadius = radius + halfThickness;
	constexpr int NUM_SIDES = 32;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	Vertex_PCU verts[ NUM_VERTS ];
	constexpr float DEGREES_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);

	for (int sideNum = 0; sideNum < NUM_SIDES; ++sideNum)
	{
		//Compute angle-related terms
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum + 1);
		float cosStart = CosDegrees(startDegrees);
		float sinStart = SinDegrees(startDegrees);
		float cosEnd = CosDegrees(endDegrees);
		float sinEnd =SinDegrees(endDegrees);

		//Compute inner & outer position
		Vec3 innerStartPos(center.x + innerRadius * cosStart, center.y + innerRadius * sinStart, 0.f);
		Vec3 outerStartPos(center.x + outerRadius * cosStart, center.y + outerRadius * sinStart, 0.f);
		Vec3 innerEndPos(center.x + innerRadius * cosEnd, center.y + innerRadius * sinEnd, 0.f);
		Vec3 outerEndPos(center.x + outerRadius * cosEnd, center.y + outerRadius * sinEnd, 0.f);

		int vertIndexA = (6 * sideNum) + 0;
		int vertIndexB = (6 * sideNum) + 1;
		int vertIndexC = (6 * sideNum) + 2;
		int vertIndexD = (6 * sideNum) + 3;
		int vertIndexE = (6 * sideNum) + 4;
		int vertIndexF = (6 * sideNum) + 5;

		verts[vertIndexA].m_position = innerEndPos;
		verts[vertIndexB].m_position = innerStartPos;
		verts[vertIndexC].m_position = outerStartPos;
		verts[vertIndexA].m_color = color;
		verts[vertIndexB].m_color = color;
		verts[vertIndexC].m_color = color;
		verts[vertIndexA].m_uvTexCoords = Vec2(0.f,0.f);
		verts[vertIndexB].m_uvTexCoords = Vec2(0.f,0.f);
		verts[vertIndexC].m_uvTexCoords = Vec2(0.f,0.f);


		verts[vertIndexD].m_position = innerEndPos;
		verts[vertIndexE].m_position = outerStartPos;
		verts[vertIndexF].m_position = outerEndPos;
		verts[vertIndexD].m_color = color;
		verts[vertIndexE].m_color = color;
		verts[vertIndexF].m_color = color;
		verts[vertIndexD].m_uvTexCoords = Vec2(0.f,0.f);
		verts[vertIndexE].m_uvTexCoords = Vec2(0.f,0.f);
		verts[vertIndexF].m_uvTexCoords = Vec2(0.f,0.f);
	}

	g_theRenderer->DrawVertexArray(NUM_VERTS, verts);
}