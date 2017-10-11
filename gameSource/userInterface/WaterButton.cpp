/*
 * Modification History
 *
 * 2006-July-5   Jason Rohrer
 * Created.
 */



#include "WaterButton.h"
#include "../glCommon.h"


#include <GL/gl.h>



WaterButton::WaterButton( double inAnchorX, double inAnchorY,
                          double inWidth, double inHeight )
    : ButtonBase( inAnchorX, inAnchorY, inWidth, inHeight ),
      mFullStatus( false ) {

    }



void WaterButton::setFull( char inFullStatus ) {
    mFullStatus = inFullStatus;
    }



void WaterButton::drawIcon( Vector3D *inCenter, double inRadius,
                            double inAlpha ) {

    glColor4f( 0, 0, 1, inAlpha );

    drawBlurCircle( inCenter, inRadius );
    }



