#include "BilinearBlendNode.h"

#include <algorithm>

#include "BlendTree.h"


namespace Animix
{
	BilinearBlendNode::BilinearBlendNode(BlendTree* tree, size_t index)
		: BlendNode(tree, index)
	{
		// Bilinear blend node has 4 inputs
		m_Inputs.resize(4, -1);
	}

	bool BilinearBlendNode::IsValid() const
	{
		for (size_t i = 0; i < 4; i++)
		{
			if (!m_Tree->DoesNodeExist(m_Inputs.at(i)))
				return false;
			if (!GetInputNode(i)->IsValid())
				return false;

			if (m_ScaleClips && GetInputNode(i)->CalculateDuration() == 0.0f)
				return false;
		}
		return true;
	}

	void BilinearBlendNode::Tick(float timeScale)
	{
		float in0_scale = 1.0f;
		float in1_scale = 1.0f;
		float in2_scale = 1.0f;
		float in3_scale = 1.0f;

		if (m_ScaleClips)
		{
			const float dur0 = GetInputNode(0)->CalculateDuration();
			const float dur1 = GetInputNode(1)->CalculateDuration();
			const float targetDuration0 = (1.0f - m_Alpha) * dur1 + m_Alpha * dur0;

			// Calculate scale that will make both clips the same duration
			in0_scale = targetDuration0 / dur1;
			in1_scale = targetDuration0 / dur0;

			const float dur2 = GetInputNode(2)->CalculateDuration();
			const float dur3 = GetInputNode(3)->CalculateDuration();
			const float targetDuration1 = (1.0f - m_Alpha) * dur3 + m_Alpha * dur2;

			// Calculate scale that will make both clips the same duration
			in2_scale = targetDuration1 / dur3;
			in3_scale = targetDuration1 / dur2;
		}

		GetInputNode(0)->Tick(in0_scale * timeScale);
		GetInputNode(1)->Tick(in1_scale * timeScale);
		GetInputNode(2)->Tick(in2_scale * timeScale);
		GetInputNode(3)->Tick(in3_scale * timeScale);
	}

	SkeletonPose BilinearBlendNode::Evaluate() const
	{
		// Perform blending
		const SkeletonPose pose1(GetInputNode(0)->Evaluate());
		const SkeletonPose pose2(GetInputNode(1)->Evaluate());
		const SkeletonPose pose3(GetInputNode(2)->Evaluate());
		const SkeletonPose pose4(GetInputNode(3)->Evaluate());

		return SkeletonPose::Lerp(
			SkeletonPose::Lerp(pose1, pose2, m_Alpha),
			SkeletonPose::Lerp(pose3, pose4, m_Alpha),
			m_Beta
		);
	}


	float BilinearBlendNode::CalculateDuration() const
	{
		return std::max(
			std::max(GetInputNode(0)->CalculateDuration(), GetInputNode(1)->CalculateDuration()),
			std::max(GetInputNode(2)->CalculateDuration(), GetInputNode(3)->CalculateDuration())
		);
	}

	bool BilinearBlendNode::IsLooping() const
	{
		return GetInputNode(0)->IsLooping() && GetInputNode(1)->IsLooping()
			&& GetInputNode(2)->IsLooping() && GetInputNode(3)->IsLooping();
	}

	bool BilinearBlendNode::HasVariableWithName(const std::string& name)
	{
		if (name == "alpha")
			return true;
		if (name == "beta")
			return true;

		return false;
	}

	ParameterObserver BilinearBlendNode::GetObserverForVariable(const std::string& name)
	{
		if (name == "alpha")
			return GetParamObserver_Alpha();
		if (name == "beta")
			return GetParamObserver_Beta();

		return {};
	}


	float BilinearBlendNode::Lerp(float a, float b, float t)
	{
		return (1.0f - t) * a + t * b;
	}


}
