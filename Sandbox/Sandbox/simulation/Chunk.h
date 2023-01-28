#pragma once

#include "simulation_global_variables.h"
#include "ParticleTypes.h"
#include "Particle.h"

#include "../Camera.h"

#include <SDL.h>

#include <iostream>

class Chunk {
public:
	uint16_t idX = 0;
	uint16_t idY = 0;

	// Renders chunk with given SDL_Renderer
	void Render(SDL_Renderer* rend, Camera* cam) {
		int xOffset = (CHUNKSIZE * idX * SCALE);
		int yOffset = -(CHUNKSIZE * idY * SCALE);

		for (int x = 0; x < CHUNKSIZE; x++) {
			for (int y = 0; y < CHUNKSIZE; y++) {
				if (particles[x][y].type != ParticleTypes::none) {
					// Adjust rect size with the scale found in "simulation_global_variables.h"
					SDL_Rect rect;
					rect.h = SCALE;
					rect.w = SCALE;
					// Offset with camera position
					rect.x = xOffset + (x * SCALE) - cam->x;
					rect.y = yOffset + ((CHUNKSIZE * (1 / SCALE) - y) * SCALE) + cam->y;

					// TODO: Should probably skip the rendering if the particles are outside of the cameras view

					// Hardcoded color values for each particle here
					// Sets the drawing color to it
					// TODO: Skip setting color if it already is selected
					if (particles[x][y].type == ParticleTypes::sand) {
						SDL_SetRenderDrawColor(rend, 255, 200, 100, 255);
					}
					else if (particles[x][y].type == ParticleTypes::rock) {
						SDL_SetRenderDrawColor(rend, 225, 225, 225, 255);
					}
					else if (particles[x][y].type == ParticleTypes::water) {
						SDL_SetRenderDrawColor(rend, 60, 185, 255, 255);
					}

					// Draws the rect defined at the start of if statement
					SDL_RenderFillRect(rend, &rect);
				}

				// Set the particle update state to false to enable it's editing in the next simulation tick
				particles[x][y].updated = false;
			}
		}
	}

	Particle particles[CHUNKSIZE][CHUNKSIZE] = { Particle::None() };
};