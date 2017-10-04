///////////////////////////////////////////////////////////////////////
// A slight encapsulation of a Frame Buffer Object (i'e' Render
// Target) and its associated texture.  When the FBO is "Bound", the
// output of the graphics pipeline is captured into the texture.  When
// it is "Unbound", the texture is available for use as any normal
// texture.
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
using namespace gl;

#include "fbo.h"

void FBO::CreateFBO(const int w, const int h)
{
	width = w;
	height = h;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Create a render buffer, and attach it to FBO's depth attachment
	unsigned int depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
		width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, depthBuffer);

	// Create texture and attach FBO's color 0 attachment
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_RGBA32F_ARB, width, height,
		0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, texture, 0);

	// Check for completeness/correctness
	int status = (int)glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("FBO Error: %d\n", status);

	// Unbind the fbo.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void FBO::Bind() { glBindFramebuffer(GL_FRAMEBUFFER, fbo); }
void FBO::Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

