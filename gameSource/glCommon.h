/*
 * Modification History
 *
 * 2006-August-14   Jason Rohrer
 * Created.
 *
 * 2006-November-8   Jason Rohrer
 * Added corner colors to textured quad function.
 */


#ifndef GL_COMMON_INCLUDED
#define GL_COMMON_INCLUDED



#include "minorGems/math/geometry/Vector3D.h"
#include "minorGems/math/geometry/Angle3D.h"
#include "minorGems/graphics/Color.h"


#include "minorGems/graphics/openGL/SingleTextureGL.h"


/**
 * Must be called after calling functions that create common textures
 * before exiting to avoid memory leaks.
 */
void destroyCommonTextures();



/**
 * Draws a blurry, filled circle using a texture map.
 *
 * Depends on inited textures.
 *
 * @param inCenter the position of the center.  Destroyed by caller.
 * @param inRadius the radius in world units.
 */
void drawBlurCircle( Vector3D *inCenter, double inRadius );




/**
 * Draws a plus using a texture map.
 *
 * Depends on inited textures.
 *
 * @param inCenter the position of the center.  Destroyed by caller.
 * @param inRadius the radius in world units.
 * @param inRotation rotation of plus, or NULL for default.
 *   Destroyed by caller.
 */
void drawBlurPlus( Vector3D *inCenter, double inRadius,
                   Angle3D *inRotation = NULL );




/**
 * Draws a texture-mapped quad.
 *
 * @param inTexture the texture.  Destroyed by caller.
 * @param inCenter the position of the center.  Destroyed by caller.
 * @param inRadius the radius in world units.
 * @param inRotation rotation of quad, or NULL for default.
 *   Destroyed by caller.
 * @param inCornerColors the colors to set for each corner.
 *   Must contain an array of four colors, or NULL to not set corner colors.
 *   Defaults to NULL.
 */
void drawTextureQuad( SingleTextureGL *inTexture,
                      Vector3D *inCenter, double inRadius,
                      Angle3D *inRotation = NULL,
                      Color **inCornerColors = NULL );



/**
 * Draws a circle of vertices.
 *
 * Must be enclosed in a glBegin() / glEnd() construct by caller.
 *
 * Circle is an open loop (last vertex is one segment away from frist vertex).
 *
 * @param inCenter the position of the center.  Destroyed by caller.
 * @param inRadius the radius in world units.
 * @param inNumSegments the number of segments in the circle.
 */
void drawCircle( Vector3D *inCenter, double inRadius, int inNumSegments );



/**
 * Sets the current OpenGL drawing color.
 *
 * Equivalent to glColor4f( inColor->r, inColor->g, inColor->b, inColor->a );
 *
 * @param inColor the color to set.  Destroyed by caller.
 */
void setGLColor( Color *inColor );



#endif




