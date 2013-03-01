/// nit - Noriter Framework
/// A Cross-platform Open Source Integration for Game-oriented Apps
///
/// http://www.github.com/ellongrey/nit
///
/// Copyright (c) 2013 by Jun-hyeok Jang
/// 
/// (see each file to see the different copyright owners)
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// Author: ellongrey

#include "NITRENDER_pch.h"

#include "egl_ios.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <QuartzCore/QuartzCore.h>

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class EAGLDisplayImpl
{
public:
	EAGLDisplayImpl(EGLNativeWindowType display_id);
	
public:
	EGLBoolean							Initialize(EGLint* major, EGLint* minor);
	EGLBoolean							ChooseConfig(const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_configs);
	EGLBoolean							GetConfigAttrib(EGLConfig config, EGLint attribute, EGLint* value);
	EGLContext							CreateContext(EGLConfig config, EGLContext share_context, const EGLint* attrib_list);
	EGLSurface							CreateWindowSurface(EGLConfig config, EGLNativeWindowType win, const EGLint* attrib_list);
	EGLBoolean							MakeCurrent(EGLSurface draw, EGLSurface read, EGLContext ctx);
	EGLBoolean							QuerySurface(EGLSurface surface, EGLint attribute, EGLint* value);
	EGLBoolean							SwapBuffers(EGLSurface surface);
	EGLBoolean							DestroySurface(EGLSurface surface);
	EGLBoolean							DestroyContext(EGLContext ctx);
	EGLBoolean							Terminate();
	
protected:
	GLint								m_BackingWidth;
	GLint								m_BackingHeight;
	
	EAGLRenderingAPI					m_API;
	GLuint								m_DepthFormat;
	GLuint								m_PixelFormat;
	
	bool								m_Opaque;
	bool								m_MultiSamplingResolveBox;
	bool								m_SwapBehaviorPreserved;
	
	// the GLES names for the framebuffer and renderbuffer used to render to this display
	GLuint								m_DefaultFrameBuffer;
	GLuint								m_ColorRenderBuffer;
	GLuint								m_DepthBuffer;

	// MSAA support
	bool								m_MultiSampling;
	GLuint								m_SamplesToUse;
	GLuint								m_MsaaFrameBuffer;
	GLuint								m_MsaaColorBuffer;
	
	CAEAGLLayer*						m_EaglLayer;
	EAGLContext*						m_Context;
};

////////////////////////////////////////////////////////////////////////////////

EAGLDisplayImpl::EAGLDisplayImpl(EGLNativeWindowType display_id)
{
	m_EaglLayer				= (CAEAGLLayer*)display_id;
	m_Context				= nil;
	
	m_BackingWidth			= [m_EaglLayer frame].size.width;
	m_BackingHeight			= [m_EaglLayer frame].size.height;
	
	// ChooseConfig will determine
	m_API					= 0;
	m_DepthFormat			= 0;
	m_PixelFormat			= 0;
	
	m_Opaque				= true;
	m_MultiSamplingResolveBox = false;
	m_SwapBehaviorPreserved = false;
	
	// GL handle
	m_DefaultFrameBuffer	= 0;
	m_ColorRenderBuffer		= 0;
	m_DepthBuffer			= 0;
	
	// multi sample related
	m_MultiSampling			= false;
	m_SamplesToUse			= 0;
	m_MsaaFrameBuffer		= 0;
	m_MsaaColorBuffer		= 0;	
}

EGLBoolean EAGLDisplayImpl::Initialize(EGLint *major, EGLint *minor)
{
	// Emulate EGL version 1.4
	*major = 1;
	*minor = 4;
	
	return EGL_TRUE;
}

