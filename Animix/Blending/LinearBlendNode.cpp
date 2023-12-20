#include "LinearBlendNode.h"

#include "BlendTree.h"


namespace Animix
{

	LinearBlendNode::LinearBlendNode(BlendTree* tree, size_t index)
		: BlendNode(tree, index)
	{
		// This node can have 2 children
		m_Inputs.resize(2, -1);
	}

	bool LinearBlendNode::IsValid() const
	{
		// Validate parameters

		// Validate children
		if (!(m_Tree->DoesNodeExist(m_Inputs.at(0))
			&& m_Tree->DoesNodeExist(m_Inputs.at(1))))
			return false;

		if (!(GetInputNode(0)->IsValid()
			&& GetInputNode(1)->IsValid()))
			return false;

		if (m_ScaleClips)
		{
			if (GetInputNode(0)->CalculateDuration() == 0.0f ||
				GetInputNode(1)->CalculateDuration() == 0.0f)
				return false;
		}

		return true;
	}

	void LinearBlendNode::Tick(float timeScale)
	{
		float in0_scale = 1.0f;
		float in1_scale = 1.0f;
		// Check for clip scaling
		if (m_ScaleClips)
		{
			const float dur0 = GetInputNode(0)->CalculateDuration();
			const float dur1 = GetInputNode(1)->CalculateDuration();
			const float targetDuration = (1.0f - m_Alpha) * dur1 + m_Alpha * dur0;

			// Calculate scale that will make both clips the same duration
			in0_scale = targetDuration / dur1;
			in1_scale = targetDuration / dur0;
		}

		GetInputNode(0)->Tick(in0_scale * timeScale);
		GetInputNode(1)->Tick(in1_scale * timeScale);
	}

	SkeletonPose LinearBlendNode::Evaluate() const
	{
		// Perform blending
		const SkeletonPose pose1(GetInputNode(0)->Evaluate());
		const SkeletonPose pose2(GetInputNode(1)->Evaluate());

		return SkeletonPose::Lerp(pose1, pose2, m_Alpha);
	}

	float LinearBlendNode::CalculateDuration() const
	{
		return std::max(GetInputNode(0)->CalculateDuration(), GetInputNode(1)->CalculateDuration());
	}

	bool LinearBlendNode::IsLooping() const
	{
		return GetInputNode(0)->IsLooping() && GetInputNode(1)->IsLooping();
	}

	bool LinearBlendNode::HasVariableWithName(const std::string& name)
	{
		if (name == "alpha")
			return true;

		return false;
	}

	ParameterObserver LinearBlendNode::GetObserverForVariable(const std::string& name)
	{
		if (name == "alpha")
			return GetParamObserver_Alpha();

		return {};
	}

}
