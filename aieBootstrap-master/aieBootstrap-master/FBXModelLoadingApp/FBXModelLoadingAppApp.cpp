#include "FBXModelLoadingAppApp.h"
#include "Gizmos.h"
#include "Input.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <gl_core_4_4.h>
#include "Camera.h"
#include "FBXFile.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;
using aie::Gizmos;

FBXModelLoadingAppApp::FBXModelLoadingAppApp() {

}

FBXModelLoadingAppApp::~FBXModelLoadingAppApp() {

}

bool FBXModelLoadingAppApp::startup() {
	
	setBackgroundColour(0.25f, 0.25f, 0.25f);

	// initialise gizmo primitive counts
	Gizmos::create(10000, 10000, 10000, 10000);


	// create simple camera transforms
	m_viewMatrix = glm::lookAt(vec3(10), vec3(0), vec3(0, 1, 0));
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 1000.0f);

	m_myFbxModel = new FBXFile();
	m_myFbxModel->load("models/tank/tank.fbx", FBXFile::UNITS_CENTIMETER);
	CreateFBXOpenGLBuffers(m_myFbxModel);

	LoadShaders();

	return true;
}

void FBXModelLoadingAppApp::shutdown() {

	UnloadShaders();

	CleanupFBXOpenGLBuffers(m_myFbxModel);
	m_myFbxModel->unload();
	delete m_myFbxModel;

	Gizmos::destroy();
}

void FBXModelLoadingAppApp::update(float deltaTime) {

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

void FBXModelLoadingAppApp::draw() {

	// wipe the screen to the background colour
	clearScreen();

	// update perspective based on screen size
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, getWindowWidth() / (float)getWindowHeight(), 0.1f, 1000.0f);

	// scale Matrix to reduce the models size
	// some models are very large, some are smaller, so this can be helpful
	float s = 1.0f;
	glm::mat4 model = glm::mat4(
		s, 0, 0, 0,
		0, s, 0, 0,
		0, 0, s, 0,
		0, 0, 0, 1
	);
	glm::mat4 modelViewProjection = m_projectionMatrix * m_viewMatrix * model;

	glUseProgram(m_shader);

	// send uniform variables, in his case the "projectionViewWorldMatrix"
	unsigned int mvpLoc = glGetUniformLocation(m_shader, "projectionViewWorldMatrix");
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &modelViewProjection[0][0]);

	// loop through each mesh within the fbx file
	for (unsigned int i = 0; i < m_myFbxModel->getMeshCount(); i++) 
	{
		FBXMeshNode* mesh = m_myFbxModel->getMeshByIndex(i);
		GLMesh* glData = (GLMesh*)mesh->m_userData;

		// get the texture from the model
		unsigned int diffuseTexture = m_myFbxModel->getTextureByIndex(mesh->m_material->DiffuseTexture);

		// bid the texture and send it to our shader
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseTexture);
		glUniform1i(glGetUniformLocation(m_shader, "diffuseTexture"), 0);

		// draw the mesh
		glBindVertexArray(glData->vao);
		glDrawElements(GL_TRIANGLES, mesh->m_indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	
	glUseProgram(0);

	Gizmos::draw(m_projectionMatrix * m_viewMatrix);
}

void FBXModelLoadingAppApp::CreateFBXOpenGLBuffers(FBXFile * fbx)
{
	// FBX Files contain multiple meshes, each with seperate material information
	// loop through each mesh within the FBX file and cretae VAO, VBO and IBO buffers for each mesh.
	// We can store that information within the mesh object via its "user data" void pointer variable.
	for (unsigned int i = 0; i < fbx->getMeshCount(); i++)
	{
		//Get the current mesh from file
		FBXMeshNode *fbxMesh = fbx->getMeshByIndex(i);

		GLMesh *glData = new GLMesh();

		glGenVertexArrays(1, &glData->vao);
		glBindVertexArray(glData->vao);

		glGenBuffers(1, &glData->vbo);
		glGenBuffers(1, &glData->ibo);

		glBindBuffer(GL_ARRAY_BUFFER, glData->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glData->ibo);

		// fill the vbo with our vertices
		// the FBXLLoader has conveniently already defiend a vertex structure for us.
		glBufferData(GL_ARRAY_BUFFER,
			fbxMesh->m_vertices.size() * sizeof(FBXVertex),
			fbxMesh->m_vertices.data(), GL_STATIC_DRAW);

		// fill the ibo with the indices.
		// fbx meshes can be large, so indices are stored as an unsigned int.
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			fbxMesh->m_indices.size() * sizeof(unsigned int),
			fbxMesh->m_indices.data(), GL_STATIC_DRAW);

		// setup vertex Attrib pointers
		// remember, we only need to setip the appropriate attributes for the shaders that will be rendering.
		// this fbx object.
		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), 0);


		glEnableVertexAttribArray(1); // normal
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char*)0) + FBXVertex::NormalOffset);

		glEnableVertexAttribArray(2); // uv
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char*)0) + FBXVertex::TexCoord1Offset);

		// TODO: add any additional attribute pointers required for shader use.

		// unbind
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//attach our GLMesh object to the m_userData pointer.
		fbxMesh->m_userData = glData;
	}
}

void FBXModelLoadingAppApp::CleanupFBXOpenGLBuffers(FBXFile * fbx)
{
	for (unsigned int i = 0; i < fbx->getMeshCount(); i++)
	{
		FBXMeshNode * fbxMesh = fbx->getMeshByIndex(i);
		GLMesh *glData = (GLMesh *)fbxMesh->m_userData;

		glDeleteVertexArrays(1, &glData->vao);
		glDeleteBuffers(1, &glData->vbo);
		glDeleteBuffers(1, &glData->ibo);

		delete glData;
	}
}

void FBXModelLoadingAppApp::LoadShaders()
{
	const char* vsSource =
	"#version 410\n													\
	in vec4 position; \n											\
	in vec4 normal; \n												\
	in vec2 uv; \n													\
	out vec4 vNormal; \n											\
	out vec2 vuv; \n												\
	uniform mat4 projectionViewWorldMatrix; \n						\
	void main() { \n												\
		vNormal = normal; \n										\
		vuv = uv; \n												\
		gl_Position = projectionViewWorldMatrix*position; \n		\
	}";

	const char* fsSource =
	"#version 410\n \
	in vec4 vNormal; \n\
	in vec2 vuv; \n\
	out vec4 FragColor; \n\
	uniform sampler2D diffuseTexture; \n\
	void main() { \n\
		FragColor = texture(diffuseTexture, vuv) * vec4(1,1,1,1); \n\
	}";

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, (const char**)&vsSource, 0);
	glCompileShader(vertexShader);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fragmentShader, 1, (const char**)&fsSource, 0);
	glCompileShader(fragmentShader);

	m_shader = glCreateProgram();

	glAttachShader(m_shader, vertexShader);
	glAttachShader(m_shader, fragmentShader);

	glBindAttribLocation(m_shader, 0, "position");
	glBindAttribLocation(m_shader, 1, "normal");
	glBindAttribLocation(m_shader, 2, "uv");

	glLinkProgram(m_shader);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void FBXModelLoadingAppApp::UnloadShaders()
{
	glDeleteProgram(m_shader);
}


GLMesh::GLMesh()
{
}

GLMesh::~GLMesh()
{
}