EGLBoolean EAGLDisplayImpl::ChooseConfig(const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_configs)
{
	// HACK: apply the config view code wants.
	if (config_size < 1)
		return EGL_FALSE;
	
	// Defaul values
	int alphaSize		= 0;
	int blueSize		= 8;
	int greenSize		= 8;
	int redSize			= 8;
	int depthSize		= 24;
	int samples			= 0;
	int renderableType	= EGL_OPENGL_ES_BIT;
	int surfaceType		= EGL_WINDOW_BIT | EGL_BUFFER_DESTROYED | EGL_MULTISAMPLE_RESOLVE_DEFAULT;
	
	for (const EGLint* itr = attrib_list; *itr; )
	{
		const EGLint attrib = *itr++;
		if (attrib == EGL_NONE) break;
		
		const EGLint param = *itr++;
		
		switch (attrib)
		{
		case EGL_ALPHA_SIZE:			alphaSize = param; break;
		case EGL_BLUE_SIZE:				blueSize = param; break;
		case EGL_GREEN_SIZE:			greenSize = param; break;
		case EGL_RED_SIZE:				redSize = param; break;
		case EGL_DEPTH_SIZE:			depthSize = param; break;
		case EGL_SAMPLES:				samples = param; break;
		case EGL_RENDERABLE_TYPE:		renderableType = param; break;
		case EGL_SURFACE_TYPE:			surfaceType = param; break;
		}
	}
	
	bool ok = true;
	
	if (alphaSize == 8 && blueSize == 8 && greenSize == 8 && redSize == 8)
	{
		m_PixelFormat = GL_RGBA8_OES;
		m_Opaque = false;
	}
	else if (blueSize == 8 && greenSize == 8 && redSize == 8)
	{
		m_PixelFormat = GL_RGBA8_OES;
		m_Opaque = true;
	}
	else if (blueSize == 5 && greenSize == 6 && redSize == 5)
	{
		m_PixelFormat = GL_RGB565_OES;
		m_Opaque = true;
	}
	else 
		ok = false;
	
	if (depthSize == 0)
		m_DepthFormat = 0;
	else if (depthSize == 16)
		m_DepthFormat = GL_DEPTH_COMPONENT16_OES;
	else if (depthSize == 24)
		m_DepthFormat = GL_DEPTH_COMPONENT24_OES;
	else
		ok = false;

	if (renderableType & EGL_OPENGL_ES2_BIT)
		m_API = kEAGLRenderingAPIOpenGLES2;
	else if (renderableType & EGL_OPENGL_ES_BIT)
		m_API = kEAGLRenderingAPIOpenGLES1;
	else
		ok = false;
		
	m_MultiSamplingResolveBox = (surfaceType & EGL_MULTISAMPLE_RESOLVE_BOX_BIT) != 0;
	m_SwapBehaviorPreserved = (surfaceType & EGL_SWAP_BEHAVIOR_PRESERVED_BIT) != 0;
	
	if (ok)
	{
		*num_configs = 1;
		configs[0] = (EGLConfig)this;
		return EGL_TRUE;
	}
	else
	{
		*num_configs = 0;
		return EGL_FALSE;
	}
}

EGLBoolean EAGLDisplayImpl::GetConfigAttrib(EGLConfig config, EGLint attribute, EGLint* value)
{
	if (config != (EGLConfig)this)
		return EGL_FALSE;
	
	switch (attribute)
	{
		case EGL_ALPHA_SIZE:
			if (m_Opaque && m_PixelFormat == GL_RGBA8_OES)	{ *value = 0; return EGL_TRUE; }
			if (m_PixelFormat == GL_RGBA8_OES)			{ *value = 8; return EGL_TRUE; }
			if (m_PixelFormat == GL_RGB565_OES)			{ *value = 0; return EGL_TRUE; }
			return EGL_FALSE;

		case EGL_RED_SIZE:
		case EGL_BLUE_SIZE:
			if (m_PixelFormat == GL_RGBA8_OES)			{ *value = 8; return EGL_TRUE; }
			if (m_PixelFormat == GL_RGB565_OES)			{ *value = 5; return EGL_TRUE; }
			return EGL_FALSE;
			
		case EGL_GREEN_SIZE:
			if (m_PixelFormat == GL_RGBA8_OES)			{ *value = 8; return EGL_TRUE; }
			if (m_PixelFormat == GL_RGB565_OES)			{ *value = 6; return EGL_TRUE; }
			return EGL_FALSE;
			
		case EGL_DEPTH_SIZE:
			if (m_DepthFormat == GL_DEPTH_COMPONENT16_OES)	{ *value = 16; return EGL_TRUE; }
			if (m_DepthFormat == GL_DEPTH_COMPONENT24_OES)	{ *value = 24; return EGL_TRUE; }
			if (m_DepthFormat == 0)							{ *value = 0; return EGL_TRUE; }
			
		case EGL_RENDERABLE_TYPE:
			*value = 0;
			if (m_API == kEAGLRenderingAPIOpenGLES1)	{ *value = EGL_OPENGL_ES_BIT; return EGL_TRUE; }
			if (m_API == kEAGLRenderingAPIOpenGLES2)	{ *value = EGL_OPENGL_ES2_BIT; return EGL_TRUE; }
			return EGL_FALSE;
			
		case EGL_SURFACE_TYPE:
			*value = EGL_WINDOW_BIT; 
			if (m_MultiSamplingResolveBox)	*value |= EGL_MULTISAMPLE_RESOLVE_BOX_BIT;
			if (m_SwapBehaviorPreserved)	*value |= EGL_SWAP_BEHAVIOR_PRESERVED_BIT;
			return EGL_TRUE;
			
		default:
			return EGL_FALSE;
	}
}

