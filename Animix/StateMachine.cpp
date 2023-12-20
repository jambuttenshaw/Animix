#include "StateMachine.h"

#include <cassert>

#include "AnimationEngine.h"


namespace Animix
{

	TransitionType StateTransition::TransitionTypeFromString(const std::string& name)
	{
		static std::map<std::string, TransitionType> s_Types{
			{ "immediate", TransitionType::Immediate },
			{ "smooth", TransitionType::Smooth },
			{ "frozen", TransitionType::Frozen }
		};

		if (s_Types.find(name) == s_Types.end())
			return TransitionType::Immediate;
		return s_Types.at(name);
	}



	AnimatorState::AnimatorState(Animator* owner, std::string name)
		: m_Owner(owner)
		, m_Name(std::move(name))
	{
		assert(m_Owner);
		m_BlendTree = std::make_unique<BlendTree>();
	}

	bool AnimatorState::EvaluateBlendTree(SkeletonPose& outPose) const
	{
		return m_BlendTree->TickAndEvaluateTree(outPose);
	}

	
	void AnimatorState::AddTransition(const std::string& name, StateTransition&& transition)
	{
		if (m_Transitions.find(name) == m_Transitions.end())
			m_Transitions.emplace(name, std::move(transition));
	}

	bool AnimatorState::HasTransition(const std::string& transitionName)
	{
		return m_Transitions.find(transitionName) != m_Transitions.end();
	}

	const StateTransition& AnimatorState::GetTransition(const std::string& transitionName) const
	{
		return m_Transitions.at(transitionName);
	}


	void AnimatorState::SetEndTransition(const std::string& name)
	{
		if (m_Transitions.find(name) != m_Transitions.end())
		{
			m_HasEndTransition = true;
			m_EndTransition = name;
		}
	}
}
