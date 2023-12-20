#pragma once

#include <unordered_map>

#include "Animix/Skeleton.h"
#include "maths/matrix44.h"


// Forward declarations
class btDiscreteDynamicsWorld;
class btRigidBody;
class btTypedConstraint;


namespace AniPhysix
{
	struct RagdollJoint
	{
		btRigidBody* RB;				// The physically simulated rigid body
		gef::Matrix44 OffsetMatrix;		// The matrix to transform from joint to RB
		gef::Matrix44 InvOffsetMatrix;
	};

	/*
	 * Loads a ragdoll from a physics file
	 */
	class Ragdoll
	{
	public:
		Ragdoll(Animix::SkeletonPose bindPose, btDiscreteDynamicsWorld* dynamicsWorld, const char* physicsFilename);
		~Ragdoll();

		Ragdoll(const Ragdoll&) = delete;
		Ragdoll& operator=(const Ragdoll&) = delete;

		Ragdoll(Ragdoll&&) = default;
		Ragdoll& operator=(Ragdoll&&) = default;


		// Creates a pose based upon the current position of the simulated rigid bodies
		// Joints not connected to a rigid body won't be manipulated, therefore pass bind pose into this function to see correct results
		Animix::SkeletonPose CreatePoseFromSimulation() const;
		// Positions the rigid bodies that make up this ragdoll to match the input pose
		void MatchPose(const Animix::SkeletonPose& pose) const;

		// Dirty flag
		inline bool IsDirty() const { return m_IsDirty; }
		inline void SetDirty(bool dirty) { m_IsDirty = dirty; }

	private:
		// The bind pose of the skeleton this ragdoll is created from
		Animix::SkeletonPose m_BindPose;

		// The physics world this ragdoll exists in
		btDiscreteDynamicsWorld* m_World = nullptr;

		// A map of joint index to ragdoll joint
		std::unordered_map<size_t, RagdollJoint> m_RigidBodies;
		std::vector<btTypedConstraint*> m_Constraints;

		// A flag to keep track of when this ragdoll is being used
		// This is so that when the ragdoll is not currently being sampled,
		// it can be made to match the current pose of the skeleton its attached to
		bool m_IsDirty = false;
	};
}
