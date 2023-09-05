
#pragma once

// std
#include <memory>
#include <vector>

// glm
#include <glm/glm.hpp>

// project
#include "ray.hpp"


// forward declare scene components
class Light;
class SceneObject;
class Shape;
class Material;


// Ray intersection class that stores information about a rays
// interaction with the surface of a SceneObject.
// Fields are valid iff 'm_valid' is true.
class RayIntersection {
public:
	// set to true if there was actually an intersection
	bool m_valid = false;

	// distance along the ray the intersection occured
	float m_distance = std::numeric_limits<float>::infinity();
	
	// position, normal and uv coordinates of the surface
	glm::vec3 m_position;
	glm::vec3 m_normal;
	glm::vec2 m_uv_coord; // challenge only!

	// pointers to the original shape and material
	Shape * m_shape = nullptr;
	Material * m_material = nullptr;
};


class Scene {
private:
	std::vector<std::shared_ptr<SceneObject>> m_objects;
	std::vector<std::shared_ptr<Light>> m_lights;

public:

	Scene() { }

	Scene(std::vector<std::shared_ptr<SceneObject>> objects, std::vector<std::shared_ptr<Light>> lights)
		: m_objects(objects), m_lights(lights) { }

	// return an intersetion for a ray in the scene
	RayIntersection intersect(const Ray &ray);

	// returns a vector of the objects in the scene
	std::vector<std::shared_ptr<SceneObject>> objects() const { return m_objects; }

	// returns a vector of the lights in the scene
	std::vector<std::shared_ptr<Light>> lights() const { return m_lights; }


	// Simple scene with a single sphere, box, and light.
	// requires Sphere
	static Scene simpleScene();

	// Scene to test with more complex lighting
	// 2 point lights and one directional light.
	// requires Sphere and PointLight
	static Scene lightScene();
	
	// Large scene with a grid of spheres with
	// slightly different material paramters to
	// test the correctness of lighting calculations.
	// requires Sphere
	static Scene materialScene();
	
	// Scene to demostrate intersection with all
	// shape types : AABB, Sphere, Plane, Disk, Triangle
	// to be filled in by user
	static Scene shapeScene();

	// Typical raytracing scene
	// requires Sphere and PointLight
	static Scene cornellBoxScene();
};