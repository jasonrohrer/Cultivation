/*
 * Modification History
 *
 * 2006-July-9   Jason Rohrer
 * Created.
 */



#include "EmotionButton.h"



#include <GL/gl.h>



EmotionButton::EmotionButton( double inAnchorX, double inAnchorY,
                              double inWidth, double inHeight )
    : ButtonBase( inAnchorX, inAnchorY, inWidth, inHeight ) {

    }



void EmotionButton::drawIcon( Vector3D *inCenter, double inRadius,
                              double inAlpha ) {

    // draw a red heart

    glColor4f( 1, 0, 0, inAlpha );

    glBegin( GL_TRIANGLE_STRIP ); {
        glVertex2d( inCenter->mX - inRadius, inCenter->mY + inRadius );
        glVertex2d( inCenter->mX, inCenter->mY + inRadius / 2 );        
        glVertex2d( inCenter->mX, inCenter->mY - inRadius );

        // one more vert to define second triangle
        glVertex2d( inCenter->mX + inRadius, inCenter->mY + inRadius );
        }
    glEnd(); 
    }



