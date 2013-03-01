#include "nit2d_pch.h"

/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
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

// ideas taken from:
//	 . The ocean spray in your face [Jeff Lander]
//		http://www.double.co.nz/dust/col0798.pdf
//	 . Building an Advanced Particle System [John van der Burg]
//		http://www.gamasutra.com/features/20000623/vanderburg_01.htm
//   . LOVE game engine
//		http://love2d.org/
//
//
// Radius mode support, from 71 squared
//		http://particledesigner.71squared.com/
//
// IMPORTANT: Particle Designer is supported by cocos2d, but
// 'Radius Mode' in Particle Designer uses a fixed emit rate of 30 hz. Since that can't be guarateed in cocos2d,
//  cocos2d uses a another approach, but the results are almost identical. 
//

#include "CCParticleSystem.h"
#include "ccTypes.h"
#include "CCTextureCache.h"
#include "support/base64.h"
#include "CCPointExtension.h"

NS_CC_BEGIN;

// ideas taken from:
//	 . The ocean spray in your face [Jeff Lander]
//		http://www.double.co.nz/dust/col0798.pdf
//	 . Building an Advanced Particle System [John van der Burg]
//		http://www.gamasutra.com/features/20000623/vanderburg_01.htm
//   . LOVE game engine
//		http://love2d.org/
//
//
// Radius mode support, from 71 squared
//		http://particledesigner.71squared.com/
//
// IMPORTANT: Particle Designer is supported by cocos2d, but
// 'Radius Mode' in Particle Designer uses a fixed emit rate of 30 hz. Since that can't be guarateed in cocos2d,
//  cocos2d uses a another approach, but the results are almost identical. 
//

CCParticleSystem::CCParticleSystem()
	:m_fElapsed(0)
	,m_pParticles(NULL)
	,m_fEmitCounter(0)
	,m_uParticleIdx(0)
	,m_bIsActive(true)
	,m_uParticleCount(0)
	,m_fDuration(0)
	,m_tSourcePosition(CCPointZero)
	,m_tPosVar(CCPointZero)
	,m_fLife(0)
	,m_fLifeVar(0)
	,m_fAngle(0)
	,m_fAngleVar(0)
	,m_fStartSize(0)
	,m_fStartSizeVar(0)
	,m_fEndSize(0)
	,m_fEndSizeVar(0)
	,m_fStartSpin(0)
	,m_fStartSpinVar(0)
	,m_fEndSpin(0)
	,m_fEndSpinVar(0)
	,m_fEmissionRate(0)
	,m_uTotalParticles(0)
	,m_pTexture(NULL)
	,m_bIsBlendAdditive(false)
	,m_ePositionType(kCCPositionTypeFree)
	,m_bIsAutoRemoveOnFinish(false)
	,m_nEmitterMode(kCCParticleModeGravity)
{
	modeA.gravity = CCPointZero;
	modeA.speed = 0;
	modeA.speedVar = 0;
	modeA.tangentialAccel = 0;
	modeA.tangentialAccelVar = 0;
	modeA.radialAccel = 0;
	modeA.radialAccelVar = 0;
	modeB.startRadius = 0;
	modeB.startRadiusVar = 0;
	modeB.endRadius = 0;
	modeB.endRadiusVar = 0;			
	modeB.rotatePerSecond = 0;
	modeB.rotatePerSecondVar = 0;
	m_tBlendFunc.src = CC_BLEND_SRC;
	m_tBlendFunc.dst = CC_BLEND_DST;
}

bool CCParticleSystem::initWithFile(StreamSource* plistSource)
{
	m_PlistSource = plistSource;

	return initWithRecord(DataValue::load(plistSource));
}

