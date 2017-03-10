#pragma once

#include "Application.h"
#include <glm/mat4x4.hpp>
#include <gl_core_4_4.h>

class Post_ProcessingApp : public aie::Application {
public:

	Post_ProcessingApp();
	virtual ~Post_ProcessingApp();

	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime);
	virtual void draw();

	void loadShader();

protected:

	unsigned int m_fbo;
	unsigned int m_fboTexture;
	unsigned int m_fboDepth;

	unsigned int m_vbo;
	unsigned int m_vao;
	unsigned int m_ibo;

	// camera transforms
	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;
};