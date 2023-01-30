#include "trackball.h"
#include "disable_all_warnings.h"
#include "window.h"
// Suppress warnings in third-party code.
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/quaternion.hpp>
DISABLE_WARNINGS_POP()
#include <algorithm>
#include <iostream>
#include <limits>
#include <helpers/printHelper.h>
#include <settings.h>

static constexpr float rotationSpeedFactor = 0.3f;
static constexpr float translationSpeedFactor = 0.1f;
static constexpr float zoomSpeedFactor = 0.5f;

// NOTE(Mathijs): field-of-view in radians.
Trackball::Trackball(Window* pWindow, float fovy, float distFromLookAt, float rotationX, float rotationY)
	: Trackball(pWindow, fovy, glm::vec3(0.0f), distFromLookAt, rotationX, rotationY)
{
}



Trackball::Trackball(Window* pWindow, float fovy, const glm::vec3& lookAt, float distFromLookAt, float rotationX, float rotationY)
	: m_pWindow(pWindow)
	, m_fovy(fovy)
	, m_lookAt(lookAt)
	, m_distanceFromLookAt(distFromLookAt)
	, m_rotationEulerAngles(rotationX, rotationY, 0)
{
	m_rotationEulerAngles.z = 0;

    pWindow->registerKeyCallback(
        [this](int key, int scancode, int action, int mods) {
            keyboardCallback(key, scancode, action, mods);
        });
	pWindow->registerMouseButtonCallback(
		[this](int key, int action, int mods) {
			mouseButtonCallback(key, action, mods);
		});
	pWindow->registerMouseMoveCallback(
		[this](const glm::vec2& pos) {
			mouseMoveCallback(pos);
		});
	pWindow->registerScrollCallback(
		[this](const glm::vec2& offset) {
			mouseScrollCallback(offset);
		});

    SET_CAMERA
}

void Trackball::printHelp()
{
	std::cout << "Left button: turn in XY," << std::endl;
	std::cout << "Right button: translate in XY," << std::endl;
	std::cout << "Middle button: move along Z." << std::endl;
}

void Trackball::disableTranslation()
{
	m_canTranslate = false;
}

void Trackball::setCamera(const glm::vec3 lookAt, const glm::vec3 rotations, const float dist)
{
	m_lookAt = lookAt;
	m_rotationEulerAngles = rotations;
	m_distanceFromLookAt = dist;
}

glm::vec3 Trackball::position() const
{
	return m_lookAt + glm::quat(m_rotationEulerAngles) * glm::vec3(0, 0, -m_distanceFromLookAt);
}

glm::vec3 Trackball::lookAt() const
{
	return m_lookAt;
}

glm::mat4 Trackball::viewMatrix() const
{
	return glm::lookAt(position(), m_lookAt, up());
}

glm::mat4 Trackball::projectionMatrix() const
{
	return glm::perspective(m_fovy, m_pWindow->aspectRatio(), 1.0f, 100.0f);
}


glm::vec3 Trackball::forward() const
{
	return glm::quat(m_rotationEulerAngles) * glm::vec3(0, 0, 1);
}

glm::vec3 Trackball::up() const
{
	return glm::quat(m_rotationEulerAngles) * glm::vec3(0, 1, 0);
}

glm::vec3 Trackball::left() const
{
	return glm::quat(m_rotationEulerAngles) * glm::vec3(-1, 0, 0);
}

