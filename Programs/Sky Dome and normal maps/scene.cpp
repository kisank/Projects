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
#include "libs\\glm\\glm\gtc\matrix_transform.hpp"
#include<glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    lightTilt = -30.0;
    lightDist = 40;

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

	//Create shadow shader program from source code files
	shadowProgram = new ShaderProgram();
	shadowProgram->AddShader("shadow.vert", GL_VERTEX_SHADER);
	shadowProgram->AddShader("shadow.frag", GL_FRAGMENT_SHADER);

	glBindAttribLocation(shadowProgram->programId, 0, "vertex");
	glBindAttribLocation(shadowProgram->programId, 1, "vertexNormal");
	glBindAttribLocation(shadowProgram->programId, 2, "vertexTexture");
	glBindAttribLocation(shadowProgram->programId, 3, "vertexTangent");
	shadowProgram->LinkProgram();

	//Create reflection shader from source code file
	reflectionProgram = new ShaderProgram();
	reflectionProgram->AddShader("reflection.vert", GL_VERTEX_SHADER);
	reflectionProgram->AddShader("reflection.frag", GL_FRAGMENT_SHADER);

	glBindAttribLocation(reflectionProgram->programId, 0, "vertex");
	glBindAttribLocation(reflectionProgram->programId, 1, "vertexNormal");
	glBindAttribLocation(reflectionProgram->programId, 2, "vertexTexture");
	glBindAttribLocation(reflectionProgram->programId, 3, "vertexTangent");
	reflectionProgram->LinkProgram();

	//Initialize shadow FBO
	fboObject.CreateFBO(2048, 2048);

	//Initialize reflection FBO's
	fboObjectRef1.CreateFBO(2048, 2048);
	fboObjectRef2.CreateFBO(2048, 2048);

	//Initialize shadow Projection matrix
	shadowProjection = Perspective(2.0, 1.0, 0.01, 2000);

    // Create all the Polygon shapes
    Shape* TeapotPolygons =  new Teapot(12);
    Shape* BoxPolygons = new Ply("box.ply");
    Shape* SpherePolygons = new Sphere(32);
    Shape* GroundPolygons = new Plane(60.0,20);
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
                             vec3(0.3), vec3(0.0), 1);

    Object* ground = new Object(GroundPolygons, groundId,
                                vec3(0.3, 0.2, 0.1), vec3(0.0, 0.0, 0.0), 1);

    Object* sea = new Object(SeaPolygons, seaId,
                             vec3(0.3, 0.3, 1.0), vec3(1.0, 1.0, 1.0), 120);

	//Load the sky texture and set it to sky object,size of radius for the sky is defined below
	skyTexture = new Texture("sky.jpg");
	sky->SetTexture(skyTexture, Identity, 0);

	//Load the surface normal and surface texture and set it to ground object,Size of ground object is defined above in ground polygons
	surfaceTexture = new Texture("Standard_red_pxr128.png");
	surfaceNormal = new Texture("Standard_red_pxr128_normal.png");
	ground->SetTexture(surfaceTexture,Identity, surfaceNormal);

    // Scene is composed of sky, ground, sea, and ... models
	objectRoot->add(sky, Scale(195.0, 190.5, 195.0));
    objectRoot->add(ground);
    //objectRoot->add(sea);

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
    
    invert(&WorldView, &WorldInverse);


