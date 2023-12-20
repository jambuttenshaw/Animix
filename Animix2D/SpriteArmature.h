#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include "SpriteArmatureTypes.h"
#include "SpriteAnimationTypes.h"


namespace Animix2D
{

	// Forward-declarations
	class TextureAtlas;


	class SpriteArmature
	{
	public:
		bool LoadFromJson(const std::string& filename);
		bool AssociateTextureAtlas(TextureAtlas* textureAtlas);

		void ClearMetadata();
		void ClearTextureAtlas();

		inline bool IsValid() const { return m_LoadedMetaData && m_LoadedTextureAtlas; }

		// Getters
		inline float GetFrameRate() const { return FrameRate; }

		inline size_t GetSkinSlotCount() const { return m_SkinSlots.size(); }
		inline const SpriteSkinSlot& GetSkinSlot(size_t index) const { return m_SkinSlots[index]; }

		inline bool HasAnimation(const std::string& name) const { return m_Animations.find(name) != m_Animations.end(); }
		inline const SpriteAnimation& GetAnimation(const std::string& name) const { return m_Animations.at(name); }
		inline const std::string& GetDefaultAnimation() const { return m_DefaultAnimation; }

		void UpdateSpriteSlotInstance(const SpriteAnimationInstance& animInstance, SpriteSlotInstance& slotInstance) const;

		void BuildSkeletonPose(SpriteAnimationInstance& animInstance) const;

	protected:
		// Get the world transform of a slot (bind pose)
		gef::Matrix33 GetSlotWorldTransform(const SpriteSlotInstance& slotInstance) const;

		// Apply the correct sub-texture and sprite offset transform from the texture atlas
		void ApplySubTexture(SpriteSlotInstance& slotInstance) const;

	protected:
		// flags
		bool m_LoadedMetaData = false;
		bool m_LoadedTextureAtlas = false;

		// metadata
		std::string SpriteSheetName;
		float FrameRate = 0.0f;		// in frames per second

		std::vector<SpriteBone> m_Skeleton;
		std::vector<SpriteSkinSlot> m_SkinSlots;

		std::unordered_map<std::string, SpriteAnimation> m_Animations;
		std::string m_DefaultAnimation;

		// texture atlas
		TextureAtlas* m_TextureAtlas = nullptr;
	};
}
