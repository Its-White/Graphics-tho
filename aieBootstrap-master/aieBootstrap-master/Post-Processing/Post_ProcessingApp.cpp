#include "Post_ProcessingApp.h"
#include "Gizmos.h"
#include "Input.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using glm::vec3;
using glm::vec4;
using glm::mat4;
using aie::Gizmos;

Post_ProcessingApp::Post_ProcessingApp() {

}

Post_ProcessingApp::~Post_ProcessingApp() {

}

bool Post_ProcessingApp::startup() {
	
	setBackgroundColour(0.25f, 0.25f, 0.25f);

	// initialise gizmo primitive counts
	Gizmos::create(10000, 10000, 10000, 10000);

	// create simple camera transforms
	m_viewMatrix = glm::lookAt(vec3(10), vec3(0), vec3(0, 1, 0));
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 1000.0f);

	loadShader();

	return true;
}

void Post_ProcessingApp::shutdown() {

	Gizmos::destroy();
}

void Post_ProcessingApp::update(float deltaTime) {

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

void Post_ProcessingApp::draw() {

	// wipe the screen to the background colour
	clearScreen();

	// update perspective based on screen size
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, getWindowWidth() / (float)getWindowHeight(), 0.1f, 1000.0f);

	Gizmos::draw(m_projectionMatrix * m_viewMatrix);
}

void Post_ProcessingApp::loadShader()
{
	//setup framebuffer
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glGenTextures(1, &m_fboTexture);
	glBindTexture(GL_TEXTURE_2D, m_fboTexture); 

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1280, 720);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_fboTexture, 0); 
	glGenRenderbuffers(1, &m_fboDepth); glBindRenderbuffer(GL_RENDERBUFFER, m_fboDepth);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1280, 720);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_fboDepth);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };

	glDrawBuffers(1, drawBuffers); 
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glm::vec2 halfTexel = 1.0f / glm::vec2(1280, 720) * 0.5f;

	float vertexData[] = { 
		-1, -1, 0, 1, halfTexel.x, halfTexel.y,
		1, 1, 0, 1, 1 - halfTexel.x, 1 - halfTexel.y,
		-1, 1, 0, 1, halfTexel.x, 1 - halfTexel.y,
		-1, -1, 0, 1, halfTexel.x, halfTexel.y,
		1, -1, 0, 1, 1 - halfTexel.x, halfTexel.y,
		1, 1, 0, 1, 1 - halfTexel.x, 1 - halfTexel.y,
	};
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	glGenBuffers(1, &m_vbo);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 6, vertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); 
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0); 

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, ((char*)0) + 16); 

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	static const char* vertex_shader =
		"#version 400"
		"layout (location = 0) in vec4 position;"
		"layout (locatino = 1) in vec2 texCoord;"
		"out vec2 fTexCoord;"
		"void main()"
		"{"
			"gl_Position = position;"
			"fTexCoord = texCoord;"
		"}";

	static const char* fragment_shader =
		"#version 400"
		"in vec2 fTexCoord;"
		"out vec4 fragColor;"
		"uniform sampler2D target;"
		"vec4 Simple()"
		"{"
			"return texture(target, fTexCoord);"
		"}"
		"void main()"
		"{"
			"fragColor = Simple();"
		"}";

	// bind our target
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, 1280, 720);

	// clear the target
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	// draw our 3D scene
	// gizmos for now, but replace with a 3D scene if desired
	Gizmos::draw(m_camera->getProjectionView());

	// bind the back-buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
	glViewport(0, 0, 1280, 720);
	// just clear the back-buffer depth as
	// each pixel will be filled
	glClear(GL_DEPTH_BUFFER_BIT); 
	// draw our full-screen quad

	glUseProgram(m_postShaderProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fboTexture);

	int loc = glGetUniformLocation(m_program, "target");

	glUniform1i(loc, 0); glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

