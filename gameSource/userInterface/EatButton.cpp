/*
 * Modification History
 *
 * 2006-July-6   Jason Rohrer
 * Created.
 */



#include "EatButton.h"
#include "../glCommon.h"

#include "minorGems/graphics/filters/BoxBlurFilter.h"



#include <GL/gl.h>



EatButton::EatButton( double inAnchorX, double inAnchorY,
                      double inWidth, double inHeight )
    : ButtonBase( inAnchorX, inAnchorY, inWidth, inHeight, true ) {


    // construct mouth texture


    // a blurry white circle with a mouth cut out
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

    double halfMouthAngle = M_PI / 4;
    
    int y;
    int x;
    for( y=0; y<textureSize; y++ ) {

        double yRelative = y - halfTextureSize;
        
        for( x=0; x<textureSize; x++ ) {

            double alphaValue = 0;

            
            double xRelative = x - halfTextureSize;
 
            double pixelRadius = sqrt( xRelative * xRelative +
                                       yRelative * yRelative );

            int pixelIndex = y * textureSize + x;

            // compute angle to cut out "mouth" in circle            
            double angle;
            if( pixelRadius > 0 ) {
                angle = acos( xRelative / pixelRadius );
                }
            else {
                angle = 0;
                }
            
            if( yRelative < 0 ) {
                angle = 2 * M_PI - angle;
                }
            
            
            if( pixelRadius <= radius ) {

                // don't set alpha inside mouth
                if( angle > halfMouthAngle
                    && angle < 2 * M_PI - halfMouthAngle ) {
                    
                    alphaValue = 1;
                    }
                }

            channels[3][ pixelIndex ] = alphaValue;
            // black where transparent
            channels[0][ pixelIndex ] = alphaValue;
            channels[1][ pixelIndex ] = alphaValue;
            channels[2][ pixelIndex ] = alphaValue;
            }
        }

    
    // blur alpha
    int blurRadius = 1;
    BoxBlurFilter blur( blurRadius );

    // blur all channels
    // all are identical, so blur one and copy
    textureImage->filter( &blur, 0 );

    memcpy( channels[1], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[2], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[3], channels[0], pixelsPerChannel * sizeof( double ) );
    

    mMouthTexture = new SingleTextureGL( textureImage,
                                        // no wrap
                                        false );

    delete textureImage;
    }



EatButton::~EatButton() {
    delete mMouthTexture;
    }



void EatButton::drawIcon( Vector3D *inCenter, double inRadius,
                          double inAlpha ) {

    // ofset of eating mouth and target from center
    double offset = inRadius / 3;

    double drawRadius = inRadius - offset;

    Vector3D targetCenter( inCenter );
    targetCenter.mX += offset;
    
    if( mSelectedObject != NULL ) {
        // draw selected object
        mSelectedObject->draw( &targetCenter, drawRadius );
        }


    Vector3D mouthCenter( inCenter );
    mouthCenter.mX -= offset;

    // radius of eat icon is 0.666 of texture radius
    drawRadius = drawRadius / 0.6666;
    
    glColor4f( 1, 1, 1, inAlpha );
    drawTextureQuad( mMouthTexture, &mouthCenter, drawRadius );
    }



