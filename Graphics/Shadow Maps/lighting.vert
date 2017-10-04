/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldInverse, WorldProj,ModelTr,NormalTr,B,ShadowViewMatrix,ShadowWorldProjection;

// Computes vectors toward light and eye and outputs them to the fragment shader
  uniform vec3 eyePos,lightPos;
  
in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

out vec3 worldPos;
out vec3 normalVec;
out vec3  lightVec, eyeVec;
  
out vec2 texCoord;
out vec4 shadowCoord;
out vec4 position;

void main()
{ 
	texCoord = vertexTexture; 
	
	worldPos=(ModelTr*vertex).xyz;
   
   // Computes the points projection on the screen 
    gl_Position=WorldProj*WorldView*ModelTr*vertex;
	position=gl_Position;
	
	normalVec = vertexNormal*mat3(NormalTr);
	
	lightVec=lightPos-worldPos;
	
	//was having plain white value so used inverse for better effects
	vec3 eyePos2=vec3(WorldInverse*vec4(0.0,0.0,0.0,1.0));
    eyeVec =eyePos2-worldPos;

	//Calculate shadow matrix
	mat4 ShadowMatrix = B * ShadowWorldProjection * ShadowViewMatrix;
	shadowCoord = ShadowMatrix * ModelTr * vertex;
}
