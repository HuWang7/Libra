#include "Game/Map.hpp"
#include "Game/App.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/HeatMaps.hpp"

extern RandomNumberGenerator* g_rng;
extern Renderer* g_theRenderer;
extern App* g_theApp;
extern Game* m_theGame;
extern BitmapFont*	 g_theFont;

Map::Map(Game* gameInstance, MapDefinition const& mapDef)
	:m_mapDef(mapDef)
	,m_game(gameInstance)
	,m_dimensions(mapDef.m_dimensions)
{
	
	
}

Map::~Map()
{

}

void Map::Startup()
{
	g_theApp->m_worldCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(16.f, 8.f));
	g_theApp->m_consoleCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(16.f, 8.f));
	m_player = m_entityListsByType[ ENTITY_TYPE_GOOD_PLAYER ][0];
	PopulateTiles();
	PopulateHeatMap();
	PopulateEntities();
	m_heatMapForAmphibians = TileHeatMap(IntVec2(m_dimensions.x, m_dimensions.y));
	m_heatMapForLandBased = TileHeatMap(IntVec2(m_dimensions.x, m_dimensions.y));
}

void Map::Update(float deltaSeconds)
{
	if (g_theApp->m_isSlowMo == true)
	{
		deltaSeconds = deltaSeconds / 10.f;
	}
	else if (g_theApp->m_isPaused == true)
	{
		deltaSeconds = 0.f;
	}
	else if (g_theApp->m_isFastMo)
	{
		deltaSeconds = 4.f * deltaSeconds;
	}
	UpdateCamera();
	UpdateEntities(deltaSeconds);
	if (!m_player->m_isDead)
	{
		CheckForBulletHits();
	}
	if (!g_theApp->m_developerCheat)
	{
		PushEntitiesOutOfWalls();
		PushAllEntitiesOutOfEachOther();
	}
	DeleteGarbageEntities();
}

void Map::Render() const
{
	RenderTiles();
	RenderHeatMap();
	RenderEntities();
	DebugRender();
}

void Map::ShutDown()
{
	delete m_player;
	m_player = nullptr;
}

int Map::GetTileIndexForTileCoords(int tileX, int tileY) const
{
	return tileX + (tileY * m_dimensions.x);
}

IntVec2 Map::GetTileCoordsForWorldPos(Vec2 const& worldPos) const
{
	int tileX = RoundDownToInt(worldPos.x);
	int tileY = RoundDownToInt(worldPos.y);
	return IntVec2(tileX, tileY);
}

int Map::GetTileIndexForWorldPos(Vec2 const& worldPos) const
{
	IntVec2 tileCoords = GetTileCoordsForWorldPos(worldPos);
	return GetTileIndexForTileCoords(tileCoords.x, tileCoords.y);
}

void Map::SetTileType(int tileX, int tileY, TileDefinition* type)
{
	int tileIndex = GetTileIndexForTileCoords(tileX, tileY);
	SetTileType(tileIndex, type);
}

void Map::SetTileType(int tileIndex, TileDefinition* type)
{
	m_tiles[tileIndex].m_tileType = type;
}

void Map::PopulateTiles()
{
	m_tiles.clear();
	int numTiles = m_dimensions.x * m_dimensions.y;
	m_tiles.reserve(numTiles);
	for (int tileIndexY = 0; tileIndexY < m_dimensions.y; ++tileIndexY)
	{
		for (int tileIndexX = 0; tileIndexX < m_dimensions.x; ++tileIndexX)
		{
			Tile tile = Tile();
			tile.m_tileCoords = IntVec2(tileIndexX, tileIndexY);
			
			if (tileIndexX == m_dimensions.x-1 || tileIndexX == 0 || tileIndexY == m_dimensions.y-1 || tileIndexY == 0)
			{
				tile.SetType(TileDefinition::GetTileDefinition(m_mapDef.m_edgeTileType));
			}
			else
			{
				tile.SetType(TileDefinition::GetTileDefinition(m_mapDef.m_fillTileType));
			}
			m_tiles.push_back(tile);
		}
	}
	TileDefinition* worm1TileDef = TileDefinition::GetTileDefinition(m_mapDef.m_worm1TileType);
	TileDefinition* worm2TileDef = TileDefinition::GetTileDefinition(m_mapDef.m_worm2TileType);
	GenerateWorms(worm1TileDef, m_mapDef.m_worm1Count, m_mapDef.m_worm1MaxLength);
	GenerateWorms(worm2TileDef, m_mapDef.m_worm2Count, m_mapDef.m_worm2MaxLength);
	GenerateSpecialArea();
}

void Map::PopulateHeatMap()
{
	m_heatMap = TileHeatMap(IntVec2(m_dimensions.x, m_dimensions.y));
	PopulateDistanceField(m_heatMap, IntVec2(1, 1), 999.f, true);
	FillMap(m_heatMap);

	for (int i = 0; i < 1000; ++i)
	{
		if (m_heatMap.GetValue(IntVec2(m_dimensions.x - 2, m_dimensions.y - 2)) == 999.f)
		{
			PopulateTiles();
			PopulateDistanceField(m_heatMap, IntVec2(1, 1), 999.f, true);
			FillMap(m_heatMap);
		}
		else
		{
			break;
		}
	}
}

