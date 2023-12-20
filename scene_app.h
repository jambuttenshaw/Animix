#pragma once

#include <memory>

#include <system/application.h>

#include <maths/vector4.h>

#include <input/input_manager.h>

#include <graphics/renderer_3d.h>
#include <graphics/sprite_renderer.h>
#include <graphics/scene.h>
#include <graphics/mesh.h>
#include <graphics/mesh_instance.h>

#include "CustomCharacters.h"
#include "primitive_builder.h"

#include "Animix/AnimationEngine.h"
#include "Animix/AniPhysix/PhysicsWorld.h"

#include "Animix2D/SpriteArmature.h"
#include "Animix2D/TextureAtlas.h"


// FRAMEWORK FORWARD DECLARATIONS
namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Renderer3D;
	class InputManager;
	class Font;
	class Scene;
}


class SceneApp : public gef::Application
{
public:
	SceneApp(gef::Platform& platform);

	void Init() override;
	void CleanUp() override;

	bool Update(float frame_time) override;
	void Render() override;

	// For different demo modes
	void Update2D(float frame_time);
	void Update3D(float frame_time);

	void Render2D();
	void Render3D();

	void DrawImGui2D();
	void DrawImGui3D();

private:
	void SetupLights() const;

	// ImGui
	void ImGuiBeginFrame();
	void ImGuiEndFrame();
	void DrawImGui();
	void UpdateImGuiIO();

	std::unique_ptr<gef::Mesh> GetFirstMesh(gef::Scene* scene) const;

private:
	// Core objects
	std::unique_ptr<gef::SpriteRenderer> m_SpriteRenderer;
	std::unique_ptr<gef::Renderer3D> m_Renderer3D;
	std::unique_ptr<gef::InputManager> m_InputManager;

	std::unique_ptr<PrimitiveBuilder> m_PrimitiveBuilder;

	// Which demo to show
	bool m_Show3D = false;

	// 3D System
	std::unique_ptr<AniPhysix::PhysicsWorld> m_PhysicsWorld;
	std::unique_ptr<Animix::AnimationEngine> m_AnimationEngine;

	// Model
	std::unique_ptr<gef::Scene> m_ModelScene;
	std::unique_ptr<gef::Mesh> m_Mesh;
	std::unique_ptr<gef::MeshInstance> m_Player;

	std::unique_ptr<gef::Mesh> m_FloorMesh;
	gef::MeshInstance m_Floor;

	bool m_ShowPhysics = false;

	// Animator and parameters
	Animix::Animator* m_PlayerAnimator = nullptr;

	// Parameters controlling the player
	float m_WalkSpeed = 0.0f;
	float m_WalkDir = 0.0f;
	float m_Injured = 0.0f;


	// 2D System

	std::unique_ptr<Animix2D::TextureAtlas> m_DinoTextureAtlas;
	std::unique_ptr<Animix2D::TextureAtlas> m_BoyTextureAtlas;
	std::unique_ptr<Animix2D::SpriteArmature> m_DinoArmature;
	std::unique_ptr<Animix2D::SpriteArmature> m_BoyArmature;

	std::unique_ptr<DinoCharacter> m_DinoCharacter;
	std::unique_ptr<BoyCharacter> m_BoyCharacter;

	// Misc

	float m_FPS = 0.0f;

	gef::Vector4 m_CameraEye{ -1.0f, 1.0f, 4.0f };
	gef::Vector4 m_CameraLookAt{ 0.0f, 1.0f, 0.0f };
	gef::Vector4 m_CameraUp{ 0.0f, 1.0f, 0.0f };
	float m_CameraFOV;
	float m_NearPlane = 0.1f;
	float m_FarPlane = 100.0f;
};
