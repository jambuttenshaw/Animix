#pragma once

#include "SpriteAnimator.h"


namespace gef
{
	class InputManager;
	class SpriteRenderer;
}

namespace Animix2D
{
	
	class Character
	{
	public:
		Character(gef::InputManager* inputManager, SpriteArmature* armature);
		virtual ~Character() = default;
	
		inline void SetPosition(const gef::Vector2& pos) { m_Position = pos; }
		inline const gef::Vector2& GetPosition() const { return m_Position; }
	
		inline void SetRotation(float rot) { m_Rotation = rot; }
		inline float GetRotation() const { return m_Rotation; }
	
		inline void SetScale(float scale) { m_Scale = scale; }
		inline float GetScale() const { return m_Scale; }
	
		inline const std::unique_ptr<SpriteAnimator>& GetAnimator() const { return m_Animator; }
	
		virtual void Tick(float deltaTime);
		void Render(gef::SpriteRenderer* spriteRenderer) const;
		
	protected:
		gef::InputManager* m_InputManager;
	
		// world-space transform
		gef::Vector2 m_Position; // current position of the character in world space
		float m_Rotation = 0.0f; // current rotation of the character in world space
		float m_Scale = 1.0f;    // current scale of the character in world space
	
		std::unique_ptr<SpriteAnimator> m_Animator;
	};
}
