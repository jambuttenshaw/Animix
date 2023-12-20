#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include <graphics/texture.h>
#include <graphics/sprite.h>

#include "maths/matrix33.h"

namespace gef
{
	class Vector2;

	class Platform;
}

namespace Animix2D
{

	struct SubTexture
	{
		std::string Name;

		uint32_t X, Y;
		uint32_t Width, Height;

		int32_t FrameX, FrameY;
		int32_t FrameWidth, FrameHeight;

		gef::Matrix33 Transform;

		SubTexture()
			: X(0), Y(0),
			Width(0), Height(0),
			FrameX(0), FrameY(0),
			FrameWidth(0), FrameHeight(0)
		{
			Transform.SetIdentity();
		}

		void BuildTransform();
	};


	class TextureAtlas
	{
	public:
		explicit TextureAtlas(gef::Platform& platform);

		// Load a new texture atlas from a JSON description
		bool LoadFromJson(const std::string& filename);

		// Clear all data from the texture atlas
		void Clear();

		inline const std::string& GetName() const { return m_Name; }

		bool SubTextureExistsWithName(const std::string& name) const;

		// Apply a Sub-texture from the atlas to a sprite
		void Apply(gef::Sprite& sprite, const std::string& subTextureName) const;

		const gef::Matrix33& GetSpriteOffsetTransform(const std::string& subTextureName) const;

	private:
		gef::Platform& m_Platform;

		std::string m_Name;

		std::unordered_map<std::string, SubTexture> m_SubTextures;
		size_t m_SubTextureCount = -1;

		std::string m_TexturePath;
		std::unique_ptr<gef::Texture> m_Texture;
		size_t m_Width = -1,
			m_Height = -1;
		// value of 1 pixel in UV space
		float m_UUnits = 0.0f,
			m_VUnits = 0.0f;
	};
}
