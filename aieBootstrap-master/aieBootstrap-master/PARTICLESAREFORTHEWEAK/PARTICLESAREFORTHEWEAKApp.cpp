#include "PARTICLESAREFORTHEWEAKApp.h"
#include "Gizmos.h"
#include "Input.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <gl_core_4_4.h>

using glm::vec3;
using glm::vec4;
using glm::mat4;
using aie::Gizmos;

PARTICLESAREFORTHEWEAKApp::PARTICLESAREFORTHEWEAKApp()
	:m_particles(nullptr),
	m_firstDead(0),
	m_maxParticles(0),
	m_position(0, 0, 0),
	m_vao(0), m_vbo(0), m_ibo(0),
	m_vertexData(nullptr) {

}

PARTICLESAREFORTHEWEAKApp::~PARTICLESAREFORTHEWEAKApp() {
	delete[] m_particles;
	delete[] m_vertexData;

	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ibo);
}

bool PARTICLESAREFORTHEWEAKApp::startup() {
	
	setBackgroundColour(0.25f, 0.25f, 0.25f);

	// initialise gizmo primitive counts
	Gizmos::create(10000, 10000, 10000, 10000);

	// create simple camera transforms
	m_viewMatrix = glm::lookAt(vec3(10), vec3(0), vec3(0, 1, 0));
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 1000.0f);

	return true;
}

void PARTICLESAREFORTHEWEAKApp::shutdown() {

	Gizmos::destroy();
}

void PARTICLESAREFORTHEWEAKApp::init(unsigned int a_maxParticles, unsigned int a_emitRate, float a_lifetimeMin, float a_lifetimeMax, float a_velocityMin, float a_velocityMax, float a_startSize, float a_endSize, const vec4 & a_startColour, const vec4 & a_endcolour)
{
	//setup emit timers
	m_emitTimer = 0;
	m_emitRate = 1.0f / a_emitRate;

	//store all variables passed in
	m_startColor = a_startColour;
	m_endColor = a_endcolour;
	m_startSize = a_startSize;
	m_endSize = a_endSize;
	m_velocityMin = a_velocityMin;
	m_velocityMax = a_velocityMax;
	m_lifespanMin = a_lifetimeMin;
	m_lifespanMax = a_lifetimeMax;
	m_maxParticles = a_maxParticles;

	//create particle array
	m_particles = new Particle[m_maxParticles];
	m_firstDead = 0;

	//create the array of vertices for the particles
	//4 vertices per particle for a quad
	//will be filled during update
	m_vertexData = new ParticleVertex[m_maxParticles * 4];

	//create the index buffer data for the particles, 6 indices per quad of 2 triangles, fill it now as it never changes
	unsigned int* indexData = new unsigned int[m_maxParticles * 6];
	for (unsigned int i = 0; i < m_maxParticles; i++)
	{
		indexData[i * 6 + 0] = i * 4 + 0;
		indexData[i * 6 + 1] = i * 4 + 1;
		indexData[i * 6 + 2] = i * 4 + 2;

		indexData[i * 6 + 3] = i * 4 + 0;
		indexData[i * 6 + 4] = i * 4 + 2;
		indexData[i * 6 + 5] = i * 4 + 3;

	}

	//create OpenGL Buffers
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ibo);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_maxParticles * 4 * sizeof(ParticleVertex), m_vertexData, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_maxParticles * 6 * sizeof(unsigned int), indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);//position
	glEnableVertexAttribArray(1);//color
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), ((char*)0) + 16);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete[] indexData;
}

void PARTICLESAREFORTHEWEAKApp::emit()
{
	//only emit if there is a dead particle to use
	if (m_firstDead >= m_maxParticles) {
		return;
	}

	//ressurect the first dead particle
	Particle& particle = m_particles[m_firstDead++];

	//assign its starting position
	particle.position = m_position;

	//randomise its lifespan
	particle.lifetime = 0;
	particle.lifespan = (rand() / (float)RAND_MAX) * (m_lifespanMax - m_lifespanMin) + m_lifespanMin;

	//set the starting size and color
	particle.color = m_startColor;
	particle.size = m_startSize;

	//randomise velocity direction and strength
	float velocity = (rand() / (float)RAND_MAX) * (m_velocityMax - m_velocityMin) + m_velocityMin;
	particle.velocity.x = (rand() / (float)RAND_MAX) * 2 - 1;
	particle.velocity.y = (rand() / (float)RAND_MAX) * 2 - 1;
	particle.velocity.z = (rand() / (float)RAND_MAX) * 2 - 1;
	particle.velocity = glm::normalize(particle.velocity) * velocity;
}

