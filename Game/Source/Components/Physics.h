// define all ECS components related to movement & collision
#ifndef PHYSICS_H
#define PHYSICS_H

// example space game (avoid name collisions)
namespace AVT 
{
	// ECS component types should be *strongly* typed for proper queries
	// typedef is tempting but it does not help templates/functions resolve type
	struct Position { GW::MATH2D::GVECTOR2F value; };
	struct Velocity { GW::MATH2D::GVECTOR2F value; };
	struct Orientation { GW::MATH2D::GMATRIX2F value; };
	struct Acceleration { GW::MATH2D::GVECTOR2F value; };
	struct Mesh {
		GW::MATH::GMATRIXF t;
		GW::MATH::GOBBF obb;
		unsigned tfIndex;
		unsigned modelIndex;
	};
	struct DevMode { bool dev = false; };

	// Individual TAGs
	struct Collidable {}; 
	
	// ECS Relationship tags
	struct CollidedWith {};
	struct HitByBullet {};
};

#endif