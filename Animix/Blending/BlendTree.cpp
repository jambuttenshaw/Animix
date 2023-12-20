#include "BlendTree.h"

#include "Animix/AnimationEngine.h"


namespace Animix
{
	bool BlendTree::TickAndEvaluateTree(SkeletonPose& outPose, float timeScale) const
	{
		if (m_BlendTree.empty() || !Root()->IsValid())
			return false;

		// Tick all nodes in the tree
		Root()->Tick(timeScale);
		// Evaluate the pose of the tree
		outPose = Root()->Evaluate();
		return true;
	}

	float BlendTree::CalculateRemainingDuration() const
	{
		// Calculate if the animation of this tree has completed
		const float duration = CalculateDuration();
		return duration - (g_AnimixEngine->GetGlobalTime() - m_StartTime);
	}

	float BlendTree::CalculateDuration() const
	{
		return Root()->CalculateDuration();
	}


	void BlendTree::Start()
	{
		m_StartTime = g_AnimixEngine->GetGlobalTime();
		for (const auto& node : m_BlendTree)
		{
			if (node) node->Begin();
		}
	}


	bool BlendTree::DoesNodeExist(BlendNodeID index) const
	{
		if (index >= m_BlendTree.size())
			return false;

		// Check if the unique_ptr is actually pointing to an object
		return m_BlendTree.at(index) != nullptr;
	}

	BlendNode* BlendTree::GetNode(BlendNodeID index) const
	{
		return m_BlendTree.at(index).get();
	}

	bool BlendTree::SetOutputNode(BlendNodeID index)
	{
		if (!DoesNodeExist(index))
			return false;

		m_OutputNode = index;
		return true;
	}
}
