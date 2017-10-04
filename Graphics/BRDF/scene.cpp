//////////////////////////////////////////////////////////////////////
// Defines and draws a scene.  There are two main procedures here:
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////

#include "math.h"
#include <fstream>
#include <stdlib.h>

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
using namespace gl;

#include <freeglut.h>
#include <glu.h>                // For gluErrorString

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
using namespace glm;

#include "shader.h"
#include "shapes.h"
#include "scene.h"
#include "object.h"
#include "texture.h"
#include "transform.h"

const float PI = 3.14159f;
const float rad = PI/180.0f;    // Convert degrees to radians

MAT4 Identity;

const float grndSize = 100.0;    // Island radius;  Minimum about 20;  Maximum 1000 or so
const int   grndTiles = int(grndSize);
const float grndOctaves = 4.0;  // Number of levels of detail to compute
const float grndFreq = 0.03;    // Number of hills per (approx) 50m
const float grndPersistence = 0.03; // Terrain roughness: Slight:0.01  rough:0.05
const float grndLow = -3.0;         // Lowest extent below sea level
const float grndHigh = 5.0;        // Highest extent above sea level

// Simple procedure to print a 4x4 matrix -- useful for debugging
void PrintMat(const MAT4& m)
{
    for (int i=0;  i<4;  i++)
        for (int j=0;  j<4;  j++)
            printf("%9.4f%c", m[i][j], j==3?'\n':' ');
    printf("\n");
}

////////////////////////////////////////////////////////////////////////
// This macro makes it easy to sprinkle checks for OpenGL errors
// through out your code.  Most OpenGL calls can record errors, and a
// careful programmer will check the error status *often*, perhaps as
// often as right after every OpenGL call.  At the very least, once
// per refresh will tell you if something is going wrong.
#define CHECKERROR {\
GLenum err = glGetError(); \
if (err != GL_NO_ERROR) { \
	(stderr, "OpenGL error (at line %d): %s\n", __LINE__, gluErrorString(err));\
	exit(-1);}\
}

vec3 HSV2RGB(const float h, const float s, const float v)
{
    if (s == 0.0)
        return vec3(v,v,v);

    int i = (int)(h*6.0);
    float f = (h*6.0f) - i;
    float p = v*(1.0f - s);
    float q = v*(1.0f - s*f);
    float t = v*(1.0f - s*(1.0f-f));
    if (i%6 == 0)     return vec3(v,t,p);
    else if (i == 1)  return vec3(q,v,p);
    else if (i == 2)  return vec3(p,v,t);
    else if (i == 3)  return vec3(p,q,v);
    else if (i == 4)  return vec3(t,p,v);
    else if (i == 5)  return vec3(v,p,q);
}

Object* SphereOfSpheres(Shape* SpherePolygons)
{
    Object* ob = new Object(NULL, nullId);
    
    for (float angle=0.0;  angle<360.0;  angle+= 18.0)
        for (float row=0.075;  row<PI/2.0;  row += PI/2.0/6.0) {   
            vec3 color = HSV2RGB(angle/360.0, 1.0f-2.0f*row/PI, 1.0f);

            Object* sp = new Object(SpherePolygons, spheresId,
                                    color, vec3(1.0, 1.0, 1.0), 120.0);
            float s = sin(row);
            float c = cos(row);
            ob->add(sp, Rotate(2,angle)*Translate(c,0,s)*Scale(0.075*c,0.075*c,0.075*c));
        }
    return ob;
}

////////////////////////////////////////////////////////////////////////
// Called regularly to update the atime global variable.
float atime = 0.0;
void animate(int value)
{
    atime = 360.0*glutGet((GLenum)GLUT_ELAPSED_TIME)/36000;
    glutPostRedisplay();

    // Schedule next call to this function
    glutTimerFunc(30, animate, 1);
}

