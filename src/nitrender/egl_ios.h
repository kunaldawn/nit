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

#pragma once

#include "nitrender/nitrender.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// EAGL -> EGL wrapper

typedef int EGLint;
typedef unsigned int EGLBoolean;

typedef void* EGLNativeWindowType;
typedef void* EGLNativeDisplayType;
typedef void* EGLConfig;
typedef void* EGLContext;
typedef void* EGLDisplay;
typedef void* EGLSurface;

#define EGL_VERSION_1_0					1
#define EGL_VERSION_1_1					1
#define EGL_VERSION_1_2					1
#define EGL_VERSION_1_3					1
#define EGL_VERSION_1_4					1

#define EGL_FALSE						0
#define EGL_TRUE						1

#define EGL_DEFAULT_DISPLAY				((EGLNativeDisplayType)0)
#define EGL_NO_CONTEXT					((EGLContext)0)
#define EGL_NO_DISPLAY					((EGLContext)0)
#define EGL_NO_SURFACE					((EGLSurface)0)

#define EGL_SUCCESS						0x3000
#define EGL_NOT_INITIALIZED				0x3001
#define EGL_BAD_ACCESS					0x3002
#define EGL_BAD_ALLOC					0x3003
#define EGL_BAD_ATTRIBUTE				0x3004
#define EGL_BAD_CONFIG					0x3005
#define EGL_BAD_CONTEXT					0x3006
#define EGL_BAD_CURRENT_SURFACE			0x3007
#define EGL_BAD_DISPLAY					0x3008
#define EGL_BAD_MATCH					0x3009
#define EGL_BAD_NATIVE_PIXMAP			0x300A
#define EGL_BAD_NATIVE_WINDOW			0x300B
#define EGL_BAD_PARAMETER				0x300C
#define EGL_BAD_SURFACE					0x300D
#define EGL_CONTEXT_LOST				0x300E

#define EGL_ALPHA_SIZE					0x3021
#define EGL_BLUE_SIZE					0x3022
#define EGL_GREEN_SIZE					0x3023
#define EGL_RED_SIZE					0x3024
#define EGL_DEPTH_SIZE					0x3025
#define EGL_SAMPLES						0x3031
#define EGL_NATIVE_RENDERABLE			0x302D
#define EGL_NATIVE_VISUAL_ID			0x302E
#define EGL_NATIVE_VISUAL_TYPE			0x302F
#define EGL_RENDERABLE_TYPE				0x3040
#define EGL_SURFACE_TYPE				0x3033
#define EGL_NONE						0x3038 // attrib list terminator

// EGL_SURFACE_TYPE mask bits
#define EGL_PBUFFER_BIT					0x0001
#define EGL_PIXMAP_BIT					0x0002
#define EGL_WINDOW_BIT					0x0004
#define EGL_VG_COLORSPACE_LINEAR_BIT	0x0020
#define EGL_VG_ALPHA_FORMAT_PRE_BIT		0x0040
#define EGL_MULTISAMPLE_RESOLVE_BOX_BIT	0x0200
#define EGL_SWAP_BEHAVIOR_PRESERVED_BIT	0x0400

// EGL_RENDERABLE_TYPE mask bits
#define EGL_OPENGL_ES_BIT				0x0001
#define EGL_OPENVG_BIT					0x0002
#define EGL_OPENGL_ES2_BIT				0x0004
#define EGL_OPENGL_BIT					0x0008

// QuerySurface / SurfaceAttrib / CreatePbufferSurface targets
#define EGL_HEIGHT						0x3056
#define EGL_WIDTH						0x3057
#define EGL_LARGEST_PBUFFER				0x3058
#define EGL_TEXTURE_FORMAT				0x3080
#define EGL_TEXTURE_TARGET				0x3081
#define EGL_MIPMAP_TEXTURE				0x3082
#define EGL_MIPMAP_LEVEL				0x3083
#define EGL_RENDER_BUFFER				0x3086
#define EGL_VG_COLORSPACE				0x3087
#define EGL_VG_ALPHA_FORMAT				0x3088
#define EGL_HORIZONTAL_RESOLUTION		0x3090
#define EGL_VERTICAL_RESOLUTION			0x3091
#define EGL_PIXEL_ASPECT_RATIO			0x3092
#define EGL_SWAP_BEHAVIOR				0x3093
#define EGL_MULTISAMPLE_RESOLVE			0x3099

// EGL_RENDER_BUFFER values / BindTexImage / RelaseTexImage buffer targets
#define EGL_BACK_BUFFER					0x3084
#define EGL_SINGLE_BUFFER				0x3085

// Constant scale factor by which fractional display resolutions &
// aspect ratio are scaled when queried as integer values
#define EGL_DISPLAY_SCALING				10000
#define EGL_UNKNOWN						((EGLint)-1)

// EGL_SWAP_BEHAVIOR: Back buffer swap behaviors
#define EGL_BUFFER_PRESERVED			0x3094
#define EGL_BUFFER_DESTROYED			0x3095

// EGL_MULTISAMPLE_RESOLVE: Multisample resolution behaviors
#define EGL_MULTISAMPLE_RESOLVE_DEFAULT	0x309A
#define EGL_MULTISAMPLE_RESOLVE_BOX		0x309B

// BindAPI / QueryAPI targets
#define EGL_OPENGL_ES_API				0x30A0

// GetCurrentSurface targets
#define EGL_DRAW						0x3059
#define EGL_READ						0x305A

EGLDisplay	eglGetDisplay(EGLNativeWindowType display_id);
EGLBoolean	eglInitialize(EGLDisplay dpy, EGLint* major, EGLint* minor);
EGLBoolean	eglChooseConfig(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_config);
EGLBoolean	eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value);
EGLContext	eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint* attrib_list);
EGLSurface	eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint* attrib_list);
EGLBoolean	eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
EGLBoolean	eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint* value);
EGLBoolean	eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
EGLBoolean	eglDestroySurface(EGLDisplay dpy, EGLSurface surface);
EGLBoolean	eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
EGLBoolean	eglTerminate(EGLDisplay dpy);

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
