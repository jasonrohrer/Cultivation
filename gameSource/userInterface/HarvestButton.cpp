/*
 * Modification History
 *
 * 2006-July-23   Jason Rohrer
 * Created.
 */



#include "HarvestButton.h"



#include <GL/gl.h>



HarvestButton::HarvestButton( double inAnchorX, double inAnchorY,
                              double inWidth, double inHeight )
    : ButtonBase( inAnchorX, inAnchorY, inWidth, inHeight ) {

    }



void HarvestButton::drawIcon( Vector3D *inCenter, double inRadius,
                              double inAlpha ) {

    if( mSelectedObject != NULL ) {
        // draw selected object
        mSelectedObject->draw( inCenter, inRadius );
        }
    }



