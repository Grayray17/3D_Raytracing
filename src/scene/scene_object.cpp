
// std
#include <iostream>

// glm
#include <glm/gtc/matrix_transform.hpp>

// project
#include "scene_object.hpp"
#include "ray.hpp"
#include "scene.hpp"


using namespace std;


SceneObject::SceneObject(std::shared_ptr<Shape> shape, std::shared_ptr<Material> material)
	: m_shape(shape), m_material(material)
{
	assert(shape.get()); // assert not null
	assert(material.get()); // assert not null
}


RayIntersection SceneObject::intersect(const Ray &ray) {
	RayIntersection intersect = m_shape->intersect(ray);
	intersect.m_material = m_material.get();
	return intersect;
}