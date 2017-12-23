// External includes
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <algorithm>

// Raytracer
#include "window.h"
#include "shaderprogram.h"
#include "framebuffer.h"
#include "quad.h"
#include "sphere.h"
#include "voxelData.h"
//#include "skybox.h"

#define W 1000
#define H 1000

// Run with ./main gridDimension gridSize noiseScale isoValue

bool WIREFRAME = false;
bool BOUNDINGBOXES = false;
int FOG = 0;
int CRAZY = 0;
float STARTTIME = 0;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && WIREFRAME)
		WIREFRAME = false;
	else if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		WIREFRAME = true;

	if (key == GLFW_KEY_B && action == GLFW_PRESS && BOUNDINGBOXES)
		BOUNDINGBOXES = false;
	else if(key == GLFW_KEY_B && action == GLFW_PRESS)
		BOUNDINGBOXES = true;

	if (key == GLFW_KEY_F && action == GLFW_PRESS && FOG == 1)
		FOG = 0;
	else if(key == GLFW_KEY_F && action == GLFW_PRESS)
		FOG = 1;

	if (key == GLFW_KEY_C && action == GLFW_PRESS && CRAZY == 1)
	{
		CRAZY = 0;
		STARTTIME = glfwGetTime();
	}
	else if(key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		CRAZY = 1;
		STARTTIME = glfwGetTime();
	}
}

int main(int argc, const char * argv[])
{
	// Variables for the fps-counter
	double t0 = 0.0;
	int frames = 0;
	char titlestring[200];

	// Define window
	GLFWwindow *window = nullptr;
	Window w = Window(window, W, H);
	glfwSetKeyCallback(window, key_callback);		

	// Define meshes
	Quad quad = Quad();

	glm::vec3 lightDirection = glm::normalize(glm::vec3(0.0, 1.0, -3.0));

	GLint screenLoc; 
	Framebuffer screenBuffer = Framebuffer(W, H);
	Framebuffer blurBuffer = Framebuffer(W, H);

	// Define shaders
	ShaderProgram phong_shader("shaders/phong.vert", "shaders/phong.frag");
	ShaderProgram screen_shader("shaders/screen.vert", "shaders/screen.frag");

	GLuint fogLoc = glGetUniformLocation(phong_shader.getProgram(), "fogEnabled");
	GLuint crazyLoc = glGetUniformLocation(phong_shader.getProgram(), "crazyEnabled");
	GLuint startTimeLoc = glGetUniformLocation(phong_shader.getProgram(), "startTime");

	// Controls
	MouseRotator rotator;
	rotator.init(window);

	// Get user input on grid parameters
	int gridDimension = 100;
	float gridSize = 0.5;
	float noiseScale = 0.1;
	float isoValue = 0.55;
	int cellGrid = 1; 
	bool useLODs = false;

	if(argc > 1 && atof(argv[1]) > 0)
		gridDimension = atof(argv[1]);
	if(argc > 2 && atof(argv[2]) > 0.05)
		gridSize = atof(argv[2]);
	if(argc > 3 && atof(argv[3]))
		noiseScale = atof(argv[3]);
	if(argc > 4 && atof(argv[4]))
		cellGrid = atof(argv[4]);
	if(argc > 5 && atof(argv[5]))
		useLODs = atof(argv[5]);

	float startTime = glfwGetTime();

	// Create data-volumes
	std::vector<VoxelData> volumes;
	int triangles = 0;
	int cellNumber = 0;
	std::cout << std::endl;
	for(int i = -cellGrid/2; i <= cellGrid/2; i++)
	{
		for(int j = -cellGrid/2; j <= cellGrid/2; j++)
		{
			glm::vec3 center = glm::vec3((gridSize) * i, 0.0, (gridSize) * j);

			int levelOfDetail = gridDimension;			
			//If level of detail should be used.
			if(useLODs)
			{
				levelOfDetail = std::max(6, gridDimension / (std::max(abs(i), abs(j)) + 1));
				center = center * 0.9f;
			}
			else
			{
				center = center * 0.95f;
			}
			volumes.push_back(VoxelData(levelOfDetail, gridSize, center));
			
			volumes[volumes.size() - 1].generateData(noiseScale);
			volumes[volumes.size() - 1].generateTriangles(isoValue);
			triangles += volumes[volumes.size() - 1].getNumberOfTriangles();
			cellNumber++;
			std::cout << "Generating cells " << cellNumber << " of " << pow(cellGrid + (1 - cellGrid%2),2) << std::flush << "\r";
		}
	}

	float timeElapsed = glfwGetTime() - startTime;
	std::cout << "\nNumber of triangles generated: " << triangles;
	std::cout << "\nTime elapsed: " << timeElapsed << " seconds" << std::endl;

	glm::vec3 clear_color = glm::vec3(1.0f, 1.0f, 1.0f);

	w.initFrame();
	
	do
	{
		glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
		rotator.poll(window);
		
		//Checks if any events are triggered (like keyboard or mouse events)
		glfwPollEvents();


		if(WIREFRAME){
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}else{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		
		// Draw to buffer
		screenBuffer.bindBuffer();
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		phong_shader();
		phong_shader.updateCommonUniforms(rotator, W, H, glfwGetTime(), clear_color, lightDirection);
		
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		
		for(unsigned i = 0; i < volumes.size(); i++)
		{
			volumes[i].draw();
			if(BOUNDINGBOXES)
			{
				glLineWidth(3.0);				
				volumes[i].drawBoundingBox();
				glLineWidth(1.0);								
			}
		}
		// Enable/disable fog
		glUniform1i(fogLoc, FOG);
		glUniform1i(crazyLoc, CRAZY);
		glUniform1f(startTimeLoc, STARTTIME);		

		screenLoc = glGetUniformLocation(screen_shader, "screenTexture");
		glUniform1i(screenLoc, 0);
		glActiveTexture(GL_TEXTURE0);
		screenBuffer.bindTexture();		

		// Draw to display
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
		screen_shader();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		screen_shader.updateCommonUniforms(rotator, W, H, glfwGetTime(), clear_color, lightDirection);
		quad.draw();

		glfwSwapBuffers(window);
		glfwPollEvents();

		//Show fps in window title
		double t = glfwGetTime();
		// If one second has passed, or if this is the very first frame
		if ((t - t0) > 1.0 || frames == 0)
		{
			double fps = (double)frames / (t - t0);
			sprintf(titlestring, "Procedurally generated marching cubes (%.1f fps)", fps);
			glfwSetWindowTitle(window, titlestring);
			t0 = t;
			frames = 0;
		}
		frames++;

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
			 glfwWindowShouldClose(window) == 0);

	glDisableVertexAttribArray(0);

	return 0;
}