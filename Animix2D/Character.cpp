#include "Character.h"

#include "input/input_manager.h"
#include "graphics/sprite_renderer.h"

#include "SpriteAnimator.h"


namespace Animix2D
{
	
	Character::Character(gef::InputManager* inputManager, SpriteArmature* armature)
		: m_InputManager(inputManager)
	{
		m_Animator = std::make_unique<SpriteAnimator>(armature);
	}

	void Character::Tick(float deltaTime)
	{
		// core character updates
		m_Animator->Tick(deltaTime);
	}

	void Character::Render(gef::SpriteRenderer* spriteRenderer) const
	{
		gef::Matrix33 scale, rotate;
		// scale and rotate set all members of the matrix, so don't need to set to identity first
		scale.Scale({ m_Scale, m_Scale });
		rotate.Rotate(m_Rotation);

		gef::Matrix33 transform = scale * rotate;
		transform.SetTranslation(m_Position);

		m_Animator->Render(spriteRenderer, transform);
	}
}
