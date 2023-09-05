
// std
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <random>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

// stb
#include <stb_image_write.h>

// openmp (if avaliable)
#ifdef CGRA_HAVE_OPENMP
#include <omp.h>
#endif // CGRA_HAVE_OPENMP

// project
#include "opengl.hpp"
#include "application.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_shader.hpp"


using namespace std;
using namespace glm;
using namespace cgra;

// helper functions
namespace {

	inline void draw_dummy(unsigned instances = 1) {
		static GLuint vao = 0;
		if (vao == 0) {
			glGenVertexArrays(1, &vao);
		}
		glBindVertexArray(vao);
		glDrawArraysInstanced(GL_POINTS, 0, 1, instances);
		glBindVertexArray(0);
	}

	inline vec3 project(const vec3 &v, const vec3 &n) {
		return n * (dot(v, n) / dot(v, v));
	}

	inline vec3 reject(const vec3 &v, const vec3 &n) {
		return v - project(v, n);
	}
}

Application::Application(GLFWwindow *win) : m_window(win) {
	// setup Camera
	m_camera = std::make_unique<Camera>();

	// setup default pathtracer
	m_pathtracer = std::make_unique<SimplePathTracer>(&m_scene);

	// start at same size as window to minimize aliasing
	int w = 0, h = 0;
	glfwGetWindowSize(m_window, &w, &h);
	resize(w, h);

	// setup shaders
	{
		shader_program s;
		s.set_shader(GL_VERTEX_SHADER, CGRA_WORKDIR + string("res/shaders/filter.glsl"));
		s.set_shader(GL_GEOMETRY_SHADER, CGRA_WORKDIR + string("res/shaders/filter.glsl"));
		s.set_shader(GL_FRAGMENT_SHADER, CGRA_WORKDIR + string("res/shaders/filter.glsl"));
		m_filter_prog = s.compile();
	}
	{
		shader_program s;
		s.set_shader(GL_VERTEX_SHADER, CGRA_WORKDIR + string("res/shaders/display.glsl"));
		s.set_shader(GL_GEOMETRY_SHADER, CGRA_WORKDIR + string("res/shaders/display.glsl"));
		s.set_shader(GL_FRAGMENT_SHADER, CGRA_WORKDIR + string("res/shaders/display.glsl"));
		m_display_prog = s.compile();
	}

	// setup textures
	// using nearest to avoid upscaling problems with pixel time filtering
	// texture 0
	glGenTextures(1, &m_render_texture_back);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_render_texture_back);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_render_width, m_render_height, 0, GL_RGBA, GL_FLOAT, nullptr);
	// texture 1
	glGenTextures(1, &m_render_texture_front);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_render_texture_front);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_render_width, m_render_height, 0, GL_RGBA, GL_FLOAT, nullptr);
	// filtered texture
	glGenTextures(1, &m_render_texture_filtered);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_render_texture_filtered);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_render_width, m_render_height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glGenerateMipmap(GL_TEXTURE_2D);
	// screenshot texture
	glGenTextures(1, &m_screenshot_texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_screenshot_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, m_render_width, m_render_height, 0, GL_RGBA, GL_FLOAT, nullptr);
	// pbo
	glGenBuffers(1, &m_render_pbo);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_render_pbo);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	// fbo
	glGenFramebuffers(1, &m_render_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_render_fbo);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_render_texture_filtered, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	// sceenshot fbo
	glGenFramebuffers(1, &m_screenshot_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_screenshot_fbo);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_screenshot_texture, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// start only after we finish initializing
	start();
}


Application::~Application() {
	glDeleteProgram(m_filter_prog);
	glDeleteProgram(m_display_prog);
	glDeleteTextures(1, &m_render_texture_back);
	glDeleteTextures(1, &m_render_texture_front);
	glDeleteBuffers(1, &m_render_pbo);
	glDeleteFramebuffers(1, &m_render_fbo);
	m_should_exit = true;
	m_raytrace_thread.join();
}


