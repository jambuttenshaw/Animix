#include "GeneralLinearBlendNode.h"

#include <algorithm>

#include "BlendTree.h"


namespace Animix
{

	GeneralLinearBlendNode::GeneralLinearBlendNode(BlendTree* tree, size_t index)
		: BlendNode(tree, index)
	{
		// This node requires at least two inputs
		m_Inputs.resize(2, -1);
		// default values for input alphas
		m_InputAlphas = {
			{0, 0.0f },
			{1, 1.0f }
		};
	}

	bool GeneralLinearBlendNode::IsValid() const
	{
		if (m_Inputs.size() != m_InputAlphas.size())
			return false;

		if (m_CurrentInA >= m_Inputs.size() || m_CurrentInB >= m_Inputs.size())
			return false;

		// Validate children
		if (!(m_Tree->DoesNodeExist(m_Inputs.at(m_CurrentInA))
			&& m_Tree->DoesNodeExist(m_Inputs.at(m_CurrentInB))))
			return false;
		
		if (!(GetInputNode(m_CurrentInA)->IsValid()
			&& GetInputNode(m_CurrentInB)->IsValid()))
			return false;

		if (m_ScaleClips)
		{
			if (GetInputNode(m_CurrentInA)->CalculateDuration() == 0.0f ||
				GetInputNode(m_CurrentInB)->CalculateDuration() == 0.0f)
				return false;
		}

		return true;
	}

	void GeneralLinearBlendNode::Tick(float timeScale)
	{
		float in0_scale = 1.0f;
		float in1_scale = 1.0f;

		// Check for clip scaling
		if (m_ScaleClips)
		{
			const float dur0 = GetInputNode(m_CurrentInA)->CalculateDuration();
			const float dur1 = GetInputNode(m_CurrentInB)->CalculateDuration();
			const float targetDuration = (1.0f - m_MappedAlpha) * dur0 + m_MappedAlpha * dur1;

			// Calculate scale that will make both clips the same duration
			in0_scale = targetDuration / dur1;
			in1_scale = targetDuration / dur0;
		}

		GetInputNode(m_CurrentInA)->Tick(in0_scale * timeScale);
		GetInputNode(m_CurrentInB)->Tick(in1_scale * timeScale);

		// Tick the rest of the inputs to keep them in sync
		for (size_t input = 0; input < m_Inputs.size(); input++)
		{
			if (input == m_CurrentInA || input == m_CurrentInB)
				continue;

			float scale = 1.0f;
			if (m_ScaleClips)
			{
				// work out whether to tick this animation at the speed of A or B
				// this can be decided by whichever of A or B is closest to this in the blend space
				// basically work out if the current alpha is greater or less than this inputs alpha
				const auto it = std::find_if(m_InputAlphas.begin(), m_InputAlphas.end(), 
					[input](const InputData& in) -> bool
					{
						return in.InputIndex == input;
					});
				scale = it->Alpha > m_Alpha ? in0_scale : in1_scale;
			}

			GetInputNode(input)->Tick(scale * timeScale);
		}
	}

	SkeletonPose GeneralLinearBlendNode::Evaluate() const
	{
		// Perform blending
		const SkeletonPose pose1(GetInputNode(m_CurrentInA)->Evaluate());
		const SkeletonPose pose2(GetInputNode(m_CurrentInB)->Evaluate());

		return SkeletonPose::Lerp(pose1, pose2, m_MappedAlpha);
	}

	float GeneralLinearBlendNode::CalculateDuration() const
	{
		return std::max(GetInputNode(m_CurrentInA)->CalculateDuration(), GetInputNode(m_CurrentInB)->CalculateDuration());
	}

	bool GeneralLinearBlendNode::IsLooping() const
	{
		return GetInputNode(m_CurrentInA)->IsLooping() && GetInputNode(m_CurrentInB)->IsLooping();
	}

	bool GeneralLinearBlendNode::HasVariableWithName(const std::string& name)
	{
		if (name == "alpha")
			return true;

		return false;
	}

	ParameterObserver GeneralLinearBlendNode::GetObserverForVariable(const std::string& name)
	{
		if (name == "alpha")
			return GetParamObserver_Alpha();

		return {};
	}

	bool GeneralLinearBlendNode::SetInput(size_t inputIndex, BlendNodeID inputNode)
	{
		// Make sure child is a valid index
		if (!m_Tree->DoesNodeExist(inputNode))
			return false;

		// Inputs can be of any size, so long as there are no gaps
		// This is ensured by only allowing inputs to be added sequentially
		if (inputIndex > m_Inputs.size())
			return false;

		if (inputIndex == m_Inputs.size())
			m_Inputs.push_back(inputNode);
		else
			m_Inputs[inputIndex] = inputNode;

		return true;
	}

	void GeneralLinearBlendNode::SetAlpha(float alpha)
	{
		m_Alpha = alpha;

		// Recalculate the two input nodes and the mapped alpha
		size_t current = 0;
		for (;
			current < m_InputAlphas.size() - 1
			&& m_Alpha > m_InputAlphas[current + 1].Alpha;
			current++)
		{}

		if (current == m_InputAlphas.size() - 1)
		{
			// Special case when m_Alpha is greater than the alpha of any input
			m_CurrentInA = m_InputAlphas[current].InputIndex;
			m_CurrentInB = m_InputAlphas[current].InputIndex;
			m_MappedAlpha = 0.0f;
		}
		else
		{
			m_CurrentInA = m_InputAlphas[current].InputIndex;
			m_CurrentInB = m_InputAlphas[current + 1].InputIndex;

			m_MappedAlpha = (m_Alpha - m_InputAlphas[current].Alpha) / (m_InputAlphas[current + 1].Alpha - m_InputAlphas[current].Alpha);
			// mapped alpha may be less than 0 if m_Alpha is less than m_InputAlphas[m_CurrentInA]
			// Clamp to [0,1]
			m_MappedAlpha = std::min(std::max(m_MappedAlpha, 0.0f), 1.0f);
		}
	}

	void GeneralLinearBlendNode::SetAlphaForInput(size_t inputIndex, float alpha)
	{
		const auto it = std::find_if(m_InputAlphas.begin(), m_InputAlphas.end(), 
			[inputIndex](const InputData& in) -> bool
			{
				return in.InputIndex == inputIndex;
			});

		if (it == m_InputAlphas.end())
			m_InputAlphas.emplace_back(InputData{ inputIndex, alpha });
		else
			it->Alpha = alpha;

		// Sort input alphas by alpha
		std::sort(m_InputAlphas.begin(), m_InputAlphas.end(), 
			[](const InputData& a, const InputData& b) -> bool
			{
				return a.Alpha < b.Alpha;
			});
	}

}