bool CCParticleSystem::initWithRecord(DataRecord* rec)
{
	int maxParticles = rec->get("maxParticles");

	if (!initWithTotalParticles(maxParticles)) // self, not super
		NIT_THROW(EX_MEMORY);

	// NOTE: Remembering DataKey is fast but that's effective when iterating large looping. 
	// We do not use loop here, so implement by a plain string as key.

	// angle
	m_fAngle							= rec->get("angle");
	m_fAngleVar							= rec->get("angleVariance");

	// duration
	m_fDuration							= rec->get("duration");

	// blend function
	m_tBlendFunc.src 					= rec->get("blendFuncSource").toInt();
	m_tBlendFunc.dst 					= rec->get("blendFuncDestination").toInt();
	m_bIsBlendAdditive					= rec->get("blendAdditive");

	// color
	m_tStartColor.r 					= rec->get("startColorRed");
	m_tStartColor.g 					= rec->get("startColorGreen");
	m_tStartColor.b 					= rec->get("startColorBlue");
	m_tStartColor.a 					= rec->get("startColorAlpha");

	m_tStartColorVar.r 					= rec->get("startColorVarianceRed");
	m_tStartColorVar.g 					= rec->get("startColorVarianceGreen");
	m_tStartColorVar.b 					= rec->get("startColorVarianceBlue");
	m_tStartColorVar.a 					= rec->get("startColorVarianceAlpha");
	
	m_tEndColor.r 						= rec->get("finishColorRed");
	m_tEndColor.g 						= rec->get("finishColorGreen");
	m_tEndColor.b 						= rec->get("finishColorBlue");
	m_tEndColor.a 						= rec->get("finishColorAlpha");

	m_tEndColorVar.r 					= rec->get("finishColorVarianceRed");
	m_tEndColorVar.g 					= rec->get("finishColorVarianceGreen");
	m_tEndColorVar.b 					= rec->get("finishColorVarianceBlue");
	m_tEndColorVar.a 					= rec->get("finishColorVarianceAlpha");

	// particle size
	m_fStartSize						= rec->get("startParticleSize");
	m_fStartSizeVar						= rec->get("startParticleSizeVariance");
	m_fEndSize							= rec->get("finishParticleSize");
	m_fEndSizeVar						= rec->get("finishParticleSizeVariance");

	// position
	float x 							= rec->get("sourcePositionx");
	float y 							= rec->get("sourcePositiony");
	setPosition(ccp(x, y));
	m_tPosVar.x 						= rec->get("sourcePositionVariancex");
	m_tPosVar.y 						= rec->get("sourcePositionVariancey");

	// spinning
	m_fStartSpin						= rec->get("rotationStart");
	m_fStartSpinVar						= rec->get("rotationStartVariance");
	m_fEndSpin							= rec->get("rotationEnd");
	m_fEndSpinVar						= rec->get("rotationEndVariance");

	m_nEmitterMode						= rec->get("emitterType");

	if (m_nEmitterMode == kCCParticleModeGravity)
	{
		// Mode A: Gravity + tangential accel + radial accel

		// gravity
		modeA.gravity.x 				= rec->get("gravityx");
		modeA.gravity.y 				= rec->get("gravityy");

		// speed
		modeA.speed						= rec->get("speed");
		modeA.speedVar					= rec->get("speedVariance");

		// radial acceleration
		modeA.radialAccel				= rec->get("radialAcceleration");
		modeA.radialAccelVar			= rec->get("radialAccelVariance");

		// tangential acceleration
		modeA.tangentialAccel			= rec->get("tangentialAcceleration");
		modeA.tangentialAccelVar		= rec->get("tangentialAccelVariance");
	}
	else if (m_nEmitterMode == kCCParticleModeRadius)
	{
		// Mode B: radius movement
		modeB.startRadius				= rec->get("maxRadius");
		modeB.startRadiusVar			= rec->get("maxRadiusVariance");
		modeB.endRadius					= rec->get("minRadius");
		modeB.endRadiusVar				= rec->get("minRadiusVariance");
		modeB.rotatePerSecond			= rec->get("rotatePerSecond");
		modeB.rotatePerSecondVar		= rec->get("rotatePerSecondVariance");
	}
	else
	{
		NIT_THROW_FMT(EX_SYNTAX, "CCParticleSystem: Invalid emitter type");
	}

	// life span
	m_fLife								= rec->get("particleLifespan");
	m_fLifeVar							= rec->get("particleLifespanVariance");
	
	// emission rate
	m_fEmissionRate = m_uTotalParticles / m_fLife;

	// texture
	// Try to get the texture from the cache
	const char* textureName				= rec->get("textureFileName").c_str();
	CCTexture2D* tex = NULL;

	if (strlen(textureName) > 0)
	{
		Ref<StreamSource> texSource = m_PlistSource->getLocator()->locate(textureName, NULL);
		if (texSource)
			tex = CCTextureCache::sharedTextureCache()->addImage(texSource);
	}

	if (tex)
	{
		this->m_pTexture = tex;
	}
	else
	{
		const char* textureData = rec->get("textureImageData").c_str();

		int dataLen = strlen(textureData);
		if (dataLen == 0)
			NIT_THROW_FMT(EX_SYNTAX, "CCParticleSystem: no image data specified");

		unsigned char *buffer = NULL;
		int decodeLen = base64Decode((uint8*)textureData, dataLen, &buffer);

		if (buffer == NULL)
			NIT_THROW_FMT(EX_READ, "CCParticleSystem: error decoding particle textureImageData");

		String key = m_PlistSource->getLocator()->makeUrl(textureName);
		Ref<MemorySource> msrc = new MemorySource(key, new MemoryBuffer(buffer, decodeLen));
		CC_SAFE_DELETE_ARRAY(buffer); // Delete the buffer at once cause we've copied to msrc

		msrc->getBuffer()->uncompress();

		// WARNING: Above code retains the image soure as memory resident all the time.

		// TODO: Implement a StreamSource which reads the image part of a plist file separately as:
		// class ParticleXmlImageSource -> StreamSource : reads xml again on StreamSource::Open(),
		// and re-decode base64 section of 'textureImageData'

		m_pTexture = CCTextureCache::sharedTextureCache()->addImage(msrc);
	}

	if (m_pTexture == NULL)
		NIT_THROW_FMT(EX_READ, "CCParticleSystem: error loading the texture");

	m_pTexture->retain();

	return true;
}

