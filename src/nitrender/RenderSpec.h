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

#define NIT_RENDER_SPEC_GROUP_MASK		0xFFFF0000
#define NIT_RENDER_SPEC_GROUP_SHIFT	16
#define NIT_RENDER_SPEC_BIT_MASK		0x0000001F
#define NIT_RENDER_SPEC(GROUP, BIT)	(BIT) | (GROUP << NIT_RENDER_SPEC_GROUP_SHIFT)

class NITRENDER_API RenderSpec : public WeakSupported
{
public:
	enum Groups
	{
		GROUP_RENDER					= 0,
		GROUP_DEVICE,
		GROUP_FORMAT,
		GROUP_BUFFER,
		GROUP_SHADER,
		GROUP_TEXTURE,
		GROUP_RT,
		GROUP_GL,
		GROUP_DX,

		_NUM_GROUPS
	};

	enum Specs							// max 32 spec for each group (0 ~ 31)
	{
		UNKNOWN							= 0,

		RENDER_BLENDING					= NIT_RENDER_SPEC(GROUP_RENDER, 0),
		RENDER_BLENDING_EXT				= NIT_RENDER_SPEC(GROUP_RENDER, 1),
		RENDER_ANISOTROPY				= NIT_RENDER_SPEC(GROUP_RENDER, 2),
		RENDER_DOT3						= NIT_RENDER_SPEC(GROUP_RENDER, 3),
		RENDER_CUBE_MAP					= NIT_RENDER_SPEC(GROUP_RENDER, 4),
		RENDER_ALPHA_TO_COVERAGE		= NIT_RENDER_SPEC(GROUP_RENDER, 5),
		RENDER_SCISSOR_TEST				= NIT_RENDER_SPEC(GROUP_RENDER, 6),
		RENDER_STENCIL					= NIT_RENDER_SPEC(GROUP_RENDER, 7),
		RENDER_STENCIL_WRAP				= NIT_RENDER_SPEC(GROUP_RENDER, 8),
		RENDER_STENCIL_TWO_SIDED		= NIT_RENDER_SPEC(GROUP_RENDER, 9),
		RENDER_OCCLUSION				= NIT_RENDER_SPEC(GROUP_RENDER, 10),
		RENDER_USER_CLIP_PLANES			= NIT_RENDER_SPEC(GROUP_RENDER, 11),
		RENDER_INFINITE_FAR_PLANE		= NIT_RENDER_SPEC(GROUP_RENDER, 12),
		RENDER_VERTEX_TEXTURE_FETCH		= NIT_RENDER_SPEC(GROUP_RENDER, 13),
		RENDER_MIPMAP_LOD_BIAS			= NIT_RENDER_SPEC(GROUP_RENDER, 14),
		RENDER_POINT_SPRITES			= NIT_RENDER_SPEC(GROUP_RENDER, 15),
		RENDER_POINT_SPRITES_EXT		= NIT_RENDER_SPEC(GROUP_RENDER, 16),

		DEVICE_POSSIBLE_LOST			= NIT_RENDER_SPEC(GROUP_DEVICE, 0),
		DEVICE_MAIN_MEM_SHARES_FRAME	= NIT_RENDER_SPEC(GROUP_DEVICE, 1),
		DEVICE_MAIN_MEM_SHARES_TEX		= NIT_RENDER_SPEC(GROUP_DEVICE, 2),
		DEVICE_MAIN_MEM_SHARES_GEOMETRY	= NIT_RENDER_SPEC(GROUP_DEVICE, 3),
		DEVICE_MULTI_DEVICE				= NIT_RENDER_SPEC(GROUP_DEVICE, 4),

		FORMAT_VERTEX_UBYTE4			= NIT_RENDER_SPEC(GROUP_FORMAT, 0),
		FORMAT_TEXTURE_DXT				= NIT_RENDER_SPEC(GROUP_FORMAT, 1),
		FORMAT_TEXTURE_PVRTC			= NIT_RENDER_SPEC(GROUP_FORMAT, 2),
		FORMAT_TEXTURE_ETC1				= NIT_RENDER_SPEC(GROUP_FORMAT, 3),

		BUFFER_VERTEX					= NIT_RENDER_SPEC(GROUP_BUFFER, 0),
		BUFFER_INDEX					= NIT_RENDER_SPEC(GROUP_BUFFER, 1),
		BUFFER_PIXEL					= NIT_RENDER_SPEC(GROUP_BUFFER, 2),

		SHADER_PIXEL					= NIT_RENDER_SPEC(GROUP_SHADER, 0),
		SHADER_VERTEX					= NIT_RENDER_SPEC(GROUP_SHADER, 1),
		SHADER_GEOMETRY					= NIT_RENDER_SPEC(GROUP_SHADER, 2),
		SHADER_TEXTURE_UNIT_SHARED		= NIT_RENDER_SPEC(GROUP_SHADER, 3),

