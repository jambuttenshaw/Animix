#include "TextureAtlas.h"

#include <fstream>

#include "graphics/texture.h"
#include "graphics/sprite.h"

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

#include "load_texture.h"


namespace Animix2D
{



	void SubTexture::BuildTransform()
	{
		Transform.SetIdentity();

		// operations must be performed in this order!
		// relies on the face that gef::Matrix33::SetTranslation ONLY sets the required elements
		// in the matrix and does not touch the rest of the matrix

		Transform.Scale({ static_cast<float>(Width), static_cast<float>(Height) });
		Transform.SetTranslation({
			0.5f * static_cast<float>(Width) - (0.5f * static_cast<float>(FrameWidth) + static_cast<float>(FrameX)),
			0.5f * static_cast<float>(Height) - (0.5f * static_cast<float>(FrameHeight) + static_cast<float>(FrameY)),
			});
	}



	TextureAtlas::TextureAtlas(gef::Platform& platform)
		: m_Platform(platform)
	{
	}

	bool TextureAtlas::LoadFromJson(const std::string& filename)
	{
		rapidjson::Document AtlasJson;

		// load and parse json
		{
			std::ifstream input_file(filename.c_str());
			if (!input_file.good())
				// didn't get far enough to have anything to clean up
				return false;

			rapidjson::IStreamWrapper stream(input_file);
			AtlasJson.ParseStream(stream);
		} // file resources released

		// if it fails to find some members that could be fatal
		// simple macro to verify a member exists and cleanup and exit if it doesn't
#define CHECK_MEMBER_REQUIRED(json, name) if (!(json).HasMember(name)) { Clear(); return false; }

		CHECK_MEMBER_REQUIRED(AtlasJson, "name")
			m_Name = AtlasJson["name"].GetString();

		CHECK_MEMBER_REQUIRED(AtlasJson, "width")
			m_Width = AtlasJson["width"].GetInt();
		CHECK_MEMBER_REQUIRED(AtlasJson, "height")
			m_Height = AtlasJson["height"].GetInt();

		CHECK_MEMBER_REQUIRED(AtlasJson, "imagePath")
			m_TexturePath = AtlasJson["imagePath"].GetString();

		// Parse sub-textures
		CHECK_MEMBER_REQUIRED(AtlasJson, "SubTexture")
			m_SubTextureCount = AtlasJson["SubTexture"].Size();
		for (rapidjson::SizeType index = 0; index < m_SubTextureCount; ++index)
		{
			// Load one sub-texture
			SubTexture NewSubTexture;
			rapidjson::Value& SubTextureJson = AtlasJson["SubTexture"][index];

			CHECK_MEMBER_REQUIRED(SubTextureJson, "name")
				NewSubTexture.Name = SubTextureJson["name"].GetString();

			CHECK_MEMBER_REQUIRED(SubTextureJson, "x")
				NewSubTexture.X = SubTextureJson["x"].GetUint();
			CHECK_MEMBER_REQUIRED(SubTextureJson, "y")
				NewSubTexture.Y = SubTextureJson["y"].GetUint();

			CHECK_MEMBER_REQUIRED(SubTextureJson, "width")
				NewSubTexture.Width = SubTextureJson["width"].GetUint();
			CHECK_MEMBER_REQUIRED(SubTextureJson, "height")
				NewSubTexture.Height = SubTextureJson["height"].GetUint();

			if (SubTextureJson.HasMember("frameX"))
				NewSubTexture.FrameX = SubTextureJson["frameX"].GetInt();
			else
				NewSubTexture.FrameX = 0;
			if (SubTextureJson.HasMember("frameY"))
				NewSubTexture.FrameY = SubTextureJson["frameY"].GetInt();
			else
				NewSubTexture.FrameY = 0;

			if (SubTextureJson.HasMember("frameWidth"))
				NewSubTexture.FrameWidth = SubTextureJson["frameWidth"].GetInt();
			else
				NewSubTexture.FrameWidth = static_cast<int32_t>(NewSubTexture.Width);
			if (SubTextureJson.HasMember("frameHeight"))
				NewSubTexture.FrameHeight = SubTextureJson["frameHeight"].GetInt();
			else
				NewSubTexture.FrameHeight = static_cast<int32_t>(NewSubTexture.Height);

			// Build matrix
			NewSubTexture.BuildTransform();

			m_SubTextures.insert({ NewSubTexture.Name, NewSubTexture });
		}

		// json parsed

		// load texture
		m_Texture = std::unique_ptr<gef::Texture>(CreateTextureFromPNG(m_TexturePath.c_str(), m_Platform));
		if (!m_Texture)
		{
			return false;
		}
		m_UUnits = 1.0f / static_cast<float>(m_Width - 1);
		m_VUnits = 1.0f / static_cast<float>(m_Height - 1);

		return true;
	}

	void TextureAtlas::Clear()
	{
		m_SubTextures.clear();
		m_SubTextureCount = -1;

		m_Texture.reset();
	}

	bool TextureAtlas::SubTextureExistsWithName(const std::string& name) const
	{
		return m_SubTextures.find(name) != m_SubTextures.end();
	}



	void TextureAtlas::Apply(gef::Sprite& sprite, const std::string& subTextureName) const
	{
		// check texture exists
		if (m_SubTextures.find(subTextureName) == m_SubTextures.end())
			return;

		sprite.set_texture(m_Texture.get());

		// get subTexture
		const SubTexture& subTexture = m_SubTextures.at(subTextureName);

		// calculate UV's
		sprite.set_uv_position({
			static_cast<float>(subTexture.X) * m_UUnits,
			static_cast<float>(subTexture.Y) * m_VUnits
			});
		sprite.set_uv_width(static_cast<float>(subTexture.Width) * m_UUnits);
		sprite.set_uv_height(static_cast<float>(subTexture.Height) * m_VUnits);
	}


	const gef::Matrix33& TextureAtlas::GetSpriteOffsetTransform(const std::string& subTextureName) const
	{
		if (m_SubTextures.find(subTextureName) == m_SubTextures.end())
			return gef::Matrix33::kIdentity;

		return m_SubTextures.at(subTextureName).Transform;
	}
}