bool CCParticleSystem::initWithTotalParticles(unsigned int numberOfParticles)
{
	m_uTotalParticles = numberOfParticles;

    CC_SAFE_DELETE_ARRAY(m_pParticles);
	
	m_pParticles = new tCCParticle[m_uTotalParticles];

	if( ! m_pParticles )
	{
		CCLOG("Particle system: not enough memory");
		this->release();
		return false;
	}

	// default, active
	m_bIsActive = true;

	// default blend function
	m_tBlendFunc.src = CC_BLEND_SRC;
	m_tBlendFunc.dst = CC_BLEND_DST;

	// default movement type;
	m_ePositionType = kCCPositionTypeFree;

	// by default be in mode A:
	m_nEmitterMode = kCCParticleModeGravity;

	// default: modulate
	// XXX: not used
	//	colorModulate = YES;

	m_bIsAutoRemoveOnFinish = false;

	// Optimization: compile udpateParticle method
	//updateParticleSel = @selector(updateQuadWithParticle:newPosition:);
	//updateParticleImp = (CC_UPDATE_PARTICLE_IMP) [self methodForSelector:updateParticleSel];

	// udpate after action in run!
	CCDirector::sharedDirector()->getTimer()->channel()->priority(1)->
		bind(nit::Events::OnTick, this, &CCParticleSystem::OnTick);

	return true;
}

