CS541_Project5

In this project,I implemented the sky dome and normal mapping using the previous projects done in the course.

For sky dome,I used the texture provided in the lib folder and applied it to a sphere created in initiliaze
function which basically acts as a sky.The reflection of the sky dome is visible on tea pot. However, the
texture which is used was getting stretch if I had a bigger radius for the sky,so I reduced the size of spehere
to make it fit with the scene. Due to this, the zoom out does not go out too much and the teapot and rotating
spheres would not be visible if zoomed out too much. 

For normal mapping, I tried two different textures. The first one I tried is the one I am using in the project.
The second one was in the handout file. The results I got for the first are better than the one I got for
second so I used the first one. For the best effect of normal map the eye should be somewhere between 20 to 50
degrees. The top view does not produce good effect in the normal map.

I tried to fix the resolution problem by increasing the size of the FBO, I do not know how exactly I can 
improve the resolution of the shadows and reflection. Also, I do not know how to test the effect of normal
mapping in the reflection, but I have included the code required to create the normal map in the normal scene.
I had an issue with the sky dome,when I was using the texture for with radius around 2000, the texture was
spreaded and there was thin white line generated for some reason. So I found something called as lod(Level of
detail I guess) and used that to wrap the image around the sphere(used in lighting shader). 