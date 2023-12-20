#include "SpriteArmature.h"

#include <fstream>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

#include "TextureAtlas.h"


namespace Animix2D
{


	bool SpriteArmature::LoadFromJson(const std::string& filename)
	{
		// loading new metadata will invalidate any pre-existing state
		ClearMetadata();
		ClearTextureAtlas();

		rapidjson::Document DocJSON;

		// load json data
		{
			std::ifstream input_file(filename);
			if (!input_file.good())
				return false;

			rapidjson::IStreamWrapper stream(input_file);
			DocJSON.ParseStream(stream);
		}

		// parse data

		// handy macros for quick error checking
#define CHECK_MEMBER_REQUIRED(json, name) if (!(json).HasMember(name)) { ClearMetadata(); return false; }
#define CHECK_ARRAY_NOT_EMPTY_REQUIRED(jsonArr) if ((jsonArr).Size() == 0) { ClearMetadata(); return false; }

	// load armature
	// only supports loading a single armature currently

		CHECK_MEMBER_REQUIRED(DocJSON, "armature")
			CHECK_ARRAY_NOT_EMPTY_REQUIRED(DocJSON["armature"])
			rapidjson::Value& ArmatureJSON = DocJSON["armature"][0];

		CHECK_MEMBER_REQUIRED(ArmatureJSON, "name")
			SpriteSheetName = ArmatureJSON["name"].GetString();

		CHECK_MEMBER_REQUIRED(ArmatureJSON, "frameRate")
			FrameRate = static_cast<float>(ArmatureJSON["frameRate"].GetInt());

		// for resolving bone references later
		// bone references need to also be resolved by the slots
		// loading animation data also makes use of the bone indices to construct animation data
		std::unordered_map<std::string, size_t> boneIndices;
		// root is always -1
		boneIndices["root"] = -1;

		// the skin data needs to access the slots
		std::unordered_map<std::string, size_t> slotIndices;


		{
			// load skeleton
			CHECK_MEMBER_REQUIRED(ArmatureJSON, "bone")
				CHECK_ARRAY_NOT_EMPTY_REQUIRED(ArmatureJSON["bone"])

				rapidjson::SizeType boneCount = ArmatureJSON["bone"].Size();

			for (rapidjson::SizeType boneIndex = 0; boneIndex < boneCount; ++boneIndex)
			{
				rapidjson::Value& BoneJSON = ArmatureJSON["bone"][boneIndex];

				CHECK_MEMBER_REQUIRED(BoneJSON, "name")
					const std::string boneName = BoneJSON["name"].GetString();

				if (boneName == "root")
					continue;

				// create new bone
				m_Skeleton.emplace_back();
				SpriteBone& bone = m_Skeleton.back();

				bone.BoneName = boneName;
				boneIndices[boneName] = m_Skeleton.size() - 1;

				// all bones other than root must have a parent
				CHECK_MEMBER_REQUIRED(BoneJSON, "parent")
					bone.ParentName = BoneJSON["parent"].GetString();

				if (BoneJSON.HasMember("transform"))
				{
					rapidjson::Value& TransformJSON = BoneJSON["transform"];
					// load transform
					if (TransformJSON.HasMember("x"))
						bone.LocalX = TransformJSON["x"].GetFloat();
					if (TransformJSON.HasMember("y"))
						bone.LocalY = TransformJSON["y"].GetFloat();
					if (TransformJSON.HasMember("skX"))
						bone.LocalRotation = TransformJSON["skX"].GetFloat() * 0.0174533f;

					gef::Matrix33& mat = bone.LocalTransform;
					mat.SetIdentity();
					mat.Rotate(bone.LocalRotation);
					mat.SetTranslation({ bone.LocalX, bone.LocalY });
				}
				else
				{
					bone.LocalTransform.SetIdentity();
				}
			}

			// resolve bone references
			for (SpriteBone& bone : m_Skeleton)
			{
				assert(boneIndices.find(bone.ParentName) != boneIndices.end() && "Bone has non-existent parent");
				bone.ParentIndex = static_cast<int32_t>(boneIndices[bone.ParentName]);
			}

			if (m_Skeleton.empty())
			{
				// in the case where there are no bones,
				// root should be added to the skeleton
				m_Skeleton.emplace_back();
				SpriteBone& bone = m_Skeleton.back();
				bone.BoneName = "root";
				bone.ParentName = "root"; // doesn't really matter
				bone.ParentIndex = -1;
				bone.LocalTransform = gef::Matrix33::kIdentity;
			}
		}


		{
			// load slots
			// slots are mapped to bones

			// the order slots are defined is important as this is the order they will be rendered

			CHECK_MEMBER_REQUIRED(ArmatureJSON, "slot")
				CHECK_ARRAY_NOT_EMPTY_REQUIRED(ArmatureJSON["slot"])

				rapidjson::SizeType NumSlots = ArmatureJSON["slot"].Size();
			m_SkinSlots.resize(NumSlots);

			for (rapidjson::SizeType slotIndex = 0; slotIndex < NumSlots; ++slotIndex)
			{
				rapidjson::Value& SlotJSON = ArmatureJSON["slot"][slotIndex];
				SpriteSkinSlot& slot = m_SkinSlots[slotIndex];

				CHECK_MEMBER_REQUIRED(SlotJSON, "name")
					slot.SlotName = SlotJSON["name"].GetString();

				CHECK_MEMBER_REQUIRED(SlotJSON, "parent")
					slot.ParentBoneName = SlotJSON["name"].GetString();
				slot.ParentBoneIndex = boneIndices[slot.ParentBoneName];

				slotIndices[slot.SlotName] = slotIndex;
			}
		}


		{
			// load skin
			// only supports one skin currently
			// a skin may have many slots

			CHECK_MEMBER_REQUIRED(ArmatureJSON, "skin")
				CHECK_ARRAY_NOT_EMPTY_REQUIRED(ArmatureJSON["skin"])
				CHECK_MEMBER_REQUIRED(ArmatureJSON["skin"][0], "slot")
				CHECK_ARRAY_NOT_EMPTY_REQUIRED(ArmatureJSON["skin"][0]["slot"])

				// iterate over slots
				// slots have already been defined in order of rendering, but now we need to apply their skin data

				for (rapidjson::SizeType skinIndex = 0; skinIndex < ArmatureJSON["skin"][0]["slot"].Size(); ++skinIndex)
				{
					rapidjson::Value& SkinJSON = ArmatureJSON["skin"][0]["slot"][skinIndex];

					CHECK_MEMBER_REQUIRED(SkinJSON, "name")
						const std::string& slotName = SkinJSON["name"].GetString();

					assert(slotIndices.find(slotName) != slotIndices.end() && "Missing slot index!");
					size_t slotIndex = slotIndices[slotName];

					CHECK_MEMBER_REQUIRED(SkinJSON, "display")

						rapidjson::SizeType NumDisplays = SkinJSON["display"].Size();
					m_SkinSlots[slotIndex].DisplayFrames.resize(NumDisplays);

					for (rapidjson::SizeType displayIndex = 0; displayIndex < NumDisplays; ++displayIndex)
					{
						rapidjson::Value& DisplayJSON = SkinJSON["display"][displayIndex];
						SpriteSlotDisplay& DisplayFrame = m_SkinSlots[slotIndex].DisplayFrames[displayIndex];

						CHECK_MEMBER_REQUIRED(DisplayJSON, "name")
							DisplayFrame.SubTextureName = DisplayJSON["name"].GetString();

						if (DisplayJSON.HasMember("transform"))
						{
							rapidjson::Value& TransformJSON = DisplayJSON["transform"];
							// load transform
							float x = 0.0f,
								y = 0.0f,
								rot = 0.0f;
							if (TransformJSON.HasMember("x"))
								x = TransformJSON["x"].GetFloat();
							if (TransformJSON.HasMember("y"))
								y = TransformJSON["y"].GetFloat();
							if (TransformJSON.HasMember("skX"))
								// convert rotation to radians
								rot = TransformJSON["skX"].GetFloat() * 0.0174533f;

							gef::Matrix33& mat = DisplayFrame.OffsetTransform;
							mat.SetIdentity();
							mat.Rotate(rot);
							mat.SetTranslation({ x, y });
						}
					}
				}
		}

		if (ArmatureJSON.HasMember("animation"))
		{
			// load animations
			CHECK_ARRAY_NOT_EMPTY_REQUIRED(ArmatureJSON["animation"])
				rapidjson::Value& AnimationsJSON = ArmatureJSON["animation"];

			rapidjson::SizeType AnimationCount = AnimationsJSON.Size();

			for (rapidjson::SizeType index = 0; index < AnimationCount; ++index)
			{
				rapidjson::Value& AnimationJSON = AnimationsJSON[index];
				SpriteAnimation spriteAnimation;

				CHECK_MEMBER_REQUIRED(AnimationJSON, "name")
					spriteAnimation.AnimationName = AnimationJSON["name"].GetString();

				CHECK_MEMBER_REQUIRED(AnimationJSON, "duration")
					spriteAnimation.Duration = static_cast<float>(AnimationJSON["duration"].GetInt());

				// flip-book animations will animate the slots
				// skeletal animations will animate the bones
				// therefore it isn't required that slots are present

				if (AnimationJSON.HasMember("slot"))
				{
					CHECK_ARRAY_NOT_EMPTY_REQUIRED(AnimationJSON["slot"])

						// Allocate a container for each slots animation
						// Note: not every slot must have data. But animation data is accessed by slot index so its important an entry for each slot exists
						spriteAnimation.SlotAnimations.resize(m_SkinSlots.size());

					for (rapidjson::SizeType slot = 0; slot < AnimationJSON["slot"].Size(); ++slot)
					{
						rapidjson::Value& SlotJSON = AnimationJSON["slot"][slot];

						CHECK_MEMBER_REQUIRED(SlotJSON, "name")
							const std::string& slotName = SlotJSON["name"].GetString();

						assert(slotIndices.find(slotName) != slotIndices.end() && "Missing slot index!");
						size_t slotIndex = slotIndices[slotName];

						if (SlotJSON.HasMember("displayFrame"))
						{
							rapidjson::Value& displayFrame = SlotJSON["displayFrame"];
							spriteAnimation.SlotAnimations[slotIndex].DisplayFrameSamples.resize(displayFrame.Size());

							for (rapidjson::SizeType frame = 0; frame < displayFrame.Size(); ++frame)
							{
								if (displayFrame[frame].HasMember("value"))
									spriteAnimation.SlotAnimations[slotIndex].DisplayFrameSamples[frame] = displayFrame[frame]["value"].GetInt();
								else
									// no value specified should default to 0
									spriteAnimation.SlotAnimations[slotIndex].DisplayFrameSamples[frame] = 0;
							}
						}
						else
						{
							// no display frames specified in the animation data: therefore the display frame remains constant at index 0
							spriteAnimation.SlotAnimations[slotIndex].DisplayFrameSamples.push_back(0);
						}
					}
				}


				if (AnimationJSON.HasMember("bone"))
				{
					CHECK_ARRAY_NOT_EMPTY_REQUIRED(AnimationJSON["bone"])

						// Allocate a container for each bones animation
						// Note: not every bone will have data. But animation data is accessed by bone index so its important an entry for each bone exists
						spriteAnimation.BoneAnimations.resize(m_Skeleton.size());

					for (rapidjson::SizeType bone = 0; bone < AnimationJSON["bone"].Size(); ++bone)
					{
						rapidjson::Value& BoneJSON = AnimationJSON["bone"][bone];

						CHECK_MEMBER_REQUIRED(BoneJSON, "name")
							const std::string& boneName = BoneJSON["name"].GetString();

						assert(boneIndices.find(boneName) != boneIndices.end() && "Animation data contains data for a bone that doesn't exist!");
						size_t boneIndex = boneIndices[boneName];
						SpriteBoneAnimation& BoneAnimation = spriteAnimation.BoneAnimations[boneIndex];

						// handle translation frames
						if (BoneJSON.HasMember("translateFrame"))
						{
							rapidjson::SizeType frameCount = BoneJSON["translateFrame"].Size();
							BoneAnimation.TranslationKeys.resize(frameCount);

							float startFrame = 0.0f;
							for (rapidjson::SizeType frameIndex = 0; frameIndex < frameCount; ++frameIndex)
							{
								rapidjson::Value& frameJSON = BoneJSON["translateFrame"][frameIndex];
								SpriteBoneTranslationKey& translationKey = BoneAnimation.TranslationKeys[frameIndex];

								float frameDuration = 0.0f;
								if (frameJSON.HasMember("duration"))
									// durations are handled in floats to make interpolation easier late
									frameDuration = static_cast<float>(frameJSON["duration"].GetInt());

								translationKey.StartTime = startFrame;
								translationKey.Duration = frameDuration;

								// get frame data
								if (frameJSON.HasMember("x"))
									translationKey.X = frameJSON["x"].GetFloat();
								if (frameJSON.HasMember("y"))
									translationKey.Y = frameJSON["y"].GetFloat();

								startFrame += frameDuration;
							}
						}

						// handle rotation frames
						if (BoneJSON.HasMember("rotateFrame"))
						{
							rapidjson::SizeType frameCount = BoneJSON["rotateFrame"].Size();
							BoneAnimation.RotationKeys.resize(frameCount);

							float startFrame = 0.0f;
							for (rapidjson::SizeType frameIndex = 0; frameIndex < frameCount; ++frameIndex)
							{
								rapidjson::Value& frameJSON = BoneJSON["rotateFrame"][frameIndex];
								SpriteBoneRotationKey& rotationKey = BoneAnimation.RotationKeys[frameIndex];

								float frameDuration = 0.0f;
								if (frameJSON.HasMember("duration"))
									// durations are handled in floats to make interpolation easier late
									frameDuration = static_cast<float>(frameJSON["duration"].GetInt());

								rotationKey.StartTime = startFrame;
								rotationKey.Duration = frameDuration;

								// get frame data
								if (frameJSON.HasMember("rotate"))
									rotationKey.Angle = frameJSON["rotate"].GetFloat() * 0.0174533f;

								startFrame += frameDuration;
							}
						}
					}
				}

				m_Animations.insert(std::make_pair(spriteAnimation.AnimationName, spriteAnimation));
			}
		}

		{
			// load default actions
			// this will load the default animation
			bool foundDefaultAnimation = false;
			if (ArmatureJSON.HasMember("defaultActions") && ArmatureJSON["defaultActions"].Size() > 0)
			{
				if (ArmatureJSON["defaultActions"][0].HasMember("gotoAndPlay"))
				{
					m_DefaultAnimation = ArmatureJSON["defaultActions"][0]["gotoAndPlay"].GetString();
					foundDefaultAnimation = true;
				}
			}

			if (!foundDefaultAnimation)
			{
				// if no default animation specified in json, then take whatever animation is in m_Animations.begin()
				m_DefaultAnimation = m_Animations.at(m_Animations.begin()->first).AnimationName;
			}
		}

		m_LoadedMetaData = true;
		return true;

		// this animator will not become valid until it is associated with the correct texture atlas
	}

