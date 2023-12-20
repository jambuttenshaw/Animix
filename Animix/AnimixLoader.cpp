#include "AnimixLoader.h"

#include "AnimationEngine.h"
#include "Skeleton.h"

// gef Includes
#include "animation/skeleton.h"
#include "animation/animation.h"
#include "graphics/scene.h"
#include "system/file.h"
#include "system/memory_stream_buffer.h"

#include <fstream>

#include "Animator.h"
#include "Blending/BilinearBlendNode.h"
#include "Blending/BlendTree.h"
#include "Blending/BlendNode.h"
#include "Blending/ClipSampleNode.h"
#include "Blending/GeneralLinearBlendNode.h"
#include "Blending/LinearBlendNode.h"
#include "Blending/RagdollNode.h"

#include "rapidjson/istreamwrapper.h"


namespace Animix
{
	// handy macros for quick error checking
#define CHECK_MEMBER_REQUIRED(json, name) if (!(json).HasMember(name)) { return false; }


	uint32_t AnimixLoader::LoadSkeletonsFromScene(const std::string& filename, std::vector<SkeletonID>& skeletons)
	{
		// Read file into gef stream
		const auto scene = std::make_unique<gef::Scene>();
		const bool success = ReadGefSceneFromFile(filename, scene.get());
		if (!success)
			return 0;

		uint32_t skeletonsCreated = 0;

		for (const auto gefSkeleton : scene->skeletons)
		{
			// construct an animix skeleton from the gef skeleton

			// create joints for animix skeleton from gef skeleton
			std::vector<Joint> newJoints(gefSkeleton->joint_count());
			for (Int32 j = 0; j < gefSkeleton->joint_count(); j++)
			{
				const auto& gefJoint = gefSkeleton->joint(j);
				auto& joint = newJoints[j];

				//scene->string_id_table.Find(gefJoint.name_id, joint.Name);
				joint.Name = gefJoint.name_id;
				joint.Parent = static_cast<int32_t>(gefJoint.parent);
				joint.InvBindPose = gefJoint.inv_bind_pose;
			}

			const SkeletonID sID = g_AnimixEngine->CreateSkeleton(std::move(newJoints));

			skeletons.push_back(sID);
			skeletonsCreated++;
		}

		return skeletonsCreated;
	}

	bool AnimixLoader::LoadAndNameAnimationFromScene(const std::string& filename, SkeletonID target, const std::string& animName)
	{
		// Read file into gef stream
		const auto scene = std::make_unique<gef::Scene>();
		const bool success = ReadGefSceneFromFile(filename, scene.get());
		if (!success)
			return false;
		if (scene->animations.empty())
			return false;

		const auto skeleton = g_AnimixEngine->GetSkeleton(target);

		// To assist with constructing the animation, a map of joint names to indices is constructed
		std::unordered_map<gef::StringId, size_t> jointIndices;
		for (size_t jointIndex = 0; jointIndex < skeleton->Joints.size(); jointIndex++)
			jointIndices[skeleton->Joints[jointIndex].Name] = jointIndex;

		// construct an animix animation from a gef animation
		const gef::Animation* gefAnim = scene->animations.begin()->second;
		AnimationClip* animClip = g_AnimixEngine->CreateAnimationClip(animName, target);

		animClip->SetDuration(gefAnim->duration());

		// Populate animation with animation data
		for (const auto& gefJoint : gefAnim->anim_nodes())
		{
			// get joint name and index
			const size_t jointIndex = jointIndices.at(gefJoint.first);

			// Get all joint samples from the animation
			JointSamples jointSamples;
			// joint node type should be transform
			if (gefJoint.second->type() == gef::AnimNode::kTransform)
			{
				// Construct position keys
				for (const auto& key : dynamic_cast<gef::TransformAnimNode*>(gefJoint.second)->translation_keys())
				{
					jointSamples.PositionKeys.emplace_back(PositionKey{ key.time, key.value.x(), key.value.y(), key.value.z() });
				}

				// Construct rotation keys
				for (const auto& key : dynamic_cast<gef::TransformAnimNode*>(gefJoint.second)->rotation_keys())
				{
					jointSamples.RotationKeys.emplace_back(RotationKey{ key.time, key.value });
				}
			}

			// Move joint samples into the animation object
			animClip->SetJointSamples(jointIndex, std::move(jointSamples));
		}

		return true;
	}


	


