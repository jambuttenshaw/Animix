#pragma once

#include <memory>
#include <vector>

#include "StateMachine.h"
#include "Blending/ParameterTable.h"

#include "AniPhysix/Ragdoll.h"


namespace Animix
{
	// Forward declarations
	class SkeletalMeshInstance;

	/**
	 * The animator combines a blend tree, a property table, and a skeletal mesh instance
	 */
	class Animator
	{
	public:
		Animator(SkeletonID target);

		// Get rid of all state machine data
		void Clear();
		// Load state machine data from a JSON file
		bool LoadFromJSON(const std::string& filename);

		inline const std::vector<gef::Matrix44>& GetMatrixPalette() const { return m_MatrixPalette; }
		inline const SkeletonPose& GetBindPose() const { return m_BindPose; }

		// Called by animation engine
		void UpdatePose();

		// State machine API
		AnimatorState* CreateState(const std::string& name);
		AnimatorState* GetState(const std::string& name) const;

		// Manipulate animation state
		bool Transition(const std::string& transitionName);

		// Parameter table
		ParameterTable* GetParameterTable() const { return m_ParameterTable.get(); }

		// Physics-based simulation
		// Create a ragdoll for this skeleton
		void CreateRagdoll(btDiscreteDynamicsWorld* dynamicsWorld, const char* physicsFilename);
		AniPhysix::Ragdoll* GetRagdoll() const { return m_Ragdoll.get(); }

	private:
		void BuildMatrixPalette(const SkeletonPose& globalPose);

		bool BeginTransitionInternal(const std::string& transitionName);

	private:
		SkeletonID m_Target = MAX_SKELETONS;
		SkeletonPose m_BindPose;
		std::vector<gef::Matrix44> m_MatrixPalette;

		// State machine
		std::unordered_map<std::string, std::unique_ptr<AnimatorState>> m_States;

		AnimatorState* m_CurrentState = nullptr;

		AnimatorState* m_NextState = nullptr;						// The to-state in a transition
		float m_TransitionDuration = 0.0f;							// The duration of the transition that is currently occurring
		TransitionType m_TransitionType = TransitionType::Smooth;	// Is this a smooth or frozen transition

		// Variable table
		std::unique_ptr<ParameterTable> m_ParameterTable;

		// Physics-based animation
		std::unique_ptr<AniPhysix::Ragdoll> m_Ragdoll;
	};
}