		TEXTURE_AUTO_MIPMAP				= NIT_RENDER_SPEC(GROUP_TEXTURE, 0),
		TEXTURE_VOLUME					= NIT_RENDER_SPEC(GROUP_TEXTURE, 1),
		TEXTURE_FLOAT					= NIT_RENDER_SPEC(GROUP_TEXTURE, 2),
		TEXTURE_NPOT					= NIT_RENDER_SPEC(GROUP_TEXTURE, 3),
		TEXTURE_NPOT_LIMITED			= NIT_RENDER_SPEC(GROUP_TEXTURE, 4),

		RT_TEXTURE						= NIT_RENDER_SPEC(GROUP_RT, 0),
		RT_VERTEX						= NIT_RENDER_SPEC(GROUP_RT, 1),
		RT_TEXTURE_NPOT					= NIT_RENDER_SPEC(GROUP_RT, 2),
		RT_MRT							= NIT_RENDER_SPEC(GROUP_RT, 3),
		RT_MRT_DIFF_DEPTHS				= NIT_RENDER_SPEC(GROUP_RT, 4),

		GL_FBO							= NIT_RENDER_SPEC(GROUP_GL, 0),

		DX_PER_STAGE_CONST				= NIT_RENDER_SPEC(GROUP_DX, 0),
	};

public:
	static String						toName(uint32 spec);
	static uint32						toSpec(const String& name);

	static void							allSpecs(vector<std::pair<String, uint32> >::type& outSpecs);

public:
	bool								isCapable(uint32 spec);

	uint16								getNumWorldMatrices()					{ return _numWorldMatrices; }
	uint16								getNumTextureUnits()					{ return _numTextureUnits; }
	uint16								getNumVertexBlendMatrices()				{ return _numVertexBlendMatrices; }
	uint16								getNumVertexTextureUnits()				{ return _numVertexTextureUnits; }
	uint16								getMaxTextureSize()						{ return _maxTextureSize; }
	uint16								getMaxViewportWidth()					{ return _maxViewportWidth; }
	uint16								getMaxViewportHeight()					{ return _maxViewportHeight; }
	uint16								getNumDepthBufferBits()					{ return _numDepthBufferBits; }
	uint16								getNumStencilBufferBits()				{ return _numStencilBufferBits; }
	uint16								getNumVertexShaderFloatConsts()			{ return _numVertexShaderFloatConsts; }
	uint16								getNumVertexShaderIntConsts()			{ return _numVertexShaderIntConsts; }
	uint16								getNumVertexShaderBoolConsts()			{ return _numVertexShaderBoolConsts; }
	uint16								getNumGeometryShaderFloatConsts()		{ return _numGeometryShaderFloatConsts; }
	uint16								getNumGeometryShaderIntConsts()			{ return _numGeometryShaderIntConsts; }
	uint16								getNumGeometryShaderBoolConsts()		{ return _numGeometryShaderBoolConsts; }
	uint16								getNumPixelShaderFloatConsts()			{ return _numPixelShaderFloatConsts; }
	uint16								getNumPixelShaderIntConsts()			{ return _numPixelShaderIntConsts; }
	uint16								getNumPixelShaderBoolConsts()			{ return _numPixelShaderBoolConsts; }
	uint16								getNumMultiRenderTargets()				{ return _numMultiRenderTargets; }

	virtual void						report(LogChannel* ch = NULL);

protected:
	RenderSpec();

	void								setCapable(uint32 spec, bool flag=true);

	uint32								_specBits[_NUM_GROUPS];

	uint16								_numWorldMatrices;
	uint16								_numTextureUnits;
	uint16								_numVertexBlendMatrices;
	uint16								_numVertexTextureUnits;

	uint16								_maxTextureSize;
	uint16								_maxViewportWidth;
	uint16								_maxViewportHeight;

	uint16								_numDepthBufferBits;
	uint16								_numStencilBufferBits;

	uint16								_numVertexShaderFloatConsts;
	uint16								_numVertexShaderIntConsts;
	uint16								_numVertexShaderBoolConsts;

	uint16								_numGeometryShaderFloatConsts;
	uint16								_numGeometryShaderIntConsts;
	uint16								_numGeometryShaderBoolConsts;

	uint16								_numPixelShaderFloatConsts;
	uint16								_numPixelShaderIntConsts;
	uint16								_numPixelShaderBoolConsts;

	uint16								_numMultiRenderTargets;

	float								_maxPointSize;
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
