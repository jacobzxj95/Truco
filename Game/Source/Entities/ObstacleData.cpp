#include "ObstacleData.h"

#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "Prefabs.h"

bool AVT::ObstacleData::Load(	std::shared_ptr<flecs::world> _game)
{
	// Create prefab for any collidable obstacle
	// add prefab to ECS
	auto obsPrefab = _game->prefab()
		.override<Obstacle>() // Tag this prefab as an obstacle (for queries/systems)
		.override<Collidable>() // can be collided with
	    .override<Mesh>();
	// register this prefab by name so other systems can use it
	RegisterPrefab("Obstacle Prefab", obsPrefab);

	return true;
}

bool AVT::ObstacleData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all obstacles and their prefabs
	_game->defer_begin(); // required when removing while iterating!
	_game->each([](flecs::entity e, Obstacle&) {
		e.destruct(); // destroy this entitiy (happens at frame end)
	});
	_game->defer_end(); // required when removing while iterating!

	// unregister this prefab by name
	UnregisterPrefab("Obstacle Prefab");

	return true;
}
