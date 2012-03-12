#include <GL\glew.h>
#include <GL\glfw.h>
#include "Uniform.h"

bool running = true;

void update()
{

}

void display()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
	glBegin(GL_QUADS);
	glColor3f(1.0, 0.0, 0.0);
	glVertex2f(100, 100);
	glVertex2f(700, 100);
	glVertex2f(700, 500);
	glVertex2f(100, 500);
	glEnd();
	glfwSwapBuffers();
}

GLuint CreateVBO()
{
	GLuint id;
	float vertices[] = {0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0}; 
	glGenBuffers(1, &id);
	return id;
}

int Exit()
{
	running = false;
	return 0;
}

int main(int argc, char**argv)
{
	if(!glfwInit())
		return 1;

	Uniform* uniform = new Uniform();

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 0);

	if (!glfwOpenWindow(800, 600, 8, 8, 8, 8, 24, 8, GLFW_WINDOW))
		return 1;

	glfwSetWindowCloseCallback(Exit);	

	while(running)
	{
		update();
		display();		
	}

	glfwTerminate();
}