#pragma once

#include "ParticleTypes.h"

struct Particle {
	ParticleTypes type = ParticleTypes::none;
	float velocity = 0;
	bool freeFalling = false;
	bool updated = false;

	static Particle None() {
		Particle particle;
		particle.type = ParticleTypes::none;
		return particle;
	}

	static Particle Error() {
		Particle particle;
		particle.type = ParticleTypes::error;
		return particle;
	}
};