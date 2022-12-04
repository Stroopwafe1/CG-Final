#pragma once
#include <glm/glm.hpp>
struct Material {
	glm::vec3 ambient_colour; // The ambient colour of this material
	glm::vec3 diffuse_colour; // The diffuse colour of this material
	glm::vec3 specular; // The specular of this material
	float power; // The power of the specular of this material
};