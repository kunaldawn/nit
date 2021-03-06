#include "nit2d_pch.h"

/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2009		Leonardo Kasperavičius
Copyright (c) 2011      Zynga Inc.

http://www.cocos2d-x.org
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "CCParticleSystemQuad.h"
#include "CCSpriteFrame.h"

#include "nitrender/RenderService.h"

NS_CC_BEGIN;

CCParticleSystemQuad::CCParticleSystemQuad() 
: m_pQuads(NULL)
, m_pIndices(NULL)
{
	m_VertexBuf = new GLESBufferHandle();
}


//implementation CCParticleSystemQuad
// overriding the init method
bool CCParticleSystemQuad::initWithTotalParticles(unsigned int numberOfParticles)
{
	// base initialization
	if( CCParticleSystem::initWithTotalParticles(numberOfParticles) ) 
	{
		// allocating data space
		m_pQuads = new ccV2F_C4B_T2F_Quad[m_uTotalParticles];
		m_pIndices = new GLushort[m_uTotalParticles * 6];

		if( !m_pQuads || !m_pIndices) 
		{
			CCLOG("cocos2d: Particle system: not enough memory");
			if( m_pQuads )
				delete [] m_pQuads;
			if(m_pIndices)
				delete [] m_pIndices;
			this->release();
			return NULL;
		}

		// initialize only once the texCoords and the indices
        if (m_pTexture)
        {
            this->initTexCoordsWithRect(CCRectMake((float)0, (float)0, (float)m_pTexture->getPixelsWide(), (float)m_pTexture->getPixelsHigh()));
        }
        else
        {
            this->initTexCoordsWithRect(CCRectMake((float)0, (float)0, (float)1, (float)1));
        }

		this->initIndices();

		return true;
	}
	return false;
}
CCParticleSystemQuad::~CCParticleSystemQuad()
{
	CC_SAFE_DELETE_ARRAY(m_pQuads);
	CC_SAFE_DELETE_ARRAY(m_pIndices);
}

// implementation CCParticleSystemQuad
CCParticleSystemQuad * CCParticleSystemQuad::particleWithFile(StreamSource* plistSource)
{
    CCParticleSystemQuad *pRet = new CCParticleSystemQuad();
    if (pRet && pRet->initWithFile(plistSource))
    {
        pRet->autorelease();
        return pRet;
    }

	CC_SAFE_DELETE(pRet)
	return pRet;
}

// pointRect should be in Texture coordinates, not pixel coordinates
void CCParticleSystemQuad::initTexCoordsWithRect(const CCRect& pointRect)
{
    // convert to pixels coords

    CCRect rect = pointRect.ToPixels();

    GLfloat wide = (GLfloat) pointRect.size.width;
    GLfloat high = (GLfloat) pointRect.size.height;

    if (m_pTexture)
    {
        wide = (GLfloat)m_pTexture->getPixelsWide();
        high = (GLfloat)m_pTexture->getPixelsHigh();
    }

#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
    GLfloat left = (rect.origin.x*2+1) / (wide*2);
    GLfloat bottom = (rect.origin.y*2+1) / (high*2);
    GLfloat right = left + (rect.size.width*2-2) / (wide*2);
    GLfloat top = bottom + (rect.size.height*2-2) / (high*2);
#else
    GLfloat left = rect.origin.x / wide;
    GLfloat bottom = rect.origin.y / high;
    GLfloat right = left + rect.size.width / wide;
    GLfloat top = bottom + rect.size.height / high;
#endif // ! CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL

	float scale = CC_CONTENT_SCALE_FACTOR();
	if (scale != 1.0f)
	{
		scale = 1.0f / scale;

		left *= scale;
		bottom *= scale;
		right *= scale;
		top *= scale;
	}

	// Important. Texture in cocos2d are inverted, so the Y component should be inverted
	CC_SWAP( top, bottom, float);

	for(unsigned int i=0; i<m_uTotalParticles; i++) 
	{
		// bottom-left vertex:
		m_pQuads[i].bl.texCoords.u = left;
		m_pQuads[i].bl.texCoords.v = bottom;
		// bottom-right vertex:
		m_pQuads[i].br.texCoords.u = right;
		m_pQuads[i].br.texCoords.v = bottom;
		// top-left vertex:
		m_pQuads[i].tl.texCoords.u = left;
		m_pQuads[i].tl.texCoords.v = top;
		// top-right vertex:
		m_pQuads[i].tr.texCoords.u = right;
		m_pQuads[i].tr.texCoords.v = top;
	}
}

void CCParticleSystemQuad::setTextureWithRect(CCTexture2D *texture, const CCRect& rect)
{
	// Only update the texture if is different from the current one
	if (!m_pTexture->isSameWith(texture))
	{
		CCParticleSystem::setTexture(texture);
	}

	this->initTexCoordsWithRect(rect);
}

void CCParticleSystemQuad::setTexture(CCTexture2D* texture)
{
	const CCSize& s = texture->getContentSize();
	this->setTextureWithRect(texture, CCRectMake(0, 0, s.width, s.height));
}

void CCParticleSystemQuad::setDisplayFrame(CCSpriteFrame *spriteFrame)
{
	CCAssert( CCPoint::CCPointEqualToPoint( spriteFrame->getOffset() , CCPointZero ), "QuadParticle only supports SpriteFrames with no offsets");

	// update texture before updating texture rect
	if (!m_pTexture->isSameWith(spriteFrame->getTexture()))
	{
		this->setTexture(spriteFrame->getTexture());
	}
}

