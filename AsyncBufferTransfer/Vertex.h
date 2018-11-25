#pragma once
#include <glm/glm.hpp>

class Vertex
{
public:
	glm::vec3 position;
	glm::vec2 textureCoords;

	Vertex();
	Vertex(glm::vec3 pos, glm::vec2 tcoord);
	~Vertex();
};

