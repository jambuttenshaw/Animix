#include "BlendNode.h"

#include <cassert>

#include "BlendTree.h"
#include "Animix/AnimationEngine.h"


namespace Animix
{
	BlendNode::BlendNode(BlendTree* tree, size_t index)
		: m_Tree(tree)
		, m_TreeIndex(index)
	{
		assert(m_Tree);
	}

	bool BlendNode::SetInput(size_t inputIndex, BlendNodeID inputNode)
	{
		// Make sure child is a valid index
		if (!m_Tree->DoesNodeExist(inputNode))
			return false;

		if (inputIndex >= m_Inputs.size())
			return false;

		m_Inputs[inputIndex] = inputNode;
		return true;
	}

	BlendNode* BlendNode::GetInputNode(size_t index) const
	{
		return m_Tree->GetNode(m_Inputs.at(index));
	}

}
