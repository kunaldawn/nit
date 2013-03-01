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

#ifndef __CCCONFIG_H__
#define __CCCONFIG_H__

#include "CCPlatformConfig.h"

/** NIT setting: Forced pixel matching mode
if specified non-zero value, the following options which could affect pixel matching forcibly turned off :
- CC_FIX_ARTIFACTS_BY_STRETCHING_TEXEL	: Pixel will not match when texel stretched - so turn off.
- CC_COCOSNODE_RENDER_SUBPIXEL			: Force int on non-batched fully-transformed positions.
- CC_SPRITEBATCHNODE_RENDER_SUBPIXEL	: Force int on batched fully-transformed positions.
- CC_NODE_TRANSFORM_USING_AFFINE_MATRIX	: This option accumulates floating fractions from parent to descendants - so turn off.
*/

// TODO: Avoid compile flags - expose additional flags to CCNode class so users can tweak them on runtime

#ifndef CC_STRICT_PIXEL_MATCHING
#define CC_STRICT_PIXEL_MATCHING 1
#endif

#if CC_STRICT_PIXEL_MATCHING != 0

#undef CC_FIX_ARTIFACTS_BY_STRETCHING_TEXEL
#undef CC_COCOSNODE_RENDER_SUBPIXEL
#undef CC_SPRITEBATCHNODE_RENDER_SUBPIXEL
#undef CC_NODE_TRANSFORM_USING_AFFINE_MATRIX

#define CC_FIX_ARTIFACTS_BY_STRETCHING_TEXEL	0
#define CC_COCOSNODE_RENDER_SUBPIXEL			0
#define CC_SPRITEBATCHNODE_RENDER_SUBPIXEL		0
#define CC_NODE_TRANSFORM_USING_AFFINE_MATRIX	0

#endif // CC_STRICT_PIXEL_MATCHING

/** @def CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
If enabled, the texture coordinates will be calculated by using this formula:
- texCoord.left = (rect.origin.x*2+1) / (texture.wide*2);
- texCoord.right = texCoord.left + (rect.size.width*2-2)/(texture.wide*2);

The same for bottom and top.

This formula prevents artifacts by using 99% of the texture.
The "correct" way to prevent artifacts is by using the spritesheet-artifact-fixer.py or a similar tool.

Affected nodes:
- CCSprite / CCSpriteBatchNode and subclasses: CCLabelBMFont, CCTMXTiledMap
- CCLabelAtlas
- CCQuadParticleSystem
- CCTileMap

To enabled set it to 1. Disabled by default.

@since v0.99.5
*/
#ifndef CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
#define CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL 0
#endif

/** @def CC_DIRECTOR_FPS_INTERVAL
 Senconds between FPS updates.
 0.5 seconds, means that the FPS number will be updated every 0.5 seconds.
 Having a bigger number means a more reliable FPS
 
 Default value: 0.1f
 */
#ifndef CC_DIRECTOR_FPS_INTERVAL
#define CC_DIRECTOR_FPS_INTERVAL (0.5f)
#endif

/** @def CC_COCOSNODE_RENDER_SUBPIXEL
 If enabled, the CCNode objects (CCSprite, CCLabel,etc) will be able to render in subpixels.
 If disabled, integer pixels will be used.
 
 To enable set it to 1. Enabled by default.
 */
#ifndef CC_COCOSNODE_RENDER_SUBPIXEL
#define CC_COCOSNODE_RENDER_SUBPIXEL 1
#endif

/** @def CC_SPRITEBATCHNODE_RENDER_SUBPIXEL
 If enabled, the CCSprite objects rendered with CCSpriteBatchNode will be able to render in subpixels.
 If disabled, integer pixels will be used.
 
 To enable set it to 1. Enabled by default.
 */
#ifndef CC_SPRITEBATCHNODE_RENDER_SUBPIXEL
#define CC_SPRITEBATCHNODE_RENDER_SUBPIXEL	1
#endif

/** @def CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
 If enabled, CCNode will transform the nodes using a cached Affine matrix.
 If disabled, the node will be transformed using glTranslate,glRotate,glScale.
 Using the affine matrix only requires 2 GL calls.
 Using the translate/rotate/scale requires 5 GL calls.
 But computing the Affine matrix is relative expensive.
 But according to performance tests, Affine matrix performs better.
 This parameter doesn't affect CCSpriteBatchNode nodes.
 
 To enable set it to a value different than 0. Enabled by default.

 */
#ifndef CC_NODE_TRANSFORM_USING_AFFINE_MATRIX
#define CC_NODE_TRANSFORM_USING_AFFINE_MATRIX 1
#endif

/** @def CC_OPTIMIZE_BLEND_FUNC_FOR_PREMULTIPLIED_ALPHA
If most of your imamges have pre-multiplied alpha, set it to 1 (if you are going to use .PNG/.JPG file images).
Only set to 0 if ALL your images by-pass Apple UIImage loading system (eg: if you use libpng or PVR images)

To enable set it to a value different than 0. Enabled by default.

@since v0.99.5
*/
#ifndef CC_OPTIMIZE_BLEND_FUNC_FOR_PREMULTIPLIED_ALPHA
#define CC_OPTIMIZE_BLEND_FUNC_FOR_PREMULTIPLIED_ALPHA 1
#endif

/** @def CC_TEXTURE_ATLAS_USE_TRIANGLE_STRIP
 Use GL_TRIANGLE_STRIP instead of GL_TRIANGLES when rendering the texture atlas.
 It seems it is the recommend way, but it is much slower, so, enable it at your own risk
 
 To enable set it to a value different than 0. Disabled by default.

 */
#ifndef CC_TEXTURE_ATLAS_USE_TRIANGLE_STRIP
#define CC_TEXTURE_ATLAS_USE_TRIANGLE_STRIP 0
#endif

#endif // __CCCONFIG_H__
