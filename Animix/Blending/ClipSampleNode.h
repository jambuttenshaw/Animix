#pragma once

#include "BlendNode.h"
#include "../ClipSampler.h"


namespace Animix
{

	class ClipSampleNode : public BlendNode
	{
	public:
		ClipSampleNode(BlendTree* tree, size_t index);
		virtual ~ClipSampleNode() override = default;

		// Disallow copying
		ClipSampleNode(const ClipSampleNode&) = delete;
		ClipSampleNode& operator=(const ClipSampleNode&) = delete;

		// Default moving
		ClipSampleNode(ClipSampleNode&&) = default;
		ClipSampleNode& operator=(ClipSampleNode&&) = default;

		// Implement BlendNode interface
		virtual bool IsValid() const override;
		virtual void Tick(float timeScale) override;
		virtual SkeletonPose Evaluate() const override;

		virtual float CalculateDuration() const override;
		virtual bool IsLooping() const override;

		virtual bool HasVariableWithName(const std::string& name) override;
		virtual ParameterObserver GetObserverForVariable(const std::string& name) override;

		virtual bool SetInput(size_t inputIndex, BlendNodeID inputNode) override;

		inline virtual void Begin() override { m_Sampler.PlayFromStart(); }

		// Methods for this type of node

		// These setters are to be used on construction of the blend tree
		// After tree construction, the node will be buried within the tree and difficult to access
		// During game play, the parameter table and observer system should be used to manipulate node properties
		bool SetClip(const std::string& animName);
		inline void SetLooping(bool looping) { m_Sampler.SetLooping(looping); }
		inline void SetPlaybackSpeed(float speed) { m_Sampler.SetPlaybackSpeed(speed); }

		// Getters for observers for node parameters
		inline ParameterObserver GetParamObserver_PlaybackSpeed()
		{
			return[this](float speed) { this->m_Sampler.SetPlaybackSpeed(speed); };
		}

	protected:
		// Additional parameters required by this node

		// The clip that will be sampled by this node
		const AnimationClip* m_Clip = nullptr;
		ClipSampler m_Sampler;
	};
}
