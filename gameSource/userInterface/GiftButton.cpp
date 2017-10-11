/*
 * Modification History
 *
 * 2006-July-24   Jason Rohrer
 * Created.
 */



#include "GiftButton.h"
#include "../glCommon.h"

#include "minorGems/graphics/filters/BoxBlurFilter.h"



#include <GL/gl.h>



GiftButton::GiftButton( double inAnchorX, double inAnchorY,
                      double inWidth, double inHeight )
    : ButtonBase( inAnchorX, inAnchorY, inWidth, inHeight, true ) {

    // construct arrow texture


    // a blurry white triangle
    // black border
    int textureSize = 32;
    double halfTextureSize = 0.5 * textureSize;
    
    Image *textureImage = new Image( textureSize, textureSize, 4, false );

    double *channels[4];

    int pixelsPerChannel = textureSize * textureSize;
    
    int i;
    int p;
    for( i=0; i<4; i++ ) {
        channels[i] = textureImage->getChannel( i );
        }

    // start all pixels as black and transparent
    for( p=0; p<pixelsPerChannel; p++ ) {
        channels[0][p] = 0;
        channels[3][p] = 0;
        }
    // 1 and 2 same as channel zero
    memcpy( channels[1], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[2], channels[0], pixelsPerChannel * sizeof( double ) );
    
    
    double radius = textureSize / 3;


    // start near right edge
    // with a single-pixel column

    // at each column, add pixels

    double endRadius = radius;

    // number of columns occupied by triangle
    int triangleWidth = (int)( radius * 2 );
    
    double radiusGrowFactorPerLine = endRadius / triangleWidth;

    int startColumn = (int)( halfTextureSize + radius );
    int endColumn = (int)( halfTextureSize - radius );

    double currentRadius = 0;

    // get more transparent as radius increases
    double currentAlpha = 1;

    double alphaStep = -currentAlpha / triangleWidth;
    
    int y;
    int x;

    for( x=startColumn; x>endColumn; x-- ) {

        int startY = (int)( halfTextureSize - currentRadius );
        int endY = (int)( halfTextureSize + currentRadius );

        for( y=startY; y<endY; y++ ) {
            int pixelIndex = y * textureSize + x; 
            channels[3][ pixelIndex ] = currentAlpha;

            // set all in triangle to white
            channels[0][ pixelIndex ] = 1;
            channels[1][ pixelIndex ] = 1;
            channels[2][ pixelIndex ] = 1;
            }

        currentRadius += radiusGrowFactorPerLine;

        currentAlpha += alphaStep;

        if( currentAlpha > 1 ) {
            currentAlpha = 1;
            }
        if( currentAlpha < 0 ) {
            currentAlpha = 0;
            }
        }

    
    // blur alpha
    int blurRadius = 1;
    BoxBlurFilter blur( blurRadius );

    // blur all channels
    textureImage->filter( &blur, 3 );
    
    // rgb are identical, so blur one and copy
    textureImage->filter( &blur, 0 );

    memcpy( channels[1], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[2], channels[0], pixelsPerChannel * sizeof( double ) );
    

    mArrowTexture = new SingleTextureGL( textureImage,
                                         // no wrap
                                         false );

    delete textureImage;
    }



GiftButton::~GiftButton() {
    delete mArrowTexture;
    }



void GiftButton::drawIcon( Vector3D *inCenter, double inRadius,
                          double inAlpha ) {


    // ofset of arrow and item from center
    double offset = inRadius / 3;

    double drawRadius = inRadius - offset;

    Vector3D targetCenter( inCenter );
    targetCenter.mX -= offset;
    
    if( mSelectedObject != NULL ) {
        // draw selected object
        mSelectedObject->draw( &targetCenter, drawRadius );
        }


    Vector3D arrowCenter( inCenter );
    arrowCenter.mX += offset;

    // radius of arrow icon is 0.666 of texture radius
    drawRadius = drawRadius / 0.6666;
    
    glColor4f( 1, 1, 1, inAlpha );
    drawTextureQuad( mArrowTexture, &arrowCenter, drawRadius );

    }



