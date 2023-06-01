// define all ECS components related to identification
#ifndef IDENTIFICATION_H
#define IDENTIFICATION_H

// example space game (avoid name collisions)
namespace AVT
{
	struct Player {};
	struct Bullet {};
	struct Enemy {};
	struct Lives {};
	struct ControllerID {
		unsigned index = 0;
	};
	struct Obstacle {};
};

#endif