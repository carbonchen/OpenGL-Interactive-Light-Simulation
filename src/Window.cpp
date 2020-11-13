#include "Window.h"

// Window Properties
int Window::width;
int Window::height;
const char* Window::windowTitle = "OpenGL Project";

// Objects to Render
PointCloud* Window::bunnyPoints;
PointCloud* Window::sandalPoints;
PointCloud* Window::bearPoints;
PointCloud* Window::spherePoints;
Object* currObj;

// Camera Matrices 
// Projection matrix:
glm::mat4 Window::projection; 

// View Matrix:
glm::vec3 Window::eyePos(0, 0, 20);			// Camera position.
glm::vec3 Window::lookAtPoint(0, 0, 0);		// The point we are looking at.
glm::vec3 Window::upVector(0, 1, 0);		// The up direction of the camera.
glm::mat4 Window::view = glm::lookAt(Window::eyePos, Window::lookAtPoint, Window::upVector);

// Shader Program ID
GLuint Window::shaderProgram;

// Interaction options
bool Window::mouseDown;
glm::vec3 Window::lastMousePoint;
bool Window::mode1 = true;
bool Window::mode2 = false;
bool Window::mode3 = false;

bool Window::initializeProgram() {
	// Create a shader program with a vertex shader and a fragment shader.
	shaderProgram = LoadShaders("shaders/shader.vert", "shaders/shader.frag");

	// Check the shader program.
	if (!shaderProgram)
	{
		std::cerr << "Failed to initialize shader program" << std::endl;
		return false;
	}

	return true;
}

bool Window::initializeObjects()
{
	bunnyPoints = new PointCloud("../obj/bunny.obj", "bunny");
	sandalPoints = new PointCloud("../obj/SandalF20.obj", "sandal");
	bearPoints = new PointCloud("../obj/bear.obj", "bear");
	spherePoints = new PointCloud("../obj/sphere.obj", "sphere");
	currObj = bunnyPoints;
	return true;
}

void Window::cleanUp()
{
	// Deallcoate the objects.
	delete bunnyPoints;
	delete sandalPoints;
	delete bearPoints;
	delete spherePoints;

	// Delete the shader program.
	glDeleteProgram(shaderProgram);
}

GLFWwindow* Window::createWindow(int width, int height)
{
	// Initialize GLFW.
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return NULL;
	}

	// 4x antialiasing.
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__ 
	// Apple implements its own version of OpenGL and requires special treatments
	// to make it uses modern OpenGL.

	// Ensure that minimum OpenGL version is 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Enable forward compatibility and allow a modern OpenGL context
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create the GLFW window.
	GLFWwindow* window = glfwCreateWindow(width, height, windowTitle, NULL, NULL);

	// Check if the window could not be created.
	if (!window)
	{
		std::cerr << "Failed to open GLFW window." << std::endl;
		glfwTerminate();
		return NULL;
	}

	// Make the context of the window.
	glfwMakeContextCurrent(window);

#ifndef __APPLE__
	// On Windows and Linux, we need GLEW to provide modern OpenGL functionality.

	// Initialize GLEW.
	if (glewInit())
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return NULL;
	}
#endif

	// Set swap interval to 1.
	glfwSwapInterval(0);

	// Call the resize callback to make sure things get drawn immediately.
	Window::resizeCallback(window, width, height);

	return window;
}

void Window::resizeCallback(GLFWwindow* window, int width, int height)
{
#ifdef __APPLE__
	// In case your Mac has a retina display.
	glfwGetFramebufferSize(window, &width, &height); 
#endif
	Window::width = width;
	Window::height = height;
	// Set the viewport size.
	glViewport(0, 0, width, height);

	// Set the projection matrix.
	Window::projection = glm::perspective(glm::radians(60.0), 
								double(width) / (double)height, 1.0, 1000.0);
}

void Window::idleCallback()
{
	// Perform any necessary updates here 
	// currObj->update();
}

void Window::displayCallback(GLFWwindow* window)
{	
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	// Render the objects
	currObj->draw(view, projection, shaderProgram);
	spherePoints->draw(view, projection, shaderProgram);

	// Gets events, including input such as keyboard and mouse or window resizing
	glfwPollEvents();

	// Swap buffers.
	glfwSwapBuffers(window);
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	
	// Check for a key press.
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);				
			break;

		// switch between pointclouds/objects
		case GLFW_KEY_1:
			currObj = bunnyPoints;
			currObj->toRabbitMat();
			spherePoints->toRabbitMat();
			break;
		case GLFW_KEY_2:
			currObj = sandalPoints;
			currObj->toSandalMat();
			spherePoints->toSandalMat();
			break;
		case GLFW_KEY_3:
			currObj = bearPoints;
			currObj->toBearMat();
			spherePoints->toBearMat();
			break;

		// switch coloring scheme (normal vs Phong)
		case GLFW_KEY_N:
			currObj->switchRenderFunc();
			break;

		// switch between interaction modes
		case GLFW_KEY_Z:
			mode1 = true;
			mode2 = false;
			mode3 = false;
			break;
		case GLFW_KEY_X:
			mode1 = false;
			mode2 = true;
			mode3 = false;
			break;
		case GLFW_KEY_C:
			mode1 = false;
			mode2 = false;
			mode3 = true;
			break;

		default:
			break;
		}
	}
}

// when mouse button held down, allow object rotation on cursor move
void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		mouseDown = true;

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		lastMousePoint = trackball(glm::vec2(xpos, ypos));
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		mouseDown = false;
	}
}

// performs rotation based on cursor movement
void Window::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	double pos_x = xpos;
	double pos_y = ypos;
	glm::vec2 mouseCoord;
	mouseCoord.x = pos_x;
	mouseCoord.y = pos_y;

	glm::vec3 currPoint = trackball(mouseCoord);
	if (mouseDown) {
		glm::vec3 direction = currPoint - lastMousePoint;
		float velocity = glm::length(direction);
		if (velocity > 0.0001) {
			glm::vec3 rotAxis = glm::cross(lastMousePoint, currPoint);
			float rot_angle = velocity * 1.5f;
			// rotate obj if mode1
			if (mode1) {
				currObj->rotateControl(rotAxis, rot_angle);
			}
			// rotate light about model if mode2
			else if (mode2) {
				spherePoints->rotateControl(rotAxis, rot_angle);
				spherePoints->updateLight();
			}
			// rotate both light and model together
			else {
				currObj->rotateControl(rotAxis, rot_angle);
				spherePoints->rotateControl(rotAxis, rot_angle);
				spherePoints->updateLight();
			}
			lastMousePoint = currPoint;
		}
	}
}

// map 2d point on screen to 3d point on object
glm::vec3 Window::trackball(glm::vec2 mouseCoord) {
	glm::vec3 v;
	float d;
	v.x = (2.0 * mouseCoord.x - width) / width;
	v.y = (height - 2.0 * mouseCoord.y) / height;
	v.z = 0.0;
	d = glm::length(v);
	d = (d < 1.0) ? d : 1.0;
	v.z = sqrtf(1.001 - d * d);
	v = glm::normalize(v);
	return v;
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	double x_off = xoffset;
	double y_off = yoffset;
	// scale obj if mode1
	if (mode1) {
		currObj->scale(y_off);
	}
	// move light closer to/farther from object if mode2
	if (mode2) {
		spherePoints->moveCloserToModel(y_off);
		spherePoints->updateLight();
	}
	// scale obj and move light closer/farther from center if mode3
	if (mode3) {
		currObj->scale(y_off);
		spherePoints->moveCloserToModel(y_off);
		spherePoints->updateLight();
	}
}