/*
 * Modification History
 *
 * 2006-July-13   Jason Rohrer
 * Created.
 */



#include "TextDisplay.h"

#include "minorGems/util/stringUtils.h"


#include <GL/gl.h>
#include <GL/glut.h>



TextDisplay::TextDisplay( const char *inLongestPossibleText,
                          Color *inColor,
                          double inAnchorX, double inAnchorY, double inWidth,
                          double inHeight )
    : GUIComponentGL( inAnchorX, inAnchorY, inWidth, inHeight ),
      mColor(),
      mText( NULL ) {


    mColor.setValues( inColor );


    // compute scale factor
    int i;
    int length = strlen( inLongestPossibleText );

    
    int totalWidth = 0;
    for( i=0; i<length; i++ ) {
        totalWidth += glutStrokeWidth( GLUT_STROKE_ROMAN,
                                       inLongestPossibleText[i] );
        }

    mScaleFactor = mWidth / totalWidth;
    }



TextDisplay::~TextDisplay() {
    if( mText != NULL ) {
        delete [] mText;
        }
    }



void TextDisplay::setText( const char *inText ) {
    if( mText != NULL ) {
        delete [] mText;
        }
    mText = stringDuplicate( inText );

    
    // try computing a new scale factor
    int i;
    int length = strlen( mText );

    
    int totalWidth = 0;
    for( i=0; i<length; i++ ) {
        totalWidth += glutStrokeWidth( GLUT_STROKE_ROMAN,
                                       mText[i] );
        }

    double newScaleFactor = mWidth / totalWidth;

    if( newScaleFactor < mScaleFactor ) {
        // this new text longer than the longest we've seen so far

        mScaleFactor = newScaleFactor;
        }
    }

        

void TextDisplay::fireRedraw() {


    glLineWidth( 2 );
    glColor4f( mColor.r, mColor.g, mColor.b, mColor.a );  
    
    glPushMatrix();

    

    glTranslatef( mAnchorX, mAnchorY, 0 );

    glScalef( mScaleFactor, mScaleFactor, mScaleFactor );

    
    int length = strlen( mText );
    
    for( int i=0; i<length; i++ ) {

        glutStrokeCharacter( GLUT_STROKE_ROMAN, mText[i] );
        }
    
    glPopMatrix();
    
    }