	bool AnimixLoader::LoadAnimatorFromJSON(Animator& animator, const std::string& filename)
	{
		rapidjson::Document DocJSON;

		// load json data
		{
			std::ifstream input_file(filename);
			if (!input_file.good())
				return false;

			rapidjson::IStreamWrapper stream(input_file);

			DocJSON.ParseStream(stream);
			if (DocJSON.HasParseError())
				return false;
		}

		// first parse parameter table
		if (DocJSON.HasMember("param"))
		{
			// Iterate over params and add them to param table
			for (const auto& paramJSON : DocJSON["param"].GetArray())
			{
				CHECK_MEMBER_REQUIRED(paramJSON, "name")
				CHECK_MEMBER_REQUIRED(paramJSON, "default")
				animator.GetParameterTable()->CreateParam(paramJSON["name"].GetString(), paramJSON["default"].GetFloat());
			}
		}

		// then parse states
		if (DocJSON.HasMember("state"))
		{
			for (const auto& stateJSON : DocJSON["state"].GetArray())
			{
				CHECK_MEMBER_REQUIRED(stateJSON, "name")
				AnimatorState* newState = animator.CreateState(stateJSON["name"].GetString());

				// Load blend tree
				// Blend tree is required, as without a tree no animation will be played
				CHECK_MEMBER_REQUIRED(stateJSON, "tree")

				BlendTree* blendTree = newState->GetBlendTree();
				size_t nodeIndex;

				if (!LoadBlendNodeFromJSON(animator, blendTree, nodeIndex, stateJSON["tree"]))
					return false;

				if (!blendTree->SetOutputNode(nodeIndex))
					return false;


				// Load transitions
				if (stateJSON.HasMember("transition"))
				{
					for (const auto& transitionJSON : stateJSON["transition"].GetArray())
					{
						// Name and destination are required
						CHECK_MEMBER_REQUIRED(transitionJSON, "name")
						CHECK_MEMBER_REQUIRED(transitionJSON, "destination")

						StateTransition newTransition{
								transitionJSON["destination"].GetString(),
								TransitionType::Immediate,
								0.0f
						};

						if (transitionJSON.HasMember("type"))
							newTransition.Type = StateTransition::TransitionTypeFromString(transitionJSON["type"].GetString());

						if (transitionJSON.HasMember("duration"))
							newTransition.Duration = transitionJSON["duration"].GetFloat();

						newState->AddTransition(transitionJSON["name"].GetString(), std::move(newTransition));
					}
				}

				// Does the state define an end transition
				if (stateJSON.HasMember("endTransition"))
				{
					newState->SetEndTransition(stateJSON["endTransition"].GetString());
				}
			}
		}

		return true;
	}


	bool AnimixLoader::ReadGefSceneFromFile(const std::string& filename, gef::Scene* scene)
	{
		// Copied straight from gef's own implementation, except it doesn't require a platform object to be passed as a parameter
		// the original implementation didn't actually use the platform, but it prohibited animix from making use of the existing function

		bool success = true;
		void* file_data = NULL;
		gef::File* file = gef::File::Create();
		Int32 file_size;

		success = file->Open(filename.c_str());
		if (success)
		{
			success = file->GetSize(file_size);
			if (success)
			{
				file_data = malloc(file_size);
				success = file_data != NULL;
				if (success)
				{
					Int32 bytes_read;
					success = file->Read(file_data, file_size, bytes_read);
					if (success)
						success = bytes_read == file_size;
				}

				if (success)
				{
					gef::MemoryStreamBuffer stream_buffer((char*)file_data, file_size);

					std::istream input_stream(&stream_buffer);
					success = scene->ReadScene(input_stream);

					// don't need the font file data any more
					free(file_data);
					file_data = NULL;
				}

			}

			file->Close();
		}
		return success;
	}


