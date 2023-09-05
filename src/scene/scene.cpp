
// std
#include <limits>

// glm
#include <glm/gtc/matrix_transform.hpp>

// project
#include "scene.hpp"
#include "scene_object.hpp"
#include "light.hpp"


using namespace std;
using namespace glm;


RayIntersection Scene::intersect(const Ray &ray) {
	RayIntersection closest_intersect;
	
	// go through the vector of objects and return the closest intersection
	for (shared_ptr<SceneObject> &object : m_objects) {
		RayIntersection intersect = object->intersect(ray);
		if (intersect.m_valid && intersect.m_distance < closest_intersect.m_distance) {
			closest_intersect = intersect;
		}
	}

    // translate a bit to the outside to avoid inside intersections
    if (closest_intersect.m_valid)
        closest_intersect.m_position += closest_intersect.m_normal * .0001f;

	return closest_intersect;
}


Scene Scene::simpleScene() {
	vector<shared_ptr<SceneObject>> objects;
	vector<shared_ptr<Light>> lights;

	// declare materials
	shared_ptr<Material> shiny_red = make_shared<Material>(vec3(1, 0, 0), 10, 0.5f, 0);
	shared_ptr<Material> green = make_shared<Material>(vec3(0, 0.8f, 0), 1.05f, 0.1f, 0);

	// create a box on a sphere
	objects.push_back(make_shared<SceneObject>(make_shared<Sphere>(vec3(0, -2, -10), 1), shiny_red));
	objects.push_back(make_shared<SceneObject>(make_shared<AABB>(vec3(0, -3.5f, -10), vec3(3, 0.5, 3)), green));

	// one directional light
	lights.push_back(make_shared<DirectionalLight>(vec3(-1, -1, -1), vec3(0.5f), vec3(0.05f)));

	return Scene(objects, lights);
}


Scene Scene::lightScene() {
	vector<shared_ptr<SceneObject>> objects;
	vector<shared_ptr<Light>> lights;

	// declare materials
	shared_ptr<Material> shiny_red = make_shared<Material>(vec3(1, 0, 0), 10, 0.5f, 0);
	shared_ptr<Material> green = make_shared<Material>(vec3(0, 0.8f, 0), 1.05f, 0.1f, 0);

	// create a box on a sphere
	objects.push_back(make_shared<SceneObject>(make_shared<Sphere>(vec3(0, -2, -10), 1), shiny_red));
	objects.push_back(make_shared<SceneObject>(make_shared<AABB>(vec3(0, -3.5f, -10), vec3(3, 0.5f, 3)), green));

	// wall blocking one of the point lights
	objects.push_back(make_shared<SceneObject>(make_shared<AABB>(vec3(3.5f, 0, -10), vec3(0.5f, 3, 3)), green));

	// one directional light
	lights.push_back(make_shared<DirectionalLight>(vec3(-1, -1, -1), vec3(0.5f), vec3(0.05f)));

	// two point lights
	lights.push_back(make_shared<PointLight>(vec3(-5, 0, -10), vec3(50), vec3(0.05f)));
	lights.push_back(make_shared<PointLight>(vec3(5, 0, -10), vec3(50), vec3(0.05f)));

	return Scene(objects, lights);
}


Scene Scene::materialScene() {
	vector<shared_ptr<SceneObject>> objects;
	vector<shared_ptr<Light>> lights;

	// declare materials
	shared_ptr<Material> green = make_shared<Material>(vec3(0, 0.8f, 0), 1.05f, 0.1f, 0);

	// create a grid of materials with varying shininess and specular ratios
	for (int shin = 0; shin <= 10; shin++) {
		for (int spec = 0; spec <= 10; spec++) {
			float shininess = 1 * exp(float(shin)); // value increasing from 1
			float specular_ratio = spec / 10.f; // ratio in [0,1]

			shared_ptr<Material> m = make_shared<Material>(vec3(1, 0, 0), shininess, specular_ratio, 0);

			objects.push_back(make_shared<SceneObject>(make_shared<Sphere>(vec3(5 - shin, -2, -5 - spec), 0.4), m));
		}
	}

	objects.push_back(make_shared<SceneObject>(make_shared<AABB>(vec3(0, -3, -10), vec3(6, 0.5f, 6)), green));

	lights.push_back(make_shared<DirectionalLight>(vec3(-1, -1, -1), vec3(0.5f), vec3(0.05f)));

	return Scene(objects, lights);
}