void Map::PopulateEntities()
{
	for (int i = 0; i < m_mapDef.m_entitySpawnCounts[ENTITY_TYPE_EVIL_SCORPIO]; ++i)
	{
		Vec2 position_1 = GetRandomPos();
		float orientationDegrees_1 = g_rng->RollRandomFloatInRange(0.f, 360.f);
		SpawnNewEntity(ENTITY_TYPE_EVIL_SCORPIO, position_1, orientationDegrees_1);
	}

	for (int i = 0; i < m_mapDef.m_entitySpawnCounts[ENTITY_TYPE_EVIL_LEO]; ++i)
	{
		Vec2 position_2 = GetRandomPos();
		float orientationDegrees_2 = g_rng->RollRandomFloatInRange(0.f, 360.f);
		SpawnNewEntity(ENTITY_TYPE_EVIL_LEO, position_2, orientationDegrees_2);
	}

	for (int i = 0; i < m_mapDef.m_entitySpawnCounts[ENTITY_TYPE_EVIL_ARIES]; ++i)
	{
		Vec2 position_3 = GetRandomPos();
		float orientationDegrees_3 = g_rng->RollRandomFloatInRange(0.f, 360.f);
		SpawnNewEntity(ENTITY_TYPE_EVIL_ARIES, position_3, orientationDegrees_3);
	}
	for (int i = 0; i < m_mapDef.m_entitySpawnCounts[ENTITY_TYPE_EVIL_CAPRICORN]; ++i)
	{
		Vec2 position_4 = GetRandomPos();
		float orientationDegrees_4 = g_rng->RollRandomFloatInRange(0.f, 360.f);
		SpawnNewEntity(ENTITY_TYPE_EVIL_CAPRICORN, position_4, orientationDegrees_4);
	}
}

void Map::GenerateSpecialArea()
{
	int startAreaSize = g_gameConfigBlackboard.GetValue("startAreaSize", 5);
	int endAreaSize = g_gameConfigBlackboard.GetValue("endAreaSize", 7);
	for (int i = 1; i <= startAreaSize; ++i)
	{
		for (int j = 1; j <= startAreaSize; ++j)
		{
			int tileIndex = GetTileIndexForTileCoords(j, i);
			if(i == 4 && j > 1 && j < startAreaSize) m_tiles[tileIndex].SetType(TileDefinition::GetTileDefinition(m_mapDef.m_startBunkerTileType));
			else if(j == 4 && i > 1 && i < startAreaSize) m_tiles[tileIndex].SetType(TileDefinition::GetTileDefinition(m_mapDef.m_startBunkerTileType));
			else m_tiles[tileIndex].SetType(TileDefinition::GetTileDefinition(m_mapDef.m_startFloorTileType));
		}
	}
	for (int i = m_dimensions.y - endAreaSize; i < m_dimensions.y-1; ++i)
	{
		for (int j = m_dimensions.x - endAreaSize; j < m_dimensions.x-1; ++j)
		{
			int tileIndex = GetTileIndexForTileCoords(j, i);
			if (i == m_dimensions.y-6 && j > m_dimensions.x-7 && j < m_dimensions.x-2) m_tiles[tileIndex].SetType(TileDefinition::GetTileDefinition(m_mapDef.m_endBunkerTileType));
			else if (j == m_dimensions.x-6 && i > m_dimensions.y-7 && i < m_dimensions.y-2) m_tiles[tileIndex].SetType(TileDefinition::GetTileDefinition(m_mapDef.m_endBunkerTileType));
			else m_tiles[tileIndex].SetType(TileDefinition::GetTileDefinition(m_mapDef.m_endFloorTileType));
		}
	}
	TileDefinition* entryTileDef = TileDefinition::GetTileDefinition(m_mapDef.m_mapEntry);
	TileDefinition* exitTileDef =  TileDefinition::GetTileDefinition(m_mapDef.m_mapExit);
	m_tiles[GetTileIndexForTileCoords(1,1)].SetType(entryTileDef);
	m_tiles[GetTileIndexForTileCoords( m_dimensions.x-2,  m_dimensions.y-2)].SetType(exitTileDef);
}

void Map::GenerateWorms(TileDefinition* type, int wormCount, int wormLength)
{
	int tem_x = 0;
	int tem_y=0;
	for (int i = 0; i < wormCount; ++i)
	{
		int x = g_rng->RollRandomIntInRange(1, m_dimensions.x - 2);
		int y = g_rng->RollRandomIntInRange(1, m_dimensions.y - 2);
		int tileIndex = GetTileIndexForTileCoords(x, y);
		m_tiles[tileIndex].SetType(type);
		for (int j = 0; j < wormLength; ++j)
		{
			int num = g_rng->RollRandomIntInRange(1, 4);
			switch (num)
			{
			case 1:
				tem_x = x + 1;
				tem_y = y;
				break;
			case 2:
				tem_x = x;
				tem_y = y - 1;
				break;
			case 3:
				tem_x = x - 1;
				tem_y = y;
				break;
			case 4:
				tem_x = x;
				tem_y = y + 1;
				break;
			}
			if (tem_x == 0 || tem_x == m_dimensions.x-1 || tem_y == 0 || tem_y == m_dimensions.y-1)
			{
				continue;
			}
			else
			{
				x = tem_x;
				y = tem_y;
				int temtileIndex = GetTileIndexForTileCoords(x, y);
				m_tiles[temtileIndex].SetType(type);
			}
		}
	}
}

void Map::PopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 startCoords, float maxCost, bool treatWaterAsSolid/*=true */) const
{
	out_distanceField.SetAllValues(maxCost);
	out_distanceField.SetValue(startCoords, 0.0f);
	float currentValue = 0;
	bool loop = true;
	while (loop)
	{
		if (currentValue == ((float)m_dimensions.x-2 + (float)m_dimensions.y-2))
		{
			loop = false;
		}
		for (int i = 0; i < out_distanceField.GetDimensions().y-1; ++i)
		{
			for (int j = 0; j < out_distanceField.GetDimensions().x-1; ++j)
			{
				IntVec2 currentPos = IntVec2(j, i);
				if (out_distanceField.GetValue(currentPos) == currentValue)
				{
					SetHeatMapValue(out_distanceField, currentPos, currentValue, 1, treatWaterAsSolid);
					SetHeatMapValue(out_distanceField, currentPos, currentValue, 2, treatWaterAsSolid);
					SetHeatMapValue(out_distanceField, currentPos, currentValue, 3, treatWaterAsSolid);
					SetHeatMapValue(out_distanceField, currentPos, currentValue, 4, treatWaterAsSolid);
				}
			}
		}
		currentValue += 1.f;
	}
}

