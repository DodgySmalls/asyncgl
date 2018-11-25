#include "TextureWrapper.h"


TextureWrapper::TextureWrapper(std::string filename)
{
	//LOAD TEXTURE
	//sample texture snipper derived from SO post https://stackoverflow.com/questions/23150123/loading-png-with-stb-image-for-opengl-texture-gives-wrong-colors
	int w;
	int h;
	int comp;
	unsigned char* image = stbi_load(filename.c_str(), &w, &h, &comp, STBI_rgb);
		
	if (image == nullptr)
	{
		fprintf(stderr, "image not loaded\n\n\n\n\n\n\n");
	}

	glGenTextures(1, &mTextureId);
	fprintf(stderr, "textureId : %u \n", mTextureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTextureId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (comp == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	}
	else if (comp == 4)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(image);

}


TextureWrapper::~TextureWrapper()
{
}