void Application::keyCallback(int key, int, int action, int) {
	// get window size
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);

	if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
		m_preview_mode = !m_preview_mode;
		glfwSetCursorPos(m_window, width * 0.5, height * 0.5);
		glfwSetInputMode(m_window, GLFW_CURSOR, (m_preview_mode) ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
		m_restart_render = true;
	}
}


void Application::render() {
	// get window size
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);

	// calulate delta time and update last render
	const auto time_now = chrono::steady_clock::now();
	m_dt_last = float((time_now - m_time_last) / 1.0s);
	m_time_last = time_now;

	// clear the back-buffer to a solid grey/blueish background
	glViewport(0, 0, width, height);
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// update camera
	updateCameraMovement(width, height);
	if (m_restart_render) start();

	glActiveTexture(GL_TEXTURE0);

	// swap front and back render textures and upload the cpu-side
	// pixel data through m_render_pbo to m_render_texture_back
	// iif previous upload complete
	if (!m_sync_render_texture || glClientWaitSync(m_sync_render_texture, 0, 0) == GL_ALREADY_SIGNALED) {
		if (m_sync_render_texture) glDeleteSync(m_sync_render_texture);
		m_sync_render_texture = nullptr;

		// swap buffers (start displaying previous upload)
		swap(m_render_texture_back, m_render_texture_front);

		// upload new data, use pbo for async
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_render_pbo);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, m_render_data.size() * sizeof(pixel), nullptr, GL_STREAM_DRAW);
		auto *pbodata = reinterpret_cast<pixel *>(glMapBufferRange(
			GL_PIXEL_UNPACK_BUFFER,
			0,
			m_render_data.size() * sizeof(pixel),
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
		));
		copy(m_render_data.begin(), m_render_data.end(), pbodata);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		glBindTexture(GL_TEXTURE_2D, m_render_texture_back);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_render_width, m_render_height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		// fence so we know when upload finishes
		m_sync_render_texture = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	}

	// filter m_render_texture_front to m_render_texture_filtered
	// reconstructs out of date pixel data based on timestamp
	glBindTexture(GL_TEXTURE_2D, m_render_texture_filtered);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_render_width, m_render_height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_render_fbo);
	glViewport(0, 0, m_render_width, m_render_height);
	glUseProgram(m_filter_prog);
	glBindTexture(GL_TEXTURE_2D, m_render_texture_front);
	glUniform1i(glGetUniformLocation(m_filter_prog, "uTexture0"), 0);
	glUniform1f(glGetUniformLocation(m_filter_prog, "uFrameTime"), m_frame_time);
	draw_dummy();

	// bind screen fbo and regenerate mipmaps for m_render_texture_filtered
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, m_render_texture_filtered);
	glGenerateMipmap(GL_TEXTURE_2D);

	// resize the viewport to the aspect ration of the image
	float window_aspect_ratio = float(width) / height;
	float image_aspect_ratio = float(m_render_width) / m_render_height;

	if (window_aspect_ratio > image_aspect_ratio) {
		int width_buffer = int(width - (m_render_width * float(height) / m_render_height));
		glViewport(width_buffer / 2, 0, width - width_buffer, height);
	} else {
		int height_buffer = int(height - (m_render_height * float(width) / m_render_width));
		glViewport(0, height_buffer / 2, width, height - height_buffer);
	}

	// finally draw the final render given the exposure
	// while converting to suitable SRGB format
	//glEnable(GL_FRAMEBUFFER_SRGB);
	glUseProgram(m_display_prog);
	glUniform1i(glGetUniformLocation(m_display_prog, "uTexture0"), 0);
	glUniform1f(glGetUniformLocation(m_display_prog, "uExposure"), m_exposure);
	glUniform1f(glGetUniformLocation(m_display_prog, "uFrameTime"), m_frame_time);
	draw_dummy();
	//glDisable(GL_FRAMEBUFFER_SRGB);

	// safety for overdraw
	this_thread::sleep_for(5ms);
}