void Map::EnemyPopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 startCoords, float maxCost, bool treatWaterAsSolid /*= true*/) const
{
	out_distanceField.SetAllValues(maxCost);
	out_distanceField.SetValue(startCoords, 0.0f);
	float currentValue = 0;
	bool loop = true;
	while (loop)
	{
		if (currentValue == ((float)m_dimensions.x - 2 + (float)m_dimensions.y - 2))
		{
			loop = false;
		}
		for (int i = 0; i < out_distanceField.GetDimensions().y; ++i)
		{
			for (int j = 0; j < out_distanceField.GetDimensions().x; ++j)
			{
				IntVec2 currentPos = IntVec2(j, i);
				if (out_distanceField.GetValue(currentPos) == currentValue)
				{
					SetEnemyHeatMapValue(out_distanceField, currentPos, currentValue, 1, treatWaterAsSolid);
					SetEnemyHeatMapValue(out_distanceField, currentPos, currentValue, 2, treatWaterAsSolid);
					SetEnemyHeatMapValue(out_distanceField, currentPos, currentValue, 3, treatWaterAsSolid);
					SetEnemyHeatMapValue(out_distanceField, currentPos, currentValue, 4, treatWaterAsSolid);
				}
			}
		}
		currentValue += 1.f;
	}
}

void Map::SelfPopulateDistanceField(TileHeatMap& out_distanceField) const
{
	for (int i = 0; i < out_distanceField.GetDimensions().y; ++i)
	{
		for (int j = 0; j < out_distanceField.GetDimensions().x; ++j)
		{
			IntVec2 currentPos = IntVec2(j, i);
			if (out_distanceField.GetValue(currentPos) != 999.f)
			{
				out_distanceField.SetValue(currentPos, 0.f);
			}
		}
	}
}

void Map::EntityDiedExplosions(float deltaSeconds, Vec2 position)
{
	m_explosionMode = 1;
	for (int i = 0; i < 15; ++i)
	{
		m_explosionsTime += deltaSeconds;
		float angleVariance = g_rng->RollRandomFloatInRange(-360.f, 360.f);
		SpawnNewEntity(ENTITY_TYPE_NETURAL_EXPLOSION, position, angleVariance);
	}
	m_explosionsTime = 0.f;
}

void Map::EntityFireExplosions(float deltaSeconds, Vec2 position, float turretOrientation)
{
	m_explosionMode = 2;
	for (int i = 0; i < 10; ++i)
	{
		m_explosionsTime += deltaSeconds;
		float angleVariance = g_rng->RollRandomFloatInRange(-15.f, 15.f);
		SpawnNewEntity(ENTITY_TYPE_NETURAL_EXPLOSION, position, turretOrientation + angleVariance);
	}
	m_explosionsTime = 0.f;
}

void Map::BulletDiedExplosions(float deltaSeconds, Vec2 position)
{
	m_explosionMode = 3;
	for (int i = 0; i < 10; ++i)
	{
		m_explosionsTime += deltaSeconds;
		float angleVariance = g_rng->RollRandomFloatInRange(-360.f, 360.f);
		SpawnNewEntity(ENTITY_TYPE_NETURAL_EXPLOSION, position, angleVariance);
	}
	m_explosionsTime = 0.f;
}

void Map::FillMap(TileHeatMap& out_distanceField)
{
	for (int i = 0; i < out_distanceField.GetDimensions().y; ++i)
	{
		for (int j = 0; j < out_distanceField.GetDimensions().x; ++j)
		{
			IntVec2 currentPos = IntVec2(j, i);
			int tileIndex = GetTileIndexForTileCoords(j, i);
			if (j == m_dimensions.x - 1 || j == 0 || i == m_dimensions.y - 1 || i == 0)
			{
				continue;
			}
			else
			{
				if (out_distanceField.GetValue(currentPos) == 999.f)
				{
					if (m_tiles[tileIndex].IsWater())
					{
						continue;
					}
					m_tiles[tileIndex].SetType(TileDefinition::GetTileDefinition(m_mapDef.m_worm2TileType));
				}
			}
		}
	}
}

void Map::SetHeatMapValue(TileHeatMap& out_distanceField, IntVec2 currentPos, float currentValue, int direction, bool treatWaterAsSolid/*=true*/) const
{
	switch (direction)
	{
	//north
	case 1:
		if (!CheckTileIsSolid(IntVec2(currentPos.x, currentPos.y + 1), treatWaterAsSolid) && out_distanceField.GetValue(IntVec2(currentPos.x, currentPos.y + 1)) > currentValue)
		{
			out_distanceField.SetValue(IntVec2(currentPos.x, currentPos.y+1), currentValue + 1.f);
		}
		break;
	//south
	case 2:
		if (!CheckTileIsSolid(IntVec2(currentPos.x, currentPos.y - 1), treatWaterAsSolid) && out_distanceField.GetValue(IntVec2(currentPos.x, currentPos.y - 1)) > currentValue)
		{
			out_distanceField.SetValue(IntVec2(currentPos.x, currentPos.y - 1), currentValue + 1.f);
		}
		break;
	//east
	case 3:
		if (!CheckTileIsSolid(IntVec2(currentPos.x + 1, currentPos.y), treatWaterAsSolid) && out_distanceField.GetValue(IntVec2(currentPos.x + 1, currentPos.y)) > currentValue)
		{
			out_distanceField.SetValue(IntVec2(currentPos.x + 1, currentPos.y), currentValue + 1.f);
		}
		break;
	//west
	case 4:
		if (!CheckTileIsSolid(IntVec2(currentPos.x - 1, currentPos.y), treatWaterAsSolid) && out_distanceField.GetValue(IntVec2(currentPos.x - 1, currentPos.y)) > currentValue)
		{
			out_distanceField.SetValue(IntVec2(currentPos.x - 1, currentPos.y), currentValue + 1.f);
		}
		break;
	default: break;
	}
}