void PARTICLESAREFORTHEWEAKApp::loadShader()
{
	const char* vShader =
		"#version 400\n"
		"in vec4 Position;"
		"in vec4 Colour;"
		"out vec4 colour;"
		"uniform mat4 projectionView;"
		"void main()"
		"{"
		"colour = Colour;"
		"gl_Position = projectionView * Position;"
		"}";

	const char* fShader =
		"#version 400"
		"in vec4 colour;"
		"out vec4 fragColour;"
		"void main()"
		"{"
		"fragColour = colour;"
		"}";
}

void PARTICLESAREFORTHEWEAKApp::update(float deltaTime, const mat4& a_cameraTransform) {

	m_emitTimer += deltaTime;
	while (m_emitTimer > m_emitRate) {
		emit();
		m_emitTimer -= m_emitRate;
	}

	unsigned int quad = 0;

	for (unsigned int i = 0; i < m_firstDead; i++)
	{
		Particle* particle = &m_particles[i];

		particle->lifetime += deltaTime;
		if (particle->lifetime >= particle->lifespan) {
			//swap last alive with this one
			*particle = m_particles[m_firstDead - 1];
			m_firstDead--;
		}
		else{
			//move particle
			particle->position += particle->velocity * deltaTime;

			//size particle
			particle->size = glm::mix(m_startSize, m_endSize, particle->lifetime / particle->lifespan);

			//color particle
			particle->color = glm::mix(m_startColor, m_endColor, particle->lifetime / particle->lifespan);

			//make a quad the correct size and color
			float halfSize = particle->size * .5f;

			m_vertexData[quad * 4 + 0].position = vec4(halfSize, halfSize, 0, 1);
			m_vertexData[quad * 4 + 0].color = particle->color;
			m_vertexData[quad * 4 + 1].position = vec4(-halfSize, halfSize, 0, 1);
			m_vertexData[quad * 4 + 0].color = particle->color;
			m_vertexData[quad * 4 + 2].position = vec4(-halfSize, -halfSize, 0, 1);
			m_vertexData[quad * 4 + 0].color = particle->color;
			m_vertexData[quad * 4 + 3].position = vec4(halfSize, -halfSize, 0, 1);
			m_vertexData[quad * 4 + 3].color = particle->color;

			//create billboard transform
			vec3 zAxis = glm::normalize(vec3(a_cameraTransform[3]) - particle->position);
			vec3 xAxis = glm::cross(vec3(a_cameraTransform[1]), zAxis);
			vec3 yAxis = glm::cross(zAxis,xAxis);

			glm::mat4 billboard(
				vec4(xAxis, 0),
				vec4(yAxis, 0),
				vec4(zAxis, 0),
				vec4(0, 0, 0, 1)
			);

			m_vertexData[quad * 4 + 0].position = billboard * m_vertexData[quad * 4 + 0].position + vec4(particle->position, 0);
			m_vertexData[quad * 4 + 1].position = billboard * m_vertexData[quad * 4 + 1].position + vec4(particle->position, 0);
			m_vertexData[quad * 4 + 2].position = billboard * m_vertexData[quad * 4 + 2].position + vec4(particle->position, 0);
			m_vertexData[quad * 4 + 3].position = billboard * m_vertexData[quad * 4 + 3].position + vec4(particle->position, 0);

			++quad;
		}
	}

	// wipe the gizmos clean for this frame
	Gizmos::clear();

	// draw a simple grid with gizmos
	vec4 white(1);
	vec4 black(0, 0, 0, 1);
	for (int i = 0; i < 21; ++i) {
		Gizmos::addLine(vec3(-10 + i, 0, 10),
						vec3(-10 + i, 0, -10),
						i == 10 ? white : black);
		Gizmos::addLine(vec3(10, 0, -10 + i),
						vec3(-10, 0, -10 + i),
						i == 10 ? white : black);
	}

	// add a transform so that we can see the axis
	Gizmos::addTransform(mat4(1));

	// quit if we press escape
	aie::Input* input = aie::Input::getInstance();

	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();
}

void PARTICLESAREFORTHEWEAKApp::draw() {

	//sync the particle vertex buffer
	//based on how many alive particles there are
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_firstDead * 4 * sizeof(ParticleVertex), m_vertexData);

	//draw paritcles
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_firstDead * 6, GL_UNSIGNED_INT, 0);

	// wipe the screen to the background colour
	clearScreen();

	// update perspective based on screen size
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, getWindowWidth() / (float)getWindowHeight(), 0.1f, 1000.0f);

	Gizmos::draw(m_projectionMatrix * m_viewMatrix);
}