/////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////PASS 1///////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////SHADOW CALCULATIONS////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

	int shadowlocation;

	glm::mat4 shadowViewMatrix;

	MAT4 B;
	B = Translate(0.5f, 0.5f, 0.5f)*Scale(0.5f, 0.5f, 0.5f);

	//Use shadow shader
	shadowProgram->Use();
	int shadowProgramId = shadowProgram->programId;

	//Bind FBO
	fboObject.Bind();

	//Clear screen to perform shadow calculations
	//glClearColor(1, 1, 1, 1);
	glClearDepth(1);
	glViewport(0, 0, fboObject.width, fboObject.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Calculate View matrix for shadow using lookAt
	shadowViewMatrix = glm::lookAt(glm::vec3(lPos[0], lPos[1], lPos[2]), vec3(0, 0, 0), vec3(0, 0, 1));
	
	//Enable Cull Face for front to get rid of acne
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	//pass the matrix to the shadow shader
	//Used shadow projection here to have bigger perpective for the shadow map
	shadowlocation = glGetUniformLocation(shadowProgramId, "ShadowWorldProj");
	glUniformMatrix4fv(shadowlocation, 1, GL_TRUE, shadowProjection.Pntr());
	
	//this is the view matrix for shadow from lights POV
	shadowlocation = glGetUniformLocation(shadowProgramId, "ShadowWorldView");
	glUniformMatrix4fv(shadowlocation, 1, GL_FALSE, glm::value_ptr(shadowViewMatrix));

	//draws the object for shadow shader
	objectRoot->Draw(shadowProgram, Identity);

	//Disable cull face once the drawing is finished
	glDisable(GL_CULL_FACE);
	
	//Unuse the shadow shader
	shadowProgram->Unuse();

	//Unbind the FBO
	fboObject.Unbind();
	
	//Clear screen for the lighting shader
	//glClearColor(0, 0, 0, 1);
	glClearDepth(1);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
/////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////Pass 2////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Reflection Calculations////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

	//this bool is used so that teapot is not drawn during reflection,set false in lighting pass
	objectRoot->calculatingReflection = true;

	int reflectionlocation1;

	//Use reflection shader
	reflectionProgram->Use();
	int reflectionProgramId = reflectionProgram->programId;

	//Bind the first reflection Fbo
	fboObjectRef1.Bind();

	//Bind the skytexture to unit 0
	skyTexture->Bind(0);

	//Bind the surface normal to unit 2
	surfaceNormal->Bind(2);

	//Bind the surface texture to unit 1
	surfaceTexture->Bind(1);

	//pass the data generated from the reflection shader to lighting shader to calculate reflections
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, fboObject.texture);
	shadowlocation = glGetUniformLocation(reflectionProgramId, "shadowMap");
	glUniform1i(shadowlocation, 4);
	CHECKERROR;

	//pass the surface texture to the pass1 of reflection shader
	unsigned lc = glGetUniformLocation(reflectionProgramId, "surfaceTexture");
	glUniform1i(lc, 0);

	//pass the surface normal to pass1 of reflection shader
	lc = glGetUniformLocation(reflectionProgramId, "surfaceNormal");
	glUniform1i(lc, 2);
	CHECKERROR;

	//Clear screen to perform reflection calculations
	//glClearColor(0, 0, 0, 1);
	glClearDepth(1);
	glViewport(0, 0, fboObjectRef1.width, fboObjectRef1.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//pass the matrices to the shader
	shadowlocation = glGetUniformLocation(reflectionProgramId, "B");
	glUniformMatrix4fv(shadowlocation, 1, GL_TRUE, B.Pntr());

	reflectionlocation1 = glGetUniformLocation(reflectionProgramId, "WorldProj");
	glUniformMatrix4fv(reflectionlocation1, 1, GL_TRUE, WorldProj.Pntr());

	reflectionlocation1 = glGetUniformLocation(reflectionProgramId, "WorldView");
	glUniformMatrix4fv(reflectionlocation1, 1, GL_TRUE, WorldView.Pntr());

	reflectionlocation1 = glGetUniformLocation(reflectionProgramId, "WorldInverse");
	glUniformMatrix4fv(reflectionlocation1, 1, GL_TRUE, WorldInverse.Pntr());

	reflectionlocation1 = glGetUniformLocation(reflectionProgramId, "eyePos");
	glUniform3fv(reflectionlocation1, 1, &(eye[0]));

	reflectionlocation1 = glGetUniformLocation(reflectionProgramId, "lightPos");
	glUniform3fv(reflectionlocation1, 1, &(lPos[0]));

	reflectionlocation1 = glGetUniformLocation(reflectionProgramId, "Light");
	glUniform3fv(reflectionlocation1, 1, &(Light[0]));

	reflectionlocation1 = glGetUniformLocation(reflectionProgramId, "Ambient");
	glUniform3fv(reflectionlocation1, 1, &(Ambient[0]));

	reflectionlocation1 = glGetUniformLocation(reflectionProgramId, "ShadowViewMatrix");
	glUniformMatrix4fv(reflectionlocation1, 1, GL_FALSE, glm::value_ptr(shadowViewMatrix));

	reflectionlocation1 = glGetUniformLocation(reflectionProgramId, "ShadowWorldProjection");
	glUniformMatrix4fv(reflectionlocation1, 1, GL_TRUE, shadowProjection.Pntr());

	reflectionlocation1 = glGetUniformLocation(reflectionProgramId, "ReflectionID");
	glUniform1i(reflectionlocation1, 1);

	CHECKERROR;

	//Draw
	objectRoot->Draw(reflectionProgram, Identity);

	//Unuse the shader
	reflectionProgram->Unuse();

	//Unbind sky texture
	skyTexture->Unbind();

	//Unbind surface normal texture
	surfaceNormal->Unbind();

	//Unbind surface texture
	surfaceTexture->Unbind();

	//Unbind the first reflection FBO
	fboObjectRef1.Unbind();

	CHECKERROR;
	
	//Clear screen for the other shader
	glClearDepth(1);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

/////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////Pass 3////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Reflection Calculations////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

	int reflectionlocation2;

	//Use the reflection shader
	reflectionProgram->Use();
	
	//Bind the second FBO
	fboObjectRef2.Bind();

	//Bind the sky texture to unit 0
	skyTexture->Bind(0);

	//Bind the surface normal texture to unit 2
	surfaceNormal->Bind(2);

	//Bind the surface texture to unit1
	surfaceTexture->Bind(1);

	//pass the data generated from the reflection shader to reflection shader to calculate the reflections
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, fboObject.texture);
	shadowlocation = glGetUniformLocation(reflectionProgramId, "shadowMap");
	glUniform1i(shadowlocation, 4);
	CHECKERROR;

	//pass the surface texture to pass2 of reflection shader 
	lc = glGetUniformLocation(reflectionProgramId, "surfaceTexture");
	glUniform1i(lc, 0);

	//pass the surface normal to pass2 of reflection shader
	lc = glGetUniformLocation(reflectionProgramId, "surfaceNormal");
	glUniform1i(lc, 2);
	CHECKERROR;

	//Clear screen to perform reflection calculations
	//glClearColor(0,0,0,1);
	glClearDepth(1);
	glViewport(0, 0, fboObjectRef2.width, fboObjectRef2.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//pass the matrices to the shader
	shadowlocation = glGetUniformLocation(reflectionProgramId, "B");
	glUniformMatrix4fv(shadowlocation, 1, GL_TRUE, B.Pntr());

	reflectionlocation2 = glGetUniformLocation(reflectionProgramId, "WorldProj");
	glUniformMatrix4fv(reflectionlocation2, 1, GL_TRUE, WorldProj.Pntr());

	reflectionlocation2 = glGetUniformLocation(reflectionProgramId, "WorldView");
	glUniformMatrix4fv(reflectionlocation2, 1, GL_TRUE, WorldView.Pntr());

	reflectionlocation2 = glGetUniformLocation(reflectionProgramId, "WorldInverse");
	glUniformMatrix4fv(reflectionlocation2, 1, GL_TRUE, WorldInverse.Pntr());

	reflectionlocation2 = glGetUniformLocation(reflectionProgramId, "eyePos");
	glUniform3fv(reflectionlocation2, 1, &(eye[0]));

	reflectionlocation2 = glGetUniformLocation(reflectionProgramId, "lightPos");
	glUniform3fv(reflectionlocation2, 1, &(lPos[0]));

	reflectionlocation2 = glGetUniformLocation(reflectionProgramId, "Light");
	glUniform3fv(reflectionlocation2, 1, &(Light[0]));

	reflectionlocation2 = glGetUniformLocation(reflectionProgramId, "Ambient");
	glUniform3fv(reflectionlocation2, 1, &(Ambient[0]));
	
	reflectionlocation2 = glGetUniformLocation(reflectionProgramId, "ShadowViewMatrix");
	glUniformMatrix4fv(reflectionlocation2, 1, GL_FALSE, glm::value_ptr(shadowViewMatrix));

	reflectionlocation2 = glGetUniformLocation(reflectionProgramId, "ShadowWorldProjection");
	glUniformMatrix4fv(reflectionlocation2, 1, GL_TRUE, shadowProjection.Pntr());

	reflectionlocation2 = glGetUniformLocation(reflectionProgramId, "ReflectionID");
	glUniform1i(reflectionlocation2, 2);

	CHECKERROR;

	//Draw
	objectRoot->Draw(reflectionProgram, Identity);

	//Unuse the refleciton shader
	reflectionProgram->Unuse();

	//Unbind sky texture
	skyTexture->Unbind();

	//Unbind surface normal texture
	surfaceNormal->Unbind();

	//Unbind surface texture
	surfaceTexture->Unbind();

	//unbind the second FBO
	fboObjectRef2.Unbind();

	CHECKERROR;

	//Clear screen for the lighting shader
	//glClearColor(0, 0, 0, 1);
	glClearDepth(1);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

/////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////Pass 4////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////Light Calculations//////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

	//Enable teapot in calculations
	objectRoot->calculatingReflection = false;

    // Use the lighting shader
    lightingProgram->Use();
    int programId = lightingProgram->programId;

	//Bind the sky texture to unit 0
	skyTexture->Bind(0);

	//Bind the surface texture to unit 1
	surfaceTexture->Bind(1);

	//Bind the surfae normal texture to unit 2
	surfaceNormal->Bind(2);

	//pass the data generated from the shadow shader to lighting shader to draw shader
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, fboObject.texture);
	shadowlocation = glGetUniformLocation(programId, "shadowMap");
	glUniform1i(shadowlocation, 4);
	CHECKERROR;

	//pass the data generated from the reflection shader to lighting shader to draw
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, fboObjectRef1.texture);
	reflectionlocation1 = glGetUniformLocation(programId, "samplerTop");
	glUniform1i(reflectionlocation1, 5);
	CHECKERROR;

	//pass the data generated from the reflection shader to lighting shader to draw
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, fboObjectRef2.texture);
	reflectionlocation2 = glGetUniformLocation(programId, "samplerBottom");
	glUniform1i(reflectionlocation2, 6);
	CHECKERROR;

	//Pass the sky texture to lighting shader
	lc = glGetUniformLocation(programId, "skyTexture");
	glUniform1i(lc, 0);

	//Pass the surface texture to lighting shader
	lc = glGetUniformLocation(programId, "surfaceTexture");
	glUniform1i(lc, 1);
	CHECKERROR;
	
	//Pass the surface normal to lighting shader
	lc = glGetUniformLocation(programId, "surfaceNormal");
	glUniform1i(lc, 2);
	CHECKERROR;


	//The Bias matrix used for shadoow matrix calculation,translated and scaled by (0.5f) for all values
	shadowlocation = glGetUniformLocation(programId, "B");
	glUniformMatrix4fv(shadowlocation, 1, GL_TRUE, B.Pntr());

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
    
	loc = glGetUniformLocation(programId, "ShadowViewMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(shadowViewMatrix));

	loc = glGetUniformLocation(programId, "ShadowWorldProjection");
	glUniformMatrix4fv(loc, 1, GL_TRUE, shadowProjection.Pntr());

	loc = glGetUniformLocation(programId, "mode");
    glUniform1i(loc, mode);  
	
	CHECKERROR;

    // Compute any continuously animating objects
    for (std::vector<Object*>::iterator m=animated.begin();  m<animated.end();  m++)
        (*m)->animTr = Rotate(2,atime);

    // Draw all objects
    objectRoot->Draw(lightingProgram, Identity);

	//Unbind the sky texture
	skyTexture->Unbind();

	//Unbind the surface texture
	surfaceTexture->Unbind();

	//Unbind the surface normal texture
	surfaceNormal->Unbind();

	//Unuse the lighting program
    lightingProgram->Unuse();
    CHECKERROR;

	/////////////////Only for Debugging/////////////////////////////////////////////////
	/*glBindFramebuffer(GL_READ_FRAMEBUFFER, fboObjectRef1.fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, 512, 512, 0, 400, 400, 800, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboObjectRef2.fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, 512, 512, 0, 0, 400, 400, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);*/
	///////////////////////////////////////////////////////////////////////////////////
}
