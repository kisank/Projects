////////////////////////////////////////////////////////////////////////
// The scene class contains all the parameters needed to define and
// draw the (really) simple scene, including:
//   * Geometry (in a display list)
//   * Light parameters
//   * Material properties
//   * Viewport size parameters
//   * Viewing transformation values
//   * others ...
//
// Some of these parameters are set when the scene is built, and
// others are set by the framework in response to user mouse/keyboard
// interactions.  All of them should be used to draw the scene.

#include "shapes.h"
#include "object.h"
#include "texture.h"
#include "fbo.h"

enum ObjectIds {
    nullId	= 0,
    skyId	= 1,
    seaId	= 2,
    groundId	= 3,
    wallId	= 4,
    boxId	= 5,
    frameId	= 6,
    lPicId	= 7,
    rPicId	= 8,
    teapotId	= 9,
    spheresId	= 10,
};

class Shader;

class Scene
{
public:
    // Viewing transformation parameters (suggested) FIXME: This is a
    // good place for the transformation values which are set by the
    // user mouse/keyboard actions and used in DrawScene to create the
    // transformation matrices.

	//Two variables spin and tilt to record rotations around Z-axis and X-axis respectively
	float spin, tilt;

	//Variables tx, ty, to record left-right and up-down dragging of the scene
	float tx, ty;

	//Variable zoom contains the zoom in/out distance
	float zoom;

	//Variable rx contains aspect ratio,ry contains the perspective frustum height slope
	float rx,ry;

	//Variables front, and back contain distances to the front and back clipping planes
	float front, back;

	//speed of the eye when it moves
	float speed;

	//Vector eye maintains the position of the eye
	vec3 eye;

	//Boolean to change between task 2 and 3
	bool task_3 = false;

	bool keyw, keya, keys, keyd;

	bool reflectionShader;
    ProceduralGround* ground;

    // Light position parameters
    float lightSpin, lightTilt, lightDist;

	//Light and ambient color
	vec3 Light,Ambient;

    vec3 basePoint;  // Records where the scene building is centered.
    int mode; // Extra mode indicator hooked up to number keys and sent to shader
    
    // Viewport
    int width, height;

    // All objects in the scene are children of this single root object.
    Object* objectRoot;
    std::vector<Object*> animated;

    // Shader programs
    ShaderProgram* lightingProgram;
	ShaderProgram* shadowProgram;
	ShaderProgram* reflectionProgram;

	//Create fbo
	FBO fboObject;
	//Fbo for reflection map
	FBO fboObjectRef1;
	FBO fboObjectRef2;

	//Shadow Projection Matrix used for shadow projection
	MAT4 shadowProjection;

    //void append(Object* m) { objects.push_back(m); }

    void InitializeScene();
    void DrawScene();

};
