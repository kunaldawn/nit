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

#include "nitrender/RenderSpec.h"

#include "nit/runtime/NitRuntime.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class RenderSpecDict : public TRuntimeSingleton<RenderSpecDict>
{
public:
	virtual void onInit()
	{
		Register(0, 0, "UNKNOWN");

#define REGISTER_SPEC(GROUP, SPEC)		Register(RenderSpec::GROUP_##GROUP, RenderSpec::GROUP##_##SPEC, #GROUP"."#SPEC)

		REGISTER_SPEC(RENDER, 			BLENDING);
		REGISTER_SPEC(RENDER, 			BLENDING_EXT);
		REGISTER_SPEC(RENDER, 			ANISOTROPY);
		REGISTER_SPEC(RENDER, 			DOT3);
		REGISTER_SPEC(RENDER, 			CUBE_MAP);
		REGISTER_SPEC(RENDER, 			ALPHA_TO_COVERAGE);
		REGISTER_SPEC(RENDER, 			SCISSOR_TEST);
		REGISTER_SPEC(RENDER, 			STENCIL);
		REGISTER_SPEC(RENDER, 			STENCIL_WRAP);
		REGISTER_SPEC(RENDER, 			STENCIL_TWO_SIDED);
		REGISTER_SPEC(RENDER, 			OCCLUSION);
		REGISTER_SPEC(RENDER, 			USER_CLIP_PLANES);
		REGISTER_SPEC(RENDER, 			INFINITE_FAR_PLANE);
		REGISTER_SPEC(RENDER, 			VERTEX_TEXTURE_FETCH);
		REGISTER_SPEC(RENDER, 			MIPMAP_LOD_BIAS);
		REGISTER_SPEC(RENDER, 			POINT_SPRITES);
		REGISTER_SPEC(RENDER, 			POINT_SPRITES_EXT);

		REGISTER_SPEC(DEVICE,			POSSIBLE_LOST);
		REGISTER_SPEC(DEVICE,			MAIN_MEM_SHARES_FRAME);
		REGISTER_SPEC(DEVICE,			MAIN_MEM_SHARES_TEX);
		REGISTER_SPEC(DEVICE,			MAIN_MEM_SHARES_GEOMETRY);
		REGISTER_SPEC(DEVICE,			MULTI_DEVICE);

		REGISTER_SPEC(FORMAT, 			VERTEX_UBYTE4);
		REGISTER_SPEC(FORMAT, 			TEXTURE_DXT);
		REGISTER_SPEC(FORMAT, 			TEXTURE_PVRTC);
		REGISTER_SPEC(FORMAT, 			TEXTURE_ETC1);

		REGISTER_SPEC(BUFFER, 			VERTEX);
		REGISTER_SPEC(BUFFER, 			INDEX);
		REGISTER_SPEC(BUFFER, 			PIXEL);

		REGISTER_SPEC(SHADER, 			PIXEL);
		REGISTER_SPEC(SHADER, 			VERTEX);
		REGISTER_SPEC(SHADER, 			GEOMETRY);
		REGISTER_SPEC(SHADER, 			TEXTURE_UNIT_SHARED);

		REGISTER_SPEC(TEXTURE,			AUTO_MIPMAP);
		REGISTER_SPEC(TEXTURE,			VOLUME);
		REGISTER_SPEC(TEXTURE,			FLOAT);
		REGISTER_SPEC(TEXTURE,			NPOT);
		REGISTER_SPEC(TEXTURE,			NPOT_LIMITED);

		REGISTER_SPEC(RT, 				TEXTURE);
		REGISTER_SPEC(RT, 				VERTEX);
		REGISTER_SPEC(RT, 				TEXTURE_NPOT);
		REGISTER_SPEC(RT, 				MRT);
		REGISTER_SPEC(RT, 				MRT_DIFF_DEPTHS);

		REGISTER_SPEC(GL, 				FBO);

		REGISTER_SPEC(DX, 				PER_STAGE_CONST);

#undef REGISTER_SPEC_NAME
	}

	virtual void onFinish()
	{
		_fromName.clear();
		_toName.clear();
	}

	void Register(int cat, uint32 spec, const String& name)
	{
		_fromName[name] = spec;
		_toName[spec] = name;
	}

	typedef std::map<String, uint32> FromStrMap;
	typedef std::map<uint32, String> ToStrMap;

	FromStrMap							_fromName;
	ToStrMap							_toName;
};

////////////////////////////////////////////////////////////////////////////////

