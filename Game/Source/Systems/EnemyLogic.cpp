#include <random>
#include "EnemyLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "../Events/Playevents.h"

using namespace AVT; // Example Space Game

// Connects logic to traverse any players and allow a controller to manipulate them
bool AVT::EnemyLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::CORE::GEventGenerator _eventPusher,
	Level_Data &_data)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	eventPusher = _eventPusher;
	data = &_data;
	killedEnemies = 0;

	// destroy any bullets that have the CollidedWith relationship
	game->system<Enemy, Health, Mesh>("Enemy System")
		.each([this](flecs::entity e, Enemy, Health& h, Mesh& me)
		{
			// if you have no health left be destroyed
			if (e.get<Health>()->value <= 0)
			{
				data->levelTransforms[me.tfIndex].row4 = {99,99,99,0};
				// play explode sound
				e.destruct();
				AVT::PLAY_EVENT_DATA x;
				GW::GEvent explode;
				explode.Write(AVT::PLAY_EVENT::ENEMY_DESTROYED, x);
 				eventPusher.Push(explode);
				killedEnemies++;
			}
		});
	
	// system for moving enemies
	game->system<Enemy, Mesh>("Enemy Movement System")
	.each([&](flecs::entity e, Enemy, Mesh me)
	{
		auto transform = me.t;
		if (left)
		{
			data->levelTransforms[me.tfIndex].row4.x -= 0.05f;
			transform = data->levelTransforms[me.tfIndex];
			e.set<Mesh>({transform,me.obb,me.tfIndex,me.modelIndex});
			distance -= 0.05f;
			if (distance <= -40.0f)
			{
				left = false;
			}
			/*if (e.has<CollidedWith>(flecs::Wildcard))
			{
				std::cout << "enemy hit something" << std::endl;
				e.remove<CollidedWith>(flecs::Wildcard);
			}*/
		}
		else
		{
			data->levelTransforms[me.tfIndex].row4.x += 0.05f;
			transform = data->levelTransforms[me.tfIndex];
			e.set<Mesh>({transform,me.obb,me.tfIndex,me.modelIndex});
			distance += 0.05f;
			if (distance >= 40.0f)
			{
				left = true;
			}
			/*if (e.has<CollidedWith>(flecs::Wildcard))
			{
				std::cout << "enemy hit something" << std::endl;
				e.remove<CollidedWith>(flecs::Wildcard);
			}*/
		}
	});
	
	
	return true;
}

// Free any resources used to run this system
bool AVT::EnemyLogic::Shutdown()
{
	game->entity("Enemy System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
bool AVT::EnemyLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Enemy System").enable();
	}
	else {
		game->entity("Enemy System").disable();
	}
	return false;
}
