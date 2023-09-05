
// std
#include <algorithm>
#include <utility>
#include <iostream>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// project
#include "shape.hpp"


using namespace glm;


RayIntersection AABB::intersect(const Ray &ray) {
	RayIntersection intersect;
	vec3 rel_origin = ray.origin - m_center;

	// start magic
	// x
	float rdx_inv = 1 / ray.direction.x;
	float tx1 = (-m_halfsize.x - rel_origin.x) * rdx_inv;
	float tx2 = (m_halfsize.x - rel_origin.x) * rdx_inv;

	float tmin = std::min(tx1, tx2);
	float tmax = std::max(tx1, tx2);

	// y
	float rdy_inv = 1 / ray.direction.y;
	float ty1 = (-m_halfsize.y - rel_origin.y) * rdy_inv;
	float ty2 = (m_halfsize.y - rel_origin.y) * rdy_inv;

	tmin = std::max(tmin, std::min(ty1, ty2));
	tmax = std::min(tmax, std::max(ty1, ty2));

	// z
	float rdz_inv = 1 / ray.direction.z;
	float tz1 = (-m_halfsize.z - rel_origin.z) * rdz_inv;
	float tz2 = (m_halfsize.z - rel_origin.z) * rdz_inv;

	tmin = std::max(tmin, std::min(tz1, tz2));
	tmax = std::min(tmax, std::max(tz1, tz2));

	if (tmax < tmin) return intersect;
	// end magic

	intersect.m_distance = tmin < 0 ? tmax : tmin;
	intersect.m_position = ray.origin + intersect.m_distance * ray.direction;
	intersect.m_valid = tmax >= 0;
	vec3 work_out_a_name_for_it_later = abs((intersect.m_position - m_center) / m_halfsize);
	float max_v = std::max(work_out_a_name_for_it_later[0], std::max(work_out_a_name_for_it_later[1], work_out_a_name_for_it_later[2]));
	intersect.m_normal = normalize(mix(intersect.m_position - m_center, vec3(0), lessThan(work_out_a_name_for_it_later, vec3(max_v))));
	intersect.m_uv_coord = (abs(intersect.m_normal.x) > 0) ?
		vec2(intersect.m_position.y, intersect.m_position.z) :
		vec2(intersect.m_position.x, intersect.m_position.y + intersect.m_position.z);
	intersect.m_shape = this;

	return intersect;
}

RayIntersection Sphere::intersect(const Ray &ray) {
	RayIntersection intersect;
	//-------------------------------------------------------------
	// [Assignment 4] :
	// Implement the intersection method for Sphere that returns
	// a RayIntersection object with valid == false if the
	// ray doesn't intersect and true otherwise. If true, then
	// remember to fill out each feild in the object correctly:
	// - m_valid : true if object is itersected
	// - m_distance : distance along the ray of the intersection
	// - m_position : position on the surface of the intersection
	// - m_normal : normal on the surface of the intersection
	// - m_shape : set to "this"
	// - m_uv_coord : texture coordinates (challenge only)
	//-------------------------------------------------------------
    vec3 L = m_center - ray.origin;
    float tca = dot(L, ray.direction);
    float d2 = dot(L, L) - tca * tca;
    float thc = sqrt(m_radius*m_radius-d2);
    float t0 = tca - thc;
    float t1 = tca + thc;
    intersect.m_valid = false;
    if((t0>0 && t1>t0) || t0==t1){
        intersect.m_valid = true;
        intersect.m_distance = t0;
        intersect.m_position = ray.origin + t0*ray.direction;
        intersect.m_normal = intersect.m_position - m_center;
        intersect.m_shape = this;
    }
    return intersect;
}



RayIntersection Plane::intersect(const Ray &ray) {
    RayIntersection intersect;
    float denom = dot(m_normal, ray.direction);
    if (abs(denom) > 0.01) {
        float t = dot( m_point - ray.origin, m_normal) / denom;
        if (t >= 0){
            intersect.m_valid = true;
            intersect.m_distance = t;
            intersect.m_position = ray.origin + intersect.m_distance * ray.direction;
            intersect.m_normal = dot(ray.direction, m_normal) > 0 ? -m_normal : m_normal;
            intersect.m_shape = this;
        }
    }
    return intersect;
}

RayIntersection Disk::intersect(const Ray &ray) {
    Plane m_plane = Plane(m_center, m_normal);
    RayIntersection intersection = m_plane.intersect(ray);
    if (intersection.m_valid) {
        vec3 p = ray.origin + ray.direction * intersection.m_distance;
        vec3 v = p - m_center;
        float d2 = dot(v,v);
        if (sqrt(d2) > m_radius){
            intersection.m_valid = false;
        }
    }
    intersection.m_normal *= -1;
    return intersection;
}

RayIntersection Triangle::intersect(const Ray &ray) {
    RayIntersection intersect;
    vec3 v0v1 = m_corner2 - m_corner1;
    vec3 v0v2 = m_corner3 - m_corner1;

    vec3 N = cross(v0v1, v0v2);
    float area2 = length(N);

    float NdotRayDirection = dot(N, ray.direction);
    if (abs(NdotRayDirection) < 0.01){ return intersect; }
    float d = -dot(N, m_corner1);
    float t = -(dot(N, ray.origin) + d) / NdotRayDirection;
    if (t < 0) { return intersect; }
    vec3 p = ray.origin + t * ray.direction;

    vec3 c;
    vec3 vp0 = p - m_corner1;
    c = cross(v0v1, vp0);
    if (dot(N, c) < 0) { return intersect; }

    vec3 edge = m_corner3 - m_corner2;
    vec3 vp1 = p - m_corner2;
    c = cross(edge, vp1);
    if (dot(N, c) < 0) { return intersect; }

    edge = m_corner1 - m_corner3;
    vec3 vp2 = p - m_corner3;
    c = cross(edge, vp2);
    if (dot(N, c) < 0) { return intersect; }

    intersect.m_valid = true;
    intersect.m_distance = t;
    intersect.m_position = ray.origin + intersect.m_distance * ray.direction;
    intersect.m_normal = glm::normalize(glm::cross(v0v1, v0v2));
    intersect.m_shape = this;

    return intersect;
}