CCParticleSystem::~CCParticleSystem()
{
    CC_SAFE_DELETE_ARRAY(m_pParticles);
	CC_SAFE_RELEASE(m_pTexture)
}
bool CCParticleSystem::addParticle()
{
	if (this->isFull())
	{
		return false;
	}

	tCCParticle * particle = &m_pParticles[ m_uParticleCount ];
	this->initParticle(particle);
	++m_uParticleCount;

	return true;
}
void CCParticleSystem::initParticle(tCCParticle* particle)
{
	float scale = CC_CONTENT_SCALE_FACTOR();

	// timeToLive
	// no negative life. prevent division by 0
	particle->timeToLive = m_fLife + m_fLifeVar * CCRANDOM_MINUS1_1();
	particle->timeToLive = std::max(0.0f, particle->timeToLive);

	// position
	particle->pos.x = m_tSourcePosition.x + m_tPosVar.x * CCRANDOM_MINUS1_1();
    particle->pos.x *= scale;
	particle->pos.y = m_tSourcePosition.y + m_tPosVar.y * CCRANDOM_MINUS1_1();
    particle->pos.y *= scale;

	// Color
	ccColor4F start;
	start.r = clampf(m_tStartColor.r + m_tStartColorVar.r * CCRANDOM_MINUS1_1(), 0, 1);
	start.g = clampf(m_tStartColor.g + m_tStartColorVar.g * CCRANDOM_MINUS1_1(), 0, 1);
	start.b = clampf(m_tStartColor.b + m_tStartColorVar.b * CCRANDOM_MINUS1_1(), 0, 1);
	start.a = clampf(m_tStartColor.a + m_tStartColorVar.a * CCRANDOM_MINUS1_1(), 0, 1);

	ccColor4F end;
	end.r = clampf(m_tEndColor.r + m_tEndColorVar.r * CCRANDOM_MINUS1_1(), 0, 1);
	end.g = clampf(m_tEndColor.g + m_tEndColorVar.g * CCRANDOM_MINUS1_1(), 0, 1);
	end.b = clampf(m_tEndColor.b + m_tEndColorVar.b * CCRANDOM_MINUS1_1(), 0, 1);
	end.a = clampf(m_tEndColor.a + m_tEndColorVar.a * CCRANDOM_MINUS1_1(), 0, 1);

	particle->color = start;
	particle->deltaColor.r = (end.r - start.r) / particle->timeToLive;
	particle->deltaColor.g = (end.g - start.g) / particle->timeToLive;
	particle->deltaColor.b = (end.b - start.b) / particle->timeToLive;
	particle->deltaColor.a = (end.a - start.a) / particle->timeToLive;

	// size
	float startS = m_fStartSize + m_fStartSizeVar * CCRANDOM_MINUS1_1();
	startS = std::max(0.0f, startS); // No negative value
    startS *= scale;

	particle->size = startS;

	if( m_fEndSize == kCCParticleStartSizeEqualToEndSize )
	{
		particle->deltaSize = 0;
	}
	else
	{
		float endS = m_fEndSize + m_fEndSizeVar * CCRANDOM_MINUS1_1();
		endS = std::max(0.0f, endS); // No negative values
        endS *= scale;
		particle->deltaSize = (endS - startS) / particle->timeToLive;
	}

	// rotation
	float startA = m_fStartSpin + m_fStartSpinVar * CCRANDOM_MINUS1_1();
	float endA = m_fEndSpin + m_fEndSpinVar * CCRANDOM_MINUS1_1();
	particle->rotation = startA;
	particle->deltaRotation = (endA - startA) / particle->timeToLive;

	// position
	if( m_ePositionType == kCCPositionTypeFree )
	{
        CCPoint p = this->convertToWorldSpace(CCPointZero);
		particle->startPos = ccpMult( p, scale );
	}
    else if ( m_ePositionType == kCCPositionTypeRelative )
    {
        particle->startPos = ccpMult( m_tPosition, scale );
    }

	// direction
	float a = CC_DEGREES_TO_RADIANS( m_fAngle + m_fAngleVar * CCRANDOM_MINUS1_1() );	

	// Mode Gravity: A
	if( m_nEmitterMode == kCCParticleModeGravity ) 
	{
		CCPoint v(cosf( a ), sinf( a ));
		float s = modeA.speed + modeA.speedVar * CCRANDOM_MINUS1_1();
        s *= scale;

		// direction
		particle->modeA.dir = ccpMult( v, s );

		// radial accel
		particle->modeA.radialAccel = modeA.radialAccel + modeA.radialAccelVar * CCRANDOM_MINUS1_1();
        particle->modeA.radialAccel *= scale;

		// tangential accel
		particle->modeA.tangentialAccel = modeA.tangentialAccel + modeA.tangentialAccelVar * CCRANDOM_MINUS1_1();
        particle->modeA.tangentialAccel *= scale;
    }

	// Mode Radius: B
	else {
		// Set the default diameter of the particle from the source position
		float startRadius = modeB.startRadius + modeB.startRadiusVar * CCRANDOM_MINUS1_1();
		float endRadius = modeB.endRadius + modeB.endRadiusVar * CCRANDOM_MINUS1_1();
        startRadius *= scale;
        endRadius *= scale;

		particle->modeB.radius = startRadius;

		if( modeB.endRadius == kCCParticleStartRadiusEqualToEndRadius )
			particle->modeB.deltaRadius = 0;
		else
			particle->modeB.deltaRadius = (endRadius - startRadius) / particle->timeToLive;

		particle->modeB.angle = a;
		particle->modeB.degreesPerSecond = CC_DEGREES_TO_RADIANS(modeB.rotatePerSecond + modeB.rotatePerSecondVar * CCRANDOM_MINUS1_1());
	}	
}
void CCParticleSystem::stopSystem()
{
	m_bIsActive = false;
	m_fElapsed = m_fDuration;
	m_fEmitCounter = 0;
}
void CCParticleSystem::resetSystem()
{
	m_bIsActive = true;
	m_fElapsed = 0;
	for (m_uParticleIdx = 0; m_uParticleIdx < m_uParticleCount; ++m_uParticleIdx)
	{
		tCCParticle *p = &m_pParticles[m_uParticleIdx];
		p->timeToLive = 0;
	}
}
bool CCParticleSystem::isFull()
{
	return (m_uParticleCount == m_uTotalParticles);
}

