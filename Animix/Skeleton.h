#pragma once

#include <string>
#include <vector>

#include "Vector3.h"
#include "maths/matrix44.h"
#include "maths/quaternion.h"
#include "system/string_id.h"


namespace Animix
{
	using SkeletonID = uint8_t;
	constexpr size_t MAX_SKELETONS = UINT8_MAX;

	struct JointTransform
	{
		Vector3 P;
		gef::Quaternion Q;
	};

	struct Joint
	{
		gef::StringId Name;
		int32_t Parent = -1;
		gef::Matrix44 InvBindPose;
	};

	struct SkeletonPose
	{
		SkeletonID SkID = -1;
		std::vector<JointTransform> LocalPose;
		std::vector<gef::Matrix44> GlobalPose;


		// Constructor
		SkeletonPose(SkeletonID skeleton);

		void BuildGlobalPose();
		void RecoverLocalPoseFromGlobal();
		void BuildBindPose();

		static SkeletonPose Lerp(const SkeletonPose& pose1, const SkeletonPose& pose2, float t);
	};

	struct Skeleton
	{
		SkeletonID ID;
		std::vector<Joint> Joints;
	};
}