RenderSpec::RenderSpec()
{
	memset(_specBits, 0, sizeof(_specBits));

	_numWorldMatrices					= 0;
	_numTextureUnits					= 0;
	_numVertexBlendMatrices				= 0;
	_numVertexTextureUnits				= 0;
	_maxTextureSize						= 0;
	_maxViewportWidth					= 0;
	_maxViewportHeight					= 0;
	_numDepthBufferBits					= 0;
	_numStencilBufferBits				= 0;
	_numVertexShaderFloatConsts			= 0;
	_numVertexShaderIntConsts			= 0;
	_numVertexShaderBoolConsts			= 0;
	_numGeometryShaderFloatConsts		= 0;
	_numGeometryShaderIntConsts			= 0;
	_numGeometryShaderBoolConsts		= 0;
	_numPixelShaderFloatConsts			= 0;
	_numPixelShaderIntConsts			= 0;
	_numPixelShaderBoolConsts			= 0;
	_numMultiRenderTargets				= 0;
	_maxPointSize						= 0;
}

bool RenderSpec::isCapable(uint32 spec)
{
	if (spec == UNKNOWN) return false;

	int cat = (spec & NIT_RENDER_SPEC_GROUP_MASK) >> NIT_RENDER_SPEC_GROUP_SHIFT;
	int bit = 1 << (spec & NIT_RENDER_SPEC_BIT_MASK);

	if (cat > COUNT_OF(_specBits)) return false;

	bool flag = (_specBits[cat] & bit) == bit;

	return flag;
}

void RenderSpec::setCapable(uint32 spec, bool flag)
{
	int cat = (spec & NIT_RENDER_SPEC_GROUP_MASK) >> NIT_RENDER_SPEC_GROUP_SHIFT;
	int bit = 1 << (spec & NIT_RENDER_SPEC_BIT_MASK);

	uint32 bits = _specBits[cat];

	if (flag)
		bits = bits | bit;
	else
		bits = bits & ~bit;

	_specBits[cat] = bits;
}

String RenderSpec::toName(uint32 spec)
{
	RenderSpecDict& dict = RenderSpecDict::getSingleton();

	RenderSpecDict::ToStrMap::iterator itr = dict._toName.find(spec);
	return itr != dict._toName.end() ? itr->second : dict._toName[UNKNOWN];
}

uint32 RenderSpec::toSpec(const String& name)
{
	RenderSpecDict& dict = RenderSpecDict::getSingleton();

	RenderSpecDict::FromStrMap::iterator itr = dict._fromName.find(name);
	return itr != dict._fromName.end() ? itr->second : UNKNOWN;
}

void RenderSpec::allSpecs(vector<std::pair<String, uint32> >::type& outSpecs)
{
	RenderSpecDict& dict = RenderSpecDict::getSingleton();

	for (RenderSpecDict::FromStrMap::iterator itr = dict._fromName.begin(), end = dict._fromName.end(); itr != end; ++itr)
	{
		outSpecs.push_back(*itr);
	}
}

void RenderSpec::report(LogChannel* ch)
{
	LOG_SCOPE(ch, "++ * RenderSpec Report\n");

	LOG(ch, "- Max %d world matrices\n", _numWorldMatrices);
	LOG(ch, "- Max %d texture units\n", _numTextureUnits);
	LOG(ch, "- Max %d vertex blend matrices\n", _numVertexBlendMatrices);
	LOG(ch, "- Max %d vertex texture units\n", _numVertexTextureUnits);
	LOG(ch, "- Max %d x %d texture size\n", _maxTextureSize, _maxTextureSize);
	LOG(ch, "- Max %d x %d viewport size\n", _maxViewportWidth, _maxViewportHeight);
	LOG(ch, "- %d depth buffer bits\n", _numDepthBufferBits);
	LOG(ch, "- %d stencil buffer bits\n", _numStencilBufferBits);
	LOG(ch, "- Vertex shader const max %d floats, %d ints, %d bools\n", _numVertexShaderFloatConsts, _numVertexShaderIntConsts, _numVertexShaderBoolConsts);
	LOG(ch, "- Geometry shader const max %d floats, %d ints, %d bools\n", _numGeometryShaderFloatConsts, _numGeometryShaderIntConsts, _numGeometryShaderBoolConsts);
	LOG(ch, "- Pixel shader const max %d floats, %d ints, %d bools\n", _numPixelShaderFloatConsts, _numPixelShaderIntConsts, _numPixelShaderBoolConsts);
	LOG(ch, "- max %d multi render targets\n", _numMultiRenderTargets);
	LOG(ch, "- max %d point size\n", _maxPointSize);

	RenderSpecDict& dict = RenderSpecDict::getSingleton();

	for (RenderSpecDict::FromStrMap::iterator itr = dict._fromName.begin(), end = dict._fromName.end(); itr != end; ++itr)
	{
		LOG(ch, "- %s : %s\n", itr->first.c_str(), isCapable(itr->second) ? "true" : "false");
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
