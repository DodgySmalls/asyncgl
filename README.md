# Asynchronous Buffer Transfer

This respository contains code to demonstrate a threading scheme to handle asynchronous OpenGL calls, particularily in order to handle streaming data without disrupting rendering.

The current status of the code is that it is a simple example of loading a texture concurrently. In fact, it actually cheats and only renders a pre-loaded synchronous texture currently after joining a single pthread due to oversights in my understanding of which OpenGL calls are threadsafe.

https://youtu.be/WxG6CSiDBGY

The project's goal is to demonstrate arbitrary buffer transfer by using the most applicable example of textures, however textures are merely one kind of data stream among many. The final version will include a more useful data where larger amounts of data are streamed which would otherwise cause the system to pause.

[1] Patrick Cozzie, Christophe Riccio OpenGL Insights https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
