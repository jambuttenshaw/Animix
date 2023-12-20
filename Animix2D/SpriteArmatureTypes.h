#pragma once

#include <string>
#include <vector>

#include <maths/matrix33.h>
#include <graphics/sprite.h>


namespace Animix2D
{

	struct SpriteBone
	{
		std::string BoneName;
		std::string ParentName;

		int32_t ParentIndex;								// Parent index in the skeleton
		// this is signed, as root bone has index of -1
		// (in most cases: in the case where a skeleton has no other bones, root will be bone index 0)

		float LocalX = 0.0f;
		float LocalY = 0.0f;
		float LocalRotation = 0.0f;
		gef::Matrix33 LocalTransform;						// X, Y, and Rot combined in a matrix
	};

	struct SpriteSkeletonPose
	{
		std::vector<gef::Matrix33> LocalBonePoses;				// A bone-space transform for each bone
		std::vector<gef::Matrix33> WorldBonePoses;				// A world-space transform for each bone
	};


	struct SpriteSlotDisplay
	{
		// a display for one skin slot
		// a slot may have multiple displays, in the case of flip-book animation
		// or it may only have one in the case of skeletal animation

		std::string SubTextureName;							// Name of the sub-texture in the texture atlas
		gef::Matrix33 OffsetTransform						// Offset transform to be applied to the sprite before attaching to the skeleton
			= gef::Matrix33::kIdentity;
	};


	struct SpriteSkinSlot
	{
		std::string SlotName;								// Slot Name
		std::vector<SpriteSlotDisplay> DisplayFrames;		// Collection of different displays this slot could take on

		std::string ParentBoneName;							// The name of the bone in the skeleton that this slot is bound to
		size_t ParentBoneIndex = -1;						// The index of the bone in the skeleton that this slot is bound to
	};


	//
	//	The Sprite<>Instance structures are owned by a SpriteAnimator rather than a SpriteArmature because their data will vary from sprite to sprite
	//	They can include data cached from the armature to enable speedy lookup and reduce coupling
	//

	struct SpriteSlotInstance
	{
		size_t SlotIndex = -1;

		gef::Sprite Sprite;									// The sprite that is rendered to the screen
		gef::Matrix33 SubTextureTransform					// The sub-texture transform is cached from the texture atlas
			= gef::Matrix33::kIdentity;						// this is so it only needs looked up when switching frames, not every frame

		gef::Matrix33 OffsetTransform						// Offset transform is cached from the slots so that it does not need looked up every frame
			= gef::Matrix33::kIdentity;

		size_t CurrentDisplayFrame = 0;						// The cached current display frame of this slot for rendering
		// This is so it does not need to be looked up every frame
	};

}
