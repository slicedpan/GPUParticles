#include <GL\glew.h>
#include <GL\glfw.h>
#include <svl\SVL.h>
#include "Shader.h"
#include "ShaderManager.h"
#include "BasicTexture.h"
#include <FPSCamera\FPSCamera.h>
#include <FPSCamera\CameraController.h>
#include "FrameBufferObject.h"
#include "QuadDrawer.h"
#include "stb_image.h"
#include <cstdlib>
#include <math.h>

bool running = true;
bool limitFPS = true;
bool simulate = false;
bool drawFBOS = false;

double avgSleepTime;

#define BUFFER_OFFSET(i) ((char*)NULL + i)

void CreateVAO();
void CreateTextures();
void LoadTexture();
void InjectParticles();
void SetStaticUniforms();

unsigned int vertexBuf;
unsigned int indexBuf;
unsigned int vaoID;

int width = 800;
int height = 600;

unsigned int lastParticle = 0;
unsigned int particlesPerFrame = 200;

double elapsedTime = 0.0;

bool keyState[256];
bool lastKeyState[256];

int rootParticleNum = 16;

FPSCamera* camera;
CameraController* controller;

Shader* particleRenderer;
Shader* copyTex;
Shader* verlet;
Shader* addTex;
Shader* initialiseParticles;
Shader* injectParticle;

ShaderManager* shaderManager;

Mat4 Projection;

int glMajorVersion;
int glMinorVersion;
int glRev;

GLuint texID;
GLuint colorTex;
GLuint velTex;
GLuint maskTex;
BasicTexture* noiseTex;

float particleQuadSize = 0.05;

double lastTime;

Vec2 bufPixSize(1.0f / rootParticleNum, 1.0f / rootParticleNum);

float RandomFloat()
{
	return (float)rand() / RAND_MAX;
}

Vec3 randomVec(float length)
{
	Vec3 random;
	random[0] = RandomFloat() - 0.5f;
	random[1] = RandomFloat() - 0.5f;
	random[2] = RandomFloat() - 0.5f;
	random.Normalise();
	return random * length;
}

Vec3 color1(0.0f, 0.0f, 1.0f);
Vec3 color2(0.1f, 0.8f, 0.8f);

FrameBufferObject* fbos[3];

unsigned int currentBuf = 0;
unsigned int nextBuf = 1;

void Increment()
{
	++currentBuf;
	currentBuf %= 2;
	++nextBuf;
	nextBuf %= 2;
}

void CreateFBOs()
{
	for (int i = 0; i < 2; ++i)
	{
		fbos[i] = new FrameBufferObject(rootParticleNum, rootParticleNum, 0, 0, GL_RGBA32F, GL_TEXTURE_2D);
		fbos[i]->AttachTexture("position", GL_NEAREST, GL_NEAREST);
		fbos[i]->AttachTexture("last", GL_NEAREST, GL_NEAREST);
		if (!fbos[i]->CheckCompleteness())
			throw;
	}
}
	
float top = 1.0f;
float right = 1.0f;

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
	verlet = new Shader("Assets/Shaders/copy.vert", "Assets/Shaders/verlet.frag", "Verlet Integrator");
	addTex = new Shader("Assets/Shaders/copy.vert", "Assets/Shaders/add.frag", "Add textures");
	initialiseParticles = new Shader("Assets/Shaders/copy.vert", "Assets/Shaders/copyToFbo.frag", "Particle Initialisation");
	injectParticle = new Shader("Assets/Shaders/inject.vert", "Assets/Shaders/inject.frag", "Particle Inject");
	noiseTex = new BasicTexture("Assets/Textures/rgbnoise.png");
	noiseTex->Load();
	ShaderManager::GetSingletonPtr()->CompileShaders();
	CreateFBOs();
	CreateVAO();
	CreateTextures();	
	LoadTexture();
	SetStaticUniforms();
	
	memset(keyState, 0, sizeof(bool) * 256);
	memset(lastKeyState, 0, sizeof(bool) * 256);	
}

