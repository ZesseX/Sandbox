#include <SDL.h>

#include "simulation/Simulation.h"
#include "Renderer.h"

// the args are required by SDL, will not work without
int main(int argc, char* argv[]) {
	// Create a simulation and a camera
	Simulation* sim = new Simulation();
	Camera* cam = new Camera();

	// Create renderer and start it
	Renderer* rend = new Renderer();
	rend->Init(sim, cam);

	// Renderer left Init => program should exit
	return 0;
}