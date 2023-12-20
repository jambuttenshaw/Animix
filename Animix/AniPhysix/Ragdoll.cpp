#include "Ragdoll.h"

#include <cassert>
#include <d3d10.h>

#include "Animix/Skeleton.h"

#include "btBulletDynamicsCommon.h"
#include "btBulletWorldImporter.h"
#include "PhysicsWorld.h"

#include "Animix/AnimationEngine.h"
#include "Utility.h"


namespace AniPhysix
{

	Ragdoll::Ragdoll(Animix::SkeletonPose bindPose, btDiscreteDynamicsWorld* dynamicsWorld, const char* physicsFilename)
		: m_BindPose(std::move(bindPose))
		, m_World(dynamicsWorld)
	{
		const auto worldImporter = new btBulletWorldImporter(m_World);

		const bool success = worldImporter->loadFile(physicsFilename);
		assert(success && "Failed to load ragdoll from file!");

		assert(Animix::g_AnimixEngine && "Cannot create ragdoll without animation engine!");
		const auto skeleton = Animix::g_AnimixEngine->GetSkeleton(m_BindPose.SkID);

		// accelerate construction by mapping all bones to their names
		std::unordered_map<gef::StringId, size_t> jointIndices;
		for (size_t i = 0; i < skeleton->Joints.size(); i++)
			jointIndices.insert({ skeleton->Joints[i].Name, i});

		for (int i = 0; i < worldImporter->getNumRigidBodies(); i++)
		{
			const auto body = btRigidBody::upcast(worldImporter->getRigidBodyByIndex(i));

			std::string rbName = worldImporter->getNameForPointer(body);
			// trim blender object name to match skeleton
			constexpr size_t prefixLen = sizeof("OBArmature_");
			constexpr size_t postfixLen = sizeof("_hitbox");
			rbName = rbName.substr(prefixLen - 1, rbName.length() - prefixLen - postfixLen + 2);

			// Find the bone in the skeleton that this rigid body should attach to
			const gef::StringId jointNameId = gef::GetStringId(rbName);
			if (!jointNameId || jointIndices.find(jointNameId) == jointIndices.end())
				continue;

			const size_t jointIndex = jointIndices.at(jointNameId);

			// Calculate offset transform
			const gef::Matrix44 offset = TransformToMatrix(body->getCenterOfMassTransform()) * skeleton->Joints[jointIndex].InvBindPose;
			gef::Matrix44 invOffset;
			invOffset.Inverse(offset, nullptr);

			m_RigidBodies.insert(std::make_pair(jointIndex, RagdollJoint {
				body,
				offset,
				invOffset
			}));
		}

		// Need to store all the constraints in use for this ragdoll for cleanup purposes
		for (int i = 0; i < worldImporter->getNumConstraints(); i++)
			m_Constraints.push_back(worldImporter->getConstraintByIndex(i));

		delete worldImporter;
	}

	Ragdoll::~Ragdoll()
	{
		// delete constraints from physics world
		for (const auto& constraint : m_Constraints)
		{
			m_World->removeConstraint(constraint);
			delete constraint;
		}

		// delete rigid bodies from physics world
		for (const auto& rb : m_RigidBodies)
		{
			m_World->removeRigidBody(rb.second.RB);
			delete rb.second.RB;
		}
	}

	Animix::SkeletonPose Ragdoll::CreatePoseFromSimulation() const
	{
		Animix::SkeletonPose outPose(m_BindPose.SkID);

		for (size_t index = 0; index < outPose.GlobalPose.size(); index++)
		{
			// this joint has a rigid body
			if (m_RigidBodies.find(index) != m_RigidBodies.end())
			{
				const auto rb = m_RigidBodies.at(index);
				outPose.GlobalPose[index] = rb.InvOffsetMatrix * TransformToMatrix(rb.RB->getCenterOfMassTransform());
			}
			else
			{
				// this joint does not have a rigid body, it should just use bind transform pose
				outPose.GlobalPose[index] = m_BindPose.GlobalPose[index];
			}
		}

		return outPose;
	}

	void Ragdoll::MatchPose(const Animix::SkeletonPose& pose) const
	{
		for (auto& rigidBodyJoint : m_RigidBodies)
		{
			const auto rb = rigidBodyJoint.second.RB;

			const btTransform prev = rb->getCenterOfMassTransform();
			const btTransform cur = MatrixToTransform(
				rigidBodyJoint.second.OffsetMatrix * pose.GlobalPose[rigidBodyJoint.first]
			);

			rb->setCenterOfMassTransform(cur);

			// estimate linear and angular velocity from change in transform
			// get the difference transform between them
			const btTransform diff = prev.inverse() * cur;

			rb->setLinearVelocity(diff.getOrigin() / PhysicsWorld::GetTimeStep());
			btScalar aX, aY, aZ;
			diff.getRotation().getEulerZYX(aZ, aY, aX);
			rb->setAngularVelocity({
				aX / PhysicsWorld::GetTimeStep(),
				aY / PhysicsWorld::GetTimeStep(),
				aZ / PhysicsWorld::GetTimeStep()
			});
		}
	}
}
