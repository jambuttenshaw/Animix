#pragma once

#include "maths/matrix44.h"
#include "LinearMath/btTransform.h"


namespace AniPhysix
{
	gef::Matrix44 TransformToMatrix(const btTransform& transform, bool cmToM = true);
	btTransform MatrixToTransform(const gef::Matrix44& mtx, bool MtoCM = true);
}
