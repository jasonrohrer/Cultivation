/*
 * Modification History
 *
 * 2006-October-30   Jason Rohrer
 * Created.
 */



#include "FollowButton.h"

#include "../glCommon.h"


#include <GL/gl.h>



FollowButton::FollowButton( double inAnchorX, double inAnchorY,
                            double inWidth, double inHeight )
    : GiftButton( inAnchorX, inAnchorY, inWidth, inHeight ),
      mSecondSelectedObject( NULL ),
      mStopFollowingMode( false ) {

    // override value set by GiftButton
    mFadeLeft = false;
    }



void FollowButton::setStopFollowingMode( char inStopFollowing ) {
    mStopFollowingMode = inStopFollowing;
    }



void FollowButton::setSecondSelectedObject( DrawableObject *inObject ) {
    mSecondSelectedObject = inObject;
    }



void FollowButton::drawIcon( Vector3D *inCenter, double inRadius,
                             double inAlpha ) {


    // draw second gardner following first

    double offset = 2 * inRadius / 3;

    double drawRadius = inRadius / 3;

    // gardeners take up only 0.666 of their radius
    drawRadius = drawRadius / 0.66;




    Vector3D firstCenter( inCenter );
    firstCenter.mY += offset;
    
    if( mSelectedObject != NULL ) {
        // draw selected object
        mSelectedObject->draw( &firstCenter, drawRadius );
        }


    Vector3D secondCenter( inCenter );
    secondCenter.mY -= offset;
    
    if( mSecondSelectedObject != NULL ) {
        // draw selected object
        mSecondSelectedObject->draw( &secondCenter, drawRadius );
        }


    // draw arrow in between

    // rotate so vertical
    Angle3D arrowRotation( 0, 0, M_PI/2 );
    
    glColor4f( 1, 1, 1, inAlpha );
    // make arrow bigger
    double arrowRadius = drawRadius;
    
    drawTextureQuad( mArrowTexture, inCenter, arrowRadius, &arrowRotation );

    if( mStopFollowingMode ) {
        // cover with a red X

        mRedX.draw( inCenter, inRadius );
        }
    
    }