void Application::renderGUI() {

	// progress bars and total duration
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiSetCond_Once);
	ImGui::Begin("Debug", 0);
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	// total progress (passes + pixels / total passes)
	ImGui::ProgressBar((m_sample_pass_count + (float(m_sample_pixel_count) / m_render_data.size())) / m_render_perpixel_samples, ImVec2(-50, 0));
	ImGui::SameLine();
	ImGui::Text("Total");

	// pass progress (pixels / pass)
	ImGui::ProgressBar(float(m_sample_pixel_count) / m_render_data.size(), ImVec2(-50, 0));
	ImGui::SameLine();
	ImGui::Text("Pass");

	// total duration of render
	ostringstream oss;
	float duration = float(((m_end_time > m_start_time) ? (m_end_time - m_start_time) : (chrono::steady_clock::now() - m_start_time)) / 1.0s);
	oss << "Duration : " << std::fixed << std::setprecision(2) << duration << " seconds";
	ImGui::Text(oss.str().c_str());

	ImGui::Separator();
	
	ImGui::Text("Display");

	if (ImGui::Checkbox("Preview (press ` to toggle)", &m_preview_mode)) {
		int width, height;
		glfwGetFramebufferSize(m_window, &width, &height);
		glfwSetCursorPos(m_window, width * 0.5, height * 0.5);
		glfwSetInputMode(m_window, GLFW_CURSOR, (m_preview_mode) ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
		m_restart_render = true;
	}

	static int scene_index = -1;
	if (ImGui::Combo("Scene", &scene_index, "Simple Test\0Light Test\0Material Test\0Shape Test\0Cornell Box\0", 4)) {
		stop();
		switch (scene_index) {
		case 0: m_scene = Scene::simpleScene(); break;
		case 1: m_scene = Scene::lightScene(); break;
		case 2: m_scene = Scene::materialScene(); break;
		case 3: m_scene = Scene::shapeScene(); break;
		case 4: m_scene = Scene::cornellBoxScene(); break;
		}
		
		m_restart_render = true;
		start();
	}

	static int pathtracer_index = 0;
	if (ImGui::Combo("PathTracer", &pathtracer_index, "Simple\0Core\0Completion\0Challenge\0", 4)) {
		stop();
		switch (pathtracer_index) {
		case 0: m_pathtracer = make_unique<SimplePathTracer>(&m_scene); break;
		case 1: m_pathtracer = make_unique<CorePathTracer>(&m_scene); break;
		case 2: m_pathtracer = make_unique<CompletionPathTracer>(&m_scene); break;
		case 3: m_pathtracer = make_unique<ChallengePathTracer>(&m_scene); break;
		}
		m_restart_render = true;
		start();
	}

	ImGui::SliderFloat("Exposure", &m_exposure, 0, 100.0, "%.1f", 3.f);


	// screen shots
	static char filename[1024] = "";
	if (ImGui::Button("Screenshot")) {
		ImGui::OpenPopup("Save As");
	}
	if (ImGui::BeginPopup("Save As")) {
		ImGui::InputText("Filename", filename, 1024);
		if (ImGui::Button("Save")) {
			screenshot(filename);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}



	ImGui::Separator();

	ImGui::Text("Render Settings");

	static int size[2] = { m_render_width, m_render_height };
	static float samples = float(m_render_perpixel_samples);
	static int ray_depth = m_render_ray_depth;

	ImGui::InputInt2("Size (w,h)", size);
	ImGui::SliderFloat("Samples", &samples, 1, 10000, "%.0f", 5.f);
	ImGui::SliderInt("Ray depth", &ray_depth, 0, 10);

	if (ImGui::Button("Force Restart", ImVec2(-1, 0))) {
		stop();
		resize(size[0], size[1]);
		m_render_perpixel_samples = int(samples);
		m_render_ray_depth = ray_depth;
		start();
	}


	ImGui::End();
}


void Application::updateCameraMovement(int w, int h) {
	m_restart_render = false;

	// early out
	if (!m_preview_mode) return;

	const float rot_speed = 600;
	const float m_speed = 2;

	// calculate movement directions
	vec3 up = vec3(0, 1, 0);
	vec3 forward = normalize(reject(rotate(mat4(1), m_camera->yaw(), up) * vec4(0, 0, -1, 0), up));
	vec3 side = normalize(cross(forward, up));


	vec3 pos = m_camera->position();
	float yaw = m_camera->yaw();
	float pitch = m_camera->pitch();


	double x, y;
	glfwGetCursorPos(m_window, &x, &y);
	x -= w * 0.5;
	y -= h * 0.5;
	if (abs(x) > 0.5 || abs(y) > 0.5) {
		yaw += float(-x / rot_speed);
		pitch += float(-y / rot_speed);
		pitch = std::clamp(pitch, -0.49f * pi<float>(), 0.49f * pi<float>());
		glfwSetCursorPos(m_window, w * 0.5, h * 0.5);
		m_restart_render = true;
	}

	vec3 move{ 0 }; 

	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) move += forward;
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) move -= forward;
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) move -= side;
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) move += side;
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) move -= up;
	if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) move += up;

	if (length(move) > 0.1f) {
		auto dpos = normalize(move) * m_speed * m_dt_last;
		pos += dpos;
		m_restart_render = true;
	}

	m_camera->setPositionOrientation(pos, yaw, pitch);
}


