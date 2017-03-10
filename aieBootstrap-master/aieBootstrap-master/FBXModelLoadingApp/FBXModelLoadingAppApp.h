#pragma once

#include "Application.h"
#include <glm/mat4x4.hpp>
//#include "FBXFile.h"

class FBXFile;
class GLMesh {
public:
	GLMesh();
	~GLMesh();

	unsigned int vao;
	unsigned int vbo;
	unsigned int ibo;
};

class FBXModelLoadingAppApp : public aie::Application {
public:

	FBXModelLoadingAppApp();
	virtual ~FBXModelLoadingAppApp();

	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime);
	virtual void draw();

	void CreateFBXOpenGLBuffers(FBXFile *fbx);
	void CleanupFBXOpenGLBuffers(FBXFile *fbx);

	void LoadShaders();
	void UnloadShaders();

	unsigned int m_shader;

	FBXFile *m_myFbxModel;

protected:
	// camera transforms
	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;
};
