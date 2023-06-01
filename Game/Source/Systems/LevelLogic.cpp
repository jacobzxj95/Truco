#include <random>
#include "LevelLogic.h"
#include "../Components/Physics.h"
#include "../Entities/Prefabs.h"
#include "../Utils/Macros.h"

using namespace AVT; // Example Space Game

// Connects logic to traverse any players and allow a controller to manipulate them
bool AVT::LevelLogic::Init(	std::shared_ptr<flecs::world> _game,
							std::weak_ptr<const GameConfig> _gameConfig,
							GW::AUDIO::GAudio _audioEngine,
							Level_Data &_data)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	audioEngine = _audioEngine;
	data = &_data;
	// create an asynchronus version of the world
	gameAsync = game->async_stage(); // just used for adding stuff, don't try to read data
	gameLock.Create();
	// Pull enemy Y start location from config file
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	float enemy1startY = (*readCfg).at("Enemy1").at("ystart").as<float>();
	float enemy1accmax = (*readCfg).at("Enemy1").at("accmax").as<float>();
	float enemy1accmin = (*readCfg).at("Enemy1").at("accmin").as<float>();
	// level one info
	//float spawnDelay = (*readCfg).at("Level1").at("spawndelay").as<float>();
	// spins up a job in a thread pool to invoke a function at a regular interval
	/*timedEvents.Create(1, [this, enemy1startY, enemy1accmax, enemy1accmin]()
	{
		// compute random spawn location
		/*std::random_device rd;  // Will be used to obtain a seed for the random number engine
		std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
		std::uniform_real_distribution<float> x_range(-0.9f, +0.9f);
		std::uniform_real_distribution<float> a_range(enemy1accmin, enemy1accmax);
		float Xstart = x_range(gen); // normal rand() doesn't work great multi-threaded
		float accel = a_range(gen);#1#
		// grab enemy type 1 prefab
		flecs::entity et1; 
		if (RetreivePrefab("Enemy Type1", et1))
		{
			// you must ensure the async_stage is thread safe as it has no built-in synchronization
			gameLock.LockSyncWrite();
			// this method of using prefabs is pretty conveinent
			/*gameAsync.entity().is_a(et1)
				.set<Velocity>({ 0,0 })
				.set<Acceleration>({ 0, -accel })
				.set<Position>({ Xstart, enemy1startY });#1#
			for (int i = 0; i < data->blenderObjects.size(); i++)
			{
				auto iter = data->blenderObjects[i];
				if (strstr(iter.blendername,"Cylinder") != nullptr)
				{
					auto mesh = data->levelTransforms[iter.transformIndex];
					auto obb = data->levelColliders[iter.modelIndex];
					game->entity().is_a(et1)
					.set<Mesh>({mesh,obb, iter.transformIndex, iter.modelIndex});
				}
			}
			// be sure to unlock when done so the main thread can safely merge the changes
			gameLock.UnlockSyncWrite();
		//}
	//}, 1000); // wait 5 seconds to start enemy wave
		}
	});*/
	flecs::entity et1; 
	if (RetreivePrefab("Enemy Type1", et1))
	{
		for (int i = 0; i < data->blenderObjects.size(); i++)
		{
			auto iter = data->blenderObjects[i];
			if (strstr(iter.blendername,"Cylinder") != nullptr)
			{
				auto mesh = data->levelTransforms[iter.transformIndex];
				auto obb = data->levelColliders[iter.modelIndex];
				game->entity().is_a(et1)
				.set<Mesh>({mesh,obb, iter.transformIndex, iter.modelIndex});
			}
		}
	}
	// Load and play level one's music
	currentTrack.Create("../Music/Wolf-Asylum-Koord.wav", audioEngine, 0.15f);
	currentTrack.Play(true);

	return true;
}

// Free any resources used to run this system
bool AVT::LevelLogic::Shutdown()
{
	//timedEvents = nullptr; // stop adding enemies
	//gameAsync.merge(); // get rid of any remaining commands
	game->entity("Level System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}



// Toggle if a system's Logic is actively running
bool AVT::LevelLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Level System").enable();
	}
	else {
		game->entity("Level System").disable();
	}
	return false;
}

// **** SAMPLE OF MULTI_THREADED USE ****
//flecs::world world; // main world
//flecs::world async_stage = world.async_stage();
//
//// From thread
//lock(async_stage_lock);
//flecs::entity e = async_stage.entity().child_of(parent)...
//unlock(async_stage_lock);
//
//// From main thread, periodic
//lock(async_stage_lock);
//async_stage.merge(); // merge all commands to main world
//unlock(async_stage_lock);