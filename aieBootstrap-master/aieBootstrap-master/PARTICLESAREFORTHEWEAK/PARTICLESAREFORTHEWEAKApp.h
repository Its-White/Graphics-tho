#pragma once

#include "Application.h"
#include <glm/mat4x4.hpp>
#include "Camera.h"


class PARTICLESAREFORTHEWEAKApp : public aie::Application {
public:

	PARTICLESAREFORTHEWEAKApp();
	virtual ~PARTICLESAREFORTHEWEAKApp();

	virtual bool startup();
	virtual void shutdown();

	void init(unsigned int a_maxParticles, unsigned int a_emitRate, float a_lifetimeMin, float a_lifetimeMax, float a_velocityMin, float a_velocityMax, float a_startSize, float a_endSize, const vec4& a_startColour, const vec4& a_endcolour);
	void emit();
	void loadShader();

	virtual void update(float deltaTime, const mat4& a_cameraTransform);
	virtual void draw();

protected:

	struct Particle
	{
		vec3 position;
		vec3 velocity;
		vec4 color;
		float size;
		float lifetime;
		float lifespan;
	};

	struct ParticleVertex
	{
		vec4 position;
		vec4 color;
	};

	Particle*		m_particles;
	unsigned int					m_firstDead;
	unsigned int					m_maxParticles;

	unsigned int					m_vao, m_vbo, m_ibo;
	ParticleVertex*					m_vertexData;

	vec3							m_position;

	float							m_emitTimer;
	float							m_emitRate;

	float							m_lifespanMin;
	float							m_lifespanMax;

	float							m_velocityMin;
	float							m_velocityMax;

	float							m_startSize;
	float							m_endSize;

	vec4							m_startColor;
	vec4							m_endColor;

	// camera transforms
	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;

	Camera *m_camera;
};