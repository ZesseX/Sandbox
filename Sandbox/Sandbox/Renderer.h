#pragma once

#include <iostream>

#include <SDL.h>

#include "simulation/Simulation.h"

class Renderer {
public:
	void Init(Simulation* sim, Camera* camera) {
		// Initialize SDL
		int flags = SDL_WINDOW_RESIZABLE;
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
			return;
		}

		// Create SDL window and renderer
		m_pWindow = SDL_CreateWindow("Sandbox", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, flags);
		m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, 0);

		// Initialize simulation
		sim->Init();

		// Variables for limiting the fps and printing it in the console
		uint32_t fps = 0; 
		uint32_t fpsTimer = 0;
		uint32_t lastFpsTimer = 0;

		uint64_t deltaNow = SDL_GetPerformanceCounter();
		uint64_t deltaLast = 0;
		double deltaTime = 0.0;

		// Loop that is true until the program should exit
		while (PollEvents(camera, sim, deltaTime)) {
			deltaLast = deltaNow;
			deltaNow = SDL_GetPerformanceCounter();
			deltaTime = (double)((deltaNow - deltaLast) * 1000 / (double)SDL_GetPerformanceFrequency());

			uint32_t now = SDL_GetTicks();
			if (now > fpsTimer + 1000) {
				// Print fps to the console once a second
				std::cout << "FPS: " << fps << std::endl;
				fps = 0;
				fpsTimer = now;
			}

			// Simulation & rendering update
			Update(sim, camera);

			// Delay until the fps is ~30
			if (lastFpsTimer != 0 && now - lastFpsTimer < 33) {
				SDL_Delay(33 - (now - lastFpsTimer));
			}

			lastFpsTimer = now;
			fps++;
		}

		// Destroy SDL related stuff
		SDL_DestroyRenderer(m_pRenderer);
		SDL_DestroyWindow(m_pWindow);
		SDL_Quit();
	}

private:
	void Update(Simulation* sim, Camera* cam) {
		// Update simulation
		sim->Update();

		// Set window color to black
		SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);
		SDL_RenderClear(m_pRenderer);

		// Render simulation
		sim->Render(m_pRenderer, cam);

		// Present the picture
		SDL_RenderPresent(m_pRenderer);
	}

	bool PollEvents(Camera* cam, Simulation* sim, double deltaTime) {
		// Poll events to see if the program should be exited
		SDL_Event event;
		SDL_PollEvent(&event);
		switch (event.type) {
			case SDL_QUIT:
				return false;
				break;
		}

		// Get inputs
		float x = 0, y = 0;
		const uint8_t* state = SDL_GetKeyboardState(NULL);
		if (state[SDL_SCANCODE_W]) {
			y += 1;
		}
		if (state[SDL_SCANCODE_S]) {
			y -= 1;
		}
		if (state[SDL_SCANCODE_A]) {
			x -= 1;
		}
		if (state[SDL_SCANCODE_D]) {
			x += 1;
		}

		// Update the camera position with the given inputs
		cam->Update(x, y, deltaTime);

		int w = 0, h = 0;
		SDL_GetWindowSize(m_pWindow, &w, &h);

		// Check if a particle should be spawned
		if (state[SDL_SCANCODE_SPACE]) {
			sim->CreateParticle(ParticleTypes::sand, (uint16_t)((cam->x + (w / 2)) / SCALE), (uint16_t)((cam->y - (h / 2)) / SCALE));
		}
		if (state[SDL_SCANCODE_LCTRL]) {
			sim->CreateParticle(ParticleTypes::water, (uint16_t)((cam->x + (w / 2)) / SCALE), (uint16_t)((cam->y - (h / 2)) / SCALE));
		}
		if (state[SDL_SCANCODE_LSHIFT]) {
			sim->CreateParticle(ParticleTypes::rock, (uint16_t)((cam->x + (w / 2)) / SCALE), (uint16_t)((cam->y - (h / 2)) / SCALE));
		}

		return true;
	}

	SDL_Window* m_pWindow;
	SDL_Renderer* m_pRenderer;
};