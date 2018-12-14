# Asynchronous Buffer Transfer

This respository contains code to demonstrate a threading scheme to handle asynchronous OpenGL calls, particularily to sideload textures (easily extensible to any buffer). This sample demonstrates a scheme using only shared context resources, but can trivially be extended to use DMA (through a PBO) for performance, see the extended comment in Entity.cpp.

https://youtu.be/zfm9iLJi6PQ

### Dependencies

* GLEW
* GLFW
* STB Image
* pthread win32

### References

[1] Patrick Cozzie, Christophe Riccio OpenGL Insights https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
