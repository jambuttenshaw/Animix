#include "Animator.h"

#include <cassert>

#include "AnimationClip.h"
#include "AnimationEngine.h"
#include "AnimixLoader.h"
#include "Skeleton.h"

namespace Animix
{
	Animator::Animator(SkeletonID target)
		: m_Target(target)
		, m_BindPose(target)
	{
		m_ParameterTable = std::make_unique<ParameterTable>();

		// Set matrix palette to current size
		const auto sk = g_AnimixEngine->GetSkeleton(m_Target);
		m_MatrixPalette.resize(sk->Joints.size());

		m_BindPose.BuildBindPose();
		BuildMatrixPalette(m_BindPose);
	}

	void Animator::Clear()
	{
		// Release all state machine resources
		m_States.clear();

		m_CurrentState = nullptr;
		m_NextState = nullptr;

		m_ParameterTable->Clear();
	}

	bool Animator::LoadFromJSON(const std::string& filename)
	{
		std::string prevState;
		if (m_CurrentState)
			prevState = m_CurrentState->GetName();

		Clear();

		const bool success = AnimixLoader::LoadAnimatorFromJSON(*this, filename);
		if (!success)
			Clear();
		else
		{
			if (m_States.find(prevState) != m_States.end())
				m_CurrentState = m_States.at(prevState).get();
		}

		return success;
	}

	void Animator::UpdatePose()
	{
		if (m_States.empty() || !m_CurrentState)
			// Nothing to animate
			return;

		SkeletonPose blendedPose{ m_BindPose };

		// Frozen transitions should not progress time in the current state
		const float freezeTime = m_NextState && m_TransitionType == TransitionType::Frozen ? 0.0f : 1.0f;

		bool blendsValid = m_CurrentState->GetBlendTree()->TickAndEvaluateTree(blendedPose, freezeTime);

		// also handle transitions
		if (m_NextState)
		{
			SkeletonPose nextBlendedPose{ m_Target };
			blendsValid &= m_NextState->GetBlendTree()->TickAndEvaluateTree(nextBlendedPose);

			// Calculate progress through the transition
			const float t = (g_AnimixEngine->GetGlobalTime() - m_NextState->GetBlendTree()->GetStartTime()) / m_TransitionDuration;

			blendedPose = SkeletonPose::Lerp(blendedPose, nextBlendedPose, t);

			// is transition complete?
			if (t >= 1.0f)
			{
				m_CurrentState = m_NextState;
				m_NextState = nullptr;
			}
		}
		else if (m_CurrentState->HasEndTransition())
		{
			// Check for end of state transition
			const float remainingDuration = m_CurrentState->GetBlendTree()->CalculateRemainingDuration();

			const std::string& endTransition = m_CurrentState->GetEndTransition();
			const float transitionDuration = m_TransitionType == TransitionType::Frozen ? 0.0f : m_CurrentState->GetTransition(endTransition).Duration;

			if (remainingDuration <= transitionDuration)
			{
				// Transition to the next state if it has one
				Transition(endTransition);
			}
		}

		
		if (blendsValid)
		{
			blendedPose.BuildGlobalPose();
		}

		if (m_Ragdoll)
		{
			// Check if the ragdoll has been sampled this frame
			if (m_Ragdoll->IsDirty())
			{
				m_Ragdoll->SetDirty(false);
			}
			else
			{
				// ragdoll is not currently being sampled; match its pose to the skeleton
				m_Ragdoll->MatchPose(blendedPose);
			}
		}
		
		// If blend tree was not valid, blendedPose will still contain bind pose
		// Therefore do not need to re-calculate the global pose prior to constructing matrix palette
		BuildMatrixPalette(blendedPose);
	}

	void Animator::BuildMatrixPalette(const SkeletonPose& globalPose)
	{
		const auto skeleton = g_AnimixEngine->GetSkeleton(m_Target);
		const auto& joints = skeleton->Joints;

		for (size_t joint = 0; joint < skeleton->Joints.size(); joint++)
		{
			m_MatrixPalette[joint] = joints[joint].InvBindPose * globalPose.GlobalPose[joint];
		}
	}

	AnimatorState* Animator::CreateState(const std::string& name)
	{
		if (m_States.find(name) == m_States.end())
			m_States.emplace(name, std::make_unique<AnimatorState>(this, name));

		if (!m_CurrentState)
			m_CurrentState = m_States.at(name).get();

		return m_States.at(name).get();
	}

	AnimatorState* Animator::GetState(const std::string& name) const
	{
		assert(m_States.find(name) != m_States.end());
		return m_States.at(name).get();
	}

	bool Animator::Transition(const std::string& transitionName)
	{
		// Check if a transition is already in progress
		if (m_NextState)
			return false;

		if (m_CurrentState)
		{
			// Check if this is a defined transition
			if (m_CurrentState->HasTransition(transitionName))
				return BeginTransitionInternal(transitionName);
		}

		return false;
	}

	bool Animator::BeginTransitionInternal(const std::string& transitionName)
	{
		const StateTransition& transition = m_CurrentState->GetTransition(transitionName);

		// Check the destination state exists
		if (m_States.find(transition.DestinationState) == m_States.end())
			return false;

		// The action to take depends on the transition type
		switch (transition.Type)
		{
		case TransitionType::Immediate:
			{
				m_CurrentState = m_States.at(transition.DestinationState).get();
				// Call begin on the blend tree to get it ready to play its animation
				m_CurrentState->GetBlendTree()->Start();

				return true;
			}
		case TransitionType::Smooth:
			{
				m_NextState = m_States.at(transition.DestinationState).get();
				m_TransitionDuration = transition.Duration;
				// Call begin on the blend tree to get it ready to play its animation
				m_NextState->GetBlendTree()->Start();
				m_TransitionType = TransitionType::Smooth;
				return true;
			}
		case TransitionType::Frozen:
			{
				m_NextState = m_States.at(transition.DestinationState).get();
				m_TransitionDuration = transition.Duration;
				// Call begin on the blend tree to get it ready to play its animation
				m_NextState->GetBlendTree()->Start();
				m_TransitionType = TransitionType::Frozen;
				return true;
			}
		}

		return false;
	}


	// Physics based animation
	void Animator::CreateRagdoll(btDiscreteDynamicsWorld* dynamicsWorld, const char* physicsFilename)
	{
		m_Ragdoll = std::make_unique<AniPhysix::Ragdoll>(m_BindPose, dynamicsWorld, physicsFilename);
	}
}
