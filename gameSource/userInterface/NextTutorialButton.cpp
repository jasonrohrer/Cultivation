/*
 * Modification History
 *
 * 2006-December-19   Jason Rohrer
 * Created.
 */



#include "NextTutorialButton.h"

#include "../glCommon.h"

#include "minorGems/graphics/filters/BoxBlurFilter.h"


#include <GL/gl.h>



NextTutorialButton::NextTutorialButton( double inAnchorX, double inAnchorY,
                                        double inWidth, double inHeight )
    : RestartButton( inAnchorX, inAnchorY, inWidth, inHeight ) {

    }



void NextTutorialButton::drawIcon( Vector3D *inCenter, double inRadius,
                              double inAlpha ) {

    // draw a green back arrow

    glColor4f( 0, 1, 0, inAlpha );

    // compensate for small radius of texture
    double drawRadius = inRadius / 0.6666;

    // hack:  draw texture backwards using a negative radius
    drawTextureQuad( mArrowTexture, inCenter, -drawRadius );
    }



