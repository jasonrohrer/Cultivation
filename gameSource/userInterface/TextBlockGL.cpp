/*
 * Modification History
 *
 * 2006-December-19    Jason Rohrer
 * Created.
 */


#include "TextBlockGL.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SimpleVector.h"



TextBlockGL::TextBlockGL(
    double inAnchorX, double inAnchorY, double inWidth,
    double inHeight, char *inString, TextGL *inText,
    int inMaxCharactersPerLine )
    : LabelGL( inAnchorX, inAnchorY, inWidth, inHeight, inString, inText ),
      mMaxCharactersPerLine( inMaxCharactersPerLine ) {

    }


        
void TextBlockGL::fireRedraw() {

    double charWidth = mWidth / mMaxCharactersPerLine;
    
    
    int numChars = strlen( mString );

    

    
    // first, split into lines

    SimpleVector<char*> lines;

    double widestLineWidthFraction = 0;

    
    int charsLeft = numChars;
    int nextChar = 0;
    while( charsLeft > 0 ) {

        
        while( nextChar < numChars - 1  && mString[ nextChar ] == ' ' ) {
            // skip spaces at start of line
            nextChar ++;
            charsLeft --;
            }

        
        // skip chars we have done
        char *lineString = stringDuplicate( &( mString[ nextChar ] ) );

        // truncate at end of line
        char moreCharsAfterLine = false;
        if( mMaxCharactersPerLine < strlen( lineString ) ) {
            lineString[ mMaxCharactersPerLine ] = '\0';

            moreCharsAfterLine = true;
            }

        if( moreCharsAfterLine ) {
            // truncate at first space before end of line
            char foundSpace = false;
            for( int i=strlen(lineString)-1; i>=0 && !foundSpace; i-- ) {
                
                if( lineString[i] == ' ' ) {
                    lineString[i] = '\0';
                    foundSpace = true;
                    }
                }
            }
        
        int charsOnLine = strlen( lineString );
        
        lines.push_back( lineString );

        double lineWidth = mText->measureTextWidth( lineString );

        if( lineWidth > widestLineWidthFraction ) {
            widestLineWidthFraction = lineWidth;
            }
        
        nextChar += charsOnLine;
        charsLeft -= charsOnLine;
        }

    int numLines = lines.size();
    
    

    double lineHeight = mHeight / numLines;

    // each line includes between-line spacing
    double charHeightFraction = 0.65;
    double charHeight = charHeightFraction * lineHeight;

    
    // preserve character 1:1 aspect ratio
    if( charHeight < charWidth ) {
        charWidth = charHeight;
        }
    else if( charWidth < charHeight ) {
        charHeight = charWidth;
        lineHeight = charHeight / charHeightFraction;
        }

    double widestLineWidth = widestLineWidthFraction * charHeight;
    
    // can scale everything up a bit if we have room
    double scaleFactorA = mWidth / widestLineWidth;
    double scaleFactorB = ( mHeight / numLines ) / lineHeight;

    // use whatever is smaller
    double scaleFactor = scaleFactorA;
    if( scaleFactorB < scaleFactorA ) {
        scaleFactor = scaleFactorB;
        }

    charWidth *= scaleFactor;
    charHeight *= scaleFactor;
    lineHeight *= scaleFactor;


    
    double topY = mAnchorY + mHeight;

    for( int i=0; i<numLines; i++ ) {

        char *lineString = *( lines.getElement( i ) );
        
        int charsOnLine = strlen( lineString );
        
        double lineWidth = charsOnLine * charWidth; 
        
        mText->drawText( lineString, mAnchorX, topY - charHeight,
                         lineWidth, charHeight );

        delete [] lineString;
        
        topY -= lineHeight;
        }
    }



