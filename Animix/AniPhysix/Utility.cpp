#include "Utility.h"

#include "maths/quaternion.h"


namespace AniPhysix
{

	gef::Matrix44 TransformToMatrix(const btTransform& transform, bool cmToM)
	{
		gef::Matrix44 result;

		btQuaternion rotation = transform.getRotation();
		btVector3 position = transform.getOrigin();
		if (cmToM)
			position *= 100.0f;

		result.Rotation(gef::Quaternion(rotation.x(), rotation.y(), rotation.z(), rotation.w()));
		result.SetTranslation(gef::Vector4(position.x(), position.y(), position.z()));

		return result;
	}

	btTransform MatrixToTransform(const gef::Matrix44& mtx, bool MtoCM)
	{
		gef::Vector4 mtx_position = mtx.GetTranslation();
		if (MtoCM)
			mtx_position *= 0.01f;

		gef::Quaternion mtx_rot;
		mtx_rot.SetFromMatrix(mtx);

		btTransform result;
		result.setOrigin(btVector3(mtx_position.x(), mtx_position.y(), mtx_position.z()));
		result.setRotation(btQuaternion(mtx_rot.x, mtx_rot.y, mtx_rot.z, mtx_rot.w));

		return result;
	}
}
