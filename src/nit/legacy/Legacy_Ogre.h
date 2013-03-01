/*
-----------------------------------------------------------------------------
This source file is from part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
-----------------------------------------------------------------------------
*/

#pragma once

#include "nit/nit.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// from OgreMemoryAllocatorConfig.h

enum MemoryCategory
{
	/// General purpose
	MEMCATEGORY_GENERAL = 0,
	/// Geometry held in main memory
	MEMCATEGORY_GEOMETRY = 1, 
	/// Animation data like tracks, bone matrices
	MEMCATEGORY_ANIMATION = 2, 
	/// Nodes, control data
	MEMCATEGORY_SCENE_CONTROL = 3,
	/// Scene object instances
	MEMCATEGORY_SCENE_OBJECTS = 4,
	/// Other resources
	MEMCATEGORY_RESOURCE = 5,
	/// Scripting
	MEMCATEGORY_SCRIPTING = 6,
	/// Rendersystem structures
	MEMCATEGORY_RENDERSYS = 7,

	/// Default
	MEMCATEGORY_DEFAULT = 0,
	// Allocation using global memory pool with MemManager
	MEMCATEGORY_GLOBAL_POOL = 0,

	// sentinel value, do not use 
	MEMCATEGORY_COUNT = 8
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
