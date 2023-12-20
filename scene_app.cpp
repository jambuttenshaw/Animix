#include "scene_app.h"
#include <system/platform.h>
#include <input/keyboard.h>
#include <input/touch_input_manager.h>
#include <maths/math_utils.h>

// Animix
#include "Animix/AnimixLoader.h"

// ImGui
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include <platform/d3d11/system/platform_d3d11.h>
#include <platform/d3d11/input/keyboard_d3d11.h>

#include <cassert>

#include "Animix/AniPhysix/Utility.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"


SceneApp::SceneApp(gef::Platform& platform) :
	Application(platform),
	m_CameraFOV(gef::DegToRad(45.0f))
{
}

void SceneApp::Init()
{
	m_SpriteRenderer = std::unique_ptr<gef::SpriteRenderer>(gef::SpriteRenderer::Create(platform_));
	m_Renderer3D = std::unique_ptr<gef::Renderer3D>(gef::Renderer3D::Create(platform_));
	m_InputManager = std::unique_ptr<gef::InputManager>(gef::InputManager::Create(platform_));
	m_PrimitiveBuilder = std::make_unique<PrimitiveBuilder>(platform_);

	SetupLights();

	// Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer bindings
	gef::PlatformD3D11& platform_d3d = static_cast<gef::PlatformD3D11&>(platform_);

	ImGui_ImplWin32_Init(platform_d3d.hwnd());
	ImGui_ImplDX11_Init(platform_d3d.device(), platform_d3d.device_context());

	// Init 3D

	// Load model's mesh and materials
	m_ModelScene = std::make_unique<gef::Scene>();
	m_ModelScene->ReadSceneFromFile(platform_, "xbot/xbot.scn");

	m_ModelScene->CreateMaterials(platform_);
	m_Mesh = GetFirstMesh(m_ModelScene.get());

	// Create physics world
	m_PhysicsWorld = std::make_unique<AniPhysix::PhysicsWorld>();
	m_PhysicsWorld->SetupDebugDrawer(m_Renderer3D.get());

	// Create animix engine
	m_AnimationEngine = std::make_unique<Animix::AnimationEngine>();

	// Load skeletons from scene
	std::vector<Animix::SkeletonID> skeletons;
	assert(Animix::AnimixLoader::LoadSkeletonsFromScene("xbot/xbot.scn", skeletons) > 0);

	// Load animations
	assert(Animix::AnimixLoader::LoadAndNameAnimationFromScene("xbot/xbot@idle.scn", skeletons.front(), "idle"));
	assert(Animix::AnimixLoader::LoadAndNameAnimationFromScene("xbot/xbot@walking.scn", skeletons.front(), "walking"));
	assert(Animix::AnimixLoader::LoadAndNameAnimationFromScene("xbot/xbot@running.scn", skeletons.front(), "running"));
	assert(Animix::AnimixLoader::LoadAndNameAnimationFromScene("xbot/xbot@jump.scn", skeletons.front(), "jump"));
	assert(Animix::AnimixLoader::LoadAndNameAnimationFromScene("xbot/xbot@idleInjured.scn", skeletons.front(), "idleInjured"));
	assert(Animix::AnimixLoader::LoadAndNameAnimationFromScene("xbot/xbot@walkingInjured.scn", skeletons.front(), "walkingInjured"));
	assert(Animix::AnimixLoader::LoadAndNameAnimationFromScene("xbot/xbot@runningInjured.scn", skeletons.front(), "runningInjured"));
	assert(Animix::AnimixLoader::LoadAndNameAnimationFromScene("xbot/xbot@strafeRight.scn", skeletons.front(), "strafeRight"));
	assert(Animix::AnimixLoader::LoadAndNameAnimationFromScene("xbot/xbot@strafeWalkRight.scn", skeletons.front(), "strafeWalkRight"));
	assert(Animix::AnimixLoader::LoadAndNameAnimationFromScene("xbot/xbot@strafeLeft.scn", skeletons.front(), "strafeLeft"));
	assert(Animix::AnimixLoader::LoadAndNameAnimationFromScene("xbot/xbot@strafeWalkLeft.scn", skeletons.front(), "strafeWalkLeft"));
	assert(Animix::AnimixLoader::LoadAndNameAnimationFromScene("xbot/xbot@death.scn", skeletons.front(), "death"));

	m_Player = std::make_unique<gef::MeshInstance>();
	m_Player->set_mesh(m_Mesh.get());

	// Create an animator
	m_PlayerAnimator = m_AnimationEngine->CreateAnimator(skeletons.front());
	m_PlayerAnimator->CreateRagdoll(m_PhysicsWorld->GetWorld(), "xbot\\ragdoll.bullet");
	assert(m_PlayerAnimator->LoadFromJSON("xbot\\xbot_basic_state_machine.json"));

	// Load default values of parameters
	if (m_PlayerAnimator->GetParameterTable()->ParameterExists("walkSpeed"))
		m_WalkSpeed = m_PlayerAnimator->GetParameterTable()->GetParameter("walkSpeed");
	if (m_PlayerAnimator->GetParameterTable()->ParameterExists("walkDir"))
		m_WalkDir = m_PlayerAnimator->GetParameterTable()->GetParameter("walkDir");
	if (m_PlayerAnimator->GetParameterTable()->ParameterExists("injured"))
		m_Injured = m_PlayerAnimator->GetParameterTable()->GetParameter("injured");

	// Create ground
	const btRigidBody* groundRB = m_PhysicsWorld->CreateGround();
	btVector3 minAabb, maxAabb;
	groundRB->getAabb(minAabb, maxAabb);
	// convert to m
	const btVector3 halfExtents = 100.0f * 0.5f * (maxAabb - minAabb);
	m_FloorMesh = std::unique_ptr<gef::Mesh>(m_PrimitiveBuilder->CreateBoxMesh({halfExtents.x(), halfExtents.y(), halfExtents.z()}));

	m_Floor.set_mesh(m_FloorMesh.get());
	m_Floor.set_transform(AniPhysix::TransformToMatrix(groundRB->getWorldTransform()));



	// Init 2D

	// Load texture atlas
	m_DinoTextureAtlas = std::make_unique<Animix2D::TextureAtlas>(platform_);
	bool success = m_DinoTextureAtlas->LoadFromJson("Dragon_tex.json");

	m_BoyTextureAtlas = std::make_unique<Animix2D::TextureAtlas>(platform_);
	success = m_BoyTextureAtlas->LoadFromJson("boy-attack_tex.json");


	// Load skeleton
	m_DinoArmature = std::make_unique<Animix2D::SpriteArmature>();
	success = m_DinoArmature->LoadFromJson("Dragon_ske.json");
	success = m_DinoArmature->AssociateTextureAtlas(m_DinoTextureAtlas.get());

	m_BoyArmature = std::make_unique<Animix2D::SpriteArmature>();
	success = m_BoyArmature->LoadFromJson("boy-attack_ske.json");
	success = m_BoyArmature->AssociateTextureAtlas(m_BoyTextureAtlas.get());


	// create characters
	m_DinoCharacter = std::make_unique<DinoCharacter>(m_InputManager.get(), m_DinoArmature.get());
	m_DinoCharacter->SetPosition(
		gef::Vector2(
			0.5f * static_cast<float>(platform_.width()),
			0.5f * static_cast<float>(platform_.height()))
	);
	m_DinoCharacter->SetScale(0.25f);

	m_BoyCharacter = std::make_unique<BoyCharacter>(m_InputManager.get(), m_BoyArmature.get());
	m_BoyCharacter->SetPosition(
		gef::Vector2(
			0.75f * static_cast<float>(platform_.width()),
			0.5f * static_cast<float>(platform_.height()))
	);
}

