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

#define W 512
#define H 512

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
	Sphere sphere = Sphere(15, 15, 0.5f);


	GLint screenLoc; 
	Framebuffer screenBuffer = Framebuffer(W, H);

	// Define shaders
	ShaderProgram phong_shader("shaders/phong.vert", "shaders/phong.frag");
	ShaderProgram screen_shader("shaders/screen.vert", "shaders/screen.frag");

	// Controls
	MouseRotator rotator;
	rotator.init(window);

	// Get user input on grid parameters
	int gridDimension = 100;
	float gridSize = 0.5;
	if(argc > 0 && atof(argv[1]) > 0)
		gridDimension = atof(argv[1]);
	if(argc > 1 && atof(argv[2]) > 0.05)
		gridSize = atof(argv[2]);

	// Create data-volume
	VoxelData volume(gridDimension, gridSize);
	volume.generateData();
	volume.generateTriangles(0.5f);
	volume.getInfo(false, false);
	

	do
	{
		w.initFrame();
		glm::vec3 clear_color = glm::vec3(0.4f, 0.15f, 0.26f);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
		rotator.poll(window);
		
		//Checks if any events are triggered (like keyboard or mouse events)
		glfwPollEvents();


		if(WIREFRAME){
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}else{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}


		screenBuffer.bindBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		phong_shader();
		phong_shader.updateCommonUniforms(rotator, W, H, glfwGetTime(), clear_color);
		sphere.draw();
		volume.draw();


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		screen_shader();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		screenLoc = glGetUniformLocation(screen_shader, "screenTexture");
		glUniform1i(screenLoc, 0);
		glActiveTexture(GL_TEXTURE0);
		screenBuffer.bindTexture();

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