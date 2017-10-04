/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldInverse, WorldProj;
// Computes world position at a pixel used for
// light and eye vector calculations
  uniform mat4 ModelTr;
  
// Computes normal vector
// and outputs to fragment shader
  uniform mat4 NormalTr;

// Computes vectors toward light and eye
// and outputs them to the fragment shader
  uniform vec3 eyePos;
  uniform vec3 lightPos;
  
 

in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

out vec3 worldPos;
out vec3 normalVec;
out vec3  lightVec, eyeVec;
  
//out vec3 normalVec, lightVec;
out vec2 texCoord;
//uniform vec3 lightPos;

void main()
{      
    
    //vec3 worldPos = (ModelTr*vertex).xyz;

    //normalVec = vertexNormal*mat3(NormalTr); 
    //lightVec = lightPos - worldPos;

    texCoord = vertexTexture; 
	
	worldPos=(ModelTr*vertex).xyz;
   // Computes the points projection on the screen 
    gl_Position=WorldProj*WorldView*ModelTr*vertex;
	
	normalVec = vertexNormal*mat3(NormalTr);
	
	lightVec=lightPos-worldPos;
	//was having plain white value so used inverse for better effects
	vec3 eyePos2=vec3(WorldInverse*vec4(0.0,0.0,0.0,1.0));
    eyeVec =eyePos2-worldPos;


}