	bool AnimixLoader::LoadBlendNodeFromJSON(class Animator& animator, BlendTree* blendTree, size_t& outNodeIndex, const rapidjson::Value& json)
	{
		BlendNode* blendNode = nullptr;

		CHECK_MEMBER_REQUIRED(json, "type")
		if (json["type"] == "clipSample")
		{
			const auto clipSampleNode = blendTree->CreateNode<ClipSampleNode>();

			// Assign clip sample node specific properties
			CHECK_MEMBER_REQUIRED(json, "clip")
				clipSampleNode->SetClip(json["clip"].GetString());

			if (json.HasMember("looping"))
				clipSampleNode->SetLooping(json["looping"].GetBool());

			blendNode = clipSampleNode;
		}
		else if (json["type"] == "linearBlend")
		{
			const auto linearBlendNode = blendTree->CreateNode<LinearBlendNode>();

			if (json.HasMember("alpha"))
				linearBlendNode->SetAlpha(json["alpha"].GetFloat());
			if (json.HasMember("scaleClips"))
				linearBlendNode->SetScaleClips(json["scaleClips"].GetBool());

			blendNode = linearBlendNode;
		}
		else if (json["type"] == "generalLinearBlend")
		{
			const auto generalLinearBlendNode = blendTree->CreateNode<GeneralLinearBlendNode>();

			if (json.HasMember("alpha"))
				generalLinearBlendNode->SetAlpha(json["alpha"].GetFloat());
			if (json.HasMember("scaleClips"))
				generalLinearBlendNode->SetScaleClips(json["scaleClips"].GetBool());
			// General linear blend can also specify the alpha values at which to place the inputs
			if (json.HasMember("inputAlpha"))
			{
				for (const auto& inputAlphaJSON : json["inputAlpha"].GetArray())
				{
					CHECK_MEMBER_REQUIRED(inputAlphaJSON, "input")
					CHECK_MEMBER_REQUIRED(inputAlphaJSON, "alpha")
					generalLinearBlendNode->SetAlphaForInput(inputAlphaJSON["input"].GetInt(), inputAlphaJSON["alpha"].GetFloat());
				}
			}

			blendNode = generalLinearBlendNode;
		}
		else if (json["type"] == "bilinearBlend")
		{
			const auto bilinearBlendNode = blendTree->CreateNode<BilinearBlendNode>();

			if (json.HasMember("alpha"))
				bilinearBlendNode->SetAlpha(json["alpha"].GetFloat());
			if (json.HasMember("beta"))
				bilinearBlendNode->SetBeta(json["beta"].GetFloat());
			if (json.HasMember("scaleClips"))
				bilinearBlendNode->SetScaleClips(json["scaleClips"].GetBool());

			blendNode = bilinearBlendNode;
		}
		else if (json["type"] == "ragdoll")
		{
			const auto ragdollNode = blendTree->CreateNode<RagdollNode>();
			ragdollNode->SetRagdoll(animator.GetRagdoll());

			blendNode = ragdollNode;
		}

		// Unknown node type
		if (!blendNode)
			return false;

		// Process inputs
		if (json.HasMember("input"))
		{
			for (rapidjson::SizeType inputIndex = 0; inputIndex < json["input"].Size(); inputIndex++)
			{
				const auto& inputJSON = json["input"][inputIndex];

				size_t nodeIndex;
				if (!LoadBlendNodeFromJSON(animator, blendTree, nodeIndex, inputJSON))
					return false;

				if (!blendNode->SetInput(inputIndex, nodeIndex))
					return false;
			}
		}

		// Process observers
		if (json.HasMember("observer"))
		{
			for (const auto& observerJSON : json["observer"].GetArray())
			{
				CHECK_MEMBER_REQUIRED(observerJSON, "name")
				const std::string& varName = observerJSON["name"].GetString();
				CHECK_MEMBER_REQUIRED(observerJSON, "param")
				const std::string& paramName = observerJSON["param"].GetString();

				if (blendNode->HasVariableWithName(varName))
				{
					animator.GetParameterTable()->AddParamObserver(
						paramName,
						blendNode->GetObserverForVariable(varName)
					);
				}
			}
		}

		outNodeIndex = blendNode->GetNodeID();
		return true;
	}
}
