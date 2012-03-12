#include <GL\glew.h>
#include <GL\glfw.h>
#include <svl\SVL.h>
#include "Shader.h"
#include "ShaderManager.h"
#include <FPSCamera\FPSCamera.h>
#include <FPSCamera\CameraController.h>
#include <cstdlib>

bool running = true;

#define BUFFER_OFFSET(i) ((char*)NULL + i)

void CreateVBOs();
void CreateTexture();

unsigned int vertexBuf;
unsigned int indexBuf;

int width = 800;
int height = 600;

bool keyState[256];
bool lastKeyState[256];

FPSCamera* camera;
CameraController* controller;

Shader* particleRenderer;
ShaderManager* shaderManager;

float RandomFloat()
{
	return (float)rand() / RAND_MAX;
}

void setup()
{
	srand(42);
	camera = new FPSCamera();
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		printf("%s", glewGetErrorString(err));
		return;	
	}
	particleRenderer = new Shader("Assets/Shaders/particleRender.vert", "Assets/Shaders/particleRender.frag", "particleRenderer");
	ShaderManager::GetSingletonPtr()->CompileShaders();
	CreateVBOs();
	CreateTexture();
	memset(keyState, 0, sizeof(bool) * 256);
	memset(lastKeyState, 0, sizeof(bool) * 256);
}

GLuint texID;

void CreateTexture()
{
	float data[16];
	for (int i = 0; i < 16; ++i)
	{
		data[i] = (RandomFloat() * 2.0) - 1.0f;
	}
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 2, 2, 0, GL_RGBA32F, GL_FLOAT, data);
}

void update()
{

}

void display()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	particleRenderer->Uniforms("World").SetValue(Mat4(vl_one));
	particleRenderer->Uniforms("View").SetValue(camera->GetViewTransform());
	glBindBuffer(GL_VERTEX_ARRAY, vertexBuf);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 8, BUFFER_OFFSET(0));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
	particleRenderer->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	particleRenderer->Uniforms("positionTex").SetValue(0);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0, 4);
	glfwSwapBuffers();
}

unsigned short indices[] = {0, 1, 2, 0, 2, 3};

void CreateVBOs()
{
	GLuint id;
	float vertices[] = {0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0}; 
	GLsizeiptr vSize = sizeof(float) * 8;
	glGenBuffers(1, &vertexBuf);
	glGenBuffers(1, &indexBuf);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
	glBufferData(GL_ARRAY_BUFFER, vSize, (void*)vertices, GL_STATIC_DRAW);	

	vSize = sizeof(unsigned short) * 6;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vSize, indices, GL_STATIC_DRAW);
}

int Exit()
{
	running = false;
	return 0;
}

void KeyboardHandler(int keyCode, int state)
{
	keyState[keyCode] = state;
	if (keyCode == 'R' && state == GLFW_PRESS)
		ShaderManager::GetSingletonPtr()->ReloadShaders();
	if (keyCode == GLFW_KEY_ESC)
		Exit();
}

int main(int argc, char**argv)
{
	if(!glfwInit())
		return 1;

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 0);

	if (!glfwOpenWindow(800, 600, 8, 8, 8, 8, 24, 8, GLFW_WINDOW))
		return 1;

	setup();	

	glfwSetWindowCloseCallback(Exit);
	glfwSetKeyCallback(KeyboardHandler);

	while(running)
	{
		update();
		display();		
	}

	glfwTerminate();
}