EGLContext EAGLDisplayImpl::CreateContext(EGLConfig config, EGLContext share_context, const EGLint* attrib_list)
{
	if (config != (EGLConfig)this)
		return EGL_NO_CONTEXT;
	
	EAGLSharegroup* sharegroup = nil;

	if (share_context)
	{
		// Get sharegroup if share_context specified
		EAGLContext* otherContext = (EAGLContext*)share_context;
		sharegroup = [otherContext sharegroup];
	}
	
	bool opaque = false;
	NSString* colorFormatStr = NULL;
	
	switch (m_PixelFormat)
	{
		case GL_RGB565_OES:
			colorFormatStr = kEAGLColorFormatRGB565; break;
			
		case GL_RGBA8_OES:
			colorFormatStr = kEAGLColorFormatRGBA8; break;
			
		default:
			assert(false);
	}
	
	m_EaglLayer.opaque = m_Opaque;
	m_EaglLayer.drawableProperties = 
		[NSDictionary dictionaryWithObjectsAndKeys:
			[NSNumber numberWithBool: m_SwapBehaviorPreserved], kEAGLDrawablePropertyRetainedBacking,
			colorFormatStr, kEAGLDrawablePropertyColorFormat,
			nil];
	
	if (sharegroup == nil)
		m_Context = [[EAGLContext alloc] initWithAPI: m_API];
	else
		m_Context = [[EAGLContext alloc] initWithAPI: m_API sharegroup: sharegroup];
	
	if (m_Context == nil)
		return EGL_NO_CONTEXT;
	
	return (EGLContext)m_Context;
}

//#define CHECK_GL_ERROR() ({ GLenum __error = glGetError(); if(__error) printf("OpenGL error 0x%04X in %s\n", __error, __FUNCTION__); })

EGLSurface EAGLDisplayImpl::CreateWindowSurface(EGLConfig config, EGLNativeWindowType win, const EGLint* attrib_list)
{
	if (m_Context == nil)
		return EGL_NO_SURFACE;
	
	if (![EAGLContext setCurrentContext: m_Context])
		return EGL_NO_SURFACE;
	
	// Create default framebuffer object.
	glGenFramebuffersOES(1, &m_DefaultFrameBuffer);
	ASSERT_MSG(m_DefaultFrameBuffer, "Can't create default frame buffer");

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_DefaultFrameBuffer);

	// Create color buffer
	glGenRenderbuffersOES(1, &m_ColorRenderBuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_ColorRenderBuffer);

	if (!m_MultiSampling)
	{
		// Link to (actual) screen of EAGL Layer
		[m_Context renderbufferStorage: GL_RENDERBUFFER_OES fromDrawable: m_EaglLayer];

		int backingWidth, backingHeight;
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
	
		m_BackingWidth = backingWidth;
		m_BackingHeight = backingHeight;
	}
	else
	{
		// Assign as Off-screen
		// TODO: Implement MultiSampling further
		glRenderbufferStorageOES(GL_RENDERBUFFER_OES, m_PixelFormat, m_BackingWidth, m_BackingHeight);
	}
	
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, m_ColorRenderBuffer);

	// Create depth buffer
	if (m_DepthFormat)
	{
		glGenRenderbuffersOES(1, &m_DepthBuffer);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_DepthBuffer);
		glRenderbufferStorageOES(GL_RENDERBUFFER_OES, m_DepthFormat, m_BackingWidth, m_BackingHeight);
		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, m_DepthBuffer);

		// default buffer
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_ColorRenderBuffer);
	}
	
	if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
	{
		LOG(0, "!!! Failed to make complete framebuffer object: %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
		return EGL_NO_SURFACE;
	}
	
	return (EGLSurface)this; // Do not distinguish between surface and display.
}

