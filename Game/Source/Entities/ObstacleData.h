// This class creates all types of bullet prefabs
#ifndef OBSTACLEDATA_H
#define OBSTACLEDATA_H

// Contains our global game settings

// example space game (avoid name collisions)
namespace AVT
{
    class ObstacleData
    {
    public:
        // Load required entities and/or prefabs into the ECS 
        bool Load(	std::shared_ptr<flecs::world> _game);
        // Unload the entities/prefabs from the ECS
        bool Unload(std::shared_ptr<flecs::world> _game);
    };

};

#endif