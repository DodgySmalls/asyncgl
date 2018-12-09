#include "Entity.h"

Entity::Entity()
{
	mWasLoaded = false;
	mWasUploaded = false;
	mTextureId = 0;
	mFilename = EMPTY_ENTITY_FILENAME;
}


Entity::Entity(std::string filename)
{
	mWasLoaded = false;
	mWasUploaded = false;
	mTextureId = 0;
	mFilename = filename;
}

Entity::~Entity()
{
	//propose to system queue to cleanup OpenGl resources if(hasUploaded())
	//CURRENTLY LEAKS GPU MEMORY
}



bool Entity::wasLoaded()
{
	return mWasLoaded;
}

bool Entity::wasUploaded()
{
	return mWasUploaded;
}

GLuint Entity::textureId()
{
	return mTextureId;
}

void Entity::loadFromDisk()
{
	mImageData = stbi_load(mFilename.c_str(), &mW, &mH, &mComp, STBI_rgb);

	if (mImageData == nullptr)
	{
		fprintf(stderr, "ERR >> Image %s not loaded", mFilename.c_str());
	}

	mWasLoaded = true;
}

void Entity::uploadToGpu()
{
	/*
	glGenTextures(1, &mTextureId);
	fprintf(stderr, "textureId : %u \n", mTextureId);

	printf("activating texture\n");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (mComp == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mW, mH, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	}
	else if (mComp == 4)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mW, mH, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}



	glGenBuffers(1, &pbo);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, mW * mH * mComp * sizeof(unsigned char), mImageData, GL_DYNAMIC_DRAW);
	GLubyte *ptr = (GLubyte *)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE); //(GLubyte *) glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, mW * mH * mComp * sizeof(unsigned char), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	if (ptr)
	{
		memcpy(ptr, mImageData, mW * mH * mComp * sizeof(unsigned char));
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	}
	else
	{
		int x = 0;
		printf("fak");
		std::cin >> x;
		exit(0);
	}

	printf("glTexSubImage()\n");
	if (mComp == 3)
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mW, mH, GL_RGB, GL_UNSIGNED_BYTE, 0);
	}
	else if (mComp == 4)
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mW, mH, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}

	printf("unbinding\n");
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(mImageData);

	mWasUploaded = true; */

	glGenTextures(1, &mTextureId);
	fprintf(stderr, "textureId : %u \n", mTextureId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (mComp == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mW, mH, 0, GL_RGB, GL_UNSIGNED_BYTE, mImageData);
	}
	else if (mComp == 4)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mW, mH, 0, GL_RGBA, GL_UNSIGNED_BYTE, mImageData);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(mImageData);

	mWasUploaded = true;
}