void SceneApp::CleanUp()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


bool SceneApp::Update(float frame_time)
{
	m_FPS = 1.0f / frame_time;

	// read input devices
	if (m_InputManager)
	{
		m_InputManager->Update();

		UpdateImGuiIO();
	}

	ImGuiBeginFrame();

	

	ImGui::Begin("Details");

	ImGui::Text("FPS: %0.1f", m_FPS);
	ImGui::Separator();

	ImGui::Checkbox("Show 3D", &m_Show3D);
	if (m_Show3D)
	{
		Update3D(frame_time);
	}
	else
	{
		Update2D(frame_time);
	}

	ImGui::Separator();

	if (m_Show3D)
	{
		DrawImGui3D();
	}
	else
	{
		DrawImGui2D();
	}

	ImGui::End();

	ImGuiEndFrame();

	return true;
}

void SceneApp::Render()
{
	if (m_Show3D)
	{
		Render3D();
	}
	else
	{
		Render2D();
	}

	DrawImGui();
}


void SceneApp::Update2D(float frame_time)
{
	m_DinoCharacter->Tick(frame_time);
	m_BoyCharacter->Tick(frame_time);
}

void SceneApp::Update3D(float frame_time)
{
	m_PhysicsWorld->Tick(frame_time);
	m_AnimationEngine->Tick(frame_time);

	// build a transformation matrix that will position the character
	// use this to move the player around, scale it, etc.
	if (m_Player)
	{
		gef::Matrix44 player_transform;
		player_transform.SetIdentity();
		player_transform.Scale({ 0.01f, 0.01f, 0.01f, 1.0f });
		m_Player->set_transform(player_transform);
	}
}