////////////////////////////////////////////////////////////////////////
// InitializeScene is called once during setup to create all the
// textures, shape VAOs, and shader programs as well as a number of
// other parameters.
void Scene::InitializeScene()
{
    glEnable(GL_DEPTH_TEST);
    CHECKERROR;

    // FIXME: This is a good place for initializing the transformation
    // values.
	//Set the spin and tilt parameters
	spin = 0.0f;
	tilt = -90.0f;

	//set the translate parameters
	tx = 0.0f;
	ty = 0.0f;

	//set the zoom parameter
	zoom = 150.0f;

	//set the frustum and aspect ratio varaible
	rx = 0.2f;
	ry = 0.2f;

	//distance of front and back clipping plane
	front = 0.1f;
	back = 1000.0f;

	//default value of eye
	eye.x = 0.0f;
	eye.y = 0.0f;
	eye.z = 2.0f;
	
	//default Light values
	Light.x = 3.0f;
	Light.y = 3.0f;
	Light.z = 3.0f;

	//default Ambient light values
	Ambient.x = 0.2f;
	Ambient.y = 0.2f;
	Ambient.z = 0.2f;

	//set this variables only for task 3
	if (task_3) 
	{
		//set the speed of eye
		speed = 0.4;

		//set the initial position of eye vector
		eye.x = 70.0f;
		eye.y = -15.0f;
		eye.z = ground->HeightAt(eye.x, eye.y) + 3.0f;
	}

	keyw = false;
	keya = false;
	keys = false;
	keyd = false;

    objectRoot = new Object(NULL, nullId);
    
    // Set the initial light position parammeters
    lightSpin = 98.0;
    lightTilt = -80.0;
    lightDist = 1000000.0;

    // Enable OpenGL depth-testing
    glEnable(GL_DEPTH_TEST);

    // FIXME:  Change false to true to randomize the room's position.
    ground =  new ProceduralGround(grndSize, grndTiles, grndOctaves, grndFreq,
                                   grndPersistence, grndLow, grndHigh, false);

    basePoint = ground->highPoint;

    // Create the lighting shader program from source code files.
    lightingProgram = new ShaderProgram();
    lightingProgram->AddShader("lighting.vert", GL_VERTEX_SHADER);
    lightingProgram->AddShader("lighting.frag", GL_FRAGMENT_SHADER);

    glBindAttribLocation(lightingProgram->programId, 0, "vertex");
    glBindAttribLocation(lightingProgram->programId, 1, "vertexNormal");
    glBindAttribLocation(lightingProgram->programId, 2, "vertexTexture");
    glBindAttribLocation(lightingProgram->programId, 3, "vertexTangent");
    lightingProgram->LinkProgram();

    // Create all the Polygon shapes
    Shape* TeapotPolygons =  new Teapot(12);
    Shape* BoxPolygons = new Ply("box.ply");
    Shape* SpherePolygons = new Sphere(32);
    Shape* GroundPolygons = ground;
    Shape* SeaPolygons = new Plane(2000.0, 50);

    // Create all the models from which the scene is composed.  Each
    // is created with a polygon shape (possible NULL), a
    // transformation, and the curface lighting parameters Kd, Ks, and
    // alpha.
    Object* anim = new Object(NULL, nullId);
    Object* teapot = new Object(TeapotPolygons, teapotId,
                                vec3(0.5, 0.5, 0.1), vec3(0.5, 0.5, 0.5), 120);
    Object* podium = new Object(BoxPolygons, boxId,
                                vec3(0.25, 0.25, 0.1), vec3(0.3, 0.3, 0.3), 10);
    
    Object* spheres = SphereOfSpheres(SpherePolygons);
    
    
    Object* sky = new Object(SpherePolygons, skyId,
                             vec3(), vec3(), 0);

    Object* ground = new Object(GroundPolygons, groundId,
                                vec3(0.3, 0.2, 0.1), vec3(0.0, 0.0, 0.0), 1);

    Object* sea = new Object(SeaPolygons, seaId,
                             vec3(0.3, 0.3, 1.0), vec3(1.0, 1.0, 1.0), 120);

    // FIXME: This is where you could read in all the textures and
    // associate them with the various objects just created above
    // here.

    // Scene is composed of sky, ground, sea, and ... models
    objectRoot->add(sky, Scale(2000.0, 2000.0, 2000.0));
    objectRoot->add(ground);
    objectRoot->add(sea);

    // Two models have rudimentary animation (constant rotation on Z)
    animated.push_back(anim);

    objectRoot->add(podium, Translate(0,0,0));
    objectRoot->add(anim, Translate(0,0,0));
    objectRoot->add(teapot, Translate(0.0,0,1.5)*TeapotPolygons->modelTr);
    
    anim->add(spheres, Translate(0.0,0,0.0)*Scale(20,20,20));
    
    // Schedule first timed animation call
    glutTimerFunc(30, animate, 1);

    CHECKERROR;
}

