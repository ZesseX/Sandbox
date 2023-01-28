#pragma once

#include "Chunk.h"
#include "Directions.h"

#include <cstdint>
#include <vector>
#include <iostream>
#include <time.h>

class Simulation {
public:
	void Init() {
		// Init random
		srand(time(NULL));

		// Initialize chunks
		for (int x = 0; x < CHUNKCOUNT; x++) {
			for (int y = 0; y < CHUNKCOUNT; y++) {
				m_Chunks[x][y] = new Chunk();
				m_Chunks[x][y]->idX = x;
				m_Chunks[x][y]->idY = y;
			}
		}

		// FOR TESTING
		for (int i = 0; i < 96; i++) {
			for (int j = 0; j < 8; j++) {
				CreateParticle(ParticleTypes::water, 64 + i, 128 + j);

				CreateParticle(ParticleTypes::sand, 64 + i, 196 + j);
			}
			CreateParticle(ParticleTypes::rock, 0 + i, 16);
			CreateParticle(ParticleTypes::rock, 86 + i, 32);
			CreateParticle(ParticleTypes::rock, 0 + i, 48);
			CreateParticle(ParticleTypes::rock, 86 + i, 64);
		}
	}

	// Flips the direction of the x-axis loop inside a chunk
	// Not doing this can cause weird patterns in simulation behaviour
	// where it appears that particles prefer one direciton over the other
	bool flipped = false;
	void Update() {
		// Loop through the chunks
		for (uint16_t chunkX = 0; chunkX < CHUNKCOUNT; chunkX++) {
			for (uint16_t chunkY = 0; chunkY < CHUNKCOUNT; chunkY++) {

				// Loop through the particles in the chunk
				for (uint16_t localX = 0; localX < CHUNKSIZE; localX++) {
					for (uint16_t localY = 0; localY < CHUNKSIZE; localY++) {
						// Checks which direction the it should loop on the x axis
						int ncx = flipped ? (CHUNKCOUNT - 1) - chunkX : chunkX;
						int nlx = flipped ? (CHUNKSIZE - 1) - localX : localX;
						Particle* particle = &m_Chunks[ncx][chunkY]->particles[nlx][localY];

						// If particle has been updated on this tick then go to next one
						if (particle->updated) {
							continue;
						}

						// Choose the appropriate logic
						switch (particle->type) {
							case ParticleTypes::none:
								continue;
								break;
							case ParticleTypes::sand:
								SandLogic(particle, nlx, localY, ncx, chunkY);
								break;
							case ParticleTypes::water:
								WaterLogic(particle, nlx, localY, ncx, chunkY);
								break;
						}
					}
				}

			}
		}

		flipped = !flipped;
	}

	void Render(SDL_Renderer* rend, Camera* cam) {
		float scale = 1;
		for (uint16_t x = 0; x < CHUNKCOUNT; x++) {
			for (uint16_t y = 0; y < CHUNKCOUNT; y++) {
				m_Chunks[x][y]->Render(rend, cam);
			}
		}
	}

	// Create particle with global x and y coordinates
	void CreateParticle(ParticleTypes type, int16_t x, int16_t y) {
		int16_t lx, ly, cx, cy;
		lx = x % CHUNKSIZE;
		ly = y % CHUNKSIZE;
		cx = (x - lx) / CHUNKSIZE;
		cy = (y - ly) / CHUNKSIZE;

		CreateParticle(type, lx, ly, cx, cy);

		/*
		if (cx < 0 || cy < 0)
			return;
		if (cx > CHUNKCOUNT - 1 || cy > CHUNKCOUNT - 1)
			return;

		if (lx < 0 || ly < 0)
			return;
		if (lx > CHUNKSIZE - 1 || ly > CHUNKSIZE - 1)
			return;

		m_Chunks[cx][cy]->particles[lx][ly].type = type;
		*/
	}
	// Create particle with local and chunk coordinates
	void CreateParticle(ParticleTypes type, int16_t lx, int16_t ly, int16_t cx, int16_t cy) {
		if (cx < 0 || cy < 0)
			return;
		if (cx > CHUNKCOUNT - 1 || cy > CHUNKCOUNT - 1)
			return;

		if (lx < 0 || ly < 0)
			return;
		if (lx > CHUNKSIZE - 1 || ly > CHUNKSIZE - 1)
			return;

		m_Chunks[cx][cy]->particles[lx][ly].type = type;
	}

private:
	Chunk* m_Chunks[CHUNKCOUNT][CHUNKCOUNT] = { nullptr };

