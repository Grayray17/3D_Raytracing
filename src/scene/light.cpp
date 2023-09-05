
// glm
#include <glm/gtc/constants.hpp>

// project
#include "light.hpp"

using namespace glm;


bool DirectionalLight::occluded(Scene *scene, const vec3 &point) const {
	//-------------------------------------------------------------
	// [Assignment 4] :
	// Determine whether the given point is being occluded from
	// this directional light by an object in the scene.
	// Remember that directional lights are "infinitely" far away
	// so any object in the way would cause an occlusion.
	//-------------------------------------------------------------

    Ray r = Ray(point, -m_direction);
    return scene->intersect(r).m_valid;
}


vec3 DirectionalLight::incidentDirection(const vec3 &) const {
	return m_direction;
}


vec3 DirectionalLight::irradiance(const vec3 &) const {
	return m_irradiance;
}


bool PointLight::occluded(Scene *scene, const vec3 &point) const {
	//-------------------------------------------------------------
	// [Assignment 4] :
	// Determine whether the given point is being occluded from
	// this directional light by an object in the scene.
	// Remember that point lights are somewhere in the scene and
	// an occulsion has to occur somewhere between the light and
	// the given point.
	//-------------------------------------------------------------
    vec3 dir = point - m_position;
    Ray ray = Ray(point, normalize(-dir));
    if (length(dir) < 0){ return false; }
    if(scene->intersect(ray).m_valid && scene->intersect(ray).m_distance < distance(m_position, point)){
        return true;
    }
    return false;
}


vec3 PointLight::incidentDirection(const vec3 &point) const {
	//-------------------------------------------------------------
	// [Assignment 4] :
	// Return the direction of the incoming light (light to point)
	//-------------------------------------------------------------
    vec3 dir = point - m_position;
    return normalize(dir);
}


vec3 PointLight::irradiance(const vec3 &point) const {
	//-------------------------------------------------------------
	// [Assignment 4] :
	// Return the total irradiance on the given point.
	// This requires you to convert the flux of the light into
	// irradiance by dividing it by the surface of the sphere
	// it illuminates. Remember that the surface area increases
	// as the sphere gets bigger, ie. the point is further away.
	//-------------------------------------------------------------
    vec3 dir = point - m_position;
    if (length(dir) > 0){ return (m_flux / (4.0f * pi<float>() * pow(length(dir), 2.0f))); }
    else{ return m_flux; }
}