void Simulate()
{
	fbos[nextBuf]->Bind();

	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbos[currentBuf]->GetTexture(0));

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fbos[currentBuf]->GetTexture(1));

	verlet->Use();
	verlet->Uniforms("timeElapsed").SetValue((float)elapsedTime);
	verlet->Uniforms("timeElapsedSquared").SetValue((float)(elapsedTime * elapsedTime));

	QuadDrawer::DrawQuad(Vec2(-1.0, -1.0), Vec2(1.0, 1.0), bufPixSize);

	fbos[nextBuf]->Unbind();

	Increment();
}

void CopyTexToFBO()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, velTex);
	
	initialiseParticles->Use();
	initialiseParticles->Uniforms("baseTex").SetValue(0);
	initialiseParticles->Uniforms("velocityTex").SetValue(1);	

	fbos[currentBuf]->Bind();
	QuadDrawer::DrawQuad(Vec2(-1.0, -1.0), Vec2(1.0, 1.0), bufPixSize);
	fbos[currentBuf]->Unbind();
	
}

void CreateTextures()
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

	for (int i = 0; i < numFloats; ++i)
	{
		data[i] = (RandomFloat() - 0.5f) / 10.0f;
	}
	glGenTextures(1, &velTex);
	glBindTexture(GL_TEXTURE_2D, velTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, rootParticleNum, rootParticleNum, 0, GL_RGB, GL_FLOAT, data);

	for (int i = 0; i < rootParticleNum * rootParticleNum; ++i)
	{
		float f  = RandomFloat();
		Vec3 color = f * color1 + (1 - f) * color2;
		memcpy(data + (i * 3), color.Ref(), sizeof(float) * 3);
	}
	glGenTextures(1, & colorTex);
	glBindTexture(GL_TEXTURE_2D, colorTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, rootParticleNum, rootParticleNum, 0, GL_RGB, GL_FLOAT, data);

	free(data);
}

void LoadTexture()
{
	int imgWidth, imgHeight, imgComps;
	unsigned char* data = stbi_load("Assets/Textures/particlemask.png", &imgWidth, &imgHeight, &imgComps, 4);
	if (!data)
	{
		printf("Could not load mask image");
	}
	unsigned char* newData = (unsigned char*)malloc(sizeof(unsigned char) * imgWidth * imgHeight);
	for (int i = 0; i < imgWidth; ++i)
	{
		for (int j = 0; j < imgHeight; ++j)
		{
			unsigned char* datPtr = data + (i + (j * imgWidth)) * 4;
			unsigned char* newDatPtr = newData + (i + (j * imgWidth));
			*newDatPtr = datPtr[0] + datPtr[1] + datPtr[2];
			*newDatPtr /= 3;
		}
	}
	printf("\n");
	for (int i = 0; i < imgWidth; ++i)
	{

		for (int j = 0; j < imgHeight; ++j)
		{
			printf("%d ", newData[j * imgWidth + i]);
		}
		printf("\n");
	}
	glGenTextures(1, &maskTex);
	glBindTexture(GL_TEXTURE_2D, maskTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGetError();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R, imgWidth, imgHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, newData);
	GLenum error = glGetError();
	printf("Error: %s", glewGetErrorString(error));
	memset(newData, 0, sizeof(unsigned char) * imgWidth * imgHeight);
	float* fData = (float*)malloc(sizeof(float) * imgWidth * imgHeight);
	for (int i = 0; i < imgWidth; ++i)
	{

		for (int j = 0; j < imgHeight; ++j)
		{
			fData[j * imgWidth + i] = 0.0f;
		}
		printf("\n");
	}
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, fData);
	printf("\n");
	for (int i = 0; i < imgWidth; ++i)
	{

		for (int j = 0; j < imgHeight; ++j)
		{
			printf("%f ", fData[j * imgWidth + i]);
		}
		printf("\n");
	}
	stbi_image_free(data);
	free(newData);
}

void SetStaticUniforms()
{
	injectParticle->Use();
	injectParticle->Uniforms("texSide").SetValue(rootParticleNum);
	injectParticle->Uniforms("noiseSize").SetValue(256);
	injectParticle->Uniforms("noiseTex").SetValue(0);

	particleRenderer->Use();
	particleRenderer->Uniforms("positionTex").SetValue(0);
	particleRenderer->Uniforms("maskTex").SetValue(1);
	particleRenderer->Uniforms("lastPositionTex").SetValue(2);
	particleRenderer->Uniforms("rows").SetValue(rootParticleNum);
	particleRenderer->Uniforms("maxLifeTime").SetValue(10.0f);

	verlet->Use();
	verlet->Uniforms("posTex").SetValue(0);
	verlet->Uniforms("lastPosTex").SetValue(1);
}

