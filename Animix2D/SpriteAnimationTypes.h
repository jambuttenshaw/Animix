#pragma once

#include <string>
#include <vector>

#include "SpriteArmatureTypes.h"


namespace Animix2D
{

	/*
	 * Slot Animation
	 */

	struct SpriteSlotAnimation
	{
		// This is simply a collection of the display frames to be used for each animation frame for a single slot

		std::vector<size_t> DisplayFrameSamples;			// The current display frame that each slot should use in this animation sample
	};


	/*
	 * Bone Animation
	 */

	struct SpriteBoneTranslationKey
	{
		float StartTime = 0.0f;
		float Duration = 0.0f;
		float X = 0.0f;
		float Y = 0.0f;
	};


	struct SpriteBoneRotationKey
	{
		float StartTime = 0.0f;
		float Duration = 0.0f;
		float Angle = 0.0f;		// in radians
	};


	struct SpriteBoneAnimation
	{
		// Animation data for a single bone
		std::vector<SpriteBoneTranslationKey> TranslationKeys;	// Collection of translation keys that animate this bone
		std::vector<SpriteBoneRotationKey> RotationKeys;		// Collection of rotation keys that animate this bone
	};


	struct SpriteAnimation
	{
		std::string AnimationName;							// Animations are identified by their string name
		float Duration = 0.0f;								// Duration of the animation in frames

		// Slots may not necessarily be animated, in which case this vector will be empty
		// This vector should contain a animation data for each slot
		std::vector<SpriteSlotAnimation> SlotAnimations;	// Animation data for each slot

		// For skeletal animation
		std::vector<SpriteBoneAnimation> BoneAnimations;	// Animation data for each bone
	};

	//
	//	The Sprite<>Instance structures are owned by a SpriteAnimator rather than a SpriteArmature because their data will vary from sprite to sprite
	//	They can include data cached from the armature to enable speedy lookup and reduce coupling
	//


	struct SpriteAnimationInstance
	{
		// this represents a specific animation 
		// that is currently playing on a specific sprite
		// one animation may have many instances

		std::string AnimationName;							// Animations are referenced by name

		bool Loops = false;

		float StartTime = 0.0f;								// Timer timestamp when animation begun play
															// Each animator has its own timeline (for now)

		float PlaybackSpeed = 1.0f;							// Scalar multiplier for animation speed

		float CurrentFrame = 0.0f;							// The current time of the animation
		// in range [0, Animation.Duration]	

		SpriteSkeletonPose CurrentPose;						// The current pose that the skeleton is in
	};
}