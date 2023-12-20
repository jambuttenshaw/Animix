#pragma once

#include "BlendNode.h"


namespace Animix
{

	class BilinearBlendNode : public BlendNode
	{
	public:
		BilinearBlendNode(BlendTree* tree, size_t index);
		~BilinearBlendNode() override = default;

		// Disallow copying
		BilinearBlendNode(const BilinearBlendNode&) = delete;
		BilinearBlendNode& operator=(const BilinearBlendNode&) = delete;

		// Default moving
		BilinearBlendNode(BilinearBlendNode&&) = default;
		BilinearBlendNode& operator=(BilinearBlendNode&&) = default;


		// Implement BlendNode interface
		virtual bool IsValid() const override;
		virtual void Tick(float timeScale) override;
		virtual SkeletonPose Evaluate() const override;

		virtual float CalculateDuration() const override;
		virtual bool IsLooping() const override;

		virtual bool HasVariableWithName(const std::string& name) override;
		virtual ParameterObserver GetObserverForVariable(const std::string& name) override;

		// Methods for this type of node

		// These setters are to be used on construction of the blend tree
		// After tree construction, the node will be buried within the tree and difficult to access
		// During game play, the parameter table and observer system should be used to manipulate node properties
		inline void SetScaleClips(bool scaleClips) { m_ScaleClips = scaleClips; }
		inline void SetAlpha(float alpha) { m_Alpha = alpha; }
		inline void SetBeta(float beta) { m_Beta = beta; }

		// Getters for observers for node parameters
		inline ParameterObserver GetParamObserver_Alpha()
		{
			return[this](float alpha) { this->m_Alpha = alpha; };
		}
		inline ParameterObserver GetParamObserver_Beta()
		{
			return[this](float beta) { this->m_Beta = beta; };
		}

	private:
		static float Lerp(float a, float b, float t);

	protected:
		// Additional parameters required by this node

		// Should this node scale its inputs to make them both the same duration before blending them?
		bool m_ScaleClips = true;

		// The blending parameters
		float m_Alpha = 0.0f;
		float m_Beta = 0.0f;
	};
}
