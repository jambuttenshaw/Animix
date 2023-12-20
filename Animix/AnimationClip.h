#pragma once

#include <vector>

#include "Skeleton.h"
#include "Vector3.h"
#include "maths/quaternion.h"


namespace Animix
{
	// Forward declarations
	struct SkeletonPose;


	// Animation key types
	struct PositionKey
	{
		float StartTime;
		Vector3 Value;
	};

	struct RotationKey
	{
		float StartTime;
		gef::Quaternion Value;
	};

	struct FloatKey
	{
		float StartTime;
		float Value;
	};

	/**
	 * All of the animation data for a single joint
	 */
	struct JointSamples
	{
		std::vector<PositionKey> PositionKeys;
		std::vector<RotationKey> RotationKeys;
	};


	class AnimationClip
	{
	public:
		AnimationClip(SkeletonID target);

		void BuildLocalPose(float time, SkeletonPose& outPose) const;

		// Getters
		inline SkeletonID GetTarget() const { return m_Target; }
		inline float GetDuration() const { return m_Duration; }

		// Setters; only to be used in constructing the animation
		inline void SetDuration(float duration) { m_Duration = duration; }
		inline void SetJointSamples(size_t jointIndex, JointSamples&& samples) { m_JointSamples[jointIndex] = std::move(samples); }

	private:
		// Animation clips are made for a particular skeleton
		SkeletonID m_Target;

		// Animation data
		float m_Duration = 0.0f;
		std::vector<JointSamples> m_JointSamples;
	};

}