void CCParticleSystem::OnTick(const TimeEvent* evt)
{
	updateParticle(evt->getDelta());
}

// ParticleSystem - MainLoop
void CCParticleSystem::updateParticle(ccTime dt)
{
	float scale = CC_CONTENT_SCALE_FACTOR();

	if( m_bIsActive && m_fEmissionRate )
	{
		float rate = 1.0f / m_fEmissionRate;
		m_fEmitCounter += dt;
		while( m_uParticleCount < m_uTotalParticles && m_fEmitCounter > rate ) 
		{
			this->addParticle();
			m_fEmitCounter -= rate;
		}

		m_fElapsed += dt;
		if(m_fDuration != -1 && m_fDuration < m_fElapsed)
		{
			this->stopSystem();
		}
	}

	m_uParticleIdx = 0;

	CCPoint currentPosition = CCPointZero;
	if( m_ePositionType == kCCPositionTypeFree )
	{
		currentPosition = this->convertToWorldSpace(CCPointZero);
        currentPosition.x *= scale;
        currentPosition.y *= scale;
	}
    else if ( m_ePositionType == kCCPositionTypeRelative )
    {
        currentPosition = m_tPosition;
        currentPosition.x *= scale;
        currentPosition.y *= scale;
    }

	while( m_uParticleIdx < m_uParticleCount )
	{
		tCCParticle *p = &m_pParticles[m_uParticleIdx];

		// life
		p->timeToLive -= dt;

		if( p->timeToLive > 0 ) 
		{
			// Mode A: gravity, direction, tangential accel & radial accel
			if( m_nEmitterMode == kCCParticleModeGravity ) 
			{
				CCPoint tmp, radial, tangential;

				radial = CCPointZero;
				// radial acceleration
				if(p->pos.x || p->pos.y)
					radial = ccpNormalize(p->pos);
				tangential = radial;
				radial = ccpMult(radial, p->modeA.radialAccel);

				// tangential acceleration
				float newy = tangential.x;
				tangential.x = -tangential.y;
				tangential.y = newy;
				tangential = ccpMult(tangential, p->modeA.tangentialAccel);

				// (gravity + radial + tangential) * dt
				tmp = ccpAdd( ccpAdd( radial, tangential), modeA.gravity);
				tmp = ccpMult( tmp, dt);
				p->modeA.dir = ccpAdd( p->modeA.dir, tmp);
				tmp = ccpMult(p->modeA.dir, dt);
				p->pos = ccpAdd( p->pos, tmp );
			}

			// Mode B: radius movement
			else {				
				// Update the angle and radius of the particle.
				p->modeB.angle += p->modeB.degreesPerSecond * dt;
				p->modeB.radius += p->modeB.deltaRadius * dt;

				p->pos.x = - cosf(p->modeB.angle) * p->modeB.radius;
				p->pos.y = - sinf(p->modeB.angle) * p->modeB.radius;
			}

			// color
			p->color.r += (p->deltaColor.r * dt);
			p->color.g += (p->deltaColor.g * dt);
			p->color.b += (p->deltaColor.b * dt);
			p->color.a += (p->deltaColor.a * dt);

			// size
			p->size += (p->deltaSize * dt);
			p->size = std::max( 0.0f, p->size );

			// angle
			p->rotation += (p->deltaRotation * dt);

			//
			// update values in quad
			//

			CCPoint	newPos;

			if( m_ePositionType == kCCPositionTypeFree || m_ePositionType == kCCPositionTypeRelative ) 
			{
				CCPoint diff = ccpSub( currentPosition, p->startPos );
				newPos = ccpSub(p->pos, diff);
			} 
			else
			{
				newPos = p->pos;
			}

			updateQuadWithParticle(p, newPos);
			//updateParticleImp(self, updateParticleSel, p, newPos);

			// update particle counter
			++m_uParticleIdx;

		} 
		else 
		{
			// life < 0
			if( m_uParticleIdx != m_uParticleCount-1 )
			{
				m_pParticles[m_uParticleIdx] = m_pParticles[m_uParticleCount-1];
			}
			--m_uParticleCount;

			if( m_uParticleCount == 0 && m_bIsAutoRemoveOnFinish )
			{
				m_pParent->removeChild(this, true);
				return;
			}
		}
	}
}

