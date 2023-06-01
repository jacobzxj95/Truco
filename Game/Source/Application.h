#ifndef APPLICATION_H
#define APPLICATION_H

// include events
#include "Events/Playevents.h"
// Contains our global game settings
#include "GameConfig.h"
// Load all entities+prefabs used by the game 
#include "Entities/BulletData.h"
#include "Entities/PlayerData.h"
#include "Entities/EnemyData.h"
#include "Entities/ObstacleData.h"
// Include all systems used by the game and their associated components
#include "Systems/PlayerLogic.h"
#include "Systems/dxLogic.h"
#include "Systems/LevelLogic.h"
#include "Systems/PhysicsLogic.h"
#include "Systems/BulletLogic.h"
#include "Systems/EnemyLogic.h"
#include "Systems/ObstacleLogic.h"

// Allocates and runs all sub-systems essential to operating the game
class Application 
{
	// gateware libs used to access operating system
	GW::SYSTEM::GWindow window; // gateware multi-platform window
	GW::GRAPHICS::GDirectX11Surface d3d11; // gateware vulkan API wrapper
	GW::INPUT::GController gamePads; // controller support
	GW::INPUT::GInput immediateInput; // twitch keybaord/mouse
	GW::INPUT::GBufferedInput bufferedInput; // event keyboard/mouse
	GW::AUDIO::GAudio audioEngine; // can create music & sound effects
	// third-party gameplay & utility libraries
	std::shared_ptr<flecs::world> game; // ECS database for gameplay
	std::shared_ptr<GameConfig> gameConfig; // .ini file game settings
	// ECS Entities and Prefabs that need to be loaded
	AVT::BulletData weapons;
	AVT::PlayerData players;
	AVT::EnemyData enemies;
	AVT::ObstacleData obstacles;
	// specific ECS systems used to run the game
	AVT::PlayerLogic playerSystem;
	//ESG::VulkanRendererLogic vkRenderingSystem;
	AVT::dxLogic dxRenderingSystem;
	AVT::LevelLogic levelSystem;
	AVT::PhysicsLogic physicsSystem;
	AVT::BulletLogic bulletSystem;
	AVT::EnemyLogic enemySystem;
	AVT::ObstacleLogic obstacleSystem;
	// EventGenerator for Game Events
	GW::CORE::GEventGenerator eventPusher;
	Level_Data dataOrientedLoader;

	// Test Variables
	float one;
	float stop = 0;

	AVT::DevVar temp = { false, false, false, false };
	AVT::DevVar* dev = &temp;
public:
	bool Init();
	bool Run();
	bool Shutdown();
private:
	bool InitWindow();
	bool InitInput();
	bool InitAudio();
	bool InitGraphics();
	bool InitEntities();
	bool InitSystems();
	bool GameLoop();
};

#endif 