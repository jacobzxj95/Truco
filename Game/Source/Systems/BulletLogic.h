// The Bullet system is responsible for inflicting damage and cleaning up bullets
#ifndef BULLETLOGIC_H
#define BULLETLOGIC_H

// Contains our global game settings
#include "dxLogic.h"
#include "../GameConfig.h"
#include "../Entities/BulletData.h"

// example space game (avoid name collisions)
namespace AVT
{
	class BulletLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;

		Level_Data* data;
	public:
		// attach the required logic to the ECS 
		bool Init(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig,
			Level_Data &_data);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();
	};

};

#endif