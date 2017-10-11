/*
 * Modification History
 *
 * 2006-August-14   Jason Rohrer
 * Created.
 */



#include "MateButton.h"

#include "../glCommon.h"


#include <GL/gl.h>



MateButton::MateButton( double inAnchorX, double inAnchorY,
                      double inWidth, double inHeight )
    : ButtonBase( inAnchorX, inAnchorY, inWidth, inHeight ),
      mSecondSelectedObject( NULL ) {

    }



void MateButton::setSecondSelectedObject( DrawableObject *inObject ) {
    mSecondSelectedObject = inObject;
    }



void MateButton::drawIcon( Vector3D *inCenter, double inRadius,
                          double inAlpha ) {


    // draw gardeners side by side

    double offset = inRadius / 3;

    double drawRadius = inRadius - offset;

    // gardeners take up only 0.666 of their radius
    drawRadius = drawRadius / 0.6666;


    Vector3D secondCenter( inCenter );
    secondCenter.mY += offset;
    
    if( mSecondSelectedObject != NULL ) {
        // draw selected object
        mSecondSelectedObject->draw( &secondCenter, drawRadius );
        }


    Vector3D firstCenter( inCenter );
    firstCenter.mY -= offset;
    
    if( mSelectedObject != NULL ) {
        // draw selected object
        mSelectedObject->draw( &firstCenter, drawRadius );
        }


    }