	// Logic for sand particles
	void SandLogic(Particle* particle, int16_t lx, int16_t ly, int16_t cx, int16_t cy) {
		//if (cy == 0 && lx == 0) {
		//	particle->freeFalling = false;
		//	particle->velocity = 0;
		//	return;
		//}

		// Get the type of particle below
		auto other = GetParticle(Directions::S, lx, ly, cx, cy);
		if (other.type != ParticleTypes::error) {
			if (other.type == ParticleTypes::none) {
				MoveParticle(particle, Directions::S, lx, ly, cx, cy);
				particle->freeFalling = true;
				return;
			}
		}

		if (particle->freeFalling || particle->velocity != 0.0f) {
			particle->freeFalling = false;
			particle->velocity = 0.0f;
			return;
		}

		other = GetParticle(Directions::S, lx, ly, cx, cy);
		if (other.type == ParticleTypes::water && !other.updated) {
			MoveParticle(particle, Directions::S, lx, ly, cx, cy, 1.0f, false, true);
			return;
		}
		
		// Randomly pick the first direction to check for
		// Otherwise it becomes very clear that it prefers one direction over the other
		int rnd = rand() % 2;

		Directions dir = rnd == 0 ? Directions::SW : Directions::SE;
		other = GetParticle(dir, lx, ly, cx, cy);
		if (other.type != ParticleTypes::error) {
			if (other.type == ParticleTypes::none) {
				MoveParticle(particle, dir, lx, ly, cx, cy);
				particle->freeFalling = true;
				return;
			}
			if (other.type == ParticleTypes::water && !other.updated) {
				MoveParticle(particle, dir, lx, ly, cx, cy, 1.0f, false, true);
				return;
			}
		}
	}
	// Logic for water particles
	void WaterLogic(Particle* particle, int16_t lx, int16_t ly, int16_t cx, int16_t cy) {
		// Get particle type in a given direction (In this case south / below)
		auto type = GetParticleType(Directions::S, lx, ly, cx, cy);
		if (type != ParticleTypes::error) {
			if (type == ParticleTypes::none) {
				// Moves particle down if no error was given
				// and the place below is empty
				MoveParticle(particle, Directions::S, lx, ly, cx, cy);
				return;
			}
		}

		// Force the particle to stop moving for one simulation tick
		// until it can move horizontally
		if (particle->velocity != 0) {
			particle->velocity = 0;
			return;
		}

		// Prefers moving left. Making this random will cause the 
		// particle to virtally stay in the same place as it moves left and right
		// Could be changed to save the direction chosen in the first horizontal movement tick
		auto dir = Directions::W;
		type = GetParticleType(dir, lx, ly, cx, cy);
		if (type != ParticleTypes::error) {
			if (type == ParticleTypes::none) {
				MoveParticle(particle, dir, lx, ly, cx, cy);
				return;
			}
		}

		dir = Directions::E;
		type = GetParticleType(dir, lx, ly, cx, cy);
		if (type != ParticleTypes::error) {
			if (type == ParticleTypes::none) {
				MoveParticle(particle, dir, lx, ly, cx, cy);
				return;
			}
		}
	}

	void MoveParticle(Particle* particle, Directions dir, int16_t lx, int16_t ly, int16_t cx, int16_t cy, float velocity = 1.0f, bool clearOld = true, bool swapPlaces = false) {
		int16_t nlx = lx, nly = ly, ncx = cx, ncy = cy;
		if (!GetIds(dir, nlx, nly, ncx, ncy)) {
			// Don't move if it's not possible (out of bounds)
			return;
		}

		if (!swapPlaces) {
			// Particle doesn't swap places with the one it's replacing
			m_Chunks[ncx][ncy]->particles[nlx][nly].type = particle->type;
			m_Chunks[ncx][ncy]->particles[nlx][nly].velocity = velocity;
			m_Chunks[ncx][ncy]->particles[nlx][nly].updated = true;
			if (clearOld) {
				// Changes the old position to Particle::None()
				ClearParticle(lx, ly, cx, cy);
			}
			return;
		}

		// Swaps particle places
		Particle temp = m_Chunks[ncx][ncy]->particles[nlx][nly];
		m_Chunks[ncx][ncy]->particles[nlx][nly].type = particle->type;
		m_Chunks[ncx][ncy]->particles[nlx][nly].velocity = velocity;
		m_Chunks[ncx][ncy]->particles[nlx][nly].updated = true;

		m_Chunks[cx][cy]->particles[lx][ly].type = temp.type;
		m_Chunks[cx][cy]->particles[lx][ly].velocity = temp.velocity;
		m_Chunks[cx][cy]->particles[lx][ly].updated = true;
	}
	void ClearParticle(int16_t lx, int16_t ly, int16_t cx, int16_t cy) {
		m_Chunks[cx][cy]->particles[lx][ly] = Particle::None();
	}

