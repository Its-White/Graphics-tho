#include "Camera.h"
#include "Input.h"
#include <glm\glm.hpp>
#include <glm\ext.hpp>

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::Update(float deltaTime)
{
	// rotate the camera based on mouse movement
	//------------------------------------------

	aie::Input *input = aie::Input::getInstance(); // this gives us a pointer to the input manager

	//Move the camera using keys
	if (input->isKeyDown(aie::INPUT_KEY_LEFT_SHIFT)) {
		m_moveSpeed = 10.0f;
	}
	else {
		m_moveSpeed = 5.0f;
	}
	if (input->isKeyDown(aie::INPUT_KEY_W)) {
		m_position += m_moveSpeed * deltaTime * m_cameraLook;
	}
	if (input->isKeyDown(aie::INPUT_KEY_S)) {
		m_position -= m_moveSpeed * deltaTime * m_cameraLook;
	}
	
	glm::vec3 strafeVec = glm::normalize(glm::cross(m_cameraLook, m_cameraUp));

	if (input->isKeyDown(aie::INPUT_KEY_A)) {
		m_position -= m_moveSpeed * deltaTime * strafeVec;
	}
	if (input->isKeyDown(aie::INPUT_KEY_D)) {
		m_position += m_moveSpeed * deltaTime * strafeVec;
	}

	if (input->isMouseButtonDown(aie::INPUT_MOUSE_BUTTON_RIGHT)) {
		float rotationAmount = m_mouseSensitivity * deltaTime;
		int mouseX, mouseY;						// mouse X and Y coords from input
		float xOffset, yOffset;					// how far we want to rotate the camera in these axes
		static float xRot;
		input->getMouseXY(&mouseX, &mouseY);	// get mouse location and place it in our variables

		if (input->isKeyUp(aie::INPUT_KEY_LEFT_ALT)) {
			// Find out how far the mouse has moved since last frame
			xOffset = (mouseX - m_lastMouseXPos) * rotationAmount;
			yOffset = (mouseY - m_lastMouseYPos) * rotationAmount;

			// use these values in our Yaw and Pitch values (so we can rotate that far)
			m_yaw += xOffset;
			m_pitch += yOffset;
		}
		else {
			xRot = (mouseX - m_lastMouseXPos) * rotationAmount;
						 
			m_position.x = glm::cos(xRot);
			m_position.z = glm::sin(xRot);
		}
		//save last mouse position values
		m_lastMouseXPos = mouseX;
		m_lastMouseYPos = mouseY;

	}
	else {
		//track the last position of the mouse regardless of whether we're clicking or not
		input->getMouseXY(&m_lastMouseXPos, &m_lastMouseYPos);
	}
		// calculate the new cameraLook Vector
		CalculateLook();
}

void Camera::LookAt(glm::vec3 target)
{
	glm::vec3 look = glm::normalize(target - m_position); //create a vector which is the direction to the target from the position
	m_pitch = glm::degrees(glm::asin(look.y));
	m_yaw = glm::degrees(atan2(look.y, look.x));
	// Roll (you might have to look up the maths if I want to implement this)
	CalculateLook();
}

void Camera::setPosition(glm::vec3 pos)
{
	m_position = pos;
	CalculateView();
}

const glm::mat4 & Camera::GetView()
{
	return m_viewMatrix;
}

glm::vec3 Camera::getCamPos()
{
	return m_position;
}

void Camera::CalculateLook()
{
	// some circle geometry maths to convert the viewing angle from Yaw, Pitch and Roll into a single normalized vector

	glm::vec3 look;
	look.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
	look.y = glm::sin(glm::radians(m_pitch)) * glm::cos(glm::radians(m_roll));
	look.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
	m_cameraLook = glm::normalize(look);

	CalculateView();
}

void Camera::CalculateView()
{
	m_viewMatrix = glm::lookAt(m_position, m_position + m_cameraLook, m_cameraUp);
}
