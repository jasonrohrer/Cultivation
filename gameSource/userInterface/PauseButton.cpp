/*
 * Modification History
 *
 * 2006-December-14   Jason Rohrer
 * Created.
 */



#include "PauseButton.h"

#include "../glCommon.h"

#include "minorGems/graphics/filters/BoxBlurFilter.h"


#include <GL/gl.h>



PauseButton::PauseButton( double inAnchorX, double inAnchorY,
                          double inWidth, double inHeight )
    : ButtonBase( inAnchorX, inAnchorY, inWidth, inHeight ) {


    // construct pause texture


    // two blurry white vertical bars
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
        }
    // 1, 2, and 3 same as channel zero
    memcpy( channels[1], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[2], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[3], channels[0], pixelsPerChannel * sizeof( double ) );
    
    
    double radius = textureSize / 4;

    int yStart = (int)( halfTextureSize - radius );
    int yEnd = (int)( halfTextureSize + radius );

    int xStart = (int)( halfTextureSize - radius );
    int xEnd = (int)( halfTextureSize + radius );

    // space between bars is equal to bars
    int barWidth = (int)( 2 * radius / 3 );
    
    for( int y=yStart; y<=yEnd; y++ ) {

        for( int x=xStart; x<=xEnd; x++ ) {

            if( x <= xStart + barWidth ||
                x >= xEnd - barWidth ) {

                // in one of the two filled bars

                int pixelIndex = y * textureSize + x; 

                // white
                channels[0][ pixelIndex ] = 1;
                channels[1][ pixelIndex ] = 1;
                channels[2][ pixelIndex ] = 1;

                // opaque
                channels[3][ pixelIndex ] = 1;
                }
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
    

    mPauseTexture = new SingleTextureGL( textureImage,
                                         // no wrap
                                         false );

    delete textureImage;
    }



PauseButton::~PauseButton() {
    delete mPauseTexture;
    }



void PauseButton::drawIcon( Vector3D *inCenter, double inRadius,
                              double inAlpha ) {

    // draw a yellow pause symbol 

    glColor4f( 1, 1, 0, inAlpha );

    // compensate for small radius of texture
    double drawRadius = inRadius / 0.6666;
    
    drawTextureQuad( mPauseTexture, inCenter, drawRadius );
    }



