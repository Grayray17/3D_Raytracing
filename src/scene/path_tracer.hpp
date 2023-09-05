
#pragma once

// glm
#include <glm/glm.hpp>

// project
#include "ray.hpp"
#include "scene.hpp"


// The base class for the pathtracer (backwards ray tracing) which
// provides a constructor that takes a scene and a method that
// casts a ray into the scene and returns the correct color
class PathTracer {
public:
	Scene *m_scene;

	PathTracer(Scene *s) : m_scene(s) { }
	virtual glm::vec3 sampleRay(const Ray &ray, int depth) = 0;
};


// A pathtracer that renders a simple smooth grey representation of the scene
class SimplePathTracer : public PathTracer {
public : 
	SimplePathTracer(Scene *s) : PathTracer(s) { }
	virtual glm::vec3 sampleRay(const Ray &ray, int) override;
};


// A pathtracer that renders the scene with the following :
//  - Lambertian diffuse reflection
//  - Blinn-Phong specular reflection
//  - Shadow Rays
class CorePathTracer : public PathTracer {
public:
	CorePathTracer(Scene *s) : PathTracer(s) { }
	virtual glm::vec3 sampleRay(const Ray &ray, int) override;
};


// Like the CorePathTracer but with the following addition :
//  - Perfect specular reflection (for shiny objects)
class CompletionPathTracer : public PathTracer {
public:
	CompletionPathTracer(Scene *s) : PathTracer(s) { }
	virtual glm::vec3 sampleRay(const Ray &ray, int depth = 0) override;
};


// Like the CompletionPathTracer but with the following additions :
//  - Texture material support
//  - Indirect diffuse lighting instead of ambient lighting
//  - Glossy reflections instead of perfect specular
class ChallengePathTracer : public PathTracer {
public:
	ChallengePathTracer(Scene *s) : PathTracer(s) { }
	virtual glm::vec3 sampleRay(const Ray &ray, int depth = 0) override;
};