void Map::SetEnemyHeatMapValue(TileHeatMap& out_distanceField, IntVec2 currentPos, float currentValue, int direction, bool treatWaterAsSolid /*= true*/) const
{
	switch (direction)
	{
		//north
	case 1:
		if (!CheckTileIsSolid(IntVec2(currentPos.x, currentPos.y + 1), treatWaterAsSolid) && out_distanceField.GetValue(IntVec2(currentPos.x, currentPos.y + 1)) > currentValue && !CheckIfScorpio(IntVec2(currentPos.x, currentPos.y + 1)))
		{
			out_distanceField.SetValue(IntVec2(currentPos.x, currentPos.y + 1), currentValue + 1.f);
		}
		break;
		//south
	case 2:
		if (!CheckTileIsSolid(IntVec2(currentPos.x, currentPos.y - 1), treatWaterAsSolid) && out_distanceField.GetValue(IntVec2(currentPos.x, currentPos.y - 1)) > currentValue && !CheckIfScorpio(IntVec2(currentPos.x, currentPos.y - 1)))
		{
			out_distanceField.SetValue(IntVec2(currentPos.x, currentPos.y - 1), currentValue + 1.f);
		}
		break;
		//east
	case 3:
		if (!CheckTileIsSolid(IntVec2(currentPos.x + 1, currentPos.y), treatWaterAsSolid) && out_distanceField.GetValue(IntVec2(currentPos.x + 1, currentPos.y)) > currentValue && !CheckIfScorpio(IntVec2(currentPos.x + 1, currentPos.y)))
		{
			out_distanceField.SetValue(IntVec2(currentPos.x + 1, currentPos.y), currentValue + 1.f);
		}
		break;
		//west
	case 4:
		if (!CheckTileIsSolid(IntVec2(currentPos.x - 1, currentPos.y), treatWaterAsSolid) && out_distanceField.GetValue(IntVec2(currentPos.x - 1, currentPos.y)) > currentValue && !CheckIfScorpio(IntVec2(currentPos.x - 1, currentPos.y)))
		{
			out_distanceField.SetValue(IntVec2(currentPos.x - 1, currentPos.y), currentValue + 1.f);
		}
		break;
	default: break;
	}
}

bool Map::CheckIfScorpio(IntVec2 currentPos) const
{
	for (int i = 0; i < (int)m_entityListsByType[ENTITY_TYPE_EVIL_SCORPIO].size(); ++i)
	{
		Entity const* entity = m_entityListsByType[ENTITY_TYPE_EVIL_SCORPIO][i];
		if (entity)
		{
			IntVec2 scorpioPos = GetTileCoordsForWorldPos(entity->m_position);
			if (scorpioPos == currentPos)
			{
				return true;
			}
		}
	}
	return false;
}

bool Map::CheckTileIsSolid(IntVec2 currentPos, bool treatWaterAsSolid/*=true*/) const
{
	int tileIndex = Clamp( GetTileIndexForTileCoords(currentPos.x, currentPos.y), 0, m_dimensions.x * m_dimensions.y - 1);
	if (treatWaterAsSolid)
	{
		if (m_tiles[tileIndex].GettileDef().m_isWater)
		{
			return true;
		}
		else
		{
			return m_tiles[tileIndex].IsSolid();
		}
	}
	else
	{
		return m_tiles[tileIndex].IsSolid();
	}
}

void Map::AddEntityToList(Entity* entity, EntityList& list)
{
	for (int i = 0; i < list.size(); ++i)
	{
		if (list[i] == nullptr)
		{
			list[i] = entity;
			return;
		}
	}

	list.push_back(entity);
}

void Map::RemoveEntityFromMap(Entity* entity)
{
	RemoveEntityFromList(entity, m_allEntities);
	RemoveEntityFromList(entity, m_entityListsByType[entity->GetEntityType()]);
	if (entity->GetEntityType() == ENTITY_TYPE_GOOD_BULLET || entity->GetEntityType() == ENTITY_TYPE_EVIL_BULLET || entity->GetEntityType() == ENTITY_TYPE_EVIL_MISSILE || entity->GetEntityType() == ENTITY_TYPE_GOOD_MISSILE || entity->GetEntityType() == ENTITY_TYPE_EVIL_FIRES || entity->GetEntityType() == ENTITY_TYPE_GOOD_FIRES)
	{
		RemoveEntityFromList(entity,m_bulletsByFaction[entity->GetFaction()]);
	}
	else
	{
		RemoveEntityFromList(entity,m_actorsByFaction[entity->GetFaction()]);
	}
}

void Map::RemoveEntityFromList(Entity* entity, EntityList& entityList)
{
	for (int i = 0; i < entityList.size(); ++i)
	{
		Entity* entityInList = entityList[i];
		if (entityInList == entity) 
		{
			entityList[i] = nullptr;
			return;
		}
	}
}

void Map::DeleteGarbageEntities()
{

	for (int i = 0; i < m_allEntities.size(); ++i)
	{
		Entity* entity = m_allEntities[i];
		if (entity && entity->m_isGarbage)
		{
			RemoveEntityFromMap(entity);
			delete entity;

		}
	}
}

