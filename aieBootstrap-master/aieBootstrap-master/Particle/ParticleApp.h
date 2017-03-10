#pragma once

#include "Application.h"
#include <glm/mat4x4.hpp>
#include <gl_core_4_4.h>

class ParticleApp : public aie::Application {
public:

	ParticleApp();
	virtual ~ParticleApp();

	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime, const mat4& cameraTransform);
	virtual void draw();

	virtual void loadShader();

	void initialise(unsigned int maxParticles, unsigned int emitRate, float lifetimeMin, float lifetimeMax, float velocityMin, float velocityMax, float startSize, float endSize, const vec4& startColor, const vec4& endColor);
	void emit();
protected:

	struct Particle {
		vec3 position;
		vec3 velocity;
		vec4 colour;
		float size;
		float lifespan;
		float lifetime;
	};


	struct ParticleVertex {
		vec4 position;
		vec4 color;
	};

	Particle* m_particles;
	unsigned int m_firstDead;
	unsigned int m_maxParticles;

	unsigned int m_vao, m_vbo, m_ibo;
	ParticleVertex* m_vertexData;

	vec3 m_position;

	unsigned int m_emitter;

	float m_emitTimer;
	float m_emitRate;

	float m_lifespanMin;
	float m_lifespanMax;

	float m_velocityMin;
	float m_velocityMax;

	float m_startSize;
	float m_endSize;

	vec4 m_startColor;
	vec4 m_endColor;

	// camera transforms
	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;
};