#pragma once

#include <GL/glew.h>  
#include "GLFW/glfw3.h"

#include "stb_image.h"
#include <string>
#include <iostream>

#define EMPTY_ENTITY_FILENAME "NO_FILENAME"


class Entity
{
public:
	Entity();
	Entity(std::string filename);
	~Entity();
	
	void loadFromDisk();	//!< loads the entity's data onto the heap from disk
	void uploadToGpu();		//!< loads the entity's data onto the GPU, flags upload complete, then releases the local memory
	bool wasLoaded();
	bool wasUploaded();
	GLuint textureId();

	GLuint pbo;

private:
	//flags can be threadsafe since they represent unidirectional state changes.
	//as soon as a flag is ever touched, it is permanently considered true, meaning no race condition can occur even without mutual exclusion
	bool mWasLoaded;			//!< INTERNAL, flag indicates content has been loaded into memory from disk
	bool mWasUploaded;			//!< INTERNAL, flag indicates content has been loaded onto GPU from memory
	GLuint mTextureId;			//!< will not be null if hasUploaded
	int mW;
	int mH;
	int mComp;
	std::string mFilename;
	unsigned char* mImageData;
};

