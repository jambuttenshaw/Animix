#ifndef _PRIMITIVE_RENDERER_H
#define _PRIMITIVE_RENDERER_H

#include <gef.h>

namespace gef
{
	class Platform;
	class Renderer3D;
	class Vector4;
	class Colour;
	class VertexBuffer;
	class IndexBuffer;
}

class VertexColourUnlitShader;

class PrimitiveRenderer
{
public:
	PrimitiveRenderer(gef::Platform& platform, UInt32 max_num_lines = kDefaultMaxNumLines, UInt32 max_num_triangles = kDefaultMaxNumTriangles);
	~PrimitiveRenderer();

	void Render(gef::Renderer3D& renderer_3d);

	void Reset();

	void AddLine(const gef::Vector4& start, const gef::Vector4& end, const gef::Colour& colour);
	void AddTriangle(const gef::Vector4& v0, const gef::Vector4& v1, const gef::Vector4& v2, const gef::Colour& colour);

	void DrawLines(gef::Renderer3D& renderer_3d);
	void DrawTriangles(gef::Renderer3D& renderer_3d);;

	const static int kDefaultMaxNumLines = 512;
	const static int kDefaultMaxNumTriangles = 512;
private:
	VertexColourUnlitShader* shader_;
	UInt32 num_lines_;
	UInt32 num_triangles_;
	gef::VertexBuffer* lines_vertex_buffer_;
	gef::VertexBuffer* triangles_vertex_buffer_;
	gef::IndexBuffer* lines_index_buffer_;
	gef::IndexBuffer* triangles_index_buffer_;
	UInt32 max_num_lines_;
	UInt32 max_num_triangles_;
};

#endif // _PRIMITIVE_RENDERER_H