void CCParticleSystemQuad::initIndices()
{
	for(unsigned int i = 0; i < m_uTotalParticles; ++i)
	{
        const unsigned int i6 = i*6;
        const unsigned int i4 = i*4;
		m_pIndices[i6+0] = (GLushort) i4+0;
		m_pIndices[i6+1] = (GLushort) i4+1;
		m_pIndices[i6+2] = (GLushort) i4+2;

		m_pIndices[i6+5] = (GLushort) i4+1;
		m_pIndices[i6+4] = (GLushort) i4+2;
		m_pIndices[i6+3] = (GLushort) i4+3;
	}
}

void CCParticleSystemQuad::updateQuadWithParticle(tCCParticle* particle, const CCPoint& newPosition)
{
	// colors
    ccV2F_C4B_T2F_Quad *quad = &(m_pQuads[m_uParticleIdx]);

	ccColor4B color = {(GLubyte)(particle->color.r * 255), (GLubyte)(particle->color.g * 255), (GLubyte)(particle->color.b * 255), 
		(GLubyte)(particle->color.a * 255)};
	quad->bl.colors = color;
	quad->br.colors = color;
	quad->tl.colors = color;
	quad->tr.colors = color;

	// vertices
	GLfloat size_2 = particle->size/2;
	if( particle->rotation ) 
	{
		GLfloat x1 = -size_2;
		GLfloat y1 = -size_2;

		GLfloat x2 = size_2;
		GLfloat y2 = size_2;
		GLfloat x = newPosition.x;
		GLfloat y = newPosition.y;

		GLfloat r = (GLfloat)-CC_DEGREES_TO_RADIANS(particle->rotation);
		GLfloat cr = cosf(r);
		GLfloat sr = sinf(r);
		GLfloat ax = x1 * cr - y1 * sr + x;
		GLfloat ay = x1 * sr + y1 * cr + y;
		GLfloat bx = x2 * cr - y1 * sr + x;
		GLfloat by = x2 * sr + y1 * cr + y;
		GLfloat cx = x2 * cr - y2 * sr + x;
		GLfloat cy = x2 * sr + y2 * cr + y;
		GLfloat dx = x1 * cr - y2 * sr + x;
		GLfloat dy = x1 * sr + y2 * cr + y;

		// bottom-left
		quad->bl.vertices.x = ax;
		quad->bl.vertices.y = ay;

		// bottom-right vertex:
		quad->br.vertices.x = bx;
		quad->br.vertices.y = by;

		// top-left vertex:
		quad->tl.vertices.x = dx;
		quad->tl.vertices.y = dy;

		// top-right vertex:
		quad->tr.vertices.x = cx;
		quad->tr.vertices.y = cy;
	} else {
		// bottom-left vertex:
		quad->bl.vertices.x = newPosition.x - size_2;
		quad->bl.vertices.y = newPosition.y - size_2;

		// bottom-right vertex:
		quad->br.vertices.x = newPosition.x + size_2;
		quad->br.vertices.y = newPosition.y - size_2;

		// top-left vertex:
		quad->tl.vertices.x = newPosition.x - size_2;
		quad->tl.vertices.y = newPosition.y + size_2;

		// top-right vertex:
		quad->tr.vertices.x = newPosition.x + size_2;
		quad->tr.vertices.y = newPosition.y + size_2;				
	}
}

// overriding draw method
void CCParticleSystemQuad::draw(RenderContext* ctx)
{	
	CCParticleSystem::draw(ctx);

	if (!activateBuffers(ctx)) return;

	// Default GL states: GL_TEXTURE_2D, GL_VERTEX_ARRAY, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY
	// Needed states: GL_TEXTURE_2D, GL_VERTEX_ARRAY, GL_COLOR_ARRAY, GL_TEXTURE_COORD_ARRAY
	// Unneeded states: -
	glBindTexture(GL_TEXTURE_2D, m_pTexture->activate(ctx));

#define kQuadSize sizeof(m_pQuads[0].bl)

    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuf->useHandle(ctx));
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_pQuads[0])*m_uParticleCount, m_pQuads);

	glVertexPointer(2,GL_FLOAT, kQuadSize, 0);

	glColorPointer(4, GL_UNSIGNED_BYTE, kQuadSize, (GLvoid*) offsetof(ccV2F_C4B_T2F,colors) );

	glTexCoordPointer(2, GL_FLOAT, kQuadSize, (GLvoid*) offsetof(ccV2F_C4B_T2F,texCoords) );

    bool newBlend = (m_tBlendFunc.src != CC_BLEND_SRC || m_tBlendFunc.dst != CC_BLEND_DST) ? true : false;
	if( newBlend ) 
	{
		glBlendFunc( m_tBlendFunc.src, m_tBlendFunc.dst );
	}

    CCAssert( m_uParticleIdx == m_uParticleCount, "Abnormal error in particle quad");

	glDrawElements(GL_TRIANGLES, (GLsizei)(m_uParticleIdx*6), GL_UNSIGNED_SHORT, m_pIndices);	

	// restore blend state
	if( newBlend )
		glBlendFunc( CC_BLEND_SRC, CC_BLEND_DST );

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// restore GL default state
	// -
}

bool CCParticleSystemQuad::activateBuffers(RenderContext* ctx)
{
	if (m_VertexBuf->isValid(ctx)) return true;

	// create the VBO buffer
	if (!m_VertexBuf->generate(ctx))
		return false; // device not ready yet

	// initial binding
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuf->useHandle(ctx));
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_pQuads[0])*m_uTotalParticles, m_pQuads, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

NS_CC_END;