void SceneApp::Render2D()
{
	m_SpriteRenderer->Begin();

	m_DinoCharacter->Render(m_SpriteRenderer.get());
	m_BoyCharacter->Render(m_SpriteRenderer.get());

	m_SpriteRenderer->End();
}

void SceneApp::Render3D()
{
	// setup view and projection matrices
	const gef::Matrix44 projection_matrix = platform_.PerspectiveProjectionFov(m_CameraFOV, (float)platform_.width() / (float)platform_.height(), m_NearPlane, m_FarPlane);
	gef::Matrix44 view_matrix;
	view_matrix.LookAt(m_CameraEye, m_CameraLookAt, m_CameraUp);

	m_Renderer3D->set_projection_matrix(projection_matrix);
	m_Renderer3D->set_view_matrix(view_matrix);

	m_Renderer3D->Begin();

	// draw the player, the pose is defined by the bone matrices
	if (m_Player)
		m_Renderer3D->DrawSkinnedMesh(*m_Player, m_PlayerAnimator->GetMatrixPalette());

	m_Renderer3D->DrawMesh(m_Floor);
	if (m_ShowPhysics)
		m_PhysicsWorld->DebugDraw();

	m_Renderer3D->End();
}

void SceneApp::DrawImGui2D()
{
	ImGui::Text("Dragon Controls");
	ImGui::Text("Move: WASD");
	ImGui::Text("Rotate: EQ");
	ImGui::Text("Idle Animation: 1");
	ImGui::Text("Walk Animation: 2");
	ImGui::Text("Jump Animation: 3");
	ImGui::Text("Fall Animation: 4");

	ImGui::Separator();

	ImGui::Text("Boy Controls");
	ImGui::Text("Move: Arrow keys");
	ImGui::Text("Attack Animation: Space");
}

