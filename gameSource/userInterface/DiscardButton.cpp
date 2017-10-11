/*
 * Modification History
 *
 * 2006-August-15   Jason Rohrer
 * Created.
 */



#include "DiscardButton.h"
#include "../glCommon.h"



#include <GL/gl.h>



DiscardButton::DiscardButton( double inAnchorX, double inAnchorY,
                              double inWidth, double inHeight )
    : ButtonBase( inAnchorX, inAnchorY, inWidth, inHeight, true ) {

    }



void DiscardButton::drawIcon( Vector3D *inCenter, double inRadius,
                              double inAlpha ) {


    if( mSelectedObject != NULL ) {
        // draw selected object
        mSelectedObject->draw( inCenter, inRadius );
        }
    

    // draw a white x with a black shadow
    Angle3D angle( 0, 0, M_PI / 4 );

    glColor4f( 0, 0, 0, inAlpha );

    Vector3D shadowCenter( inCenter );
    //shadowCenter.mX -= 0.1 * inRadius;
    //shadowCenter.mY -= 0.1 * inRadius;
    
    drawBlurPlus( &shadowCenter, 0.9 * inRadius, &angle ); 

    
    glColor4f( 1, 1, 1, inAlpha );

    drawBlurPlus( inCenter, 0.8 * inRadius, &angle ); 

    }



