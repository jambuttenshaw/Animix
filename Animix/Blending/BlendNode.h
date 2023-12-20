#pragma once


#include "../AnimixTypes.h"
#include "../Skeleton.h"


namespace Animix
{
	// Forward declarations
	class AnimationClip;
	class BlendTree;

	/**
	* Interface any blend node must implement
	*/
	class BlendNode
	{
	public:
		BlendNode(BlendTree* tree, size_t index);
		virtual ~BlendNode() = default;

		// Disallow copying
		BlendNode(const BlendNode&) = delete;
		BlendNode& operator=(const BlendNode&) = delete;

		// Default moving
		BlendNode(BlendNode&&) = default;
		BlendNode& operator=(BlendNode&&) = default;


		virtual bool IsValid() const = 0;
		// Scale is an optional float that will scale the clips local timeline
		// which is helpful for synchronizing animations
		virtual void Tick(float timeScale = 1.0f) = 0;
		virtual SkeletonPose Evaluate() const = 0;

		virtual float CalculateDuration() const { return 0.0f; }
		virtual bool IsLooping() const { return false; }

		// For connecting variables to parameters
		// used by JSON loader
		virtual bool HasVariableWithName(const std::string& name) { return false; }
		virtual ParameterObserver GetObserverForVariable(const std::string& name) { return {}; }

		// Virtual to allow nodes to modify /verify behaviour
		// Eg Disallow adding children to some nodes (leaves)
		// or set a maximum limit on the number of children
		virtual bool SetInput(size_t inputIndex, BlendNodeID inputNode);

		// Allow nodes to respond to the animation state beginning
		virtual void Begin() {}

		// Getters
		inline BlendTree* GetBlendTree() const { return m_Tree; }
		inline BlendNodeID GetNodeID() const { return m_TreeIndex; }

		inline size_t GetInputCount() const { return m_Inputs.size(); }
		BlendNode* GetInputNode(size_t index) const;

	protected:
		// The tree that this node is a part of
		BlendTree* m_Tree = nullptr;
		// The index in the tree that this node lives
		size_t m_TreeIndex = -1;

		std::vector<size_t> m_Inputs;
	};
}
