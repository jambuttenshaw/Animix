#pragma once

#include <memory>
#include <vector>

#include "../AnimixTypes.h"
#include "BlendNode.h"

namespace Animix
{

	class BlendTree
	{
	public:
		BlendTree() = default;
		~BlendTree() = default;

		// Disable copying
		BlendTree(const BlendTree&) = delete;
		BlendTree& operator=(const BlendTree&) = delete;

		// Default moving
		BlendTree(BlendTree&&) = default;
		BlendTree& operator=(BlendTree&&) = default;


		bool TickAndEvaluateTree(SkeletonPose& outPose, float timeScale = 1.0f) const;

		float CalculateRemainingDuration() const;
		float CalculateDuration() const;

		void Start();
		inline float GetStartTime() const { return m_StartTime; }


		// API to manipulate the blend tree
		template<typename T>
		T* CreateNode()
		{
			static_assert(std::is_base_of<BlendNode, T>::value, "T is not a type of BlendNode");

			m_BlendTree.emplace_back(std::make_unique<T>(this, m_BlendTree.size()));
			return static_cast<T*>(m_BlendTree.back().get());
		}

		// The node at index does not require to be completely valid
		// (ie have valid inputs and parameters)
		// but must at least exist
		bool DoesNodeExist(BlendNodeID index) const;
		BlendNode* GetNode(BlendNodeID index) const;

		bool SetOutputNode(BlendNodeID index);

	private:
		inline BlendNode* Root() const { return m_BlendTree.at(m_OutputNode).get(); }

	private:
		// Collection of all nodes present anywhere in the tree
		// Relationships in the tree are stored via indices into this vector
		std::vector<std::unique_ptr<BlendNode>> m_BlendTree;
		// Which node in the tree should be used for output
		size_t m_OutputNode = 0;

		// The global clock timestamp at which that this state began
		float m_StartTime = 0.0f;
	};
}
