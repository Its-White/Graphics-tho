#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class Camera 
{
public:
	Camera();
	~Camera();

	void Update(float deltaTime);

	void LookAt(glm::vec3 target);
	void setPosition(glm::vec3 pos);

	const glm::mat4 & GetView();

	glm::vec3 getCamPos();

private:
	glm::mat4 m_viewMatrix; //the matrix we'll passs to openGL so i knows how the camera looks at the world

	glm::vec3 m_cameraLook; //the actual direction the camera is aiming
	glm::vec3 m_cameraUp = glm::vec3(0.0f,1.0f,0.0f); //setting camera orientation so that the y axis is always up

	glm::vec3 m_position;
	float m_yaw = 0.0f;		//left/right rotation of view
	float m_pitch = 0.0f;	//up/down rotation of view
	float m_roll = 0.0f;	//rotating the orientation of the view

	const float m_mouseSensitivity = 10.0f;
	
	int m_lastMouseXPos = 0;	//recording previous mouse position so we can see how far it has moved
	int m_lastMouseYPos = 0;

	float m_moveSpeed = 5.0f;

	void CalculateLook(); //gives us the vector of direction using yaw, pitch and roll
	void CalculateView(); //sets up the view matrix based on our camera information
};