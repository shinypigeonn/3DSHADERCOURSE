#include "Mesh.h"

Mesh::Mesh() 
{
	VAO = 0;
	VBO = 0;
	EBO = 0;
	indexCount = 0;
}

void Mesh::CreateMesh(GLfloat* vertices,unsigned int* indices,unsigned int numOfVertices, unsigned int numOfIndices) 
{
	indexCount = numOfIndices;

	glGenVertexArrays(1, &VAO); // Generate a vertex array object (VAO) and store its ID in the variable VAO
	glBindVertexArray(VAO); // Bind the VAO so that subsequent vertex attribute calls will be stored in this VAO

	glGenBuffers(1, &EBO); // Generate an element buffer object (EBO) and store its ID in the variable EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // Bind the EBO so that subsequent element buffer calls will be stored in this EBO
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * numOfIndices, indices, GL_STATIC_DRAW); // Copy the index data to the EBO  

	// Generate a vertex buffer object (VBO) and store its ID in the variable VBO
	glGenBuffers(1, &VBO);

	// Bind the VBO so that subsequent vertex buffer calls will be stored in this VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Copy the vertex data to the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * numOfVertices, vertices, GL_STATIC_DRAW);

	// Specify the layout of the vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //

	// Enable the vertex attribute array for the position attribute (index 0)
	glEnableVertexAttribArray(0);

	/// Unbind the VAO, EBO, and VBO to prevent accidental modification
	glBindVertexArray(0); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
	glBindBuffer(GL_ARRAY_BUFFER, 0); 
}

// ----- BINDING THE VAO & EBO -----

void Mesh::RenderMesh()
{
	// Bind the VAO and EBO associated with the mesh and draw the mesh using the indices
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0); 

	// After rendering, unbind the VAO to prevent accidental modification.
	glBindVertexArray(0);
}

// ----- CLEARING THE MESH -----

void Mesh::ClearMesh()
{
	// Delete the EBO, VBO, and VAO associated with the mesh to free up GPU memory

	if (EBO != 0) {
		glDeleteBuffers(1, &EBO);
		EBO = 0;
	}
	if (VBO != 0) {
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}
	if (VAO != 0) {
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}

	// Reset the index count to 0 since the mesh has been cleared
	indexCount = 0;
}

// ----- DESTRUCTOR -----

Mesh::~Mesh() 
{
	// Call the ClearMesh function to free up GPU memory when the Mesh object is destroyed
	ClearMesh();
}