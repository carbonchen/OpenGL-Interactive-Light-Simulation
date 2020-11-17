#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include "Object.h"

#include <vector>
#include <string>

using namespace std;

class Geometry : public Object
{
private:
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> normals;
	std::vector<glm::ivec3> faces;
	std::string objectName;

	GLuint VAO, VBO, EBO, VBO2;

	static glm::vec3 lightPos;

	int switchRender = 0;
	int rabbitMatInt = 1;
	int sandalMatInt = 0;
	int bearMatInt = 0;

public:
	Geometry(std::string objFilename, std::string name);
	~Geometry();
	
	void draw(const glm::mat4& view, const glm::mat4& projection, GLuint shader);
	//void update();

	void scale(int yoff);
	void rotateControl(glm::vec3 axis, float angle);
	void moveCloserToModel(int yoff);

	void switchRenderFunc();

	void toRabbitMat();
	void toSandalMat();
	void toBearMat();

	void updateLight();
};

#endif
