#include <GL\glew.h>
#include <GL\glfw.h>
#include <svl\SVL.h>
#include "Shader.h"
#include "ShaderManager.h"
#include <FPSCamera\FPSCamera.h>
#include <FPSCamera\CameraController.h>
#include "FrameBufferObject.h"
#include "QuadDrawer.h"
#include <cstdlib>
#include <math.h>

bool running = true;
bool limitFPS = true;

#define BUFFER_OFFSET(i) ((char*)NULL + i)

void CreateVBOs();
void CreateTexture();

unsigned int vertexBuf;
unsigned int indexBuf;

int width = 800;
int height = 600;

bool keyState[256];
bool lastKeyState[256];

int rootParticleNum = 1024;

FPSCamera* camera;
CameraController* controller;

Shader* particleRenderer;
Shader* copyTex;
ShaderManager* shaderManager;

Mat4 Projection;

int glMajorVersion;
int glMinorVersion;
int glRev;

GLuint texID;

double lastTime;



float RandomFloat()
{
	return (float)rand() / RAND_MAX;
}

FrameBufferObject* FBO;

void CreateFBOs()
{
	FBO = new FrameBufferObject(1024, 1024, 0, 0, GL_RGBA16F, GL_TEXTURE_2D);
	FBO->AttachTexture("first");
	if (!FBO->CheckCompleteness())
		throw;
}

void setup()
{
	srand(42);
	camera = new FPSCamera();
	camera->Position[2] = 5.0f;
	camera->Position[1] = 0.5f;
	camera->Position[0] = 0.5f;
	controller = new CameraController();
	controller->SetCamera(camera);
	controller->MaxSpeed = 0.01;
	controller->PitchAngularVelocity *= 10.0f;
	controller->YawAngularVelocity *= 10.0f;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		printf("%s", glewGetErrorString(err));
		return;	
	}
	particleRenderer = new Shader("Assets/Shaders/particleRender.vert", "Assets/Shaders/particleRender.frag", "particleRenderer");
	copyTex = new Shader("Assets/Shaders/copy.vert", "Assets/Shaders/copy.frag", "Copy");
	ShaderManager::GetSingletonPtr()->CompileShaders();
	CreateFBOs();
	CreateVBOs();
	CreateTexture();	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	memset(keyState, 0, sizeof(bool) * 256);
	memset(lastKeyState, 0, sizeof(bool) * 256);	
}

void CreateTexture()
{
	int numFloats = 3 * rootParticleNum * rootParticleNum;
	float* data = (float*)malloc(sizeof(float) *numFloats);
	for (int i = 0; i < numFloats; ++i)
	{
		data[i] = RandomFloat();
	}
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, rootParticleNum, rootParticleNum, 0, GL_RGB, GL_FLOAT, data);
}

double frameBegin;
int frameCount;
double timeCount;
int currentFps;
int dX, dY;

void update()
{
	++frameCount;
	frameBegin = glfwGetTime();
	double elapsedTime = frameBegin - lastTime;
	timeCount += elapsedTime;
	if (timeCount > 1.0)
	{
		currentFps = frameCount;
		frameCount = 0;
		timeCount = 0.0;
		char buf[100];
		sprintf(buf, "FPS: %d, GL Version %d.%d, rev %d", currentFps, glMajorVersion, glMinorVersion, glRev);
		glfwSetWindowTitle(buf);
	}
	if (keyState['W'])
		controller->MoveForward();
	if (keyState['S'])
		controller->MoveBackward();
	if (keyState['A'])
		controller->MoveLeft();
	if (keyState['D'])
		controller->MoveRight();
	if (keyState['C'])
		controller->MoveDown();
	if (keyState[' '])
		controller->MoveUp();
	controller->Update(elapsedTime * 1000.0f);
	controller->ChangePitch(-elapsedTime * dY);
	controller->ChangeYaw(-elapsedTime * dX);
	dY = 0;
	dX = 0;

}

void display()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindBuffer(GL_VERTEX_ARRAY, vertexBuf);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 8, BUFFER_OFFSET(0));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);	
	particleRenderer->Use();
	glBindTexture(GL_TEXTURE_2D, texID);
	particleRenderer->Uniforms("positionTex").SetValue(0);
	particleRenderer->Uniforms("rows").SetValue(rootParticleNum);
	particleRenderer->Uniforms("View").SetValue(camera->GetViewTransform());
	particleRenderer->Uniforms("Projection").SetValue(camera->GetProjectionMatrix());
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0, rootParticleNum * rootParticleNum);

	FBO->Bind();

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(camera->GetProjectionMatrix().Ref());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(camera->GetViewTransform().Ref());

	glUseProgram(0);

	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 0.0, 0.0);
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 1.0, 0.0);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 1.0);
	glEnd();

	FBO->Unbind();

	glBindTexture(GL_TEXTURE_2D, FBO->GetTexture("first"));

	copyTex->Use();
	copyTex->Uniforms("baseTex").SetValue(0);
	copyTex->Uniforms("pixSize").SetValue(Vec2(1.0 / rootParticleNum, 1.0 / rootParticleNum));

	/*glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(0.0, 0.0);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(0.0, 1.0);
	glTexCoord2f(1.0, 1.0);
	glVertex2f(1.0, 1.0);
	glTexCoord2f(1.0, 0.0);
	glVertex2f(1.0, 0.0);
	glEnd();*/

	//QuadDrawer::DrawQuad(Vec2(1.0, 1.0), Vec2(0.0, 0.0));

	if (limitFPS) 
		glfwSleep(0.016 - glfwGetTime() + frameBegin);
	lastTime = frameBegin;

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

bool mouseEnabled = true;
int lastX;
int lastY;

void KeyboardHandler(int keyCode, int state)
{
	keyState[keyCode] = state;
	if (keyCode == 'R' && state == GLFW_PRESS)
		ShaderManager::GetSingletonPtr()->ReloadShaders();
	if (keyCode == 'M' && state == GLFW_PRESS)
	{
		if (mouseEnabled)		
			glfwDisable(GLFW_MOUSE_CURSOR);					
		else
			glfwEnable(GLFW_MOUSE_CURSOR);
		dX = 0;
		dY = 0;
		glfwGetMousePos(&lastX, &lastY);
		mouseEnabled = !mouseEnabled;
	}
	if (keyCode == 'F' && state == GLFW_PRESS)
		limitFPS = !limitFPS;
	if (keyCode == GLFW_KEY_ESC)
		Exit();
}

void MouseMovementHandler(int x, int y)
{
	if (mouseEnabled)
	{

	}
	else
	{
		dX += x - lastX;
		dY += y - lastY;
		lastX = x;
		lastY = y;
	}
}

int main(int argc, char**argv)
{
	if(!glfwInit())
		return 1;

	lastTime = glfwGetTime();

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 4);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);

	if (!glfwOpenWindow(800, 600, 8, 8, 8, 8, 24, 8, GLFW_WINDOW))
		return 1;

	

	glfwGetGLVersion(&glMajorVersion, &glMinorVersion, &glRev);

	setup();	

	glfwSetWindowCloseCallback(Exit);
	glfwSetKeyCallback(KeyboardHandler);
	glfwSetMousePosCallback(MouseMovementHandler);

	while(running)
	{
		update();
		display();		
	}

	glfwTerminate();
}