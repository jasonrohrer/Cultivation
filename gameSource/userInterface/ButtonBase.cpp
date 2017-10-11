/*
 * Modification History
 *
 * 2006-July-4   Jason Rohrer
 * Created.
 */



#include "ButtonBase.h"
#include "../glCommon.h"

#include <GL/gl.h>

#include "minorGems/graphics/filters/BoxBlurFilter.h"


#include "minorGems/util/random/StdRandomSource.h"

extern StdRandomSource globalRandomSource;




ButtonBase::ButtonBase( double inAnchorX, double inAnchorY,
                        double inWidth, double inHeight,
                        char inFadeLeft )
    : ButtonGL( inAnchorX, inAnchorY, inWidth, inHeight ),
      mFadeLeft( inFadeLeft ),
      mIconCenter( mAnchorX + mHeight / 2,
                   mAnchorY + mHeight / 2,
                   0 ),
      mSelectedObject( NULL ) {

    mIconRadius = mWidth / 3;
    if( mHeight < mWidth ) {
        mIconRadius = mHeight / 3;
        }
    }



ButtonBase::~ButtonBase() {
    }



void ButtonBase::setSelectedObject( DrawableObject *inObject ) {
    mSelectedObject = inObject;
    }



void ButtonBase::drawPressed() {
    drawWithColor( 0.1f, 0.1f, 0.1f );
    }



void ButtonBase::drawUnpressed() {
    drawWithColor( 0.3f, 0.3f, 0.3f );
    }


void ButtonBase::drawWithColor( float inRed, float inGreen, float inBlue ) {

    double alpha = 1.0;

    if( ! isEnabled() ) {
        alpha = 0.25;
        }

    double alphaBottomRight, alphaTopRight, alphaBottomLeft, alphaTopLeft;

    if( mFadeLeft ) {
        alphaBottomRight = alpha;
        alphaTopRight = alpha;

        alphaBottomLeft = 0;
        alphaTopLeft = 0;
        }
    else {
        alphaBottomRight = 0;
        alphaTopRight = alpha;

        alphaBottomLeft = 0;
        alphaTopLeft = alpha;
        }
    
	glBegin( GL_QUADS ); {

        glColor4f( inRed, inGreen, inBlue, alphaBottomLeft );
		glVertex2d( mAnchorX, mAnchorY );

        glColor4f( inRed, inGreen, inBlue, alphaTopLeft );
        glVertex2d( mAnchorX, mAnchorY + mHeight );

        glColor4f( inRed, inGreen, inBlue, alphaTopRight );
        glVertex2d( mAnchorX + mWidth, mAnchorY + mHeight );

        glColor4f( inRed, inGreen, inBlue, alphaBottomRight );
        glVertex2d( mAnchorX + mWidth, mAnchorY );

        }
    glEnd(); 

    
    if( isEnabled() ) {
        // add a border 
        glBegin( GL_LINE_LOOP ); {

            glColor4f( 0.7f, 0.7f, 0.7f, alphaBottomLeft );
            glVertex2d( mAnchorX, mAnchorY );

            glColor4f( 0.7f, 0.7f, 0.7f, alphaTopLeft );
            glVertex2d( mAnchorX, mAnchorY + mHeight );

            glColor4f( 0.7f, 0.7f, 0.7f, alphaTopRight );
            glVertex2d( mAnchorX + mWidth, mAnchorY + mHeight );

            glColor4f( 0.7f, 0.7f, 0.7f, alphaBottomRight );
            glVertex2d( mAnchorX + mWidth, mAnchorY );
            }
        glEnd();
        }
    
    drawIcon( &mIconCenter, mIconRadius, alpha );
    }



