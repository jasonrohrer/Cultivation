/*
 * Modification History
 *
 * 2006-July-4   Jason Rohrer
 * Created.
 *
 * 2006-July-5   Jason Rohrer
 * Changed to subclass ButtonBase.
 */



#include "PlantButton.h"
#include "../features.h"


#include <GL/gl.h>



PlantButton::PlantButton( double inAnchorX, double inAnchorY,
                          double inWidth, double inHeight )
    : ButtonBase( inAnchorX, inAnchorY, inWidth, inHeight ) {

    }



void PlantButton::drawIcon( Vector3D *inCenter, double inRadius,
                            double inAlpha ) {

    if( mSelectedObject != NULL ) {
        // draw selected object, which is a leaf with it's stem at the
        // given position, and its length at the given scale

        double leafLength = 2 * inRadius;

        Vector3D stemStart( inCenter );
        stemStart.mY -= inRadius;

        if( Features::drawNicePlantLeaves ) {
            // white to let texture color show through
            glColor4f( 1, 1, 1, inAlpha );
            }
        else {
            // green to color the simple quad lines
            glColor4f( 0, 1, 0, inAlpha );
            }
        mSelectedObject->draw( &stemStart, leafLength );
        }
    }