void CCParticleSystem::cleanup()
{
	CCDirector::sharedDirector()->getTimer()->channel()->priority(1)->
		unbind(nit::Events::OnTick, this);
	CCNode::cleanup();
}

// ParticleSystem - CCTexture protocol
void CCParticleSystem::setTexture(CCTexture2D* var)
{
	CC_SAFE_RETAIN(var);
	CC_SAFE_RELEASE(m_pTexture)
	m_pTexture = var;

	// If the new texture has No premultiplied alpha, AND the blendFunc hasn't been changed, then update it
	if( m_pTexture && ! m_pTexture->getHasPremultipliedAlpha() &&		
		( m_tBlendFunc.src == CC_BLEND_SRC && m_tBlendFunc.dst == CC_BLEND_DST ) ) 
	{
		m_tBlendFunc.src = GL_SRC_ALPHA;
		m_tBlendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
	}
}
CCTexture2D * CCParticleSystem::getTexture()
{
	return m_pTexture;
}

// ParticleSystem - Additive Blending
void CCParticleSystem::setIsBlendAdditive(bool additive)
{
	if( additive )
	{
		m_tBlendFunc.src = GL_SRC_ALPHA;
		m_tBlendFunc.dst = GL_ONE;
	}
	else
	{
		if( m_pTexture && ! m_pTexture->getHasPremultipliedAlpha() )
		{
			m_tBlendFunc.src = GL_SRC_ALPHA;
			m_tBlendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
		} 
		else 
		{
			m_tBlendFunc.src = CC_BLEND_SRC;
			m_tBlendFunc.dst = CC_BLEND_DST;
		}
	}
}
bool CCParticleSystem::getIsBlendAdditive()
{
	return( m_tBlendFunc.src == GL_SRC_ALPHA && m_tBlendFunc.dst == GL_ONE);
}

// ParticleSystem - Properties of Gravity Mode 
void CCParticleSystem::setTangentialAccel(float t)
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	modeA.tangentialAccel = t;
}
float CCParticleSystem::getTangentialAccel()
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	return modeA.tangentialAccel;
}
void CCParticleSystem::setTangentialAccelVar(float t)
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	modeA.tangentialAccelVar = t;
}
float CCParticleSystem::getTangentialAccelVar()
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	return modeA.tangentialAccelVar;
}	
void CCParticleSystem::setRadialAccel(float t)
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	modeA.radialAccel = t;
}
float CCParticleSystem::getRadialAccel()
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	return modeA.radialAccel;
}
void CCParticleSystem::setRadialAccelVar(float t)
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	modeA.radialAccelVar = t;
}
float CCParticleSystem::getRadialAccelVar()
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	return modeA.radialAccelVar;
}
void CCParticleSystem::setGravity(const CCPoint& g)
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	modeA.gravity = g;
}
const CCPoint& CCParticleSystem::getGravity()
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	return modeA.gravity;
}
void CCParticleSystem::setSpeed(float speed)
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	modeA.speed = speed;
}
float CCParticleSystem::getSpeed()
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	return modeA.speed;
}
void CCParticleSystem::setSpeedVar(float speedVar)
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	modeA.speedVar = speedVar;
}
float CCParticleSystem::getSpeedVar()
{
	CCAssert( m_nEmitterMode == kCCParticleModeGravity, "Particle Mode should be Gravity");
	return modeA.speedVar;
}

