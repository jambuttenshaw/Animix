#include "CustomCharacters.h"

#include <input/input_manager.h>
#include <input/keyboard.h>


DinoCharacter::DinoCharacter(gef::InputManager* inputManager, Animix2D::SpriteArmature* armature)
	: Character(inputManager, armature)
{
	m_Animator->PlayAnimation("stand", true);
}

void DinoCharacter::Tick(float deltaTime)
{
	// character movement
	if (m_InputManager->keyboard()->IsKeyDown(gef::Keyboard::KC_E))
		m_Rotation += deltaTime;
	if (m_InputManager->keyboard()->IsKeyDown(gef::Keyboard::KC_Q))
		m_Rotation -= deltaTime;

	if (m_InputManager->keyboard()->IsKeyDown(gef::Keyboard::KC_D))
		m_Position.x += 100.0f * deltaTime;
	if (m_InputManager->keyboard()->IsKeyDown(gef::Keyboard::KC_A))
		m_Position.x -= 100.0f * deltaTime;

	if (m_InputManager->keyboard()->IsKeyDown(gef::Keyboard::KC_W))
		m_Position.y -= 100.0f * deltaTime;
	if (m_InputManager->keyboard()->IsKeyDown(gef::Keyboard::KC_S))
		m_Position.y += 100.0f * deltaTime;

	// character animations
	if (m_InputManager->keyboard()->IsKeyPressed(gef::Keyboard::KC_1))
		m_Animator->PlayAnimation("stand", true);
	if (m_InputManager->keyboard()->IsKeyPressed(gef::Keyboard::KC_2))
		m_Animator->PlayAnimation("walk", true);
	if (m_InputManager->keyboard()->IsKeyPressed(gef::Keyboard::KC_3))
		m_Animator->PlayAnimation("jump", false);
	if (m_InputManager->keyboard()->IsKeyPressed(gef::Keyboard::KC_4))
		m_Animator->PlayAnimation("fall", false);

	Character::Tick(deltaTime);
}



BoyCharacter::BoyCharacter(gef::InputManager* inputManager, Animix2D::SpriteArmature* armature)
	: Character(inputManager, armature)
{

}

void BoyCharacter::Tick(float deltaTime)
{
	// character movement
	if (m_InputManager->keyboard()->IsKeyDown(gef::Keyboard::KC_RIGHT))
		m_Position.x += 100.0f * deltaTime;
	if (m_InputManager->keyboard()->IsKeyDown(gef::Keyboard::KC_LEFT))
		m_Position.x -= 100.0f * deltaTime;

	if (m_InputManager->keyboard()->IsKeyDown(gef::Keyboard::KC_UP))
		m_Position.y -= 100.0f * deltaTime;
	if (m_InputManager->keyboard()->IsKeyDown(gef::Keyboard::KC_DOWN))
		m_Position.y += 100.0f * deltaTime;

	// character animations
	if (m_InputManager->keyboard()->IsKeyPressed(gef::Keyboard::KC_SPACE))
	{
		m_Animator->PlayAnimation("boy-attack", false);
	}

	Character::Tick(deltaTime);
}
