#include "SpriteAnimator.h"

#include "SpriteArmature.h"

#include <cassert>

#include "graphics/sprite_renderer.h"

namespace Animix2D
{

	SpriteAnimator::SpriteAnimator(SpriteArmature* armature)
		: m_Armature(armature)
	{
		assert(m_Armature && m_Armature->IsValid() && "Animation instances can only be created from valid sprite animators!");

		// Load default animation
		m_ActiveAnimationInstance.AnimationName = m_Armature->GetDefaultAnimation();
		m_Armature->BuildSkeletonPose(m_ActiveAnimationInstance);

		// default animation will be applied to slots
		CreateSlots();
	}

	void SpriteAnimator::Tick(float deltaTime)
	{
		m_GlobalTimer += deltaTime;

		if (!(m_Armature && m_Armature->IsValid()))
			return;

		if (m_PlayingAnimation)
		{
			// m_ActiveAnimationInstance is assumed to be valid

			// get current point in animation
			const SpriteAnimation& currentAnimation = m_Armature->GetAnimation(m_ActiveAnimationInstance.AnimationName);

			// get current timestamp local to the animation
			const float localTime = (m_GlobalTimer - m_ActiveAnimationInstance.StartTime) * m_ActiveAnimationInstance.PlaybackSpeed;
			// get current frame, clamped between 0 and duration
			const float frame = fmin(fmax(localTime * m_Armature->GetFrameRate(), 0.0f), currentAnimation.Duration);
			m_ActiveAnimationInstance.CurrentFrame = frame;

			if (frame > currentAnimation.Duration - 1.0f)
			{
				// animation has finished
				m_PlayingAnimation = m_ActiveAnimationInstance.Loops;

				// loop back to the start
				m_ActiveAnimationInstance.StartTime = m_GlobalTimer;
				m_ActiveAnimationInstance.CurrentFrame = 0.0f;
			}

			// apply current frame to slots
			for (size_t slotIndex = 0; slotIndex < m_Armature->GetSkinSlotCount(); ++slotIndex)
			{
				m_Armature->UpdateSpriteSlotInstance(m_ActiveAnimationInstance, m_SlotInstances[slotIndex]);
			}

			m_Armature->BuildSkeletonPose(m_ActiveAnimationInstance);
		}
	}

	bool SpriteAnimator::PlayAnimation(const std::string& animationName, bool loops, bool allowInterrupt, float playbackSpeed)
	{
		if (!(m_Armature && m_Armature->IsValid()))
			return false;

		if (!allowInterrupt && m_PlayingAnimation)
			return false;

		if (!m_Armature->HasAnimation(animationName))
			return false;

		m_ActiveAnimationInstance.AnimationName = animationName;
		m_ActiveAnimationInstance.Loops = loops;
		m_ActiveAnimationInstance.StartTime = m_GlobalTimer;
		m_ActiveAnimationInstance.PlaybackSpeed = playbackSpeed;

		m_PlayingAnimation = true;
		return true;
	}

	void SpriteAnimator::Render(gef::SpriteRenderer* spriteRenderer, const gef::Matrix33& transform) const
	{
		// get sprite offset transform
		for (const SpriteSlotInstance& slotInstance : m_SlotInstances)
		{
			const SpriteSkinSlot& skinSlot = m_Armature->GetSkinSlot(slotInstance.SlotIndex);
			gef::Matrix33 bonePose = m_ActiveAnimationInstance.CurrentPose.WorldBonePoses[skinSlot.ParentBoneIndex];

			const gef::Matrix33 drawTransform = slotInstance.SubTextureTransform * slotInstance.OffsetTransform * bonePose * transform;
			spriteRenderer->DrawSprite(slotInstance.Sprite, drawTransform);
		}
	}


	void SpriteAnimator::CreateSlots()
	{
		const size_t slotCount = m_Armature->GetSkinSlotCount();
		m_SlotInstances.resize(slotCount);

		for (size_t slotIndex = 0; slotIndex < slotCount; ++slotIndex)
		{
			m_SlotInstances[slotIndex].SlotIndex = slotIndex;
			m_SlotInstances[slotIndex].CurrentDisplayFrame = 0;

			gef::Sprite& slotSprite = m_SlotInstances[slotIndex].Sprite;
			slotSprite.set_width(1.0f);
			slotSprite.set_height(1.0f);

			m_Armature->UpdateSpriteSlotInstance(m_ActiveAnimationInstance, m_SlotInstances[slotIndex]);
		}
	}
}
