#include "RagdollNode.h"

#include "Animix/AniPhysix/Ragdoll.h"


namespace Animix
{

	RagdollNode::RagdollNode(BlendTree* tree, size_t index)
		: BlendNode(tree, index)
	{
	}

	bool RagdollNode::IsValid() const
	{
		return m_Ragdoll != nullptr;
	}

	void RagdollNode::Tick(float timeScale)
	{
		m_Ragdoll->SetDirty(true);
	}

	SkeletonPose RagdollNode::Evaluate() const
	{
		SkeletonPose pose = m_Ragdoll->CreatePoseFromSimulation();
		pose.RecoverLocalPoseFromGlobal();
		return pose;
	}

	bool RagdollNode::SetInput(size_t inputIndex, BlendNodeID inputNode)
	{
		// This node has no inputs
		return false;
	}

}