Vec2 Map::GetRandomPos()
{
	bool loop = true;
	while (loop)
	{
		int x = g_rng->RollRandomIntInRange(1, m_dimensions.x - 2);
		int y = g_rng->RollRandomIntInRange(1, m_dimensions.y - 2);
		int tileIndex = GetTileIndexForTileCoords(x, y);
		if (x < 6 && y < 6)
		{
			continue;
		}
		else if (x > m_dimensions.x - 9 && y > m_dimensions.y - 9)
		{
			continue;
		}
		else
		{
			if (!m_tiles[tileIndex].IsSolid() && !m_tiles[tileIndex].IsWater() && !CheckContainTileIndex(tileIndex))
			{
				loop = false;
				m_enemyPos.push_back(tileIndex);
				return Vec2((float)x + 0.5f, (float)y + 0.5f);
			}
		}	
	}
	return Vec2(-1.f, -1.f);
}

bool Map::CheckContainTileIndex(int tileIndex)
{
	for (int i = 0; i < m_enemyPos.size(); ++i)
	{
		if (m_enemyPos[i] == tileIndex)
		{
			return true;
		}
	}
	return false;
}

void Map::SpawnNewEntity(EntityType type, Vec2 const& position, float orientationDegrees)
{
	Entity* entity = CreateNewEntityOfType(type, position, orientationDegrees);
	AddEntityToList(entity, m_allEntities);

	switch (type)
	{
	case ENTITY_TYPE_GOOD_PLAYER:		AddEntityToList(entity, m_entityListsByType[ENTITY_TYPE_GOOD_PLAYER]);
										AddEntityToList(entity, m_actorsByFaction[FACTION_GOOD]);
										break;
	case ENTITY_TYPE_EVIL_SCORPIO:		AddEntityToList(entity, m_entityListsByType[ENTITY_TYPE_EVIL_SCORPIO]);
										AddEntityToList(entity, m_actorsByFaction[FACTION_EVIL]);
										break;
	case ENTITY_TYPE_EVIL_LEO:			AddEntityToList(entity, m_entityListsByType[ENTITY_TYPE_EVIL_LEO]);
										AddEntityToList(entity, m_actorsByFaction[FACTION_EVIL]);
										break;
	case ENTITY_TYPE_EVIL_ARIES:		AddEntityToList(entity, m_entityListsByType[ENTITY_TYPE_EVIL_ARIES]);
										AddEntityToList(entity, m_actorsByFaction[FACTION_EVIL]);
										break;
	case ENTITY_TYPE_EVIL_CAPRICORN:	AddEntityToList(entity, m_entityListsByType[ENTITY_TYPE_EVIL_CAPRICORN]);
										AddEntityToList(entity, m_actorsByFaction[FACTION_EVIL]);
										break;
	case ENTITY_TYPE_EVIL_BULLET:		AddEntityToList(entity, m_entityListsByType[ENTITY_TYPE_EVIL_BULLET]);
										AddEntityToList(entity, m_bulletsByFaction[FACTION_EVIL]);
										break;
	case ENTITY_TYPE_GOOD_BULLET:		AddEntityToList(entity, m_entityListsByType[ENTITY_TYPE_GOOD_BULLET]);
										AddEntityToList(entity, m_bulletsByFaction[FACTION_GOOD]);
										break;
	case ENTITY_TYPE_EVIL_MISSILE:		AddEntityToList(entity, m_entityListsByType[ENTITY_TYPE_EVIL_MISSILE]);
										AddEntityToList(entity, m_bulletsByFaction[FACTION_EVIL]);
										break;
	case ENTITY_TYPE_GOOD_MISSILE:		AddEntityToList(entity, m_entityListsByType[ENTITY_TYPE_GOOD_MISSILE]);
										AddEntityToList(entity, m_bulletsByFaction[FACTION_GOOD]);
										break;
	case ENTITY_TYPE_EVIL_FIRES:		AddEntityToList(entity, m_entityListsByType[ENTITY_TYPE_EVIL_FIRES]);
										AddEntityToList(entity, m_bulletsByFaction[FACTION_EVIL]);
										break;
	case ENTITY_TYPE_GOOD_FIRES:		AddEntityToList(entity, m_entityListsByType[ENTITY_TYPE_GOOD_FIRES]);
										AddEntityToList(entity, m_bulletsByFaction[FACTION_GOOD]);
										break;
	case ENTITY_TYPE_NETURAL_EXPLOSION:	AddEntityToList(entity, m_entityListsByType[ENTITY_TYPE_NETURAL_EXPLOSION]);
										AddEntityToList(entity, m_actorsByFaction[FACTION_NEUTRAL]);
										break;
	default:						
		break;
	}
}

Entity* Map::CreateNewEntityOfType(EntityType type, Vec2 const& position, float orientationDegrees)
{
	switch (type)
	{
				
	case ENTITY_TYPE_GOOD_PLAYER:		return new Player		(type, FACTION_GOOD, this, position, orientationDegrees);
	case ENTITY_TYPE_EVIL_SCORPIO:		return new Scorpio		(type, FACTION_EVIL, this, position, orientationDegrees);
	case ENTITY_TYPE_EVIL_LEO:			return new Leo			(type, FACTION_EVIL, this, position, orientationDegrees);
	case ENTITY_TYPE_EVIL_ARIES:		return new Aries		(type, FACTION_EVIL, this, position, orientationDegrees);
	case ENTITY_TYPE_EVIL_CAPRICORN:	return new Capricorn	(type, FACTION_EVIL, this, position, orientationDegrees);
	case ENTITY_TYPE_EVIL_BULLET:		return new Bullet		(type, FACTION_EVIL, this, position, orientationDegrees);
	case ENTITY_TYPE_GOOD_BULLET:		return new Bullet		(type, FACTION_GOOD, this, position, orientationDegrees);
	case ENTITY_TYPE_EVIL_MISSILE:		return new Missile		(type, FACTION_EVIL, this, position, orientationDegrees);
	case ENTITY_TYPE_GOOD_MISSILE:		return new Missile		(type, FACTION_GOOD, this, position, orientationDegrees);
	case ENTITY_TYPE_EVIL_FIRES:		return new Bullet		(type, FACTION_EVIL, this, position, orientationDegrees);
	case ENTITY_TYPE_GOOD_FIRES:		return new Bullet		(type, FACTION_GOOD, this, position, orientationDegrees);
	case ENTITY_TYPE_NETURAL_EXPLOSION:	return new Explosions	(type, FACTION_NEUTRAL, this, position, orientationDegrees);
	}
	ERROR_RECOVERABLE(Stringf("WARING: failed to create entity"));
	return nullptr;
}

