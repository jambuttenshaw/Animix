#include "AnimationClip.h"

#include "Skeleton.h"
#include "AnimationEngine.h"
#include "animation/skeleton.h"

namespace Animix
{

	AnimationClip::AnimationClip(SkeletonID target)
		: m_Target(target)
	{
		const auto skeleton = g_AnimixEngine->GetSkeleton(m_Target);
		m_JointSamples.resize(skeleton->Joints.size());
	}

	void AnimationClip::BuildLocalPose(float time, SkeletonPose& outPose) const
	{
		// Get the skeleton from the animation engine
		const auto skeleton = g_AnimixEngine->GetSkeleton(m_Target);

		// Iterate over each joint
		for (size_t joint = 0; joint < skeleton->Joints.size(); joint++)
		{
			const JointSamples& samples = m_JointSamples[joint];

			Vector3 posePosition;
			gef::Quaternion poseRotation;

			if (!samples.PositionKeys.empty())
			{
				// Handle position first
				// Find the two keys surrounding the given time
				size_t keyIndex = 0;
				for (; keyIndex < samples.PositionKeys.size() - 1 &&
					time >= samples.PositionKeys[keyIndex + 1].StartTime;
					keyIndex++)
				{}

				const PositionKey& startKey = samples.PositionKeys[keyIndex];
				if (keyIndex == samples.PositionKeys.size() - 1)
				{
					posePosition = startKey.Value;
				}
				else
				{
					const PositionKey& endKey = samples.PositionKeys[keyIndex + 1];

					const float t = (time - startKey.StartTime) / (endKey.StartTime - startKey.StartTime);
					posePosition = Vector3::Lerp(startKey.Value, endKey.Value, t);
				}
			}

			if (!samples.RotationKeys.empty())
			{
				// Handle position first
				// Find the two keys surrounding the given time
				size_t keyIndex = 0;
				for (; keyIndex < samples.RotationKeys.size() - 1 &&
					time >= samples.RotationKeys[keyIndex + 1].StartTime;
					keyIndex++)
				{
				}

				const RotationKey& startKey = samples.RotationKeys[keyIndex];
				if (keyIndex == samples.RotationKeys.size() - 1)
				{
					poseRotation = startKey.Value;
				}
				else
				{
					const RotationKey& endKey = samples.RotationKeys[keyIndex + 1];

					const float t = (time - startKey.StartTime) / (endKey.StartTime - startKey.StartTime);
					poseRotation.Slerp(startKey.Value, endKey.Value, t);
				}
			}

			// Construct local pose matrix from position and rotation
			outPose.LocalPose[joint].P = posePosition;
			outPose.LocalPose[joint].Q = poseRotation;
		}
	}
}
