#pragma once

#include "maths/vector4.h"


namespace Animix
{
	struct Vector3
	{
		float X = 0.0f;
		float Y = 0.0f;
		float Z = 0.0f;

		gef::Vector4 ToVector4() const { return { X, Y, Z }; }

		static Vector3 Lerp(const Vector3& a, const Vector3& b, float t)
		{
			return {
				(1.0f - t) * a.X + t * b.X,
				(1.0f - t) * a.Y + t * b.Y,
				(1.0f - t) * a.Z + t * b.Z
			};
		}
	};
}
