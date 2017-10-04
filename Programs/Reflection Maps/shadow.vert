#version 330

uniform mat4 ModelTr,ShadowWorldProj,ShadowWorldView;

in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

out vec4 position;

void main()
{
	gl_Position= ShadowWorldProj*ShadowWorldView*ModelTr*vertex;
	position=gl_Position;
}