/*
 * Modification History
 *
 * 2006-September-19   Jason Rohrer
 * Created.
 */



#include "PoisonButton.h"
#include "../glCommon.h"


#include <GL/gl.h>



PoisonButton::PoisonButton( double inAnchorX, double inAnchorY,
                          double inWidth, double inHeight )
    : ButtonBase( inAnchorX, inAnchorY, inWidth, inHeight ) {

    }



void PoisonButton::drawIcon( Vector3D *inCenter, double inRadius,
                            double inAlpha ) {

    glColor4f( 0, 0, 0, inAlpha );

    drawBlurCircle( inCenter, inRadius );
    }



