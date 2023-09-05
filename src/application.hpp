
#pragma once

// std
#include <atomic>
#include <thread>

// glm
#include <glm/glm.hpp>

// project
#include "opengl.hpp"
#include "scene/path_tracer.hpp"
#include "scene/scene.hpp"
#include "scene/camera.hpp"

// main application class
class Application {
private:
	// window
	GLFWwindow *m_window;

	// time-keeping
	float m_dt_last;
	std::chrono::steady_clock::time_point m_time_last = std::chrono::steady_clock::now();

	// render parameters
	int m_render_width = 0, m_render_height = 0; // current render size
	int m_render_perpixel_samples = 1;
	int m_render_ray_depth = 2;

	// render data
	float m_exposure = 1.0;
	struct pixel { float r, g, b, time; };
	std::vector<pixel> m_render_data;
	std::vector<int> m_shuffle_table;
	int m_sample_pass_count = 0;
	std::atomic<int> m_sample_pixel_count{0};

	// render thread and state
	std::thread m_raytrace_thread;
	std::atomic<bool> m_should_exit{false};
	std::chrono::time_point<std::chrono::steady_clock> m_start_time;
	std::chrono::time_point<std::chrono::steady_clock> m_end_time;
	float m_frame_time = 0;

	// preview state
	bool m_preview_mode = false;
	bool m_restart_render = false;

	// gl handles
	GLuint m_filter_prog = 0, m_display_prog = 0;
	GLuint m_render_texture_back = 0, m_render_texture_front = 0, m_render_texture_filtered = 0, m_screenshot_texture = 0;
	GLuint m_render_pbo = 0;
	GLuint m_render_fbo = 0, m_screenshot_fbo;
	GLsync m_sync_render_texture = nullptr;

	// scene
	Scene m_scene;
	std::unique_ptr<Camera> m_camera = nullptr;
	std::unique_ptr<PathTracer> m_pathtracer = nullptr;

	// updates the cameras position and rotation
	// if preview mode is enabled
	void updateCameraMovement(int w, int h);

	// saves a png screenshot of the current rendering
	void screenshot(const std::string &filename);

	// helper functions for running integration
	void resize(int w, int h);
	void start();
	void stop();

	// thread only function
	void runPathTraceIntegrator();


public:
	// setup
	Application(GLFWwindow *);
	~Application();

	// disable copy constructors (for safety)
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	// input callbacks
	void cursorPosCallback(double, double) { }
	void mouseButtonCallback(int, int, int) { }
	void scrollCallback(double, double) { }
	void keyCallback(int key, int scancode, int action, int mods);
	void charCallback(unsigned int) { }

	// rendering callbacks (every frame)
	void render();
	void renderGUI();
};