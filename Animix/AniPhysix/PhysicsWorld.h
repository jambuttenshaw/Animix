#pragma once

#include "LinearMath/btAlignedObjectArray.h"


// forward declarations
class btCollisionConfiguration;
class btDiscreteDynamicsWorld;
class btSequentialImpulseConstraintSolver;
class btBroadphaseInterface;
class btCollisionDispatcher;
class btCollisionShape;
class btRigidBody;

class GEFDebugDrawer;
namespace gef
{
	class Renderer3D;
}

namespace AniPhysix
{
	class PhysicsWorld
	{
	public:
		PhysicsWorld();
		~PhysicsWorld();

		PhysicsWorld(const PhysicsWorld&) = delete;
		PhysicsWorld& operator=(const PhysicsWorld&) = delete;

		PhysicsWorld(PhysicsWorld&&) = default;
		PhysicsWorld& operator=(PhysicsWorld&&) = default;

		// Getters
		inline constexpr static float GetTimeStep() { return g_FixedTimeStep; }

		inline btDiscreteDynamicsWorld* GetWorld() const { return m_DynamicsWorld; }

		void SetupDebugDrawer(gef::Renderer3D* renderer3D);

		void Tick(float frame_time);
		void DebugDraw() const;

		// Create objects
		void AddRigidBody(btRigidBody* rb) const;
		btRigidBody* CreateGround();

	private:
		// Physics world objects
		btDiscreteDynamicsWorld* m_DynamicsWorld = nullptr;

		btSequentialImpulseConstraintSolver* m_Solver = nullptr;
		btBroadphaseInterface* m_OverlappingPairCache = nullptr;
		btCollisionDispatcher* m_Dispatcher = nullptr;
		btCollisionConfiguration* m_CollisionConfiguration = nullptr;

		btAlignedObjectArray<btCollisionShape*> m_CollisionShapes;

		// Debug drawing
		GEFDebugDrawer* m_DebugDrawer = nullptr;

		// timer
		constexpr static float g_FixedTimeStep = 1.0f / 60.0f; // default 60Hz
		float m_Timer = 0.0f;
	};
}
