#include "Game/Player.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/Map.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include <windows.h>

extern App* g_theApp;
extern InputSystem* g_theInput;
extern Renderer* g_theRenderer;
extern Game* m_theGame;
extern AudioSystem* g_theAudio;
extern RandomNumberGenerator* g_rng;
#define UNUSED(x) (void)(x);

Player::Player(EntityType type, EntityFaction faction, Map* owner, const Vec2& startPos, float orientationDegrees)
	: Entity (type, faction, owner, startPos, orientationDegrees)
{
	m_health = 10;
	m_maxHealth = m_health;
	m_doesPushEntities = true;
	m_isPushedEntities = true;
	m_tankTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankBase.png");
	m_turretTeture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankTop.png");
	m_tankcolor = Rgba8(255, 255, 255, 255);
	m_turretcolor = Rgba8(255, 255, 255, 255);
	m_physicsRadius = g_gameConfigBlackboard.GetValue("ENTITY_PHYSICS_RADIUS", 0.3f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("ENTITY_COSMETIC_RADIUS", 0.6f);
	InitializeLocalVertsForTank();
	InitializeLocalVertsForTurret();
}

Player::~Player()
{

}

void Player::Update(float deltaSeconds)
{
	m_map->PopulateDistanceField(m_heatMap, IntVec2((int)m_position.x, (int)m_position.y), 999.f, true);

	if (m_health <= 0)
	{
		if (m_needExplosions)
		{
			m_theGame->m_currentMap->EntityDiedExplosions(deltaSeconds, m_position);
			m_needExplosions = false;
		}
		m_isDead = true;
	}
	UpdateFromKeyboard(deltaSeconds);
	UpdateFromController(deltaSeconds);
	m_tempOrientation = GetShortestAngularDispDegrees(m_orientationDegreesTank, m_orientationDegreesTurret) ;
	m_velocity = Vec2(0.f, 0.f);
}

void Player::Render() const
{
	if (m_isDead)
	{
		return;
	}
	Vertex_PCU tempTankWorldVerts[NUM_PLAYER_VERTS];
	for (int vertIndex = 0; vertIndex < NUM_PLAYER_VERTS; ++vertIndex)
	{
		tempTankWorldVerts[vertIndex] = m_localVertsTank[vertIndex];
	}

	TransfromVertexArrayXY3D(NUM_PLAYER_VERTS, tempTankWorldVerts, 1.f, m_orientationDegreesTank, m_position);
	g_theRenderer->BindTexture(m_tankTexture);
	g_theRenderer->DrawVertexArray( NUM_PLAYER_VERTS, tempTankWorldVerts );
	g_theRenderer->BindTexture(nullptr);

	Vertex_PCU tempTurretWorldVerts[NUM_PLAYER_VERTS];
	for (int vertIndex = 0; vertIndex < NUM_PLAYER_VERTS; ++vertIndex)
	{
		tempTurretWorldVerts[vertIndex] = m_localVertsTurret[vertIndex];
	}

	TransfromVertexArrayXY3D(NUM_PLAYER_VERTS, tempTurretWorldVerts, 1.f, m_orientationDegreesTurret, m_position);
	g_theRenderer->BindTexture(m_turretTeture);
	g_theRenderer->DrawVertexArray(NUM_PLAYER_VERTS, tempTurretWorldVerts);
	g_theRenderer->BindTexture(nullptr);

	if (!m_isHitByBullets)
	{
		DebugDrawRing(m_position, m_physicsRadius + 0.05f, 0.04f, Rgba8(255, 215, 0, 255));
	}

	if (g_theApp->m_showHP)
	{
		std::vector<Vertex_PCU> verts_red;
		Vec2 hpBarRedMins = m_position + Vec2(-0.4f, 0.4f);
		Vec2 hpBarRedMaxs = m_position + Vec2(0.4f, 0.5f);
		AABB2 hpBarRed = AABB2(hpBarRedMins, hpBarRedMaxs);
		AddVertsForAABB2D(verts_red, hpBarRed, Rgba8::RED);
		g_theRenderer->DrawVertexArray((int)verts_red.size(), verts_red.data());
		g_theRenderer->BindTexture(nullptr);

		std::vector<Vertex_PCU> verts_green;
		Vec2 hpBarGreenMaxs = Vec2(hpBarRedMins.x + ((hpBarRedMaxs.x - hpBarRedMins.x) * m_health / m_maxHealth), hpBarRedMaxs.y);
		AABB2 hpBarGreen = AABB2(hpBarRedMins, hpBarGreenMaxs);
		AddVertsForAABB2D(verts_green, hpBarGreen, Rgba8::GREEN);
		g_theRenderer->DrawVertexArray((int)verts_green.size(), verts_green.data());
		g_theRenderer->BindTexture(nullptr);
	}
}



void Player::DebugRender() const
{
	//float thickness = 0.04f;
	float thickness = g_gameConfigBlackboard.GetValue("debugDrawLineThickness",0.03f);
	Vec2 playerStartPos =m_position;
	Vec2 playerEndPosForward = Vec2(
		m_position.x + m_cosmeticRadius * CosDegrees(m_orientationDegreesTank),
		m_position.y + m_cosmeticRadius * SinDegrees(m_orientationDegreesTank)
	);
	Vec2 playerEndPosLeft = Vec2(
		m_position.x + m_cosmeticRadius * CosDegrees(m_orientationDegreesTank + 90.f),
		m_position.y + m_cosmeticRadius * SinDegrees(m_orientationDegreesTank + 90.f)
	);
	Vec2 tankControlLineStart = Vec2(
		m_position.x + m_cosmeticRadius * CosDegrees(direction_tank.GetOrientationDegrees()),
		m_position.y + m_cosmeticRadius * SinDegrees(direction_tank.GetOrientationDegrees())
	);
	Vec2 tankControlLineEnd = Vec2(
		m_position.x + (m_cosmeticRadius + 0.1f) * CosDegrees(direction_tank.GetOrientationDegrees()),
		m_position.y + (m_cosmeticRadius + 0.1f) * SinDegrees(direction_tank.GetOrientationDegrees())
	);
	Vec2 turretControlLineStart = Vec2(
		m_position.x + m_cosmeticRadius * CosDegrees(direction_turret.GetOrientationDegrees()),
		m_position.y + m_cosmeticRadius * SinDegrees(direction_turret.GetOrientationDegrees())
	);
	Vec2 turretControlLineEnd = Vec2(
		m_position.x + (m_cosmeticRadius + 0.1f) * CosDegrees(direction_turret.GetOrientationDegrees()),
		m_position.y + (m_cosmeticRadius + 0.1f) * SinDegrees(direction_turret.GetOrientationDegrees())
	);
	Vec2 turretEndPos = Vec2(
		m_position.x + m_cosmeticRadius * CosDegrees(m_orientationDegreesTurret ),
		m_position.y + m_cosmeticRadius * SinDegrees(m_orientationDegreesTurret )
	);
	Vec2 playerEndPosVelocity = m_position + GetForwardNormal();

	DebugDrawLine(playerStartPos, playerEndPosForward, Rgba8(255, 0, 0, 255), thickness);
	DebugDrawLine(playerStartPos, playerEndPosLeft, Rgba8(0, 255, 0, 255), thickness);
	DebugDrawRing(playerStartPos, m_cosmeticRadius, thickness, Rgba8(255, 0, 255, 255));
	DebugDrawRing(playerStartPos, m_physicsRadius, thickness, Rgba8(0, 255, 255, 255));
	DebugDrawLine(playerStartPos, turretEndPos, Rgba8(102, 0, 204, 255), thickness + 0.06f);
	XboxController const& controller = g_theInput->GetController(0);
	if (direction_tank != Vec2(0.f, 0.f)) 	
	{
		DebugDrawLine(playerStartPos, playerEndPosVelocity, Rgba8(255, 255, 0, 255), thickness - 0.02f);
		DebugDrawLine(tankControlLineStart, tankControlLineEnd, Rgba8(255, 51, 51, 255), thickness);
	}
	else if (controller.GetLeftStick().GetMagnitude() > 0.f)
	{
		Vec2 tankControlLineStartXbox = Vec2(
			m_position.x + m_cosmeticRadius * CosDegrees(controller.GetLeftStick().GetOrientationDegrees()),
			m_position.y + m_cosmeticRadius * SinDegrees(controller.GetLeftStick().GetOrientationDegrees())
		);
		Vec2 tankControlLineEndXbox = Vec2(
			m_position.x + (m_cosmeticRadius + 0.1f) * CosDegrees(controller.GetLeftStick().GetOrientationDegrees()),
			m_position.y + (m_cosmeticRadius + 0.1f) * SinDegrees(controller.GetLeftStick().GetOrientationDegrees())
		);
		DebugDrawLine(playerStartPos, playerEndPosVelocity, Rgba8(255, 255, 0, 255), thickness - 0.02f);
		DebugDrawLine(tankControlLineStartXbox, tankControlLineEndXbox, Rgba8(255, 51, 51, 255), thickness);
	}
	if (direction_turret != Vec2(0.f, 0.f))
	{
		DebugDrawLine(turretControlLineStart, turretControlLineEnd, Rgba8(102, 0, 204, 255), thickness + 0.06f);
	}
	else if (controller.GetRightStick().GetMagnitude() > 0.f)
	{
		Vec2 turretControlLineStartXbox = Vec2(
			m_position.x + m_cosmeticRadius * CosDegrees(controller.GetRightStick().GetOrientationDegrees()),
			m_position.y + m_cosmeticRadius * SinDegrees(controller.GetRightStick().GetOrientationDegrees())
		);
		Vec2 turretControlLineEndXbox = Vec2(
			m_position.x + (m_cosmeticRadius + 0.1f) * CosDegrees(controller.GetRightStick().GetOrientationDegrees()),
			m_position.y + (m_cosmeticRadius + 0.1f) * SinDegrees(controller.GetRightStick().GetOrientationDegrees())
		);
		DebugDrawLine(turretControlLineStartXbox, turretControlLineEndXbox, Rgba8(102, 0, 204, 255), thickness + 0.06f);
	}
}

void Player::InitializeLocalVertsForTank()
{
	m_localVertsTank[0].m_position = Vec3(0.5f, 0.5f, 0.0f);
	m_localVertsTank[1].m_position = Vec3(0.5f, -0.5f, 0.0f);
	m_localVertsTank[2].m_position = Vec3(-0.5f, -0.5f, 0.0f);
	
	m_localVertsTank[3].m_position = Vec3(0.5f, 0.5f, 0.0f);
	m_localVertsTank[4].m_position = Vec3(-0.5f, 0.5f, 0.0f);
	m_localVertsTank[5].m_position = Vec3(-0.5f, -0.5f, 0.0f);

	m_localVertsTank[0].m_uvTexCoords = Vec2(1.f, 1.f);
	m_localVertsTank[1].m_uvTexCoords = Vec2(1.f, 0.f);
	m_localVertsTank[2].m_uvTexCoords = Vec2(0.f, 0.f);
	
	m_localVertsTank[3].m_uvTexCoords = Vec2(1.f, 1.f);
	m_localVertsTank[4].m_uvTexCoords = Vec2(0.f, 1.f);
	m_localVertsTank[5].m_uvTexCoords = Vec2(0.f, 0.f);

	for (int vertIndex = 0; vertIndex < NUM_PLAYER_VERTS; ++vertIndex)
	{
		m_localVertsTank[vertIndex].m_color = m_tankcolor;
	}
}

void Player::InitializeLocalVertsForTurret()
{
	m_localVertsTurret[0].m_position = Vec3(0.5f, 0.5f, 0.0f);
	m_localVertsTurret[1].m_position = Vec3(0.5f, -0.5f, 0.0f);
	m_localVertsTurret[2].m_position = Vec3(-0.5f, -0.5f, 0.0f);

	m_localVertsTurret[3].m_position = Vec3(0.5f, 0.5f, 0.0f);
	m_localVertsTurret[4].m_position = Vec3(-0.5f, 0.5f, 0.0f);
	m_localVertsTurret[5].m_position = Vec3(-0.5f, -0.5f, 0.0f);

	m_localVertsTurret[0].m_uvTexCoords = Vec2(1.f, 1.f);
	m_localVertsTurret[1].m_uvTexCoords = Vec2(1.f, 0.f);
	m_localVertsTurret[2].m_uvTexCoords = Vec2(0.f, 0.f);

	m_localVertsTurret[3].m_uvTexCoords = Vec2(1.f, 1.f);
	m_localVertsTurret[4].m_uvTexCoords = Vec2(0.f, 1.f);
	m_localVertsTurret[5].m_uvTexCoords = Vec2(0.f, 0.f);

	for (int vertIndex = 0; vertIndex < NUM_PLAYER_VERTS; ++vertIndex)
	{
		m_localVertsTurret[vertIndex].m_color = m_turretcolor;
	}
}

void Player::UpdateFromKeyboard(float deltaSeconds)
{
	float playerGunTurnRate = g_gameConfigBlackboard.GetValue("playerGunTurnRate", 360.f);
	direction_tank = Vec2(0.f, 0.f);
	if (g_theApp->IsKeyDown('E'))				direction_tank += Vec2(0.f, 1.f);
	if (g_theApp->IsKeyDown('D'))				direction_tank += Vec2(0.f, -1.f);
	if (g_theApp->IsKeyDown('F'))				direction_tank += Vec2(1.f, 0.f);
	if (g_theApp->IsKeyDown('S'))				direction_tank += Vec2(-1.f, 0.f);
	if (direction_tank != Vec2(0.f, 0.f))		UpdateTankFromKeyboard(deltaSeconds, direction_tank.GetOrientationDegrees());

	direction_turret = Vec2(0.f, 0.f);
	if (g_theApp->IsKeyDown('I'))				direction_turret += Vec2(0.f, 1.f);
	if (g_theApp->IsKeyDown('K'))				direction_turret += Vec2(0.f, -1.f);
	if (g_theApp->IsKeyDown('L'))				direction_turret += Vec2(1.f, 0.f);
	if (g_theApp->IsKeyDown('J'))				direction_turret += Vec2(-1.f, 0.f);
	if (direction_turret != Vec2(0.f, 0.f))		UpdateTurretFromKeyboard(deltaSeconds, direction_turret.GetOrientationDegrees(), playerGunTurnRate);
	float playerShootCooldownSeconds = g_gameConfigBlackboard.GetValue("playerShootCooldownSeconds", 0.1f);
	m_fireTime += deltaSeconds;
	if (g_theApp->IsKeyDown(' ') && m_fireTime > playerShootCooldownSeconds && !m_isDead)
	{
		g_theAudio->StartSound(m_theGame->m_playerShootAudio);
		m_fireTime = 0.f;
		Vec2 gunpoint = m_position + Vec2::MakeFromPolarDegrees(m_orientationDegreesTurret, 0.4f);
		m_theGame->m_currentMap->SpawnNewEntity(ENTITY_TYPE_GOOD_BULLET, gunpoint, m_orientationDegreesTurret);
		m_theGame->m_currentMap->EntityFireExplosions(deltaSeconds, gunpoint, m_orientationDegreesTurret);

	}
	else if (g_theApp->IsKeyDown('M') && m_fireTime > 0.01f && !m_isDead)
	{
		m_fireTime = 0.f;
		m_fireBulletTime += deltaSeconds;
		float angleVariance = g_rng->RollRandomFloatInRange(-15.f, 15.f);
		Vec2 gunpoint = m_position + Vec2::MakeFromPolarDegrees(m_orientationDegreesTurret, 0.55f);
		m_theGame->m_currentMap->SpawnNewEntity(ENTITY_TYPE_GOOD_FIRES, gunpoint, m_orientationDegreesTurret + angleVariance);
	}
	if (g_theApp->WasKeyJustPressed('N') && m_isDead)
	{
		m_health = 10;
		m_isDead = false;
		g_theApp->m_isPaused = false;
		m_theGame->m_weatherContinue = false;
		m_needExplosions = true;
	}
	if (g_theApp->WasKeyJustPressed(VK_F2) && !m_isDead)
	{
		m_isHitByBullets = !m_isHitByBullets;
	}
}

void Player::UpdateFromController(float deltaSeconds)
{
	float playerGunTurnRate = g_gameConfigBlackboard.GetValue("playerGunTurnRate", 720.f);
	float playerTurnRate = g_gameConfigBlackboard.GetValue("playerTurnRate", 360.f);
	XboxController const& controller = g_theInput->GetController(0);

	// Drive
	float leftStickMagnitude = controller.GetLeftStick().GetMagnitude();
	if (leftStickMagnitude > 0.f)
	{
		m_thrustFraction = leftStickMagnitude;
		m_orientationDegreesTank = GetTurnedTowardDegrees(m_orientationDegreesTank, controller.GetLeftStick().GetOrientationDegrees(), playerTurnRate*deltaSeconds);
		if (controller.GetRightStick().GetMagnitude() <= 0.f)
		{
			UpdateTurretFromKeyboard(deltaSeconds, m_orientationDegreesTank + m_tempOrientation, playerTurnRate);
		}
		Vec2 forwardDirection = GetForwardNormal();
		float playerDriveSpeed = g_gameConfigBlackboard.GetValue("playerDriveSpeed", 2.f);
		m_velocity += forwardDirection * playerDriveSpeed * m_thrustFraction * deltaSeconds;
		m_position += (m_velocity);
	}

	float rightStickMagnitude = controller.GetRightStick().GetMagnitude();
	if (rightStickMagnitude > 0.f)
	{
		float rightStickAngle = controller.GetRightStick().GetOrientationDegrees();
		m_orientationDegreesTurret = GetTurnedTowardDegrees(m_orientationDegreesTurret, rightStickAngle, playerGunTurnRate * deltaSeconds);
	}
	m_fireTime += deltaSeconds;
	float playerShootCooldownSeconds = g_gameConfigBlackboard.GetValue("playerShootCooldownSeconds", 0.1f);
	if (controller.GetRightTrigger() > 0.f && m_fireTime > playerShootCooldownSeconds)
	{
		g_theAudio->StartSound(m_theGame->m_playerShootAudio);
		m_fireTime = 0.f;
		Vec2 gunpoint = m_position + Vec2::MakeFromPolarDegrees(m_orientationDegreesTurret, 0.4f);
		m_theGame->m_currentMap->SpawnNewEntity(ENTITY_TYPE_GOOD_BULLET, gunpoint, m_orientationDegreesTurret);
		m_theGame->m_currentMap->EntityFireExplosions(deltaSeconds, gunpoint, m_orientationDegreesTurret);
	}
}



void Player::UpdateTankFromKeyboard(float deltaSeconds, float degrees)
{
	float playerTurnRate = g_gameConfigBlackboard.GetValue("playerTurnRate", 360.f);
	m_orientationDegreesTank = GetTurnedTowardDegrees(m_orientationDegreesTank, degrees, playerTurnRate*deltaSeconds);
	if (direction_turret == Vec2(0.f, 0.f))
	{
		UpdateTurretFromKeyboard(deltaSeconds, m_orientationDegreesTank + m_tempOrientation, playerTurnRate);
	}
	Vec2 forwardDirection = GetForwardNormal();
	float playerDriveSpeed = g_gameConfigBlackboard.GetValue("playerDriveSpeed", 2.f);
	m_velocity += forwardDirection * playerDriveSpeed * deltaSeconds;
	m_position += (m_velocity);
}

void Player::UpdateTurretFromKeyboard(float deltaSeconds, float degrees, float turnSpeed)
{
	m_orientationDegreesTurret = GetTurnedTowardDegrees(m_orientationDegreesTurret, degrees, turnSpeed * deltaSeconds);
}