EGLBoolean EAGLDisplayImpl::MakeCurrent(EGLSurface draw, EGLSurface read, EGLContext ctx)
{
	[EAGLContext setCurrentContext: m_Context];

	return EGL_TRUE;
}

EGLBoolean EAGLDisplayImpl::SwapBuffers(EGLSurface surface)
{
	if (m_Context == nil || m_DefaultFrameBuffer == 0)
		return EGL_FALSE;

	if (m_MultiSampling)
	{
		/* Resolve from msaaFramebuffer to resolveFramebuffer */
		glDisable(GL_SCISSOR_TEST);     
		glBindFramebufferOES(GL_READ_FRAMEBUFFER_APPLE, m_MsaaFrameBuffer);
		glBindFramebufferOES(GL_DRAW_FRAMEBUFFER_APPLE, m_DefaultFrameBuffer);
		glResolveMultisampleFramebufferAPPLE();
	}
	
	bool discardFramebuffersSupported = true; // TODO: utilize RenderSpec
	
	if (m_SwapBehaviorPreserved)
		discardFramebuffersSupported = false;
	
	if (discardFramebuffersSupported)
	{
		if (m_MultiSampling)
		{
			if (m_DepthFormat)
			{
				GLenum attachments[] = {GL_COLOR_ATTACHMENT0_OES, GL_DEPTH_ATTACHMENT_OES};
				glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE, 2, attachments);
			}
			else
			{
				GLenum attachments[] = {GL_COLOR_ATTACHMENT0_OES};
				glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE, 1, attachments);
			}
			
			glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_ColorRenderBuffer);
		}
		else if (m_DepthFormat)
		{
			// not MSAA
			GLenum attachments[] = { GL_DEPTH_ATTACHMENT_OES};
			glDiscardFramebufferEXT(GL_FRAMEBUFFER_OES, 1, attachments);
		}
	}
	
	EGLBoolean ok = [m_Context presentRenderbuffer: GL_RENDERBUFFER_OES];
	
	if (ok && m_MultiSampling)
	{
		// We can safely re-bind the framebuffer here, since this will be the
		// 1st instruction of the new render loop
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_MsaaFrameBuffer);
	}

	return ok;
}

EGLBoolean EAGLDisplayImpl::QuerySurface(EGLSurface surface, EGLint attribute, EGLint* value)
{
	if (surface != (EGLSurface)this)
		return EGL_FALSE;
	
	switch (attribute)
	{
		case EGL_WIDTH:					*value = m_BackingWidth; return EGL_TRUE;
		case EGL_HEIGHT:				*value = m_BackingHeight; return EGL_TRUE;
		case EGL_RENDER_BUFFER:			*value = EGL_BACK_BUFFER; return EGL_TRUE;
		case EGL_SWAP_BEHAVIOR:			*value = m_SwapBehaviorPreserved ? EGL_BUFFER_PRESERVED: EGL_BUFFER_DESTROYED; return EGL_TRUE;
		case EGL_MULTISAMPLE_RESOLVE:	*value = m_MultiSamplingResolveBox ? EGL_MULTISAMPLE_RESOLVE_BOX : EGL_MULTISAMPLE_RESOLVE_DEFAULT; return EGL_TRUE;
	}
	
	return EGL_FALSE;
}

