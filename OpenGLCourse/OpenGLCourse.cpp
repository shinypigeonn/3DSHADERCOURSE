#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <stdio.h>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "Window.h"
#include "Mesh.h"
#include "Shader.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector <Mesh*> meshList;
std::vector <Shader*> shaderList;
GLuint fbo = 0, sceneTexture = 0, depthRbo = 0;
GLuint quadVAO = 0, quadVBO = 0;
GLuint imageTexture = 0;

//GLuint shader, uniformModel, uniformProjection;
//
//bool direction = true;
//float triOffset = 0.0f;
//float triMaxOffset = 0.7f;
//float triIncrement = 0.0006f;
//
//float curAngle = 0.5f;
//
//bool sizeDirection = true;
//float curSize = 0.4f;
//float maxSize = 0.8f;
//float minSize = 0.1f;

// Vertex Shader
static const char* vShader = "Shaders/shader.vert";
static const char* postVShader = "Shaders/post.vert";

// Fragment Shader
static const char* fShader = "Shaders/shader.frag";
static const char* postFShader = "Shaders/post.frag";

// Textures 

// Creates a triangle and sets up the vertex buffer and vertex array objects.
void CreateObjects() 
{
	// Define the vertices for the triangle
	unsigned int indices[] = {
        0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
    };

    GLfloat vertices[] = {
        -1.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

	Mesh* obj1 = new Mesh(); // Create a new Mesh object
	obj1->CreateMesh(vertices, indices, 12, 12); // Create the mesh using the defined vertices and indices
	meshList.push_back(obj1); // Add the mesh to the mesh list

    Mesh* obj2 = new Mesh(); // Create a new Mesh object
    obj2->CreateMesh(vertices, indices, 12, 12); // Create the mesh using the defined vertices and indices
    meshList.push_back(obj2); // Add the mesh to the mesh list
}  

void CreateShaders() 
{
    Shader* shader1 = new Shader(); // Create a new Shader object
    shader1->CreateFromFiles(vShader, fShader); // Create the shader program from the vertex and fragment shader source code
    shaderList.push_back(shader1); // Add the shader to the shader list

    Shader* postShader = new Shader();
    postShader->CreateFromFiles(postVShader, postFShader);
    shaderList.push_back(postShader); 
}

void CreateFramebuffer(int width, int height)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // The colour attachment: an empty texture the scene will be drawn into
    glGenTextures(1, &sceneTexture);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTexture, 0);

    // The depth attachment: so the pyramids sort correctly in pass 1
    glGenRenderbuffers(1, &depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Framebuffer is not complete!\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to the screen
}

void CreateQuad()
{
    // x, y, u, v for each corner. Two triangles cover the whole screen.
    GLfloat quadVertices[] = {
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,

        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Attribute 0: position (2 floats), starting at byte 0, 4 floats per vertex
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // Attribute 1: UV (2 floats), starting 2 floats into each vertex
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

GLuint LoadTexture(const char* path)
{
    stbi_set_flip_vertically_on_load(true); // images start at the top-left, OpenGL's UVs start at the bottom-left

    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 4); // force 4 channels (RGBA)
    if (!data)
    {
        printf("Failed to load image '%s': %s\n", path, stbi_failure_reason());
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data); // the pixels now live on the GPU, so free the CPU copy
    return tex;
}

int main()
{
	mainWindow = Window(800, 600); // Create a window with the specified width and height
	mainWindow.Initialize(); // Initialize the window and OpenGL context

	CreateObjects(); // Create the triangle and set up the vertex buffer and vertex array objects
	CreateShaders(); // Create the shader program from the vertex and fragment shader source code
    CreateFramebuffer((int)mainWindow.getBufferWidth(), (int)mainWindow.getBufferHeight());
    CreateQuad();
    imageTexture = LoadTexture("Textures/scene.png");

	GLuint uniformProjection = 0, uniformModel = 0; // Declare variables to hold the uniform locations for the projection and model matrices
	// Create a perspective projection matrix with a field of view of 45 degrees, an aspect ratio based on the window size, and near and far clipping planes at 0.1 and 100.0 units, respectively
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(), 0.1f, 100.0f);

    // Main loop
    while (!mainWindow.getShouldClose())
    {
        // Poll for and process events
        glfwPollEvents();

  //      if (direction) 
  //      {
		//	triOffset += triIncrement; // Move the triangle to the right
  //      }
  //      else 
		//{
  //          triOffset -= triIncrement; // Move the triangle to the left
  //      }

  //      if (abs(triOffset) >= triMaxOffset) 
  //      {
  //          direction = !direction; // Change direction when the triangle reaches the maximum offset
  //      }

  //      curAngle += 0.01f;
  //      if (curAngle >= 360) 
  //      {
  //          curAngle -= 360;
  //      }

  //      if (sizeDirection) 
  //      {
  //          curSize += 0.0001f;
  //      }
  //      else {
  //          curSize -= 0.0001;
  //      }

  //      if (curSize >= maxSize || curSize <= minSize) {
  //          sizeDirection = !sizeDirection;
  //      }

        // Clear the window
		// glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set the clear color
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color buffer

        // ---------- PASS 1: draw the scene into the offscreen texture ----------
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, (int)mainWindow.getBufferWidth(), (int)mainWindow.getBufferHeight());
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);   // optimization for outcome 3 (see Step 8)
        glFrontFace(GL_CW);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f); // a non-black background shows the fringing better
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderList[0]->UseShader(); // Use the shader program
		uniformModel = shaderList[0]->GetModelLocation(); // Get the location of the model matrix uniform
		uniformProjection = shaderList[0]->GetProjectionLocation(); // Get the location of the projection matrix uniform

		// Create a model matrix to translate the triangle based on the current offset
        glm::mat4 model(1.0f);

		// Apply transformations to the model matrix, where x is the current offset of the triangle, y is 0.0f, and z is -4.0f to move the triangle away from the camera
		model = glm::translate(model, glm::vec3(-0.8f, 0.0f, -2.5f));
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
        //model = glm::rotate(model, curAngle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		meshList[0]->RenderMesh(); // Render the mesh using the RenderMesh() function of the Mesh class

		// Create another model matrix to translate the second triangle based on the current offset
		model = glm::mat4(1.0f);

        model = glm::translate(model, glm::vec3(1.0f, 0.0f, -2.5f));
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
		//model = glm::rotate(model, curAngle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		meshList[1]->RenderMesh();

		glUseProgram(0); 

        // ---------- PASS 2: draw the quad to the screen with the post shader ----------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);  // the quad is flat, depth testing would only get in the way
        glDisable(GL_CULL_FACE);   // the quad's winding is the opposite of the pyramids'
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderList[1]->UseShader();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, imageTexture); 
        glUniform1i(shaderList[1]->GetUniformLocation("sceneTexture"), 0); // sampler reads texture unit 0
        glUniform1f(shaderList[1]->GetUniformLocation("strength"), 0.0f);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glUseProgram(0);

		mainWindow.swapBuffers(); // Swap the front and back buffers to display the rendered image
	}

    return 0;
}