////////////////////////////////////////////////////////////////////////
// Procedure DrawScene is called whenever the scene needs to be drawn.
void Scene::DrawScene()
{

    // Calculate the light's position.
    float lPos[4] = {
       basePoint.x+lightDist*cos(lightSpin*rad)*sin(lightTilt*rad),
       basePoint.y+lightDist*sin(lightSpin*rad)*sin(lightTilt*rad),
       basePoint.z+lightDist*cos(lightTilt*rad),
       1.0 };

    // Set the viewport, and clear the screen
    glViewport(0,0,width, height);
    glClearColor(0.5,0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

    // Compute Viewing and Perspective transformations.
    MAT4 WorldProj, WorldView, WorldInverse;

    // FIXME: When you are ready to try interactive viewing, replace
    // the following hardcoded values for WorldProj and WorldView with
    // transformation matrices calculated from variables such as spin,
    // tilt, tr, basePoint, ry, front, and back.
    /*WorldProj[0][0]=  2.368;
    WorldProj[0][1]= -0.800;
    WorldProj[0][2]=  0.000;
    WorldProj[0][3]=  0.000;
    WorldProj[1][0]=  0.384;
    WorldProj[1][1]=  1.136;
    WorldProj[1][2]=  2.194;
    WorldProj[1][3]=  0.000;
    WorldProj[2][0]=  0.281;
    WorldProj[2][1]=  0.831;
    WorldProj[2][2]= -0.480;
    WorldProj[2][3]= 42.451;
    WorldProj[3][0]=  0.281;
    WorldProj[3][1]=  0.831;
    WorldProj[3][2]= -0.480;
    WorldProj[3][3]= 43.442;*/

	eye.z = ground->HeightAt(eye.x, eye.y) + 3.0f;
	rx = ry*width / height;

	WorldProj = Perspective(rx, ry, front, back);

	if(!task_3)
		WorldView = Translate(tx, ty, -zoom) * Rotate(0, tilt + 90) * Rotate(2, -spin);

	if (task_3)
		WorldView = Rotate(0, tilt + 90)*Rotate(2, spin)*Translate(-eye.x, -eye.y, -eye.z);

	float radTheta = spin * PI / 180;

	if(keyw)
		eye += speed*vec3(sin(radTheta), cos(radTheta), 0.0);
	if (keys)
		eye -= speed*vec3(sin(radTheta), cos(radTheta), 0.0);
	if (keyd)
		eye += speed*vec3(cos(radTheta), -sin(radTheta), 0.0);
	if (keya)
		eye -= speed*vec3(cos(radTheta), -sin(radTheta), 0.0);
	
    
   /* WorldView[0][3]= -basePoint[0];
    WorldView[1][3]= -basePoint[1];
    WorldView[2][3]= -basePoint[2];*/
    
    invert(&WorldView, &WorldInverse);

    // Use the lighting shader
    lightingProgram->Use();
    int programId = lightingProgram->programId;

    // Setup the perspective and viewing matrices for normal viewing.
    int loc;
    loc = glGetUniformLocation(programId, "WorldProj");
    glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());
    loc = glGetUniformLocation(programId, "WorldView");
    glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());
    loc = glGetUniformLocation(programId, "WorldInverse");
    glUniformMatrix4fv(loc, 1, GL_TRUE, WorldInverse.Pntr());
	loc = glGetUniformLocation(programId, "eyePos");
	glUniform3fv(loc, 1, &(eye[0]));
    loc = glGetUniformLocation(programId, "lightPos");
    glUniform3fv(loc, 1, &(lPos[0]));  
	loc = glGetUniformLocation(programId, "Light");
	glUniform3fv(loc, 1, &(Light[0]));
	loc = glGetUniformLocation(programId, "Ambient");
	glUniform3fv(loc, 1, &(Ambient[0]));
    loc = glGetUniformLocation(programId, "mode");
    glUniform1i(loc, mode);  
	{
		GLenum err = glGetError(); 
		if (err != GL_NO_ERROR) {
			
				(stderr, "OpenGL error (at line %d): %s\n", __LINE__, gluErrorString(err));
				exit(-1);
		}
	}

    // Compute any continuously animating objects
    for (std::vector<Object*>::iterator m=animated.begin();  m<animated.end();  m++)
        (*m)->animTr = Rotate(2,atime);

    // Draw all objects
    objectRoot->Draw(lightingProgram, Identity);

    lightingProgram->Unuse();
    CHECKERROR;

}