// ParticleSystem - Properties of Radius Mode
void CCParticleSystem::setStartRadius(float startRadius)
{
	CCAssert( m_nEmitterMode == kCCParticleModeRadius, "Particle Mode should be Radius");
	modeB.startRadius = startRadius;
}
float CCParticleSystem::getStartRadius()
{
	CCAssert( m_nEmitterMode == kCCParticleModeRadius, "Particle Mode should be Radius");
	return modeB.startRadius;
}
void CCParticleSystem::setStartRadiusVar(float startRadiusVar)
{
	CCAssert( m_nEmitterMode == kCCParticleModeRadius, "Particle Mode should be Radius");
	modeB.startRadiusVar = startRadiusVar;
}
float CCParticleSystem::getStartRadiusVar()
{
	CCAssert( m_nEmitterMode == kCCParticleModeRadius, "Particle Mode should be Radius");
	return modeB.startRadiusVar;
}
void CCParticleSystem::setEndRadius(float endRadius)
{
	CCAssert( m_nEmitterMode == kCCParticleModeRadius, "Particle Mode should be Radius");
	modeB.endRadius = endRadius;
}
float CCParticleSystem::getEndRadius()
{
	CCAssert( m_nEmitterMode == kCCParticleModeRadius, "Particle Mode should be Radius");
	return modeB.endRadius;
}
void CCParticleSystem::setEndRadiusVar(float endRadiusVar)
{
	CCAssert( m_nEmitterMode == kCCParticleModeRadius, "Particle Mode should be Radius");
	modeB.endRadiusVar = endRadiusVar;
}
float CCParticleSystem::getEndRadiusVar()
{
	CCAssert( m_nEmitterMode == kCCParticleModeRadius, "Particle Mode should be Radius");
	return modeB.endRadiusVar;
}
void CCParticleSystem::setRotatePerSecond(float degrees)
{
	CCAssert( m_nEmitterMode == kCCParticleModeRadius, "Particle Mode should be Radius");
	modeB.rotatePerSecond = degrees;
}
float CCParticleSystem::getRotatePerSecond()
{
	CCAssert( m_nEmitterMode == kCCParticleModeRadius, "Particle Mode should be Radius");
	return modeB.rotatePerSecond;
}
void CCParticleSystem::setRotatePerSecondVar(float degrees)
{
	CCAssert( m_nEmitterMode == kCCParticleModeRadius, "Particle Mode should be Radius");
	modeB.rotatePerSecondVar = degrees;
}
float CCParticleSystem::getRotatePerSecondVar()
{
	CCAssert( m_nEmitterMode == kCCParticleModeRadius, "Particle Mode should be Radius");
	return modeB.rotatePerSecondVar;
}
bool CCParticleSystem::getIsActive()
{
	return m_bIsActive;
}
unsigned int CCParticleSystem::getParticleCount()
{
	return m_uParticleCount;
}
float CCParticleSystem::getDuration()
{
	return m_fDuration;
}
void CCParticleSystem::setDuration(float var)
{
	m_fDuration = var;
}
const CCPoint& CCParticleSystem::getSourcePosition()
{
	return m_tSourcePosition;
}
void CCParticleSystem::setSourcePosition(const CCPoint& var)
{
	m_tSourcePosition = var;
}
const CCPoint& CCParticleSystem::getPosVar()
{
	return m_tPosVar;
}
void CCParticleSystem::setPosVar(const CCPoint& var)
{
	m_tPosVar = var;
}
float CCParticleSystem::getLife()
{
	return m_fLife;
}
void CCParticleSystem::setLife(float var)
{
	m_fLife = var;
}
float CCParticleSystem::getLifeVar()
{
	return m_fLifeVar;
}
void CCParticleSystem::setLifeVar(float var)
{
	m_fLifeVar = var;
}
float CCParticleSystem::getAngle()
{
	return m_fAngle;
}
void CCParticleSystem::setAngle(float var)
{
	m_fAngle = var;
}
float CCParticleSystem::getAngleVar()
{
	return m_fAngleVar;
}
void CCParticleSystem::setAngleVar(float var)
{
	m_fAngleVar = var;
}
float CCParticleSystem::getStartSize()
{
	return m_fStartSize;
}
void CCParticleSystem::setStartSize(float var)
{
	m_fStartSize = var;
}
float CCParticleSystem::getStartSizeVar()
{
	return m_fStartSizeVar;
}
void CCParticleSystem::setStartSizeVar(float var)
{
	m_fStartSizeVar = var;
}
float CCParticleSystem::getEndSize()
{
	return m_fEndSize;
}
void CCParticleSystem::setEndSize(float var)
{
	m_fEndSize = var;
}
float CCParticleSystem::getEndSizeVar()
{
	return m_fEndSizeVar;
}
void CCParticleSystem::setEndSizeVar(float var)
{
	m_fEndSizeVar = var;
}
const ccColor4F& CCParticleSystem::getStartColor()
{
	return m_tStartColor;
}
void CCParticleSystem::setStartColor(const ccColor4F& var)
{
	m_tStartColor = var;
}
const ccColor4F& CCParticleSystem::getStartColorVar()
{
	return m_tStartColorVar;
}
void CCParticleSystem::setStartColorVar(const ccColor4F& var)
{
	m_tStartColorVar = var;
}
const ccColor4F& CCParticleSystem::getEndColor()
{
	return m_tEndColor;
}
void CCParticleSystem::setEndColor(const ccColor4F& var)
{
	m_tEndColor = var;
}
const ccColor4F& CCParticleSystem::getEndColorVar()
{
	return m_tEndColorVar;
}
void CCParticleSystem::setEndColorVar(const ccColor4F& var)
{
	m_tEndColorVar = var;
}
float CCParticleSystem::getStartSpin()
{
	return m_fStartSpin;
}
void CCParticleSystem::setStartSpin(float var)
{
	m_fStartSpin = var;
}
float CCParticleSystem::getStartSpinVar()
{
	return m_fStartSpinVar;
}
void CCParticleSystem::setStartSpinVar(float var)
{
	m_fStartSpinVar = var;
}
float CCParticleSystem::getEndSpin()
{
	return m_fEndSpin;
}
void CCParticleSystem::setEndSpin(float var)
{
	m_fEndSpin = var;
}
float CCParticleSystem::getEndSpinVar()
{
	return m_fEndSpinVar;
}
void CCParticleSystem::setEndSpinVar(float var)
{
	m_fEndSpinVar = var;
}
float CCParticleSystem::getEmissionRate()
{
	return m_fEmissionRate;
}
void CCParticleSystem::setEmissionRate(float var)
{
	m_fEmissionRate = var;
}
unsigned int CCParticleSystem::getTotalParticles()
{
	return m_uTotalParticles;
}
void CCParticleSystem::setTotalParticles(unsigned int var)
{
	m_uTotalParticles = var;
}
ccBlendFunc CCParticleSystem::getBlendFunc()
{
	return m_tBlendFunc;
}
void CCParticleSystem::setBlendFunc(ccBlendFunc var)
{
	m_tBlendFunc = var;
}
tCCPositionType CCParticleSystem::getPositionType()
{
	return m_ePositionType;
}
void CCParticleSystem::setPositionType(tCCPositionType var)
{
	m_ePositionType = var;
}
bool CCParticleSystem::getIsAutoRemoveOnFinish()
{
	return m_bIsAutoRemoveOnFinish;
}
void CCParticleSystem::setIsAutoRemoveOnFinish(bool var)
{
	m_bIsAutoRemoveOnFinish = var;
}
int CCParticleSystem::getEmitterMode()
{
	return m_nEmitterMode;
}
void CCParticleSystem::setEmitterMode(int var)
{
	m_nEmitterMode = var;
}

NS_CC_END;

