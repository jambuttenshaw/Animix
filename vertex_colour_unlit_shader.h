#ifndef _VERTEX_COLOUR_UNLIT_SHADER_H_
#define _VERTEX_COLOUR_UNLIT_SHADER_H_

#include <graphics/shader.h>
#include <gef.h>
#include <maths/vector4.h>
#include <maths/matrix44.h>

#define MAX_NUM_POINT_LIGHTS 4

namespace gef
{
	class MeshInstance;
	class Material;
}

	class VertexColourUnlitShader: public gef::Shader
	{
	public:
		struct VertexData
		{
			float x;
			float y;
			float z;
			float r;
			float g;
			float b;
			float a;
		};

		VertexColourUnlitShader(const gef::Platform& platform);
		virtual ~VertexColourUnlitShader();

		void SetSceneData(const gef::Matrix44& view_matrix, const gef::Matrix44& projection_matrix);
		void SetMeshData(const gef::Matrix44& transform);
	protected:
		VertexColourUnlitShader();

		Int32 wvp_matrix_variable_index_;
		gef::Matrix44 view_projection_matrix_;
	};


#endif /* _VERTEX_COLOUR_UNLIT_SHADER_H_ */
