#pragma once

#include <memory>
#include <string>
#include <vector>

#include "maths/matrix33.h"

#include "SpriteArmatureTypes.h"
#include "SpriteAnimationTypes.h"


// Forward declarations
namespace gef
{
	class SpriteRenderer;
}

namespace Animix2D
{
	class SpriteArmature;


	class SpriteAnimator
	{
	public:
		explicit SpriteAnimator(SpriteArmature* armature);

		void Tick(float deltaTime);
		bool PlayAnimation(const std::string& animationName, bool loops, bool allowInterrupt = true, float playbackSpeed = 1.0f);

		void Render(gef::SpriteRenderer* spriteRenderer, const gef::Matrix33& transform) const;

	protected:
		void CreateSlots();

	protected:
		// the armature from which this animator will get the animation data
		SpriteArmature* m_Armature;

		// the sprites that make up this
		std::vector<SpriteSlotInstance> m_SlotInstances;

		// animation state
		float m_GlobalTimer = 0.0f;		// in seconds

		bool m_PlayingAnimation = false;
		SpriteAnimationInstance m_ActiveAnimationInstance;	// for now only one animation can play at a time
	};
}
