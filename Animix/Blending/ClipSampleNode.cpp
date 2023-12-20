#include "ClipSampleNode.h"

#include "Animix/AnimationEngine.h"


namespace Animix
{

	ClipSampleNode::ClipSampleNode(BlendTree* tree, size_t index)
		: BlendNode(tree, index)
		, m_Sampler(nullptr)
	{
	}

	bool ClipSampleNode::IsValid() const
	{
		// Sample node does not have any inputs, so no need to validate children

		// They do need to have a valid animation clip though
		return m_Clip != nullptr;
	}

	void ClipSampleNode::Tick(float timeScale)
	{
		m_Sampler.Tick(timeScale);
	}

	SkeletonPose ClipSampleNode::Evaluate() const
	{
		// Get local time of this clip
		SkeletonPose pose(m_Clip->GetTarget());

		m_Clip->BuildLocalPose(m_Sampler.GetCurrentSampleTime(), pose);
		return pose;
	}

	float ClipSampleNode::CalculateDuration() const
	{
		return m_Sampler.GetDuration();
	}

	bool ClipSampleNode::IsLooping() const
	{
		return m_Sampler.GetLooping();
	}

	bool ClipSampleNode::HasVariableWithName(const std::string& name)
	{
		// Clip sample node has only one variable that can be manipulated by parameters
		if (name == "playbackSpeed")
			return true;

		return false;
	}

	ParameterObserver ClipSampleNode::GetObserverForVariable(const std::string& name)
	{
		if (name == "playbackSpeed")
			return GetParamObserver_PlaybackSpeed();

		return {};
	}

	bool ClipSampleNode::SetInput(size_t inputIndex, BlendNodeID inputNode)
	{
		// Clip Sample nodes do not have any inputs, immediately return false
		return false;
	}

	bool ClipSampleNode::SetClip(const std::string& animName)
	{
		m_Clip = g_AnimixEngine->GetAnimationClip(animName);
		m_Sampler.SetClip(m_Clip);
		return m_Clip;
	}
}
