/*
 * Modification History
 *
 * 2006-July-4   Jason Rohrer
 * Created.
 *
 * 2006-July-5   Jason Rohrer
 * Changed to subclass ButtonBase.
 */



#include "PlotButton.h"


#include <GL/gl.h>



PlotButton::PlotButton( double inAnchorX, double inAnchorY,
                        double inWidth, double inHeight )
    : ButtonBase( inAnchorX, inAnchorY, inWidth, inHeight ) {

    }



void PlotButton::drawIcon( Vector3D *inCenter, double inRadius,
                            double inAlpha ) {


    // white square

    glLineWidth( 1 );
    glColor4f( 1, 1, 1, inAlpha );

    glBegin( GL_LINE_LOOP ); {
		
		glVertex2d( inCenter->mX - inRadius, inCenter->mY - inRadius );
        glVertex2d( inCenter->mX - inRadius, inCenter->mY + inRadius );
        glVertex2d( inCenter->mX + inRadius, inCenter->mY + inRadius );
        glVertex2d( inCenter->mX + inRadius, inCenter->mY - inRadius );
        }
    glEnd();
    }

