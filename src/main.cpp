
// std
#include <iostream>
#include <string>
#include <stdexcept>

// project
#include "application.hpp"
#include "opengl.hpp"
#include "cgra/cgra_gui.hpp"


using namespace std;
using namespace cgra;

// Forward decleration for cleanliness
void cursorPosCallback(GLFWwindow *, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods);
void scrollCallback(GLFWwindow *win, double xoffset, double yoffset);
void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods);
void charCallback(GLFWwindow *win, unsigned int c);
void APIENTRY debugCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, GLvoid*);

// Global static pointer to application once we create it
// nessesary for interfacing with the GLFW callbacks
static Application *application_ptr = nullptr;


// Main program
// 
int main() {

	// Initialize the GLFW library
	if (!glfwInit()) {
		cerr << "Error: Could not initialize GLFW" << endl;
		abort(); // Unrecoverable error
	}

	// Force OpenGL to create a 3.3 core context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Disallow legacy functionality (helps OS X work)
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Get the version for GLFW for later
	int glfwMajor, glfwMinor, glfwRevision;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);

	// Request a debug context so we get debug callbacks.
	// Remove this for possible GL performance increases.
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	// Request and SRGB capable context for gamma-corrected drawing
	glfwWindowHint(GLFW_SRGB_CAPABLE, true);

	// Create a windowed mode window and its OpenGL context
	GLFWwindow *window = glfwCreateWindow(800, 600, "Hello World!", nullptr, nullptr);
	if (!window) {
		cerr << "Error: Could not create GLFW window" << endl;
		abort(); // Unrecoverable error
	}

	// Make the window's context current.
	// If we have multiple windows we will need to switch contexts
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	// must be done after making a GL context current (glfwMakeContextCurrent in this case)
	glewExperimental = GL_TRUE; // required for full GLEW functionality for OpenGL 3.0+
	GLenum err = glewInit();
	if (GLEW_OK != err) { // Problem: glewInit failed, something is seriously wrong.
		cerr << "Error: " << glewGetErrorString(err) << endl;
		abort(); // Unrecoverable error
	}

	// Print out our OpenGL verisions
	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "Using GLFW " << glfwMajor << "." << glfwMinor << "." << glfwRevision << endl;

	// Enable GL_ARB_debug_output if available. Not necessary, just helpful
	if (glfwExtensionSupported("GL_ARB_debug_output")) {
		// This allows the error location to be determined from a stacktrace
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		// Setup up the callback
		glDebugMessageCallbackARB(debugCallback, nullptr);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
		cout << "GL_ARB_debug_output callback installed" << endl;
	}
	else {
		cout << "GL_ARB_debug_output not available. No worries." << endl;
	}

	// Initialize ImGui
	// Second argument is true if we dont need to use GLFW bindings for input
	// if set to false we must manually call the cgra::gui callbacks when we
	// process the input.
	// if (!cgra::gui::init(window, true)) {
	if (!cgra::gui::init(window, false)) {
		cerr << "Error: Could not initialize ImGui" << endl;
		abort(); // Unrecoverable error
	}

	// Create the application object (and a global pointer to it)
	Application application(window);
	application_ptr = &application;

	// Attach input callbacks to window
	// Should not be done if we want imgui to control input
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCharCallback(window, charCallback);

	

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window)) {

		// Main Render
		application.render();

		// GUI Render on top
		cgra::gui::newFrame();
		application.renderGUI();
		cgra::gui::render();

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();
	}

	// clean up ImGui
	cgra::gui::shutdown();
	glfwTerminate();
}


void cursorPosCallback(GLFWwindow *, double xpos, double ypos) {
	// we don't need to forward this to imgui
	application_ptr->cursorPosCallback(xpos, ypos);
}


void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) {
	cgra::gui::mouseButtonCallback(win, button, action, mods); // forward to ImGui
	if (ImGui::IsMouseHoveringAnyWindow()) return; // if ImGui is active don't do anything else

	application_ptr->mouseButtonCallback(button, action, mods);
}


void scrollCallback(GLFWwindow *win, double xoffset, double yoffset) {
	cgra::gui::scrollCallback(win, xoffset, yoffset); // forward to ImGui
	if (ImGui::IsMouseHoveringAnyWindow()) return; // if ImGui is active don't do anything else

	application_ptr->scrollCallback(xoffset, yoffset);
}


void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
	cgra::gui::keyCallback(win, key, scancode, action, mods); // forward to ImGui
	if (ImGui::IsAnyItemActive()) return; // if ImGui is active don't do anything else

	application_ptr->keyCallback(key, scancode, action, mods);
}


void charCallback(GLFWwindow *win, unsigned int c) {
	cgra::gui::charCallback(win, c); // forward to ImGui
	if (ImGui::IsAnyItemActive()) return; // if ImGui is active don't do anything else

	application_ptr->charCallback(c);
}


// function to translate source to string
const char * getStringForSource(GLenum source) {
	switch (source) {
	case GL_DEBUG_SOURCE_API:
		return "API";
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return "Window System";
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return "Shader Compiler";
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return "Third Party";
	case GL_DEBUG_SOURCE_APPLICATION:
		return "Application";
	case GL_DEBUG_SOURCE_OTHER:
		return "Other";
	default:
		return "n/a";
	}
}

// function to translate severity to string
const char * getStringForSeverity(GLenum severity) {
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		return "High";
	case GL_DEBUG_SEVERITY_MEDIUM:
		return "Medium";
	case GL_DEBUG_SEVERITY_LOW:
		return "Low";
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		return "None";
	default:
		return "n/a";
	}
}

// function to translate type to string
const char * getStringForType(GLenum type) {
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		return "Error";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return "Deprecated Behaviour";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return "Undefined Behaviour";
	case GL_DEBUG_TYPE_PORTABILITY:
		return "Portability";
	case GL_DEBUG_TYPE_PERFORMANCE:
		return "Performance";
	case GL_DEBUG_TYPE_OTHER:
		return "Other";
	default:
		return "n/a";
	}
}

// actually define the function
void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, GLvoid*) {
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

	// nvidia: avoid debug spam about attribute offsets
	if (id == 131076) return;

	cerr << "GL [" << getStringForSource(source) << "] " << getStringForType(type) << ' ' << id << " : ";
	cerr << message << " (Severity: " << getStringForSeverity(severity) << ')' << endl;

	if (type == GL_DEBUG_TYPE_ERROR_ARB) throw runtime_error("GL Error: "s + message);
}