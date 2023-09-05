
#pragma once

// std
#include <memory>

// glm
#include <glm/glm.hpp>

// project
#include "scene.hpp"
#include "texture.hpp"


class Material {
private:
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;
	float m_shininess;

public:
	
	// Typical constructor that takes the diffuse, specular and shininess
	Material(const glm::vec3 &diffuse, const glm::vec3 &specular, float shininess);

	// An alternative constructor that takes a diffuse chroma and extra paramaters
	// to construct a material that conforms approximately to the following rules:
	// - the sum of diffuse and specular should not be greater than one (the ratio of which is controlled with specular_ratio)
	// - metals have a specular equal to the square-root of their diffuse
	// - non-metals have a white specular
	Material(const glm::vec3 &diffuse_chroma, float shininess, float specular_ratio, float metalicity_ratio);

	// return the (lambertian) diffuse color of this material
	virtual glm::vec3 diffuse() const { return m_diffuse; }
	
	// return the specular reflection color of this material
	virtual glm::vec3 specular() const { return m_specular; }
	
	// return the shininess of this material
	virtual float shininess() const { return m_shininess; }
};