	ParticleTypes GetParticleType(Directions dir, int16_t lx, int16_t ly, int16_t cx, int16_t cy) {
		int16_t nlx = lx, nly = ly, ncx = cx, ncy = cy;
		
		if (!GetIds(dir, nlx, nly, ncx, ncy)) {
			// Return error if id couldn't be found
			return ParticleTypes::error;
		}

		return m_Chunks[ncx][ncy]->particles[nlx][nly].type;
	}
	Particle GetParticle(Directions dir, int16_t lx, int16_t ly, int16_t cx, int16_t cy) {
		int16_t nlx = lx, nly = ly, ncx = cx, ncy = cy;

		if (!GetIds(dir, nlx, nly, ncx, ncy)) {
			// Return error if id couldn't be found
			return Particle::Error();
		}

		return m_Chunks[ncx][ncy]->particles[nlx][nly];
	}
	bool GetIds(Directions dir, int16_t& lx, int16_t& ly, int16_t& cx, int16_t& cy) {
		// Messy way to get the chunk and local coordinates with a given direction

		int16_t nlx = lx, nly = ly, ncx = cx, ncy = cy;
		switch (dir) {
			case Directions::N:
			{
				if (ly == CHUNKSIZE - 1) {
					if (cy == CHUNKCOUNT - 1) {
						return false;
					}
					else {
						nly = 0;
						ncy += 1;
					}
				}
				else {
					nly += 1;
				}
			}
			break;
			case Directions::NE:
			{
				if (ly == CHUNKSIZE - 1) {
					if (cy == CHUNKCOUNT - 1) {
						return false;
					}
					else {
						nly = 0;
						ncy += 1;
					}
				}
				else {
					nly += 1;
				}

				if (lx == CHUNKSIZE - 1) {
					if (cx == CHUNKCOUNT - 1) {
						return false;
					}
					else {
						nlx = 0;
						ncx += 1;
					}
				}
				else {
					nlx += 1;
				}
			}
			break;
			case Directions::E:
			{
				if (lx == CHUNKSIZE - 1) {
					if (cx == CHUNKCOUNT - 1) {
						return false;
					}
					else {
						nlx = 0;
						ncx += 1;
					}
				}
				else {
					nlx += 1;
				}
			}
			break;
			case Directions::SE:
			{
				if (ly == 0) {
					if (cy == 0) {
						return false;
					}
					else {
						nly = CHUNKSIZE - 1;
						ncy -= 1;
					}
				}
				else {
					nly -= 1;
				}

				if (lx == CHUNKSIZE - 1) {
					if (cx == CHUNKCOUNT - 1) {
						return false;
					}
					else {
						nlx = 0;
						ncx += 1;
					}
				}
				else {
					nlx += 1;
				}
			}
			break;
			case Directions::S:
			{
				if (ly == 0) {
					if (cy == 0) {
						return false;
					}
					else {
						nly = CHUNKSIZE - 1;
						ncy -= 1;
					}
				}
				else {
					nly -= 1;
				}
			}
			break;
			case Directions::SW:
			{
				if (ly == 0) {
					if (cy == 0) {
						return false;
					}
					else {
						nly = CHUNKSIZE - 1;
						ncy -= 1;
					}
				}
				else {
					nly -= 1;
				}

				if (lx == 0) {
					if (lx == 0) {
						return false;
					}
					else {
						nlx = CHUNKSIZE - 1;
						ncx -= 1;
					}
				}
				else {
					nlx -= 1;
				}
			}
			break;
			case Directions::W:
			{
				if (lx == 0) {
					if (lx == 0) {
						return false;
					}
					else {
						nlx = CHUNKSIZE - 1;
						ncx -= 1;
					}
				}
				else {
					nlx -= 1;
				}
			}
			break;
			case Directions::NW:
			{
				if (ly == CHUNKSIZE - 1) {
					if (cy == CHUNKCOUNT - 1) {
						return false;
					}
					else {
						nly = 0;
						ncy += 1;
					}
				}
				else {
					nly += 1;
				}

				if (lx == 0) {
					if (lx == 0) {
						return false;
					}
					else {
						nlx = CHUNKSIZE - 1;
						ncx -= 1;
					}
				}
				else {
					nlx -= 1;
				}
			}
			break;
		}

		lx = nlx;
		ly = nly;
		cx = ncx;
		cy = ncy;

		return true;
	}
};