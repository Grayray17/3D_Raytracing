
#pragma once

// std
#include <memory>

// project
#include "shape.hpp"
#include "material.hpp"
#include "path_tracer.hpp"


// A simple scene object that contains a pointer to
// its shape and material (that may be shared)
// Also provides a method for getting an intersection
// with its shape and recording its material.
class SceneObject {
private:
	std::shared_ptr<Shape> m_shape;
	std::shared_ptr<Material> m_material;

public:
	SceneObject(std::shared_ptr<Shape> shape, std::shared_ptr<Material> material);
	RayIntersection intersect(const Ray &ray);
};