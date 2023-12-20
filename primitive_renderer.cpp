#include "primitive_renderer.h"
#include "vertex_colour_unlit_shader.h"
#include "system\platform.h"
#include "graphics\vertex_buffer.h"
#include "graphics\index_buffer.h"
#include "graphics\renderer_3d.h"
#include "graphics\shader_interface.h"



PrimitiveRenderer::PrimitiveRenderer(gef::Platform& platform, UInt32 max_num_lines /* = kDefaultMaxNumLines */, UInt32 max_num_triangles /* = kDefaultMaxNumTriangles */)
	: shader_(nullptr)
	, num_lines_(0)
	, num_triangles_(0)
	, lines_vertex_buffer_(nullptr)
	, triangles_vertex_buffer_(nullptr)
	, max_num_lines_(max_num_lines)
	, max_num_triangles_(max_num_triangles)
{
	shader_ = new VertexColourUnlitShader(platform);

	lines_vertex_buffer_ = gef::VertexBuffer::Create(platform);
	lines_vertex_buffer_->Init(platform, nullptr, max_num_lines_ * 2, sizeof(VertexColourUnlitShader::VertexData), false);

	triangles_vertex_buffer_ = gef::VertexBuffer::Create(platform);
	triangles_vertex_buffer_->Init(platform, nullptr, max_num_triangles_ * 3, sizeof(VertexColourUnlitShader::VertexData), false);



	UInt32 max_num_indices = max_num_lines_ * 2 > max_num_triangles_ * 3 ? max_num_lines_ * 2 : max_num_triangles_ * 3;

	UInt32* indices;
	indices = new UInt32[max_num_indices];
	for (UInt32 i = 0; i < max_num_indices; ++i)
		indices[i] = i;

	lines_index_buffer_ = gef::IndexBuffer::Create(platform);
	lines_index_buffer_->Init(platform, indices, max_num_lines * 2, sizeof(UInt32));

	triangles_index_buffer_ = gef::IndexBuffer::Create(platform);
	triangles_index_buffer_->Init(platform, indices, max_num_triangles_ * 3, sizeof(UInt32));

	delete indices;
	indices = nullptr;
}

PrimitiveRenderer::~PrimitiveRenderer()
{
	delete triangles_index_buffer_;
	triangles_index_buffer_ = nullptr;

	delete lines_index_buffer_;
	lines_index_buffer_ = nullptr;

	delete triangles_vertex_buffer_;
	triangles_vertex_buffer_ = nullptr;

	delete lines_vertex_buffer_;
	lines_vertex_buffer_ = nullptr;

	delete shader_;
	shader_ = nullptr;
}

void PrimitiveRenderer::Render(gef::Renderer3D& renderer_3d)
{
	lines_vertex_buffer_->Update(renderer_3d.platform());
	triangles_vertex_buffer_->Update(renderer_3d.platform());

	shader_->SetSceneData(renderer_3d.view_matrix(), renderer_3d.projection_matrix());

	gef::Matrix44 world_transform;
	world_transform.SetIdentity();
	shader_->SetMeshData(world_transform);

	shader_->device_interface()->UseProgram();

	DrawTriangles(renderer_3d);
	DrawLines(renderer_3d);
}

void PrimitiveRenderer::Reset()
{
	num_lines_ = 0;
	num_triangles_ = 0;
}

void PrimitiveRenderer::AddLine(const gef::Vector4& start, const gef::Vector4& end, const gef::Colour& colour)
{
	if (num_lines_ < max_num_lines_)
	{
		VertexColourUnlitShader::VertexData* vertices = static_cast<VertexColourUnlitShader::VertexData*>(lines_vertex_buffer_->vertex_data());

		vertices += num_lines_ * 2;

		vertices->x = start.x();
		vertices->y = start.y();
		vertices->z = start.z();
		vertices->r = colour.r;
		vertices->g = colour.g;
		vertices->b = colour.b;
		vertices->a = colour.a;
		vertices++;
		vertices->x = end.x();
		vertices->y = end.y();
		vertices->z = end.z();
		vertices->r = colour.r;
		vertices->g = colour.g;
		vertices->b = colour.b;
		vertices->a = colour.a;
		num_lines_++;
	}
}

void PrimitiveRenderer::AddTriangle(const gef::Vector4& v0, const gef::Vector4& v1, const gef::Vector4& v2, const gef::Colour& colour)
{
	if (num_triangles_ < max_num_triangles_)
	{
		VertexColourUnlitShader::VertexData* vertices = static_cast<VertexColourUnlitShader::VertexData*>(triangles_vertex_buffer_->vertex_data());

		vertices += num_triangles_ * 3;

		vertices->x = v0.x();
		vertices->y = v0.y();
		vertices->z = v0.z();
		vertices->r = colour.r;
		vertices->g = colour.g;
		vertices->b = colour.b;
		vertices->a = colour.a;
		vertices++;
		vertices->x = v1.x();
		vertices->y = v1.y();
		vertices->z = v1.z();
		vertices->r = colour.r;
		vertices->g = colour.g;
		vertices->b = colour.b;
		vertices->a = colour.a;
		vertices++;
		vertices->x = v2.x();
		vertices->y = v2.y();
		vertices->z = v2.z();
		vertices->r = colour.r;
		vertices->g = colour.g;
		vertices->b = colour.b;
		vertices->a = colour.a;
		num_triangles_++;
	}
}

void PrimitiveRenderer::DrawLines(gef::Renderer3D& renderer_3d)
{
	renderer_3d.SetFillMode(gef::Renderer3D::kLines);

	lines_vertex_buffer_->Bind(renderer_3d.platform());

	// vertex format must be set after the vertex buffer is bound
	shader_->device_interface()->SetVertexFormat();

	shader_->device_interface()->SetVariableData();

	lines_index_buffer_->Bind(renderer_3d.platform());

	renderer_3d.SetPrimitiveType(gef::LINE_LIST);
	renderer_3d.DrawPrimitive(lines_index_buffer_, num_lines_ * 2);

	lines_index_buffer_->Unbind(renderer_3d.platform());
	shader_->device_interface()->ClearVertexFormat();
	lines_vertex_buffer_->Unbind(renderer_3d.platform());

	renderer_3d.SetFillMode(gef::Renderer3D::kSolid);
}

void PrimitiveRenderer::DrawTriangles(gef::Renderer3D& renderer_3d)
{
	triangles_vertex_buffer_->Bind(renderer_3d.platform());

	// vertex format must be set after the vertex buffer is bound
	shader_->device_interface()->SetVertexFormat();

	shader_->device_interface()->SetVariableData();

	triangles_index_buffer_->Bind(renderer_3d.platform());

	renderer_3d.SetPrimitiveType(gef::TRIANGLE_LIST);
	renderer_3d.DrawPrimitive(triangles_index_buffer_, num_triangles_ * 3);

	triangles_index_buffer_->Unbind(renderer_3d.platform());

	shader_->device_interface()->ClearVertexFormat();
	triangles_vertex_buffer_->Unbind(renderer_3d.platform());
}

