// Obstacle system governing colliding between level and player
#ifndef OBSTACLELOGIC_H
#define OBSTACLELOGIC_H

// Contains our global game settings
#include "../Components/Physics.h"
#include "dxLogic.h"

// example space game (avoid name collisions)
namespace AVT 
{
	class ObstacleLogic 
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// handle to our running ECS system
		flecs::system obstacleSystem;
		// renderering data
		Level_Data* data;


	public:
		// attach the required logic to the ECS 
		bool Init(	std::shared_ptr<flecs::world> _game,
					Level_Data &_data);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown(); 
	};

};

#endif