void Trackball::mouseButtonCallback(int button, int action, int /* mods */)
{

	if ((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_PRESS) {
		m_prevCursorPos = m_pWindow->getCursorPos();
	}
}

void Trackball::keyboardCallback(int key, int scancode, int action, int mods) {
    if (!action) {
        return;
    }

    switch (key) {
        case GLFW_KEY_W:
            moveCamera(FORWARD);
            break;
        case GLFW_KEY_S:
            moveCamera(BACKWARD);
            break;
        case GLFW_KEY_D:
            moveCamera(MOVE_RIGHT);
            break;
        case GLFW_KEY_A:
            moveCamera(MOVE_LEFT);
            break;
        case GLFW_KEY_I:
            printInfo();
            break;
        case GLFW_KEY_C:
        case GLFW_KEY_F:
            keypress.pressed = true;
            keypress.key = key;
            break;
        case GLFW_KEY_T:
            animate = !animate;
            std::cout << "Animation is turned " << (animate ? "On" : "Off") << " now" << std::endl;

            if (!animate) {
                current_t = 0.0;
            }

            break;
        case GLFW_KEY_COMMA:
            current_t /= 1 + ANIMATION_T_STEP;
            if (current_t < 0) {
                current_t = 0.0f;
            }
            std::cout << "Animation T: " << current_t << "  " << std::endl;
            break;
        case GLFW_KEY_PERIOD:
            current_t *= 1 + ANIMATION_T_STEP;
            if (current_t == 0.0) {
                current_t = ANIMATION_T_STEP;
            }
            std::cout << "Animation T: " << current_t << "  " << std::endl;
            break;
    }


}

void Trackball::mouseMoveCallback(const glm::vec2& pos)
{
	const bool rotateXY = m_pWindow->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
	const bool translateXY = m_canTranslate && m_pWindow->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);

	if (rotateXY || translateXY) {
		// Amount of cursor motion compared to the previous frame.
		glm::vec2 delta = pos - m_prevCursorPos;
		delta.y = -delta.y; // Vertical axis direction is inverted.

		if (rotateXY) {

            rotateCamera(delta);


		} else {
			// Translate the camera in the image plane.
			m_lookAt -= delta.x * translationSpeedFactor * left();
			m_lookAt -= delta.y * translationSpeedFactor * up();
		}
		m_prevCursorPos = pos;
	}
}

void Trackball::mouseScrollCallback(const glm::vec2& offset)
{
	// Move the camera closer/further from the look at point when the user scrolls on his/her mousewheel.
	m_distanceFromLookAt += -float(offset.y) * zoomSpeedFactor;
	m_distanceFromLookAt = std::clamp(m_distanceFromLookAt, 0.1f, 100.0f);
}

void Trackball::moveCamera(Movement movement) {

    switch (movement) {
        case FORWARD:
            m_lookAt += translationSpeedFactor * (glm::quat(m_rotationEulerAngles) * glm::vec3(0, 0, m_distanceFromLookAt));
            break;
        case BACKWARD:
            m_lookAt += translationSpeedFactor * (glm::quat(m_rotationEulerAngles) * glm::vec3(0, 0, -m_distanceFromLookAt));
            break;
        case MOVE_LEFT:
            m_lookAt += translationSpeedFactor * (glm::quat(m_rotationEulerAngles) * glm::vec3(m_distanceFromLookAt, 0, 0));
            break;
        case MOVE_RIGHT:
            m_lookAt += translationSpeedFactor * (glm::quat(m_rotationEulerAngles) * glm::vec3(-m_distanceFromLookAt, 0, 0));
            break;
    }

}

void Trackball::rotateCamera(glm::vec2 delta) {
    auto prevCameraPos = position();

    // Rotate the camera around the lookat point.
    m_rotationEulerAngles.x = std::clamp(m_rotationEulerAngles.x - glm::radians(delta.y * rotationSpeedFactor), -glm::half_pi<float>(), +glm::half_pi<float>());
    m_rotationEulerAngles.y -= glm::radians(delta.x * rotationSpeedFactor);

    auto lookVector = glm::quat(m_rotationEulerAngles) * glm::vec3(0, 0, -m_distanceFromLookAt);
    m_lookAt = prevCameraPos + -1.0f * lookVector;

}

void Trackball::printInfo() {
    std::cout << "Current LookAt: " << m_lookAt << std::endl;
    std::cout << "Current Rotations: " << m_rotationEulerAngles << std::endl;
    std::cout << "Current Distance: " << m_distanceFromLookAt << std::endl;


}

bool Trackball::isAnimate() {
    return animate;
}