void Map::UpdateCamera()
{
	float cameraPosX = Clamp(m_player->m_position.x, 8.f, m_dimensions.x - 8.f);
	float cameraPosY = Clamp(m_player->m_position.y, 4.f, m_dimensions.y - 4.f);
	if (g_theApp->m_debugCamera)
	{
		float WORLD_SIZE_X = g_gameConfigBlackboard.GetValue("WORLD_SIZE_X",200.f);
		float WORLD_SIZE_Y = g_gameConfigBlackboard.GetValue("WORLD_SIZE_Y", 100.f);
		float cameraAspect = WORLD_SIZE_X/WORLD_SIZE_Y;
		float mapAspect = (float)m_dimensions.x/(float)m_dimensions.y;
		if (cameraAspect > mapAspect)
		{
			float cameraHeight = (float)m_dimensions.y;
			float cameraWidth = cameraAspect * cameraHeight;
			g_theApp->m_worldCamera.SetOrthoView(Vec2(0, 0), Vec2(cameraWidth, cameraHeight));
			g_theApp->m_consoleCamera.SetOrthoView(Vec2(0, 0), Vec2(cameraWidth, cameraHeight));

		}
		else
		{
			float cameraWidth = (float)m_dimensions.x;
			float cameraHeight = cameraWidth / cameraAspect;
			g_theApp->m_worldCamera.SetOrthoView(Vec2(0, 0), Vec2(cameraWidth, cameraHeight));
			g_theApp->m_consoleCamera.SetOrthoView(Vec2(0, 0), Vec2(cameraWidth, cameraHeight));
		}
	}
	else
	{
		g_theApp->m_worldCamera.SetOrthoView(Vec2(cameraPosX - 8.f, cameraPosY - 4.f), Vec2(cameraPosX + 8.f, cameraPosY + 4.f));
	}
}

void Map::UpdateEntities(float deltaSeconds)
{
	for (int i = 0; i < m_allEntities.size(); ++i)
	{
		Entity* entity = m_allEntities[i];
		if (entity)
		{
			entity->Update(deltaSeconds);
		}
	}
}

void Map::PushEntitiesOutOfWalls()
{
	for (int i = 0; i < m_allEntities.size(); ++i)
	{
		Entity* entity = m_allEntities[i];
		if (entity != nullptr)
		{
			if (entity->m_entityType == ENTITY_TYPE_GOOD_BULLET || entity->m_entityType == ENTITY_TYPE_EVIL_BULLET || entity->m_entityType == ENTITY_TYPE_EVIL_MISSILE || entity->m_entityType == ENTITY_TYPE_GOOD_MISSILE || entity->m_entityType == ENTITY_TYPE_EVIL_FIRES || entity->m_entityType == ENTITY_TYPE_GOOD_FIRES || entity->m_entityType == ENTITY_TYPE_NETURAL_EXPLOSION)
			{
				continue;
			}
			else
			{
				PushEntityOutOfWalls(entity);
			}
		}
	}
}

void Map::PushEntityOutOfWalls(Entity* e)
{
	IntVec2 entityCoords = GetTileCoordsForWorldPos(e->m_position);

	PushEntityOutOfTileIfSolid(e,  entityCoords.x + 1, entityCoords.y);
	PushEntityOutOfTileIfSolid(e,  entityCoords.x - 1, entityCoords.y);
	PushEntityOutOfTileIfSolid(e,  entityCoords.x, entityCoords.y + 1);
	PushEntityOutOfTileIfSolid(e,  entityCoords.x, entityCoords.y - 1);

	PushEntityOutOfTileIfSolid(e,  entityCoords.x + 1, entityCoords.y + 1);
	PushEntityOutOfTileIfSolid(e,  entityCoords.x - 1, entityCoords.y - 1);
	PushEntityOutOfTileIfSolid(e,  entityCoords.x - 1, entityCoords.y + 1);
	PushEntityOutOfTileIfSolid(e,  entityCoords.x + 1, entityCoords.y - 1);
}

void Map::PushEntityOutOfTileIfSolid(Entity* e, int tileX, int tileY)
{
	int tileIndex = Clamp(GetTileIndexForTileCoords(tileX, tileY), 0, m_dimensions.x * m_dimensions.y - 1);
	if (e->m_canSwim)
	{
		if (m_tiles[tileIndex].IsSolid() && !m_tiles[tileIndex].IsWater())
		{
			AABB2 tileBounds = m_tiles[tileIndex].GetBounds();
			PushDiscOutOfFixedAABB2D(e->m_position, e->m_physicsRadius, tileBounds);
		}
	}
	else
	{
		if (m_tiles[tileIndex].IsSolid() || m_tiles[tileIndex].IsWater())
		{
			AABB2 tileBounds = m_tiles[tileIndex].GetBounds();
			PushDiscOutOfFixedAABB2D(e->m_position, e->m_physicsRadius, tileBounds);
		}
	}
	if (e->m_position.x - e->m_physicsRadius <= 0.f) {
		e->m_position.x = e->m_physicsRadius;
	}
	if (e->m_position.x + e->m_physicsRadius >= static_cast<float>(m_dimensions.x)) {
		e->m_position.x = static_cast<float>(m_dimensions.x) - e->m_physicsRadius;
	}
	if (e->m_position.y - e->m_physicsRadius <= 0.f) {
		e->m_position.y = e->m_physicsRadius;
	}
	if (e->m_position.y + e->m_physicsRadius >= static_cast<float>(m_dimensions.y)) {
		e->m_position.y = static_cast<float>(m_dimensions.y) - e->m_physicsRadius;
	}

}

