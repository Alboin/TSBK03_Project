// External includes
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>

// Raytracer
#include "window.h"
#include "shaderprogram.h"
#include "framebuffer.h"
#include "quad.h"
#include "sphere.h"
#include "voxelData.h"

#define W 800
#define H 800

bool WIREFRAME = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_W && action == GLFW_PRESS && WIREFRAME)
		WIREFRAME = false;
	else if(key == GLFW_KEY_W && action == GLFW_PRESS)
		WIREFRAME = true;
	
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
	//Sphere sphere = Sphere(15, 15, 0.5f);


	GLint screenLoc; 
	Framebuffer screenBuffer = Framebuffer(W, H);
	Framebuffer blurBuffer = Framebuffer(W, H);

	// Define shaders
	ShaderProgram phong_shader("shaders/phong.vert", "shaders/phong.frag");
	ShaderProgram screen_shader("shaders/screen.vert", "shaders/screen.frag");
	ShaderProgram blur_shader("shaders/screen.vert", "shaders/blur.frag");

	// Controls
	MouseRotator rotator;
	rotator.init(window);

	// Get user input on grid parameters
	int gridDimension = 100;
	float gridSize = 0.5;
	float noiseScale = 0.1;
	float isoValue = 0.5;

	if(argc > 1 && atof(argv[1]) > 0)
		gridDimension = atof(argv[1]);
	if(argc > 2 && atof(argv[2]) > 0.05)
		gridSize = atof(argv[2]);
	if(argc > 3 && atof(argv[3]))
		noiseScale = atof(argv[3]);
	if(argc > 4 && atof(argv[4]))
		isoValue = atof(argv[4]);

	float startTime = glfwGetTime();
	// Create data-volumes
	std::vector<VoxelData> volumes;
	int triangles = 0;
	for(int i = -2; i <= 2; i++)
	{
		for(int j = -2; j <= 2; j++)
		{
			glm::vec3 center = glm::vec3((gridSize/2.0) * i, 0.0, (gridSize/2.0) * j);
			volumes.push_back(VoxelData(gridDimension, gridSize, center));
			volumes[volumes.size() - 1].generateData(noiseScale);
			volumes[volumes.size() - 1].generateTriangles(isoValue);
			triangles += volumes[volumes.size() - 1].getNumberOfTriangles();			
		}
	}
	float timeElapsed = glfwGetTime() - startTime;
	std::cout << "\nNumber of triangles generated: " << triangles;
	std::cout << "\nTime elapsed: " << timeElapsed << " seconds" << std::endl;

	// VoxelData volume(gridDimension, gridSize);
	// volume.generateData();
	// volume.generateTriangles(isoValue);
	// volume.getInfo(false, false);

	// VoxelData volume2(gridDimension, gridSize, glm::vec3(gridSize/2, 0, 0));
	// volume2.generateData();
	// volume2.generateTriangles(isoValue);


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
		phong_shader.updateCommonUniforms(rotator, W, H, glfwGetTime(), clear_color);
		//sphere.draw();
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);

		for(unsigned i = 0; i < volumes.size(); i++)
			volumes[i].draw();
		//volume.draw();
		//volume2.draw();

		screenLoc = glGetUniformLocation(screen_shader, "screenTexture");
		glUniform1i(screenLoc, 0);
		glActiveTexture(GL_TEXTURE0);
		screenBuffer.bindTexture();

		// Draw to blur shader
		

		// Draw to display
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
		screen_shader();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		screen_shader.updateCommonUniforms(rotator, W, H, glfwGetTime(), clear_color);
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