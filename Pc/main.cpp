#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <stdlib.h>

#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <vector>

#define numVAOs 1

using namespace std;

int startX = 0;
int startY = 0;
int endX = 0;
int endY = 0;
int startInfo = 0;
bool render = false;
bool reset = false;

struct Matrixs
{
	glm::mat4 view;
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 projection;
};

struct Grid
{
	int level = 0;
	int activeRow = 0;
	int index = 0;

	float offset = 1.0f;
	float curX = 0.0f;
	float curY = 0.0f;

	int ratio = 0;

	int start_x = 0;
	int end_x = 0;
	int start_y = 0;
	int end_y = 0;

	int steps_x = 0;
	int steps_y = 0;

	float width[8] = { 32.0f, 16.0f, 8.0f, 4.0f, 2.0f, 1.0f };		// Designed only for resolution 320x320 for now
	int pixelCount[8] = { 10, 20, 40, 80, 160, 320 }; 
	vector<float> points;
	vector<float> colors;

	void AddPoint(float col)
	{
		if (index == 0)
		{
			ratio = pixelCount[level] / 10;

			start_x = startX * ratio;
			end_x = endX * ratio;
			start_y = startY * ratio;
			end_y = endY * ratio;

			steps_x = abs(end_x - start_x);
			steps_y = abs(end_y - start_y);

			printf("Steps: %i\n", steps_x);
		}

		if (index == ((steps_x * steps_y) - 1) || reset)
		{
			
			index = 0;
			offset -= 0.3f;
			glPolygonOffset(offset, offset);

			curX = start_x * width[level];
			curY = start_y * width[level];
			level++;

			if (level == 5 || reset)  // disable 160 and 320 modes for now... level == 8 if you want to enable it
			{
				this->points.clear();
				this->colors.clear();
				glClear(GL_COLOR_BUFFER_BIT);
				level = 0;
				offset = 1.0f;
				glPolygonOffset(offset, offset);
				reset = false;
			}

		}
		else
			index++;

		if (index % steps_x == 0 && index != 0) {
			curY += width[level];
			curX = start_x * width[level];
		}
		
		// TRIANGLE 1

		points.push_back(curX);
		points.push_back(curY);

		points.push_back(curX+width[level]);
		points.push_back(curY);

		points.push_back(curX + width[level]);
		points.push_back(curY + width[level]);

		// TRIANGLE 2

		points.push_back(curX);
		points.push_back(curY);

		points.push_back(curX + width[level]);
		points.push_back(curY + width[level]);

		points.push_back(curX);
		points.push_back(curY + width[level]);

		// COLOR

		colors.push_back(col);
		colors.push_back(col);
		colors.push_back(col);

		colors.push_back(col);
		colors.push_back(col);
		colors.push_back(col);
		
		curX += width[level];
		

		


			
	}
};

GLuint renderingProgram;
GLuint VAO, VBO, VBO1;
Grid grid;
Matrixs matrixs;
GLuint projLoc;

void processFlag(unsigned int flag)
{
	printf("StartInfo: %i\n", startInfo);

	if (startInfo == 1) {
		startInfo = 2;
		startX = flag;
	}
	else if (startInfo == 2) {
		startInfo = 3;
		startY = flag;
	}
	else if (startInfo == 3) {
		startInfo = 4;
		endX = flag;
	}
	else if (startInfo == 4) {
		startInfo = 5;
		endY = flag;
		printf("StartX: %i  StartY: %i  EndX: %i  EndY: %i\n", startX, startY, endX, endY);
		
	}
	else if (startInfo == 5) {
		startInfo = 0;
		render = true;
		reset = true;
	}
	else if (flag == 0) 
		startInfo = 1;
		


}

GLuint createShaderProgram()
{
	const char* vsShaderSource =
		"#version 330 core\n"
		"in vec2 position;\n"
		"in float color;\n"
		"out float Col;\n"
		"uniform mat4 projection;\n"
		"void main()\n"
		"{ Col = color; gl_Position = projection * vec4(position, 0.0, 1.0); }";

	const char* fsShaderSource =
		"#version 330 core\n"
		"out vec4 color;\n"
		"in float Col;\n"
		"void main()\n"
		"{ color = vec4(Col, Col, Col, 1.0); }";

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vShader, 1, &vsShaderSource, NULL);
	glShaderSource(fShader, 1, &fsShaderSource, NULL);
	glCompileShader(vShader);
	glCompileShader(fShader);

	GLuint vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);
	glLinkProgram(vfProgram);

	return vfProgram;
}


void init(GLFWwindow* window) 
{
	renderingProgram = createShaderProgram();

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VBO1);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, grid.points.size() * sizeof(float) , grid.points.data(), GL_STATIC_DRAW);

	GLint posAttrib = glGetAttribLocation(renderingProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, grid.colors.size() * sizeof(float), grid.colors.data(), GL_STATIC_DRAW);

	GLint colAttrib = glGetAttribLocation(renderingProgram, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void display(GLFWwindow* window, double currentTime)
{
	matrixs.projection = glm::ortho(0.0f, 320.0f, 0.0f, 320.0f, 0.0f, 100.0f);

	
	glUseProgram(renderingProgram);
	projLoc = glGetUniformLocation(renderingProgram, "projection");

	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(matrixs.projection));

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.02f, 1.0);
}

int main()
{
	HANDLE hComm;
	char buff[2] = { 0 };
	DWORD bytesRead = 0;
	
	hComm = CreateFileA("\\\\.\\COM4",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hComm == INVALID_HANDLE_VALUE)
		printf("Error in opening serial port\n");
	else
		printf("Openning serial port succesful\n");	

	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1000, 1000, "Valvontakamera v0.1", NULL, NULL);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	init(window);

	while (!glfwWindowShouldClose(window))
	{
		
		display(window, glfwGetTime());

		if (ReadFile(hComm, buff, 1, &bytesRead, NULL))
		{
			unsigned int flag = (unsigned int)buff[0];
			processFlag(flag);
			
			
			unsigned char color = (unsigned int)buff[0];
			float col = color / (float)255;

			printf("read characters: %i\n", color);

			if (render)
			{
				grid.AddPoint(col);

				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, grid.points.size() * sizeof(float), grid.points.data(), GL_STATIC_DRAW);

				glBindBuffer(GL_ARRAY_BUFFER, VBO1);
				glBufferData(GL_ARRAY_BUFFER, grid.colors.size() * sizeof(float), grid.colors.data(), GL_STATIC_DRAW);
			}
				
			
		}
		

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, grid.points.size()/2);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	CloseHandle(hComm);
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);

	return 0;
}
