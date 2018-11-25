#pragma once

#include <string>
#include "stb_image.h"
#include "GL/glew.h"

class TextureWrapper
{
public:
	TextureWrapper(std::string filename);
	~TextureWrapper();

	GLuint mTextureId;
};

