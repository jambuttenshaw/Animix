#pragma once

#include "Animix2D/Character.h"


class DinoCharacter : public Animix2D::Character
{
public:
	explicit DinoCharacter(
		gef::InputManager* inputManager,
		Animix2D::SpriteArmature* armature
	);

	void Tick(float deltaTime) override;
};


class BoyCharacter : public Animix2D::Character
{
public:
	explicit BoyCharacter(
		gef::InputManager* inputManager,
		Animix2D::SpriteArmature* armature
	);

	void Tick(float deltaTime) override;
};