void Map::AddVertsForTile(std::vector<Vertex_PCU>& verts, int tileIndex) const
{
	Tile const& tile = m_tiles[tileIndex];
	AABB2 bounds = tile.GetBounds();
	Rgba8 color = tile.GetColor();

	const SpriteDefinition& SpriteDefKKK = m_theGame->m_spriteSheet->GetSpriteDef(m_tiles[tileIndex].GetTerrainSpriteIndex());
	Vec2 uvAtMins, uvAtMaxs;
	SpriteDefKKK.GetUVs(uvAtMins, uvAtMaxs);
	AddVertsForAABB2D(verts, bounds, color, uvAtMins, uvAtMaxs);
	g_theRenderer->BindTexture(&SpriteDefKKK.GetTexture());
}

void Map::RenderTiles() const
{
	int bestGuessVertexCount = 3 * 2 * m_dimensions.x * m_dimensions.y;
	std::vector<Vertex_PCU> tileVerts;
	tileVerts.reserve( bestGuessVertexCount );
	for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); ++tileIndex)
	{
		AddVertsForTile( tileVerts, tileIndex );
	}
	g_theRenderer->DrawVertexArray( (int) tileVerts.size(), tileVerts.data() );
	g_theRenderer->BindTexture(nullptr);

}

void Map::RenderHeatMap() const
{
	if (g_theApp->m_heatMode)
	{
		if (g_theApp->m_heatIndex == 1)
		{
			std::vector<Vertex_PCU> heatVerts;
			heatVerts.reserve(3 * 2 * m_dimensions.x * m_dimensions.y);
			m_heatMap.AddVertsForDebugDraw(heatVerts, AABB2(0.0, 0.0, (float)m_dimensions.x, (float)m_dimensions.y), FloatRange(0.f, float(m_dimensions.x + m_dimensions.y)), Rgba8(0, 0, 0, 255), Rgba8(255, 255, 255, 255), 999, Rgba8(0, 0, 255, 255));
			g_theRenderer->BindTexture(nullptr);
			g_theRenderer->DrawVertexArray((int)heatVerts.size(), heatVerts.data());

			RenderFonts("Heat Map Debug: Distance Map from start (F6 for next mode)");
			
		}
		else if (g_theApp->m_heatIndex == 2)
		{
			EnemyPopulateDistanceField(m_heatMapForAmphibians, IntVec2(1, 1), 999.f, false);
			SelfPopulateDistanceField(m_heatMapForAmphibians);
			int tileIndex = GetTileIndexForTileCoords( m_dimensions.x - 1,  m_dimensions.y - 1);
			if(m_tiles[tileIndex].IsWater()) m_heatMapForAmphibians.SetValue(m_dimensions, 0);
			std::vector<Vertex_PCU> heatVerts;
			heatVerts.reserve(3 * 2 * m_dimensions.x * m_dimensions.y);
			m_heatMapForAmphibians.AddVertsForDebugDraw(heatVerts, AABB2(0.0, 0.0, (float)m_dimensions.x, (float)m_dimensions.y), FloatRange(0.f, float(m_dimensions.x + m_dimensions.y)), Rgba8(0, 0, 0, 255), Rgba8(255, 255, 255, 255), 999, Rgba8(0, 0, 255, 255));
			g_theRenderer->BindTexture(nullptr);
			g_theRenderer->DrawVertexArray((int)heatVerts.size(), heatVerts.data());
			RenderFonts("Heat Map Debug: Solid Map for amphibians (F6 for next mode)");
		}
		else if (g_theApp->m_heatIndex == 3)
		{
			EnemyPopulateDistanceField(m_heatMapForLandBased, IntVec2(1, 1), 999.f, true);
			SelfPopulateDistanceField(m_heatMapForLandBased);
			std::vector<Vertex_PCU> heatVerts;
			heatVerts.reserve(3 * 2 * m_dimensions.x * m_dimensions.y);
			m_heatMapForLandBased.AddVertsForDebugDraw(heatVerts, AABB2(0.0, 0.0, (float)m_dimensions.x, (float)m_dimensions.y), FloatRange(0.f, float(m_dimensions.x + m_dimensions.y)), Rgba8(0, 0, 0, 255), Rgba8(255, 255, 255, 255), 999, Rgba8(0, 0, 255, 255));
			g_theRenderer->BindTexture(nullptr);
			g_theRenderer->DrawVertexArray((int)heatVerts.size(), heatVerts.data());
			RenderFonts("Heat Map Debug: Solid Map for land-based (F6 for next mode)");
		}
		else if (g_theApp->m_heatIndex == 4)
		{
			for (int i = 0; i < m_actorsByFaction[FACTION_EVIL].size(); ++i)
			{
				Entity* entity = m_actorsByFaction[FACTION_EVIL][i];
				if (entity && entity->GetEntityType() != ENTITY_TYPE_EVIL_SCORPIO)
				{
					entity->RenderHeatMap();
					break;
				}
			}
			RenderFonts("Heat Map Debug: Distance Map to selected Entity's goal (F6 for next mode)");
		}
	}
}

void Map::RenderFonts(std::string string) const
{
	std::vector<Vertex_PCU> textVerts;
	g_theFont->AddVertsForText2D(textVerts, Vec2(0.f, g_theApp->m_worldCamera.GetOrthographicTopRight().y - 0.5f), 0.3f, string, Rgba8::WHITE, 0.5f);
	Texture* fontTexture = &g_theFont->GetTexture();
	g_theRenderer->BindTexture(fontTexture);
	g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());
	g_theRenderer->BindTexture(nullptr);
}