Scene Scene::shapeScene() {
	vector<shared_ptr<SceneObject>> objects;
	vector<shared_ptr<Light>> lights;

	//-------------------------------------------------------------
	// [Assignment 4] :
	// To show off intersection with shapes create a scene with
	// the different shapes you have implemented.
	// Core:
	//  - Sphere
	// Completion:
	//  - Plane
	//  - Disk
	//  - Triangle
	//-------------------------------------------------------------

	shared_ptr<Material> white = make_shared<Material>(vec3(1), 1.05f, 0.1, 0);
	shared_ptr<Material> test = make_shared<Material>(vec3(1,1,0), 1.05f, 0.1, 0);

	objects.push_back(make_shared<SceneObject>(make_shared<AABB>(vec3(-3, 0, -5), vec3(0.5)), white));
	objects.push_back(make_shared<SceneObject>(make_shared<Sphere>(vec3(-1, 0, -5), 0.5), white));

    objects.push_back(make_shared<SceneObject>(make_shared<Plane>(vec3(0,-3,0), vec3(0, 0.8, 0)), white));
    objects.push_back(make_shared<SceneObject>(make_shared<Disk>(vec3(1, 0, -5), vec3(-3, 0, 0.8), 1.2), white));
    objects.push_back(make_shared<SceneObject>(make_shared<Triangle>(vec3(2, -1, -5), vec3(3, -1, -5), vec3(2, 1, -5)), white));

	lights.push_back(make_shared<DirectionalLight>(vec3(-1, -1, -1), vec3(0.5f), vec3(0.05f)));

	return Scene(objects, lights);
}


Scene Scene::cornellBoxScene() {

	vector<shared_ptr<SceneObject>> objects;
	vector<shared_ptr<Light>> lights;

	auto white = make_shared<Material>(vec3(1), 1.05, 0.1, 0);
	auto green = make_shared<Material>(vec3(0, 1, 0), 1.05, 0.1, 0);
	auto red = make_shared<Material>(vec3(1, 0, 0), 1.05, 0.1, 0);

	auto gold = make_shared<Material>(vec3(1, 1, 0), 50, 0.8, 1);
	auto silver = make_shared<Material>(vec3(1, 1, 1), 1000, 0.8, 1);
	auto blue = make_shared<Material>(vec3(0.5f, 0.5f, 1), 1.1, 0.1, 0);


	// boxes
	//

	// top and bottom
	objects.push_back(make_shared<SceneObject>(make_shared<AABB>(vec3(0, 3.2f, 0), vec3(3, .2f, 13)), white));
	objects.push_back(make_shared<SceneObject>(make_shared<AABB>(vec3(0, -3.2f, 0), vec3(3, .2f, 13)), white));

	//front and back
	objects.push_back(make_shared<SceneObject>(make_shared<AABB>(vec3(0, 0, 13.2f), vec3(3, 3, .2f)), white));
	objects.push_back(make_shared<SceneObject>(make_shared<AABB>(vec3(0, 0, -13.2f), vec3(3, 3, .2f)), white));

	// left and right
	objects.push_back(make_shared<SceneObject>(make_shared<AABB>(vec3(-3.2f, 0, 0), vec3(.2f, 3, 13)), red));
	objects.push_back(make_shared<SceneObject>(make_shared<AABB>(vec3(3.2f, 0, 0), vec3(.2f, 3, 13)), green));


	// spheres
	objects.push_back(make_shared<SceneObject>(make_shared<Sphere>(vec3(1, -2, -7), 1), gold));
	objects.push_back(make_shared<SceneObject>(make_shared<Sphere>(vec3(-1.25, -2.25f, -7), .75f), silver));
	objects.push_back(make_shared<SceneObject>(make_shared<Sphere>(vec3(0, -1.5, -10), 1.5), blue));


	// lights
	lights.push_back(make_shared<PointLight>(vec3(0, 2.5f, -10), vec3(50), vec3(0.05f)));
	lights.push_back(make_shared<PointLight>(vec3(0, 2.5f, 0), vec3(50), vec3(0.05f)));
	lights.push_back(make_shared<PointLight>(vec3(0, 2.5f, 10), vec3(50), vec3(0.05f)));

	return Scene(objects, lights);
}