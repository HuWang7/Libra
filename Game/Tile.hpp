#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <string>

struct TileDefinition
{
	std::string		m_typeName					= "Grass";
	bool			m_isSolid					= false;
	AABB2			m_UVs						= AABB2::ZERO_TO_ONE;
	Rgba8			m_tint						= Rgba8::WHITE;
	int				m_terrainSpriteIndex		= -1;
	bool			m_isWater					= false;
	bool			m_destructible				= false;
	std::string		m_alternateTile				= "Dirt";
	int				m_health					= 9999999;


public:
	static void InitializeTileDefs();
	static TileDefinition* GetTileDefinition(std::string typeName);
	static std::vector<TileDefinition> s_tileDefs;

protected:
	static void DefineTileType(int index, std::string typeName, bool isSolid, int terrainSpriteIndex, Rgba8 const& tint, bool isWater, bool destructible, int health, std::string alternateTile);
};

class Tile
{
public:
	Tile();
	~Tile();
	void SetType(TileDefinition* type) const;
	AABB2 GetBounds() const;
	Rgba8 GetColor() const;
	int GetTerrainSpriteIndex() const;
	std::string GetAlternateTileName() const;
	bool IsSolid() const;
	bool IsWater() const;
	bool IsDestructible() const;
	TileDefinition GettileDef() const;

public:
	IntVec2 m_tileCoords = IntVec2(-1, -1);
	mutable TileDefinition* m_tileType = nullptr;
	mutable int				m_health = 9999999;
};