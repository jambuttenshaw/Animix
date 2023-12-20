#pragma once
#include <functional>


namespace Animix
{
	// typedefs used throughout Animix

	// Makes client code more understandable
	using BlendNodeID = size_t;

	using ParameterObserver = std::function<void(float)>;
}
