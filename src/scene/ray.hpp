
#pragma once

// glm
#include <glm/glm.hpp>


// Simple ray class with origin and direction
class Ray {
public:
	glm::vec3 origin;
	glm::vec3 direction;

	Ray() { }
	Ray(const glm::vec3 &o, const glm::vec3 &d) : origin(o), direction(d) { }
};
