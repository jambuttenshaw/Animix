#include "Skeleton.h"

#include <cassert>

#include "AnimationEngine.h"


namespace Animix
{
	SkeletonPose::SkeletonPose(SkeletonID skeleton)
		: SkID(skeleton)
	{
		const auto sk = g_AnimixEngine->GetSkeleton(SkID);
		LocalPose.resize(sk->Joints.size());
		GlobalPose.resize(sk->Joints.size());
	}


	void SkeletonPose::BuildGlobalPose()
	{
		const auto skeleton = g_AnimixEngine->GetSkeleton(SkID);

		// Assumes that the given local pose is valid
		for (size_t jointIndex = 0; jointIndex < skeleton->Joints.size(); jointIndex++)
		{
			const int32_t parent = skeleton->Joints[jointIndex].Parent;
			gef::Matrix44& globalPose = GlobalPose[jointIndex];

			auto& t = LocalPose[jointIndex];

			globalPose.SetIdentity();
			globalPose.Rotation(t.Q);
			globalPose.SetTranslation(t.P.ToVector4());

			if (parent != -1)
				globalPose = globalPose * GlobalPose[parent];
		}
	}

	void SkeletonPose::RecoverLocalPoseFromGlobal()
	{
		// Global pose must be valid for this to produce a valid local pose
		// Avoid this operation where possible; it is not cheap
		const auto skeleton = g_AnimixEngine->GetSkeleton(SkID);

		// Assumes that the given local pose is valid
		for (size_t jointIndex = 0; jointIndex < skeleton->Joints.size(); jointIndex++)
		{
			const int32_t parent = skeleton->Joints[jointIndex].Parent;
			gef::Matrix44 local;

			if (parent == -1)
			{
				local = GlobalPose[jointIndex];
			}
			else
			{
				gef::Matrix44 invParent;
				invParent.Inverse(GlobalPose[parent]);
				local = GlobalPose[jointIndex] * invParent;
			}

			auto& localPose = LocalPose[jointIndex];
			localPose.P = { local.GetTranslation().x(), local.GetTranslation().y(), local.GetTranslation().z()};
			localPose.Q = gef::Quaternion(local);
		}
	}


	void SkeletonPose::BuildBindPose()
	{
		const auto skeleton = g_AnimixEngine->GetSkeleton(SkID);

		// Assumes that the given local pose is valid
		for (size_t jointIndex = 0; jointIndex < skeleton->Joints.size(); jointIndex++)
		{
			const Joint& joint = skeleton->Joints[jointIndex];
			GlobalPose[jointIndex].Inverse(joint.InvBindPose);
		}
	}

	SkeletonPose SkeletonPose::Lerp(const SkeletonPose& pose1, const SkeletonPose& pose2, float t)
	{
		// Otherwise perform blending
		assert(pose1.SkID == pose2.SkID);
		SkeletonPose out(pose1.SkID);

		// LinearBlend local poses
		for (size_t j = 0; j < out.LocalPose.size(); j++)
		{
			// Perform a linear blend between this joint in pose1 and pose2
			const JointTransform& j1 = pose1.LocalPose[j];
			const JointTransform& j2 = pose2.LocalPose[j];
			JointTransform& jo = out.LocalPose[j];

			jo.P = Vector3::Lerp(j1.P, j2.P, t);
			jo.Q.Slerp(j1.Q, j2.Q, t);
		}

		return out;
	}
}
