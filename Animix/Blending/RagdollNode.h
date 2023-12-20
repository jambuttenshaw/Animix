#pragma once


#include "BlendNode.h"


namespace AniPhysix
{
	class Ragdoll;
}


namespace Animix
{

	class RagdollNode : public BlendNode
	{
	public:
		RagdollNode(BlendTree* tree, size_t index);
		virtual ~RagdollNode() override = default;

		// Disallow copying
		RagdollNode(const RagdollNode&) = delete;
		RagdollNode& operator=(const RagdollNode&) = delete;

		// Default moving
		RagdollNode(RagdollNode&&) = default;
		RagdollNode& operator=(RagdollNode&&) = default;

		// Implement BlendNode interface
		virtual bool IsValid() const override;
		virtual void Tick(float timeScale) override;
		virtual SkeletonPose Evaluate() const override;

		virtual bool SetInput(size_t inputIndex, BlendNodeID inputNode) override;

		// Methods for this type of node

		// These setters are to be used on construction of the blend tree
		// After tree construction, the node will be buried within the tree and difficult to access
		// During game play, the parameter table and observer system should be used to manipulate node properties
		void SetRagdoll(AniPhysix::Ragdoll* ragdoll) { m_Ragdoll = ragdoll; }

	private:
		AniPhysix::Ragdoll* m_Ragdoll = nullptr;
	};
}
