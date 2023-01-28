#pragma once

#include <cstdint>
#include <iostream>

class Camera {
public:
	double x = 0;
	double y = 1000;

	double speed = 0.25;

	// Update the camera position with the axis and deltaTime
	void Update(float xAxis, float yAxis, double deltaTime) {
		x += speed * (double)xAxis * deltaTime;
		y += speed * (double)yAxis * deltaTime;
	}
};