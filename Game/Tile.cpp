#include "Game/Tile.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/XmlUtils.hpp"


extern Game* m_theGame;

std::vector<TileDefinition> TileDefinition::s_tileDefs;

void TileDefinition::InitializeTileDefs()
{
	s_tileDefs.resize(22);
	XmlDocument xmlDocument;
	XmlResult loadResult = xmlDocument.LoadFile("Data/Definitions/TileDefinitions.xml");
	if (loadResult == tinyxml2::XML_SUCCESS)
	{
		XmlElement* root = xmlDocument.RootElement();
		XmlElement* child = root->FirstChildElement();
		int index = 0;
		while (child != nullptr)
		{
			std::string		typeName			= ParseXmlAttribute(*child, "name", "Grass");
			bool			isSoild				= ParseXmlAttribute(*child, "isSolid", false);
			Vec2			texturePos			= ParseXmlAttribute(*child, "spriteCoords", Vec2(0.f, 0.f));
			int				terrainSpriteIndex	= (int)(texturePos.x + (texturePos.y * 8));
			Rgba8			tint				= ParseXmlAttribute(*child, "tint", Rgba8(255, 255, 255, 255));
			bool			isWater				= ParseXmlAttribute(*child, "isWater", false);
			bool			destructible		= ParseXmlAttribute(*child, "destructible", false); 
			int				health				= ParseXmlAttribute(*child, "health", 9999999);
			std::string		alternateTile		= ParseXmlAttribute(*child, "TileDefinition", "Dirt");

			DefineTileType(index, typeName, isSoild, terrainSpriteIndex, tint, isWater, destructible, health, alternateTile);
			child = child->NextSiblingElement();
			index += 1;
		}
	}

	for (int i = 0; i < s_tileDefs.size(); ++i)
	{
		if (s_tileDefs[i].m_UVs == AABB2::ZERO_TO_ONE)
		{
			ERROR_RECOVERABLE(Stringf("TileDef for Type #%i was not initialized", i));
		}
	}

}


TileDefinition* TileDefinition::GetTileDefinition(std::string typeName)
{
	for (int i = 0; i < (int)s_tileDefs.size(); ++i)
	{
		if (s_tileDefs[i].m_typeName == typeName)
		{
			return &s_tileDefs[i];
		}
	}
	return nullptr;
}

void TileDefinition::DefineTileType(int index, std::string typeName, bool isSolid, int terrainSpriteIndex, Rgba8 const& tint, bool isWater, bool destructible, int health, std::string alternateTile)
{
	GUARANTEE_OR_DIE(index >= 0 || index < (int)s_tileDefs.size(), Stringf("illegal tile type"));

	TileDefinition& tileDef = s_tileDefs[index];
	tileDef.m_typeName = typeName;
	tileDef.m_isSolid = isSolid;
	tileDef.m_UVs = m_theGame->m_spriteSheet->GetSpriteUVs(terrainSpriteIndex);
	tileDef.m_tint = tint;
	tileDef.m_terrainSpriteIndex = terrainSpriteIndex;
	tileDef.m_isWater = isWater;
	tileDef.m_destructible = destructible;
	tileDef.m_health = health;
	tileDef.m_alternateTile = alternateTile;
	s_tileDefs[index] = tileDef;
}

Tile::Tile()
{
}

Tile::~Tile()
{
}

void Tile::SetType(TileDefinition* type) const
{
	m_tileType = type;
	m_health = m_tileType->m_health;
}

AABB2 Tile::GetBounds() const
{
	return AABB2((float)m_tileCoords.x, (float)m_tileCoords.y, (float)m_tileCoords.x + 1, (float)m_tileCoords.y + 1);
}

Rgba8 Tile::GetColor() const
{
	return m_tileType->m_tint;
}

int Tile::GetTerrainSpriteIndex() const
{
	return m_tileType->m_terrainSpriteIndex;
}

std::string Tile::GetAlternateTileName() const
{
	return m_tileType->m_alternateTile;
}

bool Tile::IsSolid() const
{
	return m_tileType->m_isSolid;
}

bool Tile::IsWater() const
{
	return m_tileType->m_isWater;
}

bool Tile::IsDestructible() const
{
	return m_tileType->m_destructible;
}

TileDefinition Tile::GettileDef() const
{
	return *m_tileType;
}

