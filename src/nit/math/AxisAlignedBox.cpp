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

#include "nit_pch.h"

#include "nit/runtime/MemManager.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

const AxisAlignedBox AxisAlignedBox::BOX_NULL;
const AxisAlignedBox AxisAlignedBox::BOX_INFINITE(AxisAlignedBox::EXTENT_INFINITE);

const Vector3* AxisAlignedBox::getAllCorners(void) const
{
	assert( (mExtent == EXTENT_FINITE) && "Can't get corners of a null or infinite AAB" );

	// The order of these items is, using right-handed co-ordinates:
	// Minimum Z face, starting with Min(all), then anticlockwise
	//   around face (looking onto the face)
	// Maximum Z face, starting with Max(all), then anticlockwise
	//   around face (looking onto the face)
	// Only for optimization/compatibility.
	if (!mpCorners)
		mpCorners = (Vector3*) NIT_ALLOC(sizeof(Vector3) * 8);

	mpCorners[0] = mMinimum;
	mpCorners[1].x = mMinimum.x; mpCorners[1].y = mMaximum.y; mpCorners[1].z = mMinimum.z;
	mpCorners[2].x = mMaximum.x; mpCorners[2].y = mMaximum.y; mpCorners[2].z = mMinimum.z;
	mpCorners[3].x = mMaximum.x; mpCorners[3].y = mMinimum.y; mpCorners[3].z = mMinimum.z;            

	mpCorners[4] = mMaximum;
	mpCorners[5].x = mMinimum.x; mpCorners[5].y = mMaximum.y; mpCorners[5].z = mMaximum.z;
	mpCorners[6].x = mMinimum.x; mpCorners[6].y = mMinimum.y; mpCorners[6].z = mMaximum.z;
	mpCorners[7].x = mMaximum.x; mpCorners[7].y = mMinimum.y; mpCorners[7].z = mMaximum.z;

	return mpCorners;
}

AxisAlignedBox::~AxisAlignedBox()
{
	if (mpCorners)
		NIT_DEALLOC(mpCorners, sizeof(Vector3) * 8);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