	bool SpriteArmature::AssociateTextureAtlas(TextureAtlas* textureAtlas)
	{
		if (!m_LoadedMetaData)
			// must load metadata first
			return false;

		// reset pre-existing state
		ClearTextureAtlas();
		m_LoadedTextureAtlas = false;

		if (!textureAtlas)
		{
			return false;
		}

		// create a shared pointer to the texture atlas
		m_TextureAtlas = textureAtlas;

		// verify that this texture atlas is the correct one
		if (m_TextureAtlas->GetName() != SpriteSheetName)
		{
			ClearTextureAtlas();
			return false;
		}

		m_LoadedTextureAtlas = true;
		return true;
	}

	void SpriteArmature::ClearMetadata()
	{
		// cleanup metadata
		m_LoadedMetaData = false;

		SpriteSheetName = "";
		FrameRate = 0.0f;

		m_SkinSlots.clear();
		m_Animations.clear();
	}

	void SpriteArmature::ClearTextureAtlas()
	{
		// cleanup texture atlas reference/data
		m_LoadedTextureAtlas = false;
		m_TextureAtlas = nullptr;
	}


	void SpriteArmature::UpdateSpriteSlotInstance(const SpriteAnimationInstance& animInstance, SpriteSlotInstance& slotInstance) const
	{
		const SpriteAnimation& anim = GetAnimation(animInstance.AnimationName);

		// if the slot data isn't animated, then the display frame will always be 0
		slotInstance.CurrentDisplayFrame = anim.SlotAnimations.empty() ? 0 :
			anim.SlotAnimations[slotInstance.SlotIndex].DisplayFrameSamples[static_cast<size_t>(animInstance.CurrentFrame)];

		slotInstance.OffsetTransform = m_SkinSlots[slotInstance.SlotIndex].DisplayFrames[slotInstance.CurrentDisplayFrame].OffsetTransform;

		ApplySubTexture(slotInstance);
	}


