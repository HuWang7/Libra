#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

constexpr int NUM_TRIANGLE_VERTS = 3;

void DebugDrawLine(Vec2 start, Vec2 end, Rgba8 color, float thickness);
void DebugDrawRing(Vec2 const& center, float radius, float thinkness, Rgba8 const& color);