void SceneApp::DrawImGui3D()
{
	ImGui::Text("Parameters");

	if (m_PlayerAnimator->GetParameterTable()->ParameterExists("walkSpeed")
		&& ImGui::SliderFloat("Walk Speed", &m_WalkSpeed, 0.0f, 1.0f))
	{
		m_PlayerAnimator->GetParameterTable()->SetParam("walkSpeed", m_WalkSpeed);
	}
	if (m_PlayerAnimator->GetParameterTable()->ParameterExists("walkDir")
		&& ImGui::SliderFloat("Walk Direction", &m_WalkDir, -1.0f, 1.0f))
	{
		m_PlayerAnimator->GetParameterTable()->SetParam("walkDir", m_WalkDir);
	}
	if (m_PlayerAnimator->GetParameterTable()->ParameterExists("injured")
		&& ImGui::SliderFloat("Injured", &m_Injured, 0.0f, 1.0f))
	{
		m_PlayerAnimator->GetParameterTable()->SetParam("injured", m_Injured);
	}

	ImGui::Separator();
	ImGui::Text("Actions");

	if (ImGui::Button("Idle"))
		m_PlayerAnimator->Transition("idle");
	if (ImGui::Button("Walk"))
		m_PlayerAnimator->Transition("walk");
	if (ImGui::Button("Jump"))
		m_PlayerAnimator->Transition("jump");
	if (ImGui::Button("Die"))
		m_PlayerAnimator->Transition("death");
	if (ImGui::Button("Respawn"))
		m_PlayerAnimator->Transition("respawn");

	ImGui::Separator();
	ImGui::Text("Animators");

	bool reloadVars = false;
	if (ImGui::Button("Basic State Machine"))
		reloadVars |= m_PlayerAnimator->LoadFromJSON("xbot\\xbot_basic_state_machine.json");
	if (ImGui::Button("Linear Blending"))
		reloadVars |= m_PlayerAnimator->LoadFromJSON("xbot\\xbot_linear_blending.json");
	if (ImGui::Button("Full Demo"))
		reloadVars |= m_PlayerAnimator->LoadFromJSON("xbot\\xbot.json");

	if (reloadVars)
	{
		if (m_PlayerAnimator->GetParameterTable()->ParameterExists("walkSpeed"))
			m_PlayerAnimator->GetParameterTable()->SetParam("walkSpeed", m_WalkSpeed);
		if (m_PlayerAnimator->GetParameterTable()->ParameterExists("walkDir"))
			m_PlayerAnimator->GetParameterTable()->SetParam("walkDir", m_WalkDir);
		if (m_PlayerAnimator->GetParameterTable()->ParameterExists("injured"))
			m_PlayerAnimator->GetParameterTable()->SetParam("injured", m_Injured);
	}

	ImGui::Separator();
	ImGui::Text("Debug");

	ImGui::Checkbox("Show Physics", &m_ShowPhysics);
}


void SceneApp::SetupLights() const
{
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(0.7f, 0.7f, 1.0f, 1.0f));
	default_point_light.set_position(gef::Vector4(-300.0f, -500.0f, 100.0f));

	gef::Default3DShaderData& default_shader_data = m_Renderer3D->default_shader_data();
	default_shader_data.set_ambient_light_colour(gef::Colour(0.5f, 0.5f, 0.5f, 1.0f));
	default_shader_data.AddPointLight(default_point_light);
}

std::unique_ptr<gef::Mesh> SceneApp::GetFirstMesh(gef::Scene* scene) const
{
	if (scene && !scene->mesh_data.empty())
		return std::unique_ptr<gef::Mesh>(scene->CreateMesh(platform_, scene->mesh_data.front()));

	return {};
}

void SceneApp::ImGuiBeginFrame()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Make viewport dock-able
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void SceneApp::ImGuiEndFrame()
{
	// Rendering
	ImGui::Render();
}

void SceneApp::DrawImGui()
{
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// For multiple ImGui viewports
	const ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}


void SceneApp::UpdateImGuiIO()
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDown[0] = m_InputManager->touch_manager()->is_button_down(0);
	io.MouseDown[1] = m_InputManager->touch_manager()->is_button_down(1);
	io.MouseWheel += m_InputManager->touch_manager()->mouse_rel().z() / WHEEL_DELTA;

	gef::KeyboardD3D11* keyboard_d3d11 = (gef::KeyboardD3D11*)m_InputManager->keyboard();


	io.KeyShift = keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_LSHIFT) || keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_RSHIFT);
	io.KeyCtrl = keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_LCONTROL) || keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_RCONTROL);
	io.KeyAlt = keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_LALT) || keyboard_d3d11->IsKeyDown(gef::Keyboard::KC_RALT);


	for (int key_code_num = 0; key_code_num < gef::Keyboard::NUM_KEY_CODES; ++key_code_num)
	{
		gef::Keyboard::KeyCode key_code = (gef::Keyboard::KeyCode)key_code_num;
		int scan_code = keyboard_d3d11->GetScanCode(key_code);

		int vk_code = MapVirtualKey(scan_code, MAPVK_VSC_TO_VK_EX);

		io.KeysDown[vk_code] = keyboard_d3d11->IsKeyDown(key_code);
		if (keyboard_d3d11->IsKeyPrintable(key_code) && keyboard_d3d11->IsKeyPressed(key_code))
		{
			if (!io.KeyShift && vk_code >= 'A' && vk_code <= 'Z')
				vk_code = 'a' + (vk_code - 'A');
			io.AddInputCharacter(vk_code);
		}
	}
}