void Application::screenshot(const std::string &filename) {

	// render to screenshot buffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_screenshot_fbo);

	glViewport(0, 0, m_render_width, m_render_height);

	glUseProgram(m_display_prog);
	glActiveTexture(GL_TEXTURE0);
	
	// resize target
	glBindTexture(GL_TEXTURE_2D, m_screenshot_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, m_render_width, m_render_height, 0, GL_RGBA, GL_FLOAT, nullptr);

	// bind uniforms
	glBindTexture(GL_TEXTURE_2D, m_render_texture_filtered);
	glUniform1i(glGetUniformLocation(m_display_prog, "uTexture0"), 0);
	glUniform1f(glGetUniformLocation(m_display_prog, "uExposure"), m_exposure);
	glUniform1f(glGetUniformLocation(m_display_prog, "uFrameTime"), m_frame_time);
	draw_dummy();


	// read in data
	std::vector<unsigned char> data(m_render_width * m_render_height * 3);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_screenshot_fbo);
	glReadPixels(0, 0, m_render_width, m_render_height, GL_RGB, GL_UNSIGNED_BYTE, data.data());

	std::ostringstream ss;
	ss << filename << ".png";
	if (stbi_write_png(ss.str().c_str(), m_render_width, m_render_height, 3, data.data() + (m_render_height - 1) * m_render_width * 3, -m_render_width * 3)) {
		std::cout << "Wrote image: " << ss.str() << std::endl;
	}
	else {
		std::cerr << "Failed to write image: " << ss.str() << std::endl;
	}
}


void Application::resize(int w, int h) {
	// set values (if changed)
	if (m_render_width != w || m_render_height != h) {
		m_render_width = w;
		m_render_height = h;

		m_camera->setImageSize({w, h});

		// setup shuffle table
		m_shuffle_table.resize(w * h);
		std::iota(m_shuffle_table.begin(), m_shuffle_table.end(), 0);
		std::shuffle(m_shuffle_table.begin(), m_shuffle_table.end(), minstd_rand());
	}

	// clear pixel data
	m_render_data.assign(w*h, {});
}


void Application::start() {
	if (m_raytrace_thread.joinable()) {
		if (m_should_exit) {
			stop();
		} else {
			return;
		}
	}
	// restarting the thread, so ensure image is the right size
	// (but don't bother clearing it, shuffle index randomization means it basically isnt necessary)
	m_render_data.resize(m_render_width * m_render_height);
	m_should_exit = false;
	m_sample_pass_count = 0;
	m_sample_pixel_count = 0;
	m_raytrace_thread = thread([this]() { runPathTraceIntegrator(); });
}

