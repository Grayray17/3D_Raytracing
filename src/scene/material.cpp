
// glm
#include <glm/glm.hpp>

// project
#include "material.hpp"


Material::Material(const glm::vec3 &diffuse, const glm::vec3 &specular, float shininess)
	: m_diffuse(diffuse), m_specular(specular), m_shininess(shininess) { }


Material::Material(const glm::vec3 &diffuse_chroma, float shininess, float specular_ratio, float metalicity_ratio) : m_shininess(shininess) {
	glm::vec3 specular_chroma = glm::sqrt(diffuse_chroma);
	m_specular = specular_ratio * (metalicity_ratio * specular_chroma + glm::vec3(1.f - metalicity_ratio));
	m_diffuse = (1 - specular_ratio) * diffuse_chroma;
}