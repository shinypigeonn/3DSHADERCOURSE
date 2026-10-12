#include "Window.h"
#include <iostream>

Window::Window() 
{
    width = 800;
    height = 600;

    for(size_t i = 0; i < 1024; i++)
    {
        keys[i] = false;
	}
}
 
Window::Window(GLint windowWidth, GLint windowHeight) 
{
	width = windowWidth;
	height = windowHeight;
    
    for(size_t i = 0; i < 1024; i++)
    {
        keys[i] = false;
	}
}

int Window::Initialize() 
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        glfwTerminate();
        return -1;
    }

    // Setup GLFW window properties
    // OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Setting the major version of OpenGL to 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Setting the minor version of OpenGL to 3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Setting the OpenGL profile to core
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS compatibility

    // Create a windowed mode window and its OpenGL context
    mainWindow = glfwCreateWindow(width, height, "OpenGL Course", NULL, NULL);
    if (!mainWindow)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // Get the size of the framebuffer (in pixels) for the created window 
    // (Use the window we just created and pass in the addresses of the 
    // bufferWidth and bufferHeight variables to store the results)
    glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

    // Make the window's context current
    glfwMakeContextCurrent(mainWindow);

	createCallbacks(); // Set up the callback functions for the window

    //Allow modern extension features
    glewExperimental = GL_TRUE; // This line is necessary to use modern OpenGL features

    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW\n";
        glfwDestroyWindow(mainWindow); // Destroy the window and its context before terminating GLFW
        glfwTerminate(); // Terminate GLFW
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Setup viewport size
    glViewport(0, 0, bufferWidth, bufferHeight);

	glfwSetWindowUserPointer(mainWindow, this); // Set the user pointer for the window to this instance of the Window class

	return 0; // Return 0 to indicate successful initialization
}

void Window::createCallbacks() 
{
    glfwSetKeyCallback(mainWindow, handleKeys); // Set the key callback function for the window
}

void Window::handleKeys(GLFWwindow* window, int key, int code, int action, int mode) 
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window)); // Retrieve the Window instance from the user pointer

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) // If the escape key is pressed
    {
        glfwSetWindowShouldClose(window, GL_TRUE); // Set the window to close
	}   

    if (key >= 0 && key < 1024) // If the key is within the valid range
    {
        if (action == GLFW_PRESS) // If the key is pressed
        {
            theWindow->keys[key] = true; // Set the corresponding key state to true
			printf("Key %d pressed\n", key); // Print the key code to the console
        }
        else if (action == GLFW_RELEASE) // If the key is released
        {
            theWindow->keys[key] = false; // Set the corresponding key state to false
			printf("Key %d released\n", key); // Print the key code to the console
        }
	}
}


Window::~Window()
{
    glfwDestroyWindow(mainWindow); // Destroy the window and its context before terminating GLFW
    glfwTerminate(); // Terminate GLFW
}