void Map::RenderEntities() const
{
	for (int entityType = 0; entityType < NUM_ENTITY_TYPES; ++entityType)
	{
		EntityList const& entityList = m_entityListsByType[entityType];
		for (int indexInList = 0; indexInList < (int)entityList.size(); ++indexInList)
		{
			Entity const* entity = entityList[indexInList];
			if (entity)
			{
				entity->Render();
			}
		}
	}
}

void Map::DebugRender() const
{
	if (g_theApp->m_debugDraw)
	{
		for (int i = 0; i < m_allEntities.size(); ++i)
		{
			Entity const* entity = m_allEntities[i];
			if (entity)
			{
				entity->DebugRender();
			}
		}
	}
}

void Map::PushAllEntitiesOutOfEachOther()
{
	for (int i = 0; i < (int)m_allEntities.size()-1; ++i)
	{
		Entity* entity_a = m_allEntities[i];
		for (int j = i+1; j < (int)m_allEntities.size(); ++j)
		{
			Entity* entity_b = m_allEntities[j];
			if (entity_b != nullptr && entity_a != nullptr && entity_b != entity_a)
			{
				PushEntitiesOutOfEachOther(entity_a, entity_b);
			}
		}
	}
}

void Map::PushEntitiesOutOfEachOther(Entity* a, Entity* b)
{
	bool canApushB = a->m_doesPushEntities && b->m_isPushedEntities;
	bool canBpushA = b->m_doesPushEntities && a->m_isPushedEntities;

	if (!canBpushA && !canApushB)
	{
		return; // neither can push the other
	}
	else if (canApushB && canBpushA) // push them out of each other
	{
		PushDiscsOutOfEachOther2D(a->m_position, a->m_physicsRadius, b->m_position, b->m_physicsRadius);
	}
	else if (canApushB) // push B out of A
	{
		PushDiscOutOfFixedDisc2D(b->m_position, b->m_physicsRadius, a->m_position, a->m_physicsRadius);
	}
	else // push A out of B
	{
		PushDiscOutOfFixedDisc2D(a->m_position, a->m_physicsRadius, b->m_position, b->m_physicsRadius);
	}
}

void Map::CheckForBulletHits()
{
	for (int bulletFaction = 0; bulletFaction < NUM_FACTIONS; ++bulletFaction)
	{
		EntityList& bulletsList = m_bulletsByFaction[bulletFaction];
		for (int actorFaction = 0; actorFaction < NUM_FACTIONS; ++actorFaction)
		{
			if (actorFaction == bulletFaction)
			{
				continue;
			}
			EntityList& actorsList = m_actorsByFaction[actorFaction];
			CheckBulletListVsActorList(bulletsList, actorsList);
		}
	}
}

void Map::CheckBulletListVsActorList(EntityList& bulletsList, EntityList& actorsList)
{
	for (int bulletIndex = 0; bulletIndex < (int)bulletsList.size(); ++bulletIndex)
	{
	Bullet* bullet = dynamic_cast<Bullet*>(bulletsList[bulletIndex]);
		if (!IsAlive(bullet))
		{
			continue;
		}
		for (int actorIndex = 0; actorIndex < (int)actorsList.size(); ++actorIndex)
		{
			Entity* actor = actorsList[actorIndex];
			if (!IsAlive(actor))
			{
				continue;
			}
			CheckBulletVsActor(*bullet, *actor);
		}
	}
	for (int bulletIndex = 0; bulletIndex < (int)bulletsList.size(); ++bulletIndex)
	{
		Missile* missile = dynamic_cast<Missile*>(bulletsList[bulletIndex]);
		if (!IsAlive(missile))
		{
			continue;
		}
		for (int actorIndex = 0; actorIndex < (int)actorsList.size(); ++actorIndex)
		{
			Entity* actor = actorsList[actorIndex];
			if (!IsAlive(actor))
			{
				continue;
			}
			CheckMissileVsActor(*missile, *actor);
		}
	}
}

void Map::CheckBulletVsActor(Bullet& bullet, Entity& actor)
{
	if (actor.m_faction != FACTION_NEUTRAL)
	{
		if (bullet.m_faction == actor.m_faction)
		{
			return;
		}
		if (!DoEntitiesOverlap(bullet, actor))
		{
			return;
		}
		actor.ReactToBulletHit(bullet);
	}
}

void Map::CheckMissileVsActor(Missile& missile, Entity& actor)
{
	if (actor.m_faction != FACTION_NEUTRAL)
	{
		if (missile.m_faction == actor.m_faction)
		{
			return;
		}
		if (!DoEntitiesOverlap(missile, actor))
		{
			return;
		}
		actor.ReactToBulletHit(missile);
	}
}

bool Map::IsAlive(Entity* entity)
{
	if (!entity)
	{
		return false;
	}
	if (entity->m_isGarbage)
	{
		return false;

	}
	return true;
}

bool Map::DoEntitiesOverlap(Bullet& bullet, Entity& actor)
{
	float dx = bullet.m_position.x - actor.m_position.x;
	float dy = bullet.m_position.y - actor.m_position.y;
	float distSquared = (dx * dx) + (dy * dy);
	float combinedRadii = actor.m_physicsRadius + bullet.m_physicsRadius;
	float radiiSquared = (combinedRadii * combinedRadii);
	return distSquared < radiiSquared;
}

bool Map::DoEntitiesOverlap(Missile& bullet, Entity& actor)
{
	float dx = bullet.m_position.x - actor.m_position.x;
	float dy = bullet.m_position.y - actor.m_position.y;
	float distSquared = (dx * dx) + (dy * dy);
	float combinedRadii = actor.m_physicsRadius + bullet.m_physicsRadius;
	float radiiSquared = (combinedRadii * combinedRadii);
	return distSquared < radiiSquared;
}

Vec2 Map::GetPlayerPos() const
{
	return m_player->m_position;
}

bool Map::CheckPlayerDead() const
{
	return m_player->m_isDead;
}

