#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <string>

#include "Skeleton.h"
#include "Animator.h"
#include "AnimationClip.h"

namespace Animix
{
	extern class AnimationEngine* g_AnimixEngine;

	/**
	 * The animation engine drives all animation.
	 * It provides resource management of animation resources,
	 * as well as performing the heavy lifting of the animation procedures each frame.
	 *
	 * The engine takes ownership of all animation assets
	 */
	class AnimationEngine
	{
	public:
		AnimationEngine();

		void Tick(float deltaTime);

		// Timer
		inline float GetGlobalTime() const { return m_GlobalTimer; }
		inline float GetDeltaTime() const { return m_DeltaTime; }
		inline uint64_t GetTickIndex() const { return m_TickIndex; }

		// Resource management
		const Skeleton* GetSkeleton(SkeletonID id) const { return &m_Skeletons.at(id); }

		const AnimationClip* GetAnimationClip(const std::string& AnimationName) const { return m_AnimationClips.at(AnimationName).get(); }

		// Create assets
		SkeletonID CreateSkeleton(std::vector<Joint>&& joints);
		Animator* CreateAnimator(SkeletonID target);

		AnimationClip* CreateAnimationClip(const std::string& animName, SkeletonID target);

	private:
		// Timing information
		float m_GlobalTimer = 0.0f;
		float m_DeltaTime = 0.0f;
		uint64_t m_TickIndex = 0u;

		// A collection of all skeletons recognized by the engine
		std::array<Skeleton, MAX_SKELETONS> m_Skeletons;
		SkeletonID m_SkeletonCount = 0;

		// A collection of all animators present in the game
		std::vector<Animator> m_Animators;

		// All animation clips
		std::unordered_map<std::string, std::unique_ptr<AnimationClip>> m_AnimationClips;
	};
}
