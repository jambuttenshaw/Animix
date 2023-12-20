#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "../AnimixTypes.h"


namespace Animix
{
	/*
	 * This object represents a single parameter in the table
	 * A parameter may have many observers - many nodes may be controlled by the same parameter
	 */
	class AnimationParameter
	{
	public:
		AnimationParameter(std::string name, float defaultValue);

		void SetValue(float value);
		inline float GetValue() const { return m_Value; }

		void AddObserver(ParameterObserver&& observer);
		void UpdateObservers() const;

	private:
		std::string m_ParamName;
		float m_Value = 0.0f;
		std::vector<ParameterObserver> m_Observers;
	};


	/*
	 * A class to represent a table of parameters used by an animator
	 * It allows blend nodes to register callbacks that will be called when the value of a parameter changes
	 *
	 * There is two-way communication here:
	 *  - The parameter table communicates to blend nodes when a parameter has changed
	 *	- Blend nodes need to ensure the parameter they require the value of exists
	 */
	class ParameterTable
	{
	public:
		void Clear();

		void CreateParam(const std::string& paramName, float defaultValue);
		void AddParamObserver(const std::string& paramName, ParameterObserver&& observer) const;

		// Will call any relevant observers of this parameter
		void SetParam(const std::string& paramName, float value) const;

		bool ParameterExists(const std::string& paramName) const;
		float GetParameter(const std::string& paramName) const;


		// In some cases it may be required to fire all observers to ensure they have the most up to date
		// value of all parameters
		void UpdateAllParameters() const;

	private:
		// Table of parameter values
		std::unordered_map<std::string, std::unique_ptr<AnimationParameter>> m_Parameters;
	};
}
