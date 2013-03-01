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

#include "nit2d_pch.h"

#include "nit2d/NitLibCocos.h"

using namespace cocos2d;

////////////////////////////////////////////////////////////////////////////////

// Redeclare namespace as cc -> cc.particle

namespace cc
{
	namespace particle
	{
		typedef CCParticleFire			Fire;
		typedef CCParticleFireworks		Fireworks;
		typedef CCParticleSun			Sun;
		typedef CCParticleGalaxy		Galaxy;
		typedef CCParticleFlower		Flower;
		typedef CCParticleMeteor		Meteor;
		typedef CCParticleSpiral		Spiral;
		typedef CCParticleExplosion		Explosion;
		typedef CCParticleSmoke			Smoke;
		typedef CCParticleSnow			Snow;
		typedef CCParticleRain			Rain;
	}
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::particle::Fire, CCParticleSystemQuad);

class NB_CCParticleFire : TNitClass<CCParticleFire>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCParticleFire::node()); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::particle::Fireworks, CCParticleSystemQuad);

class NB_CCParticleFireworks : TNitClass<CCParticleFireworks>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCParticleFireworks::node()); return 0; }
	
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::particle::Sun, CCParticleSystemQuad);

class NB_CCParticleSun : TNitClass<CCParticleSun>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCParticleSun::node()); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::particle::Galaxy, CCParticleSystemQuad);

class NB_CCParticleGalaxy : TNitClass<CCParticleGalaxy>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCParticleGalaxy::node()); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::particle::Flower, CCParticleSystemQuad);

class NB_CCParticleFlower : TNitClass<CCParticleFlower>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCParticleFlower::node()); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::particle::Meteor, CCParticleSystemQuad);

class NB_CCParticleMeteor : TNitClass<CCParticleMeteor>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCParticleMeteor::node()); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::particle::Spiral, CCParticleSystemQuad);

class NB_CCParticleSpiral : TNitClass<CCParticleSpiral>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCParticleSpiral::node()); return 0; }

};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::particle::Explosion, CCParticleSystemQuad);

class NB_CCParticleExplosion : TNitClass<CCParticleExplosion>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCParticleExplosion::node()); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::particle::Smoke, CCParticleSystemQuad);

class NB_CCParticleSmoke : TNitClass<CCParticleSmoke>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCParticleSmoke::node()); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::particle::Snow, CCParticleSystemQuad);

class NB_CCParticleSnow: TNitClass<CCParticleSnow>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCParticleSnow::node()); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NB_TYPE_CC_REF(NIT2D_API, ::cc::particle::Rain, CCParticleSystemQuad);

class NB_CCParticleRain : TNitClass<CCParticleRain>
{
public:
	static void Register(HSQUIRRELVM v)
	{
		PropEntry props[] =
		{
			NULL
		};

		FuncEntry funcs[] = 
		{
			CONS_ENTRY_H(				"()"),
			NULL
		};

		bind(v, props, funcs);
	}

	NB_CONS()							{ setSelf(v, CCParticleRain::node()); return 0; }
};

////////////////////////////////////////////////////////////////////////////////

NIT2D_API SQRESULT NitLibCocosParticle(HSQUIRRELVM v)
{
	NB_CCParticleFire::Register(v);
	NB_CCParticleFireworks::Register(v);
	NB_CCParticleSun::Register(v);
	NB_CCParticleGalaxy::Register(v);
	NB_CCParticleFlower::Register(v);
	NB_CCParticleMeteor::Register(v);
	NB_CCParticleSpiral::Register(v);
	NB_CCParticleExplosion::Register(v);
	NB_CCParticleSmoke::Register(v);
	NB_CCParticleSnow::Register(v);
	NB_CCParticleRain::Register(v);

	return SQ_OK;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
