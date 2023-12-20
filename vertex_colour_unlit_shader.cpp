/*
 * default_3d_shader.cpp
 *
 *  Created on: 29 Jan 2015
 *      Author: grant
 */

#include "vertex_colour_unlit_shader.h"
#include <graphics/shader_interface.h>
#include <graphics/mesh_instance.h>
#include <graphics/primitive.h>
#include <system/debug_log.h>
#include <graphics/mesh.h>
#include <graphics/material.h>
#include <graphics/colour.h>
#include <graphics/default_3d_shader_data.h>


VertexColourUnlitShader::VertexColourUnlitShader(const gef::Platform& platform)
	: gef::Shader(platform)
	, wvp_matrix_variable_index_(-1)
{
	bool success = true;

	// load vertex shader source in from a file
	char* vs_shader_source = NULL;
	Int32 vs_shader_source_length = 0;
	success = LoadShader("vertex_colour_unlit_vs", "shaders", &vs_shader_source, vs_shader_source_length, platform);

	char* ps_shader_source = NULL;
	Int32 ps_shader_source_length = 0;
	success = LoadShader("vertex_colour_unlit_ps", "shaders", &ps_shader_source, ps_shader_source_length, platform);

	device_interface_->SetVertexShaderSource(vs_shader_source, vs_shader_source_length);
	device_interface_->SetPixelShaderSource(ps_shader_source, ps_shader_source_length);

	delete[] vs_shader_source;
	vs_shader_source = NULL;
	delete[] ps_shader_source;
	ps_shader_source = NULL;

	wvp_matrix_variable_index_ = device_interface_->AddVertexShaderVariable("wvp", gef::ShaderInterface::kMatrix44);
	device_interface_->AddVertexParameter("position", gef::ShaderInterface::kVector3, 0, "POSITION", 0);
	device_interface_->AddVertexParameter("colour", gef::ShaderInterface::kVector4, 12, "COLOR", 0);
	device_interface_->set_vertex_size(sizeof(VertexColourUnlitShader::VertexData));
	device_interface_->CreateVertexFormat();

	success = device_interface_->CreateProgram();
}

VertexColourUnlitShader::VertexColourUnlitShader()
	: wvp_matrix_variable_index_(-1)
{

}

VertexColourUnlitShader::~VertexColourUnlitShader()
{
}



void VertexColourUnlitShader::SetSceneData(const gef::Matrix44& view_matrix, const gef::Matrix44& projection_matrix)
{

	view_projection_matrix_ = view_matrix * projection_matrix;
}

void VertexColourUnlitShader::SetMeshData(const gef::Matrix44& transform)
{
	// calculate world view projection matrix
	gef::Matrix44 wvp = transform * view_projection_matrix_;

	// take transpose of matrices for the shaders
	gef::Matrix44 wvpT;
	wvpT.Transpose(wvp);

	device_interface_->SetVertexShaderVariable(wvp_matrix_variable_index_, &wvpT);
}

