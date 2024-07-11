#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"
#include "Game/Tile.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/Leo.hpp"
#include "Game/Aries.hpp"
#include "Game/Capricorn.hpp"
#include "Game/Scorpio.hpp"
#include "Game/Bullet.hpp"
#include "Game/Missile.hpp"
#include "Game/Explosions.hpp"
#include "Engine/Core/HeatMaps.hpp"
#include <vector>

typedef std::vector<Entity*> EntityList;

struct MapDefinition
{
	std::string		m_name			= "";
	IntVec2			m_dimensions	= IntVec2::ZERO;
	std::string		m_fillTileType	= "";
	std::string		m_edgeTileType	= "";
	std::string		m_worm1TileType	= "";	
	int				m_worm1Count		= -1;
	int				m_worm1MaxLength	= -1;
	std::string		m_worm2TileType	= "";
	int				m_worm2Count		= -1;
	int				m_worm2MaxLength	= -1;
	std::string		m_endFloorTileType = "";
	std::string		m_endBunkerTileType = "";
	std::string		m_startFloorTileType = "";
	std::string		m_startBunkerTileType = "";
	std::string		m_mapEntry = "";
	std::string		m_mapExit = "";
	int				m_entitySpawnCounts[NUM_ENTITY_TYPES] = {};
};

class Map
{
public:
	Map(Game* gameInstance, MapDefinition const& mapDef);
	~Map();

	void	Startup();
	void	Update(float deltaSeconds);
	void	Render() const;
	void	ShutDown();
	int		GetTileIndexForTileCoords(int tileX, int tileY) const;
	IntVec2 GetTileCoordsForWorldPos(Vec2 const& worldPos) const;
	int		GetTileIndexForWorldPos(Vec2 const& worldPos) const;

	void	SetTileType( int tileX, int tileY, TileDefinition* type );
	void	SetTileType( int tileIndex, TileDefinition* type );

	Vec2	GetPlayerPos() const;
	bool	CheckPlayerDead() const;
	Vec2	GetRandomPos();
	bool	CheckContainTileIndex(int tileIndex);
	void	SpawnNewEntity(EntityType type, Vec2 const& position, float orientationDegrees);
	Entity* CreateNewEntityOfType(EntityType type, Vec2 const& position, float orientationDegrees);
	void	AddEntityToList(Entity* entity, EntityList& list);
	void	PopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 startCoords, float maxCost, bool treatWaterAsSolid = true) const;
	void	EnemyPopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 startCoords, float maxCost, bool treatWaterAsSolid = true) const;
	void	SelfPopulateDistanceField(TileHeatMap& out_distanceField) const;
	void	EntityDiedExplosions(float deltaSeconds, Vec2 position);
	void	EntityFireExplosions(float deltaSeconds, Vec2 position, float turretOrientation);
	void	BulletDiedExplosions(float deltaSeconds, Vec2 position);


public:
	Entity* m_player	 = nullptr;
	IntVec2				 m_dimensions = IntVec2(0, 0);
	MapDefinition		 m_mapDef;
	std::vector<Tile>    m_tiles;
	float				 m_explosionsTime = 0.f;
	int					 m_explosionMode = 0;

protected:
	void PopulateTiles();
	void PopulateHeatMap();
	void PopulateEntities();
	void GenerateSpecialArea();
	void GenerateWorms(TileDefinition* type, int wormCount, int wormLength);
	void FillMap(TileHeatMap& out_distanceField);
	void SetHeatMapValue(TileHeatMap& out_distanceField,IntVec2 currentPos, float currentValue, int direction, bool treatWaterAsSolid=true) const;
	void SetEnemyHeatMapValue(TileHeatMap& out_distanceField, IntVec2 currentPos, float currentValue, int direction, bool treatWaterAsSolid = true) const;
	bool CheckIfScorpio(IntVec2 currentPos) const;
	bool CheckTileIsSolid(IntVec2 currentPos, bool treatWaterAsSolid=true) const;

	void UpdateCamera();
	void UpdateEntities( float deltaSeconds );

	void PushEntitiesOutOfWalls();
	void PushEntityOutOfWalls( Entity* e );
	void PushEntityOutOfTileIfSolid( Entity* e, int tileX, int tileY );
	void PushAllEntitiesOutOfEachOther();
	void PushEntitiesOutOfEachOther(Entity* a, Entity* b);

	void CheckForBulletHits();
	void CheckBulletListVsActorList(EntityList& bulletLists, EntityList& actorLists);
	void CheckBulletVsActor(Bullet& bullet, Entity& actor);
	void CheckMissileVsActor(Missile& missile, Entity& actor);
	bool IsAlive(Entity* entity);
	bool DoEntitiesOverlap(Bullet& bullet, Entity& actor);
	bool DoEntitiesOverlap(Missile& bullet, Entity& actor);

	
	void RenderTiles() const;
	void RenderHeatMap() const;
	void RenderFonts(std::string string) const;
	void RenderEntities() const;
	void DebugRender() const;
	
	void AddVertsForTile( std::vector<Vertex_PCU>& verts, int tileIndex ) const;

	void RemoveEntityFromMap(Entity* entity);
	void RemoveEntityFromList(Entity* entity, EntityList& entityList);
	void DeleteGarbageEntities();

public:
	Game*						m_game = nullptr;
	EntityList					m_allEntities;
	EntityList					m_entityListsByType[ NUM_ENTITY_TYPES ];
	EntityList					m_bulletsByFaction[ NUM_FACTIONS ];
	EntityList					m_actorsByFaction[ NUM_FACTIONS ];
	std::vector<int>			m_enemyPos;
	mutable TileHeatMap			m_heatMap = TileHeatMap(IntVec2(0, 0));
	mutable TileHeatMap			m_heatMapForAmphibians = TileHeatMap(IntVec2(0, 0));
	mutable TileHeatMap			m_heatMapForLandBased = TileHeatMap(IntVec2(0, 0));

};