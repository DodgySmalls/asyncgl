#include "Vertex.h"



Vertex::Vertex()
{
	position = glm::vec3(0, 0, 0);
	textureCoords = glm::vec2(0, 0);
}

Vertex::Vertex(glm::vec3 pos, glm::vec2 tcoord)
{
	position = pos;
	textureCoords = tcoord;
}


Vertex::~Vertex()
{
}
