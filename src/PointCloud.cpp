#include "PointCloud.h"
#include <iostream>
#include <sstream>
#include <fstream>

// initialize static variable light position
glm::vec3 PointCloud::lightPos = glm::vec3(-8.0f, 8.0f, 0.0f);

PointCloud::PointCloud(std::string objFilename, std::string name) 
	: objectName(name)
{

	// Parsing obj file
	std::ifstream objFile(objFilename);

	// Check whether the file can be opened.
	if (objFile.is_open())
	{
		std::string line; // A line in the file.

		// Read lines from the file.
		while (std::getline(objFile, line))
		{

			// replace all "//" with " " for easy parsing of face vertices
			auto idx = line.find("//");
			while (idx != std::string::npos) {
				line.replace(idx, 2, " ");
				idx = line.find("//");
			}

			// Turn the line into a string stream for processing.
			std::stringstream ss;
			ss << line;

			// Read the first word of the line.
			std::string label;
			ss >> label;

			// If the line is about vertex (starting with a "v").
			if (label == "v")
			{
				// Read the later three float numbers and use them as the 
				// coordinates.
				glm::vec3 point;
				ss >> point.x >> point.y >> point.z;

				// Process the point. For example, you can save it to a.
				points.push_back(point);
			}

			// store normal vertices
			if (label == "vn") {
				glm::vec3 normal;
				ss >> normal.x >> normal.y >> normal.z;
				normals.push_back(normal);
			}

			// store face vertices
			if (label == "f") {
				glm::ivec3 face;
				glm::ivec3 face2; // second int in a face vertex pair

				ss >> face.x >> face2.x >> face.y >> face2.y >> face.z >> face2.z;
				// change to 1-based indexing
				face.x = face.x - 1;
				face.y = face.y - 1;
				face.z = face.z - 1;
				faces.push_back(face);
			}
		}
	}
	else
	{
		std::cerr << "Can't open the file " << objFilename << std::endl;
	}

	objFile.close();

	// find min and max coordinates of the obj along x, y, z axes
	float minX, maxX, minY, maxY, minZ, maxZ;
	for (int i = 0; i < points.size(); i++) {
		if (points[i].x < minX) {
			minX = points[i].x;
		}
		if (points[i].y < minY) {
			minY = points[i].y;
		}
		if (points[i].z < minZ) {
			minZ = points[i].z;
		}
		if (points[i].x > maxX) {
			maxX = points[i].x;
		}
		if (points[i].y > maxY) {
			maxY = points[i].y;
		}
		if (points[i].z > maxZ) {
			maxZ = points[i].z;
		}
	}
	float centerX = (minX + maxX) / 2;
	float centerY = (minY + maxY) / 2;
	float centerZ = (minZ + maxZ) / 2;
	float dist = max((maxX - minX), (maxY - minY));
	dist = max(dist, (maxZ - minZ));
	// center + scale obj to fit in 1x1x1 box
	for (int i = 0; i < points.size(); i++) {
		points[i].x = (points[i].x - centerX) * (1 / dist);
		points[i].y = (points[i].y - centerY) * (1 / dist);
		points[i].z = (points[i].z - centerZ) * (1 / dist);
	}
	// scale up by an arbitrary factor to fit the window
	for (int i = 0; i < points.size(); i++) {
		points[i].x = points[i].x * 15;
		points[i].y = points[i].y * 15;
		points[i].z = points[i].z * 15;
	}

	// Set the model matrix to an identity matrix. 
	model = glm::mat4(1);

	// if obj is light sphere, shrink + set it's position where the light is
	if (objectName == "sphere") {
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.1f));
	}

	// Generate a Vertex Array (VAO) and Vertex Buffer Object (VBO)
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Bind VAO
	glBindVertexArray(VAO);

	// Bind VBO to the bound VAO, and store the point data
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * points.size(), points.data(), GL_STATIC_DRAW);
	// Enable Vertex Attribute 0 to pass point data through to the shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	// Rendering triangles
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3)* faces.size(), faces.data(), GL_STATIC_DRAW);

	// Send normals info
	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)* normals.size(), normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	// Unbind the VBO/VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