	void SpriteArmature::ApplySubTexture(SpriteSlotInstance& slotInstance) const
	{
		// Get frame name
		const std::string& frameName = m_SkinSlots[slotInstance.SlotIndex].DisplayFrames[slotInstance.CurrentDisplayFrame].SubTextureName;

		m_TextureAtlas->Apply(slotInstance.Sprite, frameName);
		slotInstance.SubTextureTransform = m_TextureAtlas->GetSpriteOffsetTransform(frameName);
	}


	gef::Matrix33 SpriteArmature::GetSlotWorldTransform(const SpriteSlotInstance& slotInstance) const
	{
		assert(false && "deprecated");

		const SpriteSkinSlot& slot = m_SkinSlots[slotInstance.SlotIndex];
		SpriteBone bone = m_Skeleton[slot.ParentBoneIndex];
		gef::Matrix33 localToWorld = gef::Matrix33::kIdentity;

		while (bone.ParentIndex != -1)
		{
			localToWorld = localToWorld * bone.LocalTransform;
			bone = m_Skeleton[bone.ParentIndex];
		}

		return localToWorld;
	}

	void SpriteArmature::BuildSkeletonPose(SpriteAnimationInstance& animInstance) const
	{
		SpriteSkeletonPose& pose = animInstance.CurrentPose;
		const SpriteAnimation& anim = m_Animations.at(animInstance.AnimationName);

		// there should be one transform for each bone
		if (pose.LocalBonePoses.size() != m_Skeleton.size())
			pose.LocalBonePoses.resize(m_Skeleton.size());
		if (pose.WorldBonePoses.size() != m_Skeleton.size())
			pose.WorldBonePoses.resize(m_Skeleton.size());

		const bool animatesBones = !anim.BoneAnimations.empty();

		// Calculate the pose of each bone in the skeleton
		for (size_t boneIndex = 0; boneIndex < m_Skeleton.size(); ++boneIndex)
		{
			const SpriteBone* bone = &m_Skeleton[boneIndex];

			if (animatesBones)
			{
				// find current key in the animation
				const SpriteBoneAnimation& boneAnim = anim.BoneAnimations[boneIndex];

				// get current bone pose from animation data
				float poseX = 0.0f;
				float poseY = 0.0f;
				float poseRotation = 0.0f;

				if (!boneAnim.TranslationKeys.empty())
				{
					size_t keyIndex = 0;
					for (; keyIndex < boneAnim.TranslationKeys.size() - 1 &&
						animInstance.CurrentFrame >= boneAnim.TranslationKeys[keyIndex + 1].StartTime;
						keyIndex++) {
					}

					const SpriteBoneTranslationKey& startKey = boneAnim.TranslationKeys[keyIndex];
					const SpriteBoneTranslationKey& endKey = boneAnim.TranslationKeys[std::min(keyIndex + 1, boneAnim.TranslationKeys.size() - 1)];

					const float t = (animInstance.CurrentFrame - startKey.StartTime) / startKey.Duration;

					poseX = (1.0f - t) * startKey.X + t * endKey.X;
					poseY = (1.0f - t) * startKey.Y + t * endKey.Y;
				}

				if (!boneAnim.RotationKeys.empty())
				{
					size_t keyIndex = 0;
					for (; keyIndex < boneAnim.RotationKeys.size() - 1 &&
						animInstance.CurrentFrame >= boneAnim.RotationKeys[keyIndex + 1].StartTime;
						keyIndex++) {
					}

					const SpriteBoneRotationKey& startKey = boneAnim.RotationKeys[keyIndex];
					const SpriteBoneRotationKey& endKey = boneAnim.RotationKeys[std::min(keyIndex + 1, boneAnim.RotationKeys.size() - 1)];

					const float t = (animInstance.CurrentFrame - startKey.StartTime) / startKey.Duration;

					float angleDiff = endKey.Angle - startKey.Angle;
					if (fabsf(angleDiff) > 3.14159f) angleDiff += 6.28319f * angleDiff > 0.0f ? -1.0f : 1.0f;

					poseRotation = startKey.Angle + t * angleDiff;
				}

				gef::Matrix33 localToWorld = gef::Matrix33::kIdentity;
				localToWorld.Rotate(bone->LocalRotation + poseRotation);
				localToWorld.SetTranslation({ bone->LocalX + poseX, bone->LocalY + poseY });

				pose.LocalBonePoses[boneIndex] = localToWorld;
			}
			else
			{
				pose.LocalBonePoses[boneIndex] = bone->LocalTransform;
			}
		}


		// and now calculate the world space transform of every bone in the skeleton
		for (size_t boneIndex = 0; boneIndex < m_Skeleton.size(); ++boneIndex)
		{
			const SpriteBone* bone = &m_Skeleton[boneIndex];

			gef::Matrix33 localToWorld = pose.LocalBonePoses[boneIndex];

			while (bone->ParentIndex != -1)
			{
				localToWorld = localToWorld * pose.LocalBonePoses[bone->ParentIndex];
				bone = &m_Skeleton[bone->ParentIndex];
			}

			pose.WorldBonePoses[boneIndex] = localToWorld;
		}
	}
}
