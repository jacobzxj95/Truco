
#include "ObstacleLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Entities/Prefabs.h"
#include "../Utils/Macros.h"

using namespace AVT; // Example Space Game

// Connects logic to traverse any players and allow a controller to manipulate them
bool AVT::ObstacleLogic::Init(std::shared_ptr<flecs::world> _game,
								Level_Data &_data)
{
	// save a handle to the ECS & game settings
	game = _game;
	data = &_data;
	
	// grab obstacle prefab
	flecs::entity obs; 
	if (RetreivePrefab("Obstacle Prefab", obs))
	{

		for (int i = 0; i < data->blenderObjects.size(); i++)
		{
			auto iter = data->blenderObjects[i];
			if (strstr(iter.blendername,"Wall") != nullptr)
			{
				auto mesh = data->levelTransforms[iter.transformIndex];
				auto obb = data->levelColliders[iter.modelIndex];
				game->entity().is_a(obs)
				.set<Mesh>({mesh,obb,iter.transformIndex, iter.modelIndex});
			}
		}
	}

	//system updating obstacles
	/*game->system<Obstacle>("Obstacle Colliders System")
	.each([this](flecs::entity e, Obstacle)
	{
		if (e.has<CollidedWith>(flecs::Wildcard))
		{
			std::cout << "wall hit" << std::endl
		}
	});*/
	
	return true;
}

// Free any resources used to run this system
bool AVT::ObstacleLogic::Shutdown()
{
	game->entity("Obstacle System").destruct();
	game.reset();
	return true;
}



// Toggle if a system's Logic is actively running
bool AVT::ObstacleLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Obstacle System").enable();
	}
	else {
		game->entity("Obstacle System").disable();
	}
	return false;
}
