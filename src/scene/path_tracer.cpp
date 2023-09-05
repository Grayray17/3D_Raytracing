
// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

// std
#include <random>

// project
#include "scene.hpp"
#include "shape.hpp"
#include "light.hpp"
#include "material.hpp"
#include "path_tracer.hpp"


using namespace std;
using namespace glm;


vec3 SimplePathTracer::sampleRay(const Ray &ray, int) {
	// intersect ray with the scene
	RayIntersection intersect = m_scene->intersect(ray);

	// if ray hit something
	if (intersect.m_valid) {

		// simple grey shape shading
		float f = abs(dot(-ray.direction, intersect.m_normal));
		vec3 grey(0.5, 0.5, 0.5);
		return mix(grey / 2.0f, grey, f);
	}

	// no intersection - return background color
	return { 0.3f, 0.3f, 0.4f };
}



vec3 CorePathTracer::sampleRay(const Ray &ray, int) {
	//-------------------------------------------------------------
	// [Assignment 4] :
	// Implement a PathTracer that calculates the ambient, diffuse
	// and specular, for the given ray in the scene, using the 
	// Phong lighting model. Give special consideration to objects
	// that occluded from direct lighting (shadow rays). You do
	// not need to use the depth argument for this implementation.
	//-------------------------------------------------------------

    RayIntersection intersect = m_scene->intersect(ray);
    vec3 colour(0);
    if (!intersect.m_valid){ return { 0.3f, 0.3f, 0.4f }; } // Return bg on no intersect
    for (size_t i = 0; i < m_scene->lights().size(); i++) {
        std::shared_ptr<Light> light = m_scene->lights().at(i);
        vec3 diffuse = intersect.m_material->diffuse() * light->ambience();

        bool isOccluded = light->occluded(m_scene, intersect.m_position);

        float angle = glm::max(0.0f, dot(-light->incidentDirection(intersect.m_position), intersect.m_normal));
        vec3 diffuse_reflect = light->irradiance(intersect.m_position) * intersect.m_material->diffuse() * angle;

        vec3 reflect = glm::reflect(normalize(light->incidentDirection(intersect.m_position)), normalize(intersect.m_normal));

        angle = glm::max(0.0f, dot(  reflect, -ray.direction));
        angle = pow(angle, intersect.m_material->shininess());
        vec3 spec_reflect = light->irradiance(intersect.m_position) * angle * intersect.m_material->specular();


        colour += diffuse + (isOccluded ? vec3(0):diffuse_reflect + spec_reflect);
    }

	return colour;
}
//outer_color = diffuse + specular;
//if (depth > 1){
//vec3 rec_colour = CompletionPathTracer::sampleRay(Ray(intersect.m_position, normalize(glm::reflect(light->incidentDirection(intersect.m_position), intersect.m_normal))), depth-2);
//outer_color += rec_colour * intersect.m_material->specular();
//}


vec3 CompletionPathTracer::sampleRay(const Ray &ray, int depth) {
	//-------------------------------------------------------------
	// [Assignment 4] :
	// Using the same requirements for the CorePathTracer add in
	// a recursive element to calculate perfect specular reflection.
	// That is compute the reflection ray off your intersection and
	// sample a ray in that direction, using the result to additionally
	// light your object. To make this more realistic you may weight
	// the incoming light by the (1 - (1/shininess)).
	//-------------------------------------------------------------
    RayIntersection intersect = m_scene->intersect(ray);
    vec3 colour(0);
    vec3 rec_colour(0);
    if (!intersect.m_valid){ return { 0.3f, 0.3f, 0.4f }; } // Return bg on no intersect
    for (size_t i = 0; i < m_scene->lights().size(); i++) {
        std::shared_ptr<Light> light = m_scene->lights().at(i);
        vec3 diffuse = intersect.m_material->diffuse() * light->ambience();

        bool isOccluded = light->occluded(m_scene, intersect.m_position);

        float angle = glm::max(0.0f, dot(-light->incidentDirection(intersect.m_position), intersect.m_normal));
        vec3 diffuse_reflect = light->irradiance(intersect.m_position) * intersect.m_material->diffuse() * angle;

        vec3 reflect = glm::reflect(normalize(light->incidentDirection(intersect.m_position)), normalize(intersect.m_normal));

        angle = glm::max(0.0f, dot(  reflect, -ray.direction));
        angle = pow(angle, intersect.m_material->shininess());
        vec3 spec_reflect = light->irradiance(intersect.m_position) * angle * intersect.m_material->specular();
        colour += diffuse;
        if (!isOccluded){ colour += diffuse_reflect + spec_reflect; }
    }
    if (depth > 1){
        rec_colour = CompletionPathTracer::sampleRay(Ray(intersect.m_position, normalize(glm::reflect(normalize(ray.direction), normalize(intersect.m_normal)))), depth-1);
        colour += rec_colour * intersect.m_material->specular() * (1 - (1/ intersect.m_material->shininess()));
    }

    return colour;
}



vec3 ChallengePathTracer::sampleRay(const Ray &ray, int depth) {
	//-------------------------------------------------------------
	// [Assignment 4] :
	// Implement a PathTracer that calculates the diffuse and 
	// specular, for the given ray in the scene, using the 
	// Phong lighting model. Give special consideration to objects
	// that occluded from direct lighting (shadow rays).
	// Implement support for textured materials (using a texture
	// for the diffuse portion of the material).
	//
	// EXTRA FOR EXPERTS :
	// Additionally implement indirect diffuse and specular instead
	// of using the ambient lighting term.
	// The diffuse is sampled from the surface hemisphere and the
	// specular is sampled from a cone of the phong lobe (which
	// gives a glossy look). For best results you need to normalize
	// the lighting (see http://www.thetenthplanet.de/archives/255)
	//-------------------------------------------------------------

	// YOUR CODE GOES HERE
	// ...

	// no intersection - return background color
	return { 0.3f, 0.3f, 0.4f };
}