EGLBoolean EAGLDisplayImpl::DestroySurface(EGLSurface surface)
{
	if (surface != (EGLSurface)this)
		return EGL_FALSE;

	if (m_ColorRenderBuffer)	glDeleteRenderbuffersOES(1, &m_ColorRenderBuffer);
	if (m_DepthBuffer)			glDeleteRenderbuffersOES(1, &m_DepthBuffer);
	if (m_MsaaColorBuffer)		glDeleteRenderbuffersOES(1, &m_MsaaColorBuffer);
	if (m_MsaaFrameBuffer)		glDeleteRenderbuffersOES(1, &m_MsaaFrameBuffer);
	if (m_DefaultFrameBuffer)	glDeleteFramebuffersOES(1, &m_DefaultFrameBuffer);

	m_DefaultFrameBuffer = 0;
	m_ColorRenderBuffer = 0;
	m_DepthBuffer = 0;
	m_MsaaColorBuffer = 0;
	m_MsaaFrameBuffer = 0;
	
	return EGL_TRUE;
}

EGLBoolean EAGLDisplayImpl::DestroyContext(EGLContext ctx)
{
	if (ctx != m_Context)
		return EGL_FALSE;
	
	DestroySurface((EGLSurface)this);
	
	if ([EAGLContext currentContext] == m_Context)
		[EAGLContext setCurrentContext: nil];
	
	[m_Context release];
	m_Context = nil;
	
	return EGL_TRUE;
}

EGLBoolean EAGLDisplayImpl::Terminate()
{
	DestroyContext(m_Context);
	
	delete this;
	
	return EGL_TRUE;
}

////////////////////////////////////////////////////////////////////////////////

EGLDisplay eglGetDisplay(EGLNativeWindowType display_id)
{
	if (display_id == NULL) return EGL_NO_DISPLAY;
	
	EAGLDisplayImpl* disp = new EAGLDisplayImpl(display_id);
	
	return (EGLDisplay)disp;
}

EGLBoolean eglInitialize(EGLDisplay dpy, EGLint* major, EGLint* minor)
{
	EAGLDisplayImpl* disp = (EAGLDisplayImpl*)dpy;

	if (disp == NULL) return EGL_FALSE;
	
	return disp->Initialize(major, minor);
}

EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_configs)
{
	EAGLDisplayImpl* disp = (EAGLDisplayImpl*)dpy;
	
	if (disp == NULL) return EGL_FALSE;
	
	return disp->ChooseConfig(attrib_list, configs, config_size, num_configs);
}

EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value)
{
	EAGLDisplayImpl* disp = (EAGLDisplayImpl*)dpy;
	
	if (disp == NULL) return EGL_FALSE;

	return disp->GetConfigAttrib(config, attribute, value);
}

EGLBoolean eglTerminate(EGLDisplay dpy)
{
	EAGLDisplayImpl* disp = (EAGLDisplayImpl*)dpy;
	
	if (disp == NULL) return EGL_FALSE;

	return disp->Terminate();
}

EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint* attrib_list)
{
	EAGLDisplayImpl* disp = (EAGLDisplayImpl*)dpy;
	
	if (disp == NULL) return EGL_NO_SURFACE;
	
	return disp->CreateWindowSurface(config, win, attrib_list);
}

EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint* attrib_list)
{
	EAGLDisplayImpl* disp = (EAGLDisplayImpl*)dpy;
	
	if (disp == NULL) return EGL_NO_CONTEXT;
	
	return disp->CreateContext(config, share_context, attrib_list);
}

EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
	EAGLDisplayImpl* disp = (EAGLDisplayImpl*)dpy;
	
	if (disp == NULL) return EGL_FALSE;
	
	return disp->DestroyContext(ctx);
}

EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
	EAGLDisplayImpl* disp = (EAGLDisplayImpl*)dpy;
	
	if (disp == NULL) return EGL_FALSE;
	
	return disp->DestroySurface(surface);
}

EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
	EAGLDisplayImpl* disp = (EAGLDisplayImpl*)dpy;
	
	if (disp == NULL) return EGL_FALSE;
	
	return disp->MakeCurrent(draw, read, ctx);
}

EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
	EAGLDisplayImpl* disp = (EAGLDisplayImpl*)dpy;
	
	if (disp == NULL) return EGL_FALSE;

	return disp->SwapBuffers(surface);
}

EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint* value)
{
	EAGLDisplayImpl* disp = (EAGLDisplayImpl*)dpy;
	
	if (disp == NULL) return EGL_FALSE;
	
	return disp->QuerySurface(surface, attribute, value);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
