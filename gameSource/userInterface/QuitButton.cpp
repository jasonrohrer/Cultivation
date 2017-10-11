/*
 * Modification History
 *
 * 2006-July-10   Jason Rohrer
 * Created.
 */



#include "QuitButton.h"
#include "../glCommon.h"



#include <GL/gl.h>



QuitButton::QuitButton( double inAnchorX, double inAnchorY,
                              double inWidth, double inHeight )
    : ButtonBase( inAnchorX, inAnchorY, inWidth, inHeight ) {

    }



void QuitButton::drawIcon( Vector3D *inCenter, double inRadius,
                            double inAlpha ) {

    // draw a red x

    glColor4f( 1, 0, 0, inAlpha );

    Angle3D angle( 0, 0, M_PI / 4 );
    drawBlurPlus( inCenter, inRadius, &angle ); 
    }



