
#pragma once

// glm
#include <glm/glm.hpp>

// project
#include "scene.hpp"


class Light {
public:
	// return true if the point is occluded from the scene
	virtual bool occluded(Scene *scene, const glm::vec3 &point) const = 0;

	// return direction of incoming light (light to point)
	virtual glm::vec3 incidentDirection(const glm::vec3 &point) const = 0;

	// return the irradiance (flux of radiant energy per unit area) cast by this
	// light onto the ray intersection point, assuming there is no obstruction
	// and the surface is oriented towards the light
	virtual glm::vec3 irradiance(const glm::vec3 &point) const = 0;

	// return ambience (contribution of light bouncing around the scene)
	// approximates indirect lighting and does not require the light to be visable
	virtual glm::vec3 ambience() const = 0;
};


class DirectionalLight : public Light {
private:
	glm::vec3 m_direction;
	glm::vec3 m_irradiance;
	glm::vec3 m_ambience;

public:
	DirectionalLight(const glm::vec3 &direction, const glm::vec3 &irradiance, const glm::vec3 &ambience)
		: m_direction(normalize(direction)), m_irradiance(irradiance), m_ambience(ambience) { }

	virtual bool occluded(Scene *scene, const glm::vec3 &point) const override;
	virtual glm::vec3 incidentDirection(const glm::vec3 &point) const override;
	virtual glm::vec3 irradiance(const glm::vec3 & point) const override;
	virtual glm::vec3 ambience() const override { return m_ambience; }
};


class PointLight : public Light {
private:
	glm::vec3 m_position;
	glm::vec3 m_flux;
	glm::vec3 m_ambience;

public:
	PointLight(const glm::vec3 &position, const glm::vec3 &flux, const glm::vec3 &ambience)
		: m_position(position), m_flux(flux), m_ambience(ambience) { }

	virtual bool occluded(Scene *scene, const glm::vec3 &point) const override;
	virtual glm::vec3 incidentDirection(const glm::vec3 &point) const override;
	virtual glm::vec3 irradiance(const glm::vec3 &point) const override;
	virtual glm::vec3 ambience() const override { return m_ambience; }
};
