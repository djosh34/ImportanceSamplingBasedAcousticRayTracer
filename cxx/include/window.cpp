#include "window.h"
#include <iostream>

static void glfwErrorCallback(int error, const char* description)
{
	std::cerr << "GLFW error code: " << error << std::endl;
	std::cerr << description << std::endl;
	exit(1);
}

#ifdef GL_DEBUG_SEVERITY_NOTIFICATION
// OpenGL debug callback
void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
		std::cerr << "OpenGL: " << message << std::endl;
	}
}
#endif

Window::Window(std::string_view title, const glm::ivec2& windowSize, OpenGLVersion glVersion)
	: m_windowSize(windowSize)
	, m_glVersion(glVersion)
{
	glfwSetErrorCallback(glfwErrorCallback);
	if (!glfwInit()) {
		std::cerr << "Could not initialize GLFW" << std::endl;
		exit(1);
	}

	if (glVersion == OpenGLVersion::GL3) {
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}
	else if (glVersion == OpenGLVersion::GL45) {
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}
#ifndef NDEBUG // Automatically defined by CMake when compiling in Release/MinSizeRel mode.
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	// std::string_view does not guarantee that the string contains a terminator character.
	const std::string titleString{ title };
	m_pWindow = glfwCreateWindow(windowSize.x, windowSize.y, titleString.c_str(), nullptr, nullptr);
	if (m_pWindow == nullptr) {
		glfwTerminate();
		std::cerr << "Could not create GLFW window" << std::endl;
		exit(1);
	}
	glfwMakeContextCurrent(m_pWindow);
	glfwSwapInterval(1); // Enable vsync. To disable vsync set this to 0.


#if defined(GL_DEBUG_SEVERITY_NOTIFICATION) && !defined(NDEBUG)
	// Custom debug message with breakpoints at the exact error. Only supported on OpenGL 4.1 and higher.
	if (glVersionMajor > 4 || (glVersionMajor == 4 && glVersionMinor >= 3)) {
		// Set OpenGL debug callback when supported (OpenGL 4.3).
		// NOTE(Mathijs): this is not supported on macOS since Apple can't be bothered to update
		//  their OpenGL version past 4.1 which released 10 years ago!
		glDebugMessageCallback(glDebugCallback, nullptr);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
#endif


	glfwSetWindowUserPointer(m_pWindow, this);

	glfwSetKeyCallback(m_pWindow, keyCallback);
	glfwSetCharCallback(m_pWindow, charCallback);
	glfwSetMouseButtonCallback(m_pWindow, mouseButtonCallback);
	glfwSetCursorPosCallback(m_pWindow, mouseMoveCallback);
	glfwSetScrollCallback(m_pWindow, scrollCallback);
	glfwSetWindowSizeCallback(m_pWindow, windowSizeCallback);
}

Window::~Window()
{

	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}

void Window::close()
{
	glfwSetWindowShouldClose(m_pWindow, 1);
}

bool Window::shouldClose()
{
	return glfwWindowShouldClose(m_pWindow) != 0;
}

void Window::updateInput()
{
	glfwPollEvents();
}

void Window::swapBuffers()
{
	glfwSwapBuffers(m_pWindow);
}

void Window::registerKeyCallback(KeyCallback&& callback)
{
	m_keyCallbacks.push_back(std::move(callback));
}

void Window::registerCharCallback(CharCallback&& callback)
{
	m_charCallbacks.push_back(std::move(callback));
}

void Window::registerMouseButtonCallback(MouseButtonCallback&& callback)
{
	m_mouseButtonCallbacks.push_back(std::move(callback));
}

void Window::registerScrollCallback(ScrollCallback&& callback)
{
	m_scrollCallbacks.push_back(std::move(callback));
}

void Window::registerWindowResizeCallback(WindowResizeCallback&& callback)
{
	m_windowResizeCallbacks.push_back(std::move(callback));
}

void Window::registerMouseMoveCallback(MouseMoveCallback&& callback)
{
	m_mouseMoveCallbacks.push_back(std::move(callback));
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	const Window* pThisWindow = static_cast<const Window*>(glfwGetWindowUserPointer(window));
	for (auto& callback : pThisWindow->m_keyCallbacks)
		callback(key, scancode, action, mods);
}

void Window::charCallback(GLFWwindow* window, unsigned unicodeCodePoint)
{

	const Window* pThisWindow = static_cast<const Window*>(glfwGetWindowUserPointer(window));
	for (auto& callback : pThisWindow->m_charCallbacks)
		callback(unicodeCodePoint);
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	const Window* pThisWindow = static_cast<const Window*>(glfwGetWindowUserPointer(window));
	for (auto& callback : pThisWindow->m_mouseButtonCallbacks)
		callback(button, action, mods);
}

void Window::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{

	const Window* pThisWindow = static_cast<const Window*>(glfwGetWindowUserPointer(window));
	for (auto& callback : pThisWindow->m_mouseMoveCallbacks)
		callback(glm::vec2(xpos, ypos));
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	const Window* pThisWindow = static_cast<const Window*>(glfwGetWindowUserPointer(window));
	for (auto& callback : pThisWindow->m_scrollCallbacks)
		callback(glm::vec2(xoffset, yoffset));
}

void Window::windowSizeCallback(GLFWwindow* window, int width, int height)
{
	Window* pThisWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
	pThisWindow->m_windowSize = glm::ivec2{ width, height };

	for (const auto& callback : pThisWindow->m_windowResizeCallbacks)
		callback(glm::ivec2(width, height));
}

bool Window::isKeyPressed(int key) const
{
	return glfwGetKey(m_pWindow, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const
{
	return glfwGetMouseButton(m_pWindow, button) == GLFW_PRESS;
}

glm::vec2 Window::getCursorPos() const
{
	double x, y;
	glfwGetCursorPos(m_pWindow, &x, &y);
	return glm::vec2(x, y);
}

glm::vec2 Window::getNormalizedCursorPos() const
{
	return getCursorPos() / glm::vec2(m_windowSize);
}

glm::vec2 Window::getCursorPixel() const
{
	// https://stackoverflow.com/questions/45796287/screen-coordinates-to-world-coordinates
	// Coordinates returned by glfwGetCursorPos are in screen coordinates which may not map 1:1 to
	// pixel coordinates on some machines (e.g. with resolution scaling).
	glm::ivec2 screenSize;
	glfwGetWindowSize(m_pWindow, &screenSize.x, &screenSize.y);
	glm::ivec2 framebufferSize;
	glfwGetFramebufferSize(m_pWindow, &framebufferSize.x, &framebufferSize.y);

	double xpos, ypos;
	glfwGetCursorPos(m_pWindow, &xpos, &ypos);
	const glm::vec2 screenPos{ xpos, ypos };
	glm::vec2 pixelPos = screenPos * glm::vec2(framebufferSize) / glm::vec2(screenSize); // float division
	pixelPos += glm::vec2(0.5f); // Shift to GL center convention.
	return glm::vec2(pixelPos.x, static_cast<float>(framebufferSize.y) - pixelPos.y - 1);
}

void Window::setMouseCapture(bool capture)
{
	if (capture) {
		glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else {
		glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	glfwPollEvents();
}

glm::ivec2 Window::windowSize() const
{
	return m_windowSize;
}

float Window::aspectRatio() const
{
	return float(m_windowSize.x) / float(m_windowSize.y);
}