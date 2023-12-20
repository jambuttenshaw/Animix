#pragma once

#include <string>
#include <vector>

#include "Skeleton.h"

#include "rapidjson/document.h"


// gef forward declarations
namespace gef
{
	class Scene;
}

namespace Animix
{
	/**
	 * This class provides loader functions to make Animix work with gef's proprietary file formats
	 */
	class AnimixLoader
	{
	public:
		// static class
		AnimixLoader() = delete;

		// Returns number of skeletons loaded from scene
		static uint32_t LoadSkeletonsFromScene(const std::string& filename, std::vector<SkeletonID>& skeletons);

		// Loads and then names a single animation from a scene
		// Unfortunately this function is required since gef does not include the string table used to hash all the animation data
		// with the animation data, so there is no way to retrieve the name of the animation from the .scn file itself
		static bool LoadAndNameAnimationFromScene(const std::string& filename, SkeletonID target, const std::string& animName);

		static bool LoadAnimatorFromJSON(class Animator& animator, const std::string& filename);

	private:
		// Helper functions
		static bool ReadGefSceneFromFile(const std::string& filename, gef::Scene* scene);

		static bool LoadBlendNodeFromJSON(class Animator& animator, class BlendTree* blendTree, size_t& outNodeIndex, const rapidjson::Value& json);
	};
}
