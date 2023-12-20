#include "PhysicsWorld.h"

// Bullet includes
#include "btBulletDynamicsCommon.h"

#include "gef_debug_drawer.h"


namespace AniPhysix
{

	PhysicsWorld::PhysicsWorld()
	{
		m_CollisionConfiguration = new btDefaultCollisionConfiguration();

		m_Dispatcher = new btCollisionDispatcher(m_CollisionConfiguration);
		m_OverlappingPairCache = new btDbvtBroadphase();
		m_Solver = new btSequentialImpulseConstraintSolver();

		m_DynamicsWorld = new btDiscreteDynamicsWorld(m_Dispatcher, m_OverlappingPairCache, m_Solver, m_CollisionConfiguration);
		m_DynamicsWorld->setGravity({ 0.0f, -9.8f, 0.0f });
	}

	PhysicsWorld::~PhysicsWorld()
	{
		delete m_DebugDrawer;

		for (int i = m_DynamicsWorld->getNumConstraints() - 1; i >= 0; i--)
		{
			btTypedConstraint* constraint = m_DynamicsWorld->getConstraint(i);
			m_DynamicsWorld->removeConstraint(constraint);
			delete constraint;
		}

		for (int i = m_DynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			btCollisionObject* obj = m_DynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
			{
				delete body->getMotionState();
			}
			m_DynamicsWorld->removeCollisionObject(obj);
			delete obj;
		}

		for (int j = 0; j < m_CollisionShapes.size(); j++)
		{
			const btCollisionShape* shape = m_CollisionShapes[j];
			delete shape;
		}

		delete m_DynamicsWorld;
		delete m_Solver;
		delete m_OverlappingPairCache;
		delete m_Dispatcher;
		delete m_CollisionConfiguration;
	}

	void PhysicsWorld::SetupDebugDrawer(gef::Renderer3D* renderer3D)
	{
		m_DebugDrawer = new GEFDebugDrawer(renderer3D);
		m_DebugDrawer->setDebugMode(btIDebugDraw::DBG_DrawAabb | btIDebugDraw::DBG_DrawFrames);

		m_DynamicsWorld->setDebugDrawer(m_DebugDrawer);
	}



	void PhysicsWorld::Tick(float frame_time)
	{
		m_Timer += frame_time;
		if (m_Timer > g_FixedTimeStep)
		{
			m_Timer -= g_FixedTimeStep;
			m_DynamicsWorld->stepSimulation(g_FixedTimeStep, 1);
		}
	}

	void PhysicsWorld::DebugDraw() const
	{
		if (m_DebugDrawer)
		{
			m_DynamicsWorld->debugDrawWorld();
		}
	}

	void PhysicsWorld::AddRigidBody(btRigidBody* rb) const
	{
		m_DynamicsWorld->addRigidBody(rb);
	}

	btRigidBody* PhysicsWorld::CreateGround()
	{
		const btVector3 groundHalfExtents(5.0f, 0.01f, 5.0f);
		btCollisionShape* groundShape = new btBoxShape(groundHalfExtents);

		m_CollisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -groundHalfExtents.y(), 0));

		const auto motionState = new btDefaultMotionState(groundTransform);

		const btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, groundShape, { 0.0f, 0.0f, 0.0f });
		const auto body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		m_DynamicsWorld->addRigidBody(body);
		return body;
	}

}