PointCloud::~PointCloud() 
{
	// Delete the VBO and the VAO.
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &VBO2);
	glDeleteVertexArrays(1, &VAO);
}

void PointCloud::draw(const glm::mat4& view, const glm::mat4& projection, GLuint shader)
{
	// Activate the shader program 
	glUseProgram(shader);

	// Get the shader variable locations and send the uniform data to the shader 
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, false, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, false, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// let the shader know which model is shown to shade accordingly
	glUniform1i(glGetUniformLocation(shader, "switchRender"), switchRender);
	glUniform1i(glGetUniformLocation(shader, "rabbit"), rabbitMatInt);
	glUniform1i(glGetUniformLocation(shader, "sandal"), sandalMatInt);
	glUniform1i(glGetUniformLocation(shader, "bear"), bearMatInt);

	// let the shader know if obj is the light sphere
	if (objectName == "sphere") {
		glUniform1i(glGetUniformLocation(shader, "sphere"), 1);
	}
	else {
		glUniform1i(glGetUniformLocation(shader, "sphere"), 0);
	}

	// send light position
	glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, glm::value_ptr(lightPos));

	// Bind the VAO
	glBindVertexArray(VAO);
	// Draw the points using triangles
	glDrawElements(GL_TRIANGLES, 3*faces.size(), GL_UNSIGNED_INT, 0);
	// Unbind the VAO and shader program
	glBindVertexArray(0);
	glUseProgram(0);
}

/*
	void PointCloud::update()
	{
		use this function for testing purposes
		loops infinitely
	}
*/

// scale object when scrolling (mode1, mode3)
void PointCloud::scale(int yoff) {
	if (yoff > 0) {
		model = glm::scale(model, glm::vec3(1.25f));
	}
	else {
		model = glm::scale(model, glm::vec3(0.75f));
	}
}

// move light to/from center when scrolling (mode2, mode3)
void PointCloud::moveCloserToModel(int yoff) {
	// move distance is (dist from origin)/5
	float xmove = std::abs(model[3][0])/5;
	float ymove = std::abs(model[3][1])/5;
	float zmove = std::abs(model[3][2])/5;

	// decide which direction to move towards
	if (model[3][0] > 0) {
		xmove = -xmove;
	}
	if (model[3][1] > 0) {
		ymove = -ymove;
	}
	if (model[3][2] > 0) {
		zmove = -zmove;
	}

	// scroll up = move away from origin
	// scroll down = move to origin
	if (yoff > 0) {
		model[3][0] += -xmove;
		model[3][1] += -ymove;
		model[3][2] += -zmove;
	}
	else {
		model[3][0] += xmove;
		model[3][1] += ymove;
		model[3][2] += zmove;
	}
}

void PointCloud::rotateControl(glm::vec3 axis, float angle) {
	model = glm::rotate(angle, axis) * model;
}

// tell shader which render mode to use
void PointCloud::switchRenderFunc() {
	if (switchRender == 0) {
		switchRender = 1;
	}
	else {
		switchRender = 0;
	}
}

// tell shader which obj's material to render
void PointCloud::toRabbitMat() {
	rabbitMatInt = 1;
	sandalMatInt = 0;
	bearMatInt = 0;
}
void PointCloud::toSandalMat() {
	rabbitMatInt = 0;
	sandalMatInt = 1;
	bearMatInt = 0;
}
void PointCloud::toBearMat() {
	rabbitMatInt = 0;
	sandalMatInt = 0;
	bearMatInt = 1;
}

// force shader to update coloring when light is moved
void PointCloud::updateLight() {
	lightPos.x = model[3][0];
	lightPos.y = model[3][1];
	lightPos.z = model[3][2];
}