void Application::stop() {
	m_should_exit = true;
	if (m_raytrace_thread.joinable()) m_raytrace_thread.join();
}



void Application::runPathTraceIntegrator() {
	
	// was any rendering done in preview mode?
	bool was_preview = false;

	// count 'idle' preview frames so we can exit instead of spinning uselessly
	int idle_preview_frames = 0;
	int preview_frames = 0;

	// we can't break out of an openmp loop
	// so we have a variable that gets checked
	// every few iterations
	bool cancel_for = false;

	do {
		// stop rendering
		if (idle_preview_frames > 15) break;

		was_preview = m_preview_mode;

		// reset time variables
		m_end_time = chrono::steady_clock::now() - 1ms;
		m_start_time = chrono::steady_clock::now();

		// only increment the frame time if last frame was incomplete
		// prevents 'pulsating' effect when preview is spinning idly
		if (cancel_for) m_frame_time = float(fmod(m_frame_time + 0.03, 100.0));

		cancel_for = false;

		// for each sample
		for (m_sample_pass_count = 0; m_sample_pass_count < (was_preview ? 1 : m_render_perpixel_samples) && !cancel_for; m_sample_pass_count++) {

			m_sample_pixel_count = 0;

			// for each pixel
			// use 1 fewer threads in preview mode to maintain responsiveness
#pragma omp parallel for num_threads(std::max(omp_get_max_threads() - was_preview, 1))
			for (int i = 0; i < int(m_render_data.size()); ++i) {
				if (!cancel_for) {
					int idx = m_shuffle_table[(i + preview_frames * 9001) % m_render_data.size()];

					// calculate the pixel coordinate
					vec2 screen_coord(idx % m_render_width, idx / m_render_width);

					// calculate some jitter
					// glm's random is implemented with rand(), which is terrible
					static thread_local minstd_rand randgen{std::random_device()()};
					uniform_real_distribution<float> dist{0, 1};
					vec2 rand = vec2(dist(randgen), dist(randgen));
					// reduce jitter for initial samples, improves results for low sample counts
					rand = (rand - 0.5f) * (1.f - exp(float(m_sample_pass_count) * -0.4f)) + 0.5f;


					// The actual raytracing commands!!!
					// create the ray and trace the scene
					Ray ray = m_camera->generateRay(screen_coord + rand);
					vec3 sample_color = m_pathtracer->sampleRay(ray, m_render_ray_depth);


					// mix with the existing color
					float sample_mix_factor = m_sample_pass_count / float(m_sample_pass_count + 1);
					vec3 running_mean_color(m_render_data[idx].r, m_render_data[idx].g, m_render_data[idx].b);
					vec3 final_color = mix(sample_color, running_mean_color, sample_mix_factor);

					// record final color and increase sample count
					m_render_data[idx] = {final_color.r, final_color.g, final_color.b, m_frame_time};
					m_sample_pixel_count++;

					// check cancel things every some number of pixels
					if ((i & 0xFF) == 0) {
						cancel_for |= m_should_exit;
						// if preview needs restarting, bail after 30ms to maintain ~30Hz
						if (m_preview_mode && m_restart_render) {
							was_preview = true;
							if (chrono::steady_clock::now() - m_start_time > 30ms) cancel_for = true;
						}
					}
				}
			}
		}

		m_end_time = chrono::steady_clock::now();
		preview_frames += was_preview;
		if (m_preview_mode && m_restart_render) {
			idle_preview_frames = 0;
		} else {
			idle_preview_frames++;
		}

		// exit after proper render or if requested
	} while ((was_preview || m_preview_mode) && !m_should_exit);

	// we'll abuse this to indicate the thread has exited normally too
	m_should_exit = true;
}
