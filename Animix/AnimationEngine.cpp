#include "AnimationEngine.h"

#include <cassert>

#include "Animator.h"
#include "Skeleton.h"


namespace Animix
{
	AnimationEngine* g_AnimixEngine = nullptr;


	AnimationEngine::AnimationEngine()
	{
		assert(!g_AnimixEngine && "Cannot instantiate multiple animation engines!");
		g_AnimixEngine = this;
	}

	void AnimationEngine::Tick(float deltaTime)
	{
		// Update timer
		m_GlobalTimer += deltaTime;
		m_DeltaTime = deltaTime;
		m_TickIndex++;

		// Update all animators
		for (auto& animator : m_Animators)
			animator.UpdatePose();
	}

	SkeletonID AnimationEngine::CreateSkeleton(std::vector<Joint>&& joints)
	{
		assert(m_SkeletonCount < MAX_SKELETONS - 1);
		m_Skeletons[m_SkeletonCount].ID = m_SkeletonCount;
		m_Skeletons[m_SkeletonCount].Joints = std::move(joints);
		
		return m_SkeletonCount++;
	}

	Animator* AnimationEngine::CreateAnimator(SkeletonID target)
	{
		m_Animators.emplace_back(target);
		return &m_Animators.back();
	}

	AnimationClip* AnimationEngine::CreateAnimationClip(const std::string& animName, SkeletonID target)
	{
		assert(m_AnimationClips.find(animName) == m_AnimationClips.end());

		m_AnimationClips.insert(std::make_pair(animName, std::make_unique<AnimationClip>(target)));
		return m_AnimationClips.at(animName).get();
	}

}