void InjectParticles()
{
	fbos[currentBuf]->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, noiseTex->GetId());
	injectParticle->Use();	
	injectParticle->Uniforms("particleNum").SetValue((int)lastParticle);
	injectParticle->Uniforms("position").SetValue(Vec3(0.0f, 0.5f, 1.0f) + randomVec(0.01f));
	injectParticle->Uniforms("velocity").SetValue(Vec3(0.05f, 0.02f, 0.0f) + randomVec(0.001f));
	injectParticle->Uniforms("maxLifeTime").SetValue(10.0f);	
	glBindVertexArray(vaoID);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0, particlesPerFrame);
	glBindVertexArray(0);
	lastParticle+= particlesPerFrame;
	if (lastParticle >= rootParticleNum * rootParticleNum)
		lastParticle = 0;
	
	fbos[currentBuf]->Unbind();
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
	elapsedTime = frameBegin - lastTime;
	timeCount += elapsedTime;
	if (timeCount > 1.0)
	{
		currentFps = frameCount;
		frameCount = 0;
		timeCount = 0.0;
		char buf[100];
		sprintf(buf, "FPS: %d, GL Version %d.%d, rev %d, elapsedTime: %f, elapsedTimeSquared: %lf", currentFps, glMajorVersion, glMinorVersion, glRev, elapsedTime, (float)(elapsedTime * elapsedTime));
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
	if (keyState['P'])
		InjectParticles();
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

	if (simulate)
	{
		Simulate();
	}

	
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(camera->GetProjectionMatrix().Ref());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(camera->GetViewTransform().Ref());

	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);

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
	

	glBindVertexArray(vaoID);	
	particleRenderer->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbos[currentBuf]->GetTexture(0));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, maskTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fbos[currentBuf]->GetTexture(1));
	
	particleRenderer->Uniforms("viewProj").SetValue(camera->GetViewTransform() * camera->GetProjectionMatrix());
	particleRenderer->Uniforms("quadSize").SetValue(particleQuadSize);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0, rootParticleNum * rootParticleNum);
	glBindVertexArray(0);
	glDisable(GL_BLEND);

	if (drawFBOS)
	{

		copyTex->Use();
		copyTex->Uniforms("baseTex").SetValue(0);

		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, fbos[currentBuf]->GetTexture(0));	

		QuadDrawer::DrawQuad(Vec2(-0.4, -1.0), Vec2(0.1, -0.5));		

		glBindTexture(GL_TEXTURE_2D, maskTex);

		QuadDrawer::DrawQuad(Vec2(0.2, -1.0), Vec2(0.7, -0.5));
	}

	if (limitFPS)
	{
		avgSleepTime = 0.016 - glfwGetTime() + frameBegin;
		glfwSleep(avgSleepTime);
	}
	lastTime = frameBegin;

	glfwSwapBuffers();
}

unsigned short indices[] = {0, 1, 2, 0, 2, 3};

void CreateVAO()
{
	GLuint id;
	float vertices[] = {0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0}; 
	GLsizeiptr vSize = sizeof(float) * 8;
	glGenBuffers(1, &vertexBuf);
	glGenBuffers(1, &indexBuf);
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
	glBufferData(GL_ARRAY_BUFFER, vSize, (void*)vertices, GL_STATIC_DRAW);	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 2, (void*)0);
	vSize = sizeof(unsigned short) * 6;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vSize, indices, GL_STATIC_DRAW);
	glBindVertexArray(0);
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
	{
		ShaderManager::GetSingletonPtr()->ReloadShaders();
		SetStaticUniforms();
	}
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
	if (state == GLFW_PRESS)
	{
		if (keyCode == 'L')
			CopyTexToFBO();
		if (keyCode == 'T')
			simulate = !simulate;
		if (keyCode == 'Y')
			Simulate();
		if (keyCode == 'G')
			drawFBOS = !drawFBOS;
		if (keyCode == GLFW_KEY_UP)
			particleQuadSize += 0.001;
		if (keyCode == GLFW_KEY_DOWN)
			particleQuadSize -= 0.001;
	}
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

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);

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