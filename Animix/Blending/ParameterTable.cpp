#include "ParameterTable.h"


namespace Animix
{
	AnimationParameter::AnimationParameter(std::string name, float defaultValue)
		: m_ParamName(std::move(name))
		, m_Value(defaultValue)
	{
	}

	void AnimationParameter::SetValue(float value)
	{
		m_Value = value;
		UpdateObservers();
	}

	void AnimationParameter::UpdateObservers() const
	{
		for (const auto& observer : m_Observers)
			if (observer) observer(m_Value);
	}

	void AnimationParameter::AddObserver(ParameterObserver&& observer)
	{
		m_Observers.emplace_back(observer);
		m_Observers.back()(m_Value); // Send the current value to the new observer
	}


	void ParameterTable::Clear()
	{
		m_Parameters.clear();
	}

	
	void ParameterTable::CreateParam(const std::string& paramName, float defaultValue)
	{
		if (m_Parameters.find(paramName) == m_Parameters.end())
		{
			m_Parameters.insert({ paramName, std::make_unique<AnimationParameter>(paramName, defaultValue) });
		}
	}

	void ParameterTable::AddParamObserver(const std::string& paramName, ParameterObserver&& observer) const
	{
		m_Parameters.at(paramName)->AddObserver(std::move(observer));
	}

	void ParameterTable::SetParam(const std::string& paramName, float value) const
	{
		m_Parameters.at(paramName)->SetValue(value);
	}

	bool ParameterTable::ParameterExists(const std::string& paramName) const
	{
		return m_Parameters.find(paramName) != m_Parameters.end();
	}

	float ParameterTable::GetParameter(const std::string& paramName) const
	{
		return m_Parameters.at(paramName)->GetValue();
	}

	void ParameterTable::UpdateAllParameters() const
	{
		for (const auto& param : m_Parameters)
			param.second->UpdateObservers();
	}
}
