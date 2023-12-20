#pragma once

#include <unordered_map>

#include "BlendNode.h"


namespace Animix
{
	/*
	 * A significantly more flexible linear blend node,
	 * that allows for any number of inputs to be placed anywhere along a 1D axis
	*/
	class GeneralLinearBlendNode : public BlendNode
	{
	public:
		GeneralLinearBlendNode(BlendTree* tree, size_t index);
		~GeneralLinearBlendNode() override = default;

		// Disallow copying
		GeneralLinearBlendNode(const GeneralLinearBlendNode&) = delete;
		GeneralLinearBlendNode& operator=(const GeneralLinearBlendNode&) = delete;

		// Default moving
		GeneralLinearBlendNode(GeneralLinearBlendNode&&) = default;
		GeneralLinearBlendNode& operator=(GeneralLinearBlendNode&&) = default;


		// Implement BlendNode interface
		virtual bool IsValid() const override;
		virtual void Tick(float timeScale) override;
		virtual SkeletonPose Evaluate() const override;

		virtual float CalculateDuration() const override;
		virtual bool IsLooping() const override;

		virtual bool HasVariableWithName(const std::string& name) override;
		virtual ParameterObserver GetObserverForVariable(const std::string& name) override;

		virtual bool SetInput(size_t inputIndex, BlendNodeID inputNode) override;

		// Methods for this type of node

		// These setters are to be used on construction of the blend tree
		// After tree construction, the node will be buried within the tree and difficult to access
		// During game play, the parameter table and observer system should be used to manipulate node properties
		inline void SetScaleClips(bool scaleClips) { m_ScaleClips = scaleClips; }
		void SetAlpha(float alpha);
		void SetAlphaForInput(size_t inputIndex, float alpha);

		// Getters for observers for node parameters
		inline ParameterObserver GetParamObserver_Alpha()
		{
			return[this](float alpha) { this->SetAlpha(alpha); };
		}

	protected:
		// Additional parameters required by this node

		// Should this node scale its inputs to make them both the same duration before blending them?
		bool m_ScaleClips = true;

		// The blending parameter
		float m_Alpha = 0.0f;
		// The amount to blend between the two currently active clips in the blend space
		float m_MappedAlpha = 0.0f;

		struct InputData
		{
			// Which input this data is pertinent to
			size_t InputIndex;
			// Where on the alpha scale the input nodes are placed
			// In a default linear blend, they are hard-defined to be at 0 and 1
			float Alpha;
		};
		std::vector<InputData> m_InputAlphas;

		// Which input nodes are currently being sampled
		size_t m_CurrentInA = 0;
		size_t m_CurrentInB = 1;
	};
}
