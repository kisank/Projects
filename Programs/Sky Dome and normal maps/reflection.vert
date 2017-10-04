/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldInverse, WorldProj,ModelTr,NormalTr,B,ShadowViewMatrix,ShadowWorldProjection;

// Computes vectors toward light and eye and outputs them to the fragment shader
  uniform vec3 eyePos,lightPos;
  
  uniform int ReflectionID;
  
in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

out vec3 worldPos;
out vec3 normalVec;
out vec3 lightVec, eyeVec;
 
vec3 R;
vec3 Rn;
float a,b,c,d;

out vec2 texCoord;
out vec4 shadowCoord;
out vec4 position;

out vec3 tangent;

void main()
{ 
	texCoord = vertexTexture; 
	vec3 s = vec3(0,0,0.0);
	/*R = (ModelTr*vertex).xyz-s;
	
	Rn = normalize(R);
	
	a=Rn.x;
	
	b=Rn.y;
	
	c=Rn.z;
	*/
	
	
	if(ReflectionID == 1 )
	{
	
	//s = vec3(0,0,0.9);
	s = vec3(0,0,3.0);
	R = (ModelTr*vertex).xyz-s;
	
	Rn = normalize(R);
	
	a=Rn.x;
	
	b=Rn.y;
	
	c=Rn.z;
	
		d = 1 - c;
		gl_Position = vec4(a/d,b/d , -c* length(R)/100 - 0.9 , 1.0);
	}
	else if(ReflectionID == 2)
	{
//s = vec3(0,0,2.9);
	s = vec3(0,0,3.0);
	R = (ModelTr*vertex).xyz-s;
	
	Rn = normalize(R);
	
	a=Rn.x;
	
	b=Rn.y;
	
	c=Rn.z;
	
		d = 1 + c;
		gl_Position = vec4(a/d,b/d , c* length(R)/100 - 1.0 , 1.0);
	}
	worldPos=(ModelTr*vertex).xyz;
   
   // Computes the points projection on the screen 
    //gl_Position=WorldProj*WorldView*ModelTr*vertex;
	
	
	position=gl_Position;
	
	normalVec = vertexNormal*mat3(NormalTr);
	
	lightVec=lightPos-worldPos;
	
	tangent = vertexTangent * mat3(NormalTr);
	
	//was having plain white value so used inverse for better effects
	vec3 eyePos2=s;//vec3(WorldInverse*vec4(0.0,0.0,0.0,1.0));
    eyeVec =eyePos2-worldPos;

	//Calculate shadow matrix
	mat4 ShadowMatrix = B * ShadowWorldProjection * ShadowViewMatrix;
	shadowCoord = ShadowMatrix * ModelTr * vertex;
}
