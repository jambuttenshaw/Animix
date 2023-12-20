#pragma once
#include <unordered_map>

#include "Blending/BlendTree.h"


namespace Animix
{
	// Forward declarations
	class Animator;


	enum class TransitionType
	{
		Immediate,
		Frozen,
		Smooth
	};


	struct StateTransition
	{
		std::string DestinationState;
		TransitionType Type = TransitionType::Immediate;
		float Duration = 0.0f;

		static TransitionType TransitionTypeFromString(const std::string& name);
	};

	/*
	 * This class represents a single state in the state machine
	 * A state has its own blend tree that determines the output of the state
	 */
	class AnimatorState
	{
	public:
		AnimatorState(Animator* owner, std::string name);

		// Evaluate the state
		bool EvaluateBlendTree(SkeletonPose& outPose) const;

		// Get/Manipulate transitions
		void AddTransition(const std::string& name, StateTransition&& transition);
		bool HasTransition(const std::string& transitionName);
		const StateTransition& GetTransition(const std::string& transitionName) const;

		void SetEndTransition(const std::string& name);
		inline bool HasEndTransition() const { return m_HasEndTransition; }
		inline const std::string& GetEndTransition() const { return m_EndTransition; }

		// Getters
		inline Animator* GetOwner() const { return m_Owner; }
		inline const std::string& GetName() const { return m_Name; }
		inline BlendTree* GetBlendTree() const { return m_BlendTree.get(); }

	private:
		// A pointer to the owning state machine
		Animator* m_Owner = nullptr;
		std::string m_Name;

		std::unique_ptr<BlendTree> m_BlendTree;
		// Map of transition names to transitions
		// Checking if a transition exists is a common operation, so it should be as fast as possible
		std::unordered_map<std::string, StateTransition> m_Transitions;

		// What transition should occur when this state finishes playing
		// (only applies to non-looping states)
		bool m_HasEndTransition = false;
		std::string m_EndTransition;
	};
}
