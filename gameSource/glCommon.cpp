/*
 * Modification History
 *
 * 2006-August-14   Jason Rohrer
 * Created.
 *
 * 2006-November-8   Jason Rohrer
 * Added corner colors to textured quad function.
 */


#include "glCommon.h"
#include "features.h"

#include "minorGems/graphics/filters/BoxBlurFilter.h"


#include "GL/gl.h"

#include <math.h>


SingleTextureGL *commonCircleTexture = NULL;
SingleTextureGL *commonPlusTexture = NULL;



void initCommonTextures() {
    // generate texture for gardener base
    
    // a blurry white circle that is gray at edges
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

    // start all pixels as white and transparent
    for( p=0; p<pixelsPerChannel; p++ ) {
        channels[0][p] = 1;
        channels[3][p] = 0;
        }
    // 1 and 2 same as channel zero
    memcpy( channels[1], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[2], channels[0], pixelsPerChannel * sizeof( double ) );
    
    
    double radius = textureSize / 3;

    int y;
    int x;
    for( y=0; y<textureSize; y++ ) {

        double yRelative = y - halfTextureSize;
        
        for( x=0; x<textureSize; x++ ) {

            double alphaValue;

            
            double xRelative = x - halfTextureSize;
 
            double pixelRadius = sqrt( xRelative * xRelative +
                                       yRelative * yRelative );

            int pixelIndex = y * textureSize + x;
            
            if( pixelRadius <= radius ) {
                alphaValue = 1;
                }
            else {
                alphaValue = 0;
                }

            channels[3][ pixelIndex ] = alphaValue;
            }
        }

    // blur alpha
    int blurRadius = 1;
    BoxBlurFilter blur( blurRadius );

    textureImage->filter( &blur, 3 );
    

    commonCircleTexture = new SingleTextureGL( textureImage,
                                               // no wrap
                                               false );



    // plus texture

    // start all pixels as white and transparent
    for( p=0; p<pixelsPerChannel; p++ ) {
        channels[0][p] = 1;
        channels[3][p] = 0;
        }
    // 1 and 2 same as channel zero
    memcpy( channels[1], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[2], channels[0], pixelsPerChannel * sizeof( double ) );
    
    
    radius = textureSize / 3;

    int plusWidth = 2;

    // vertical bar
    for( y = (int)(halfTextureSize - radius);
         y < halfTextureSize + radius; y++ ) {
        
        for( x = (int)(halfTextureSize - plusWidth);
             x < halfTextureSize + plusWidth; x++ ) {
            int pixelIndex = y * textureSize + x;
            
            channels[3][ pixelIndex ] = 1;
            }
        }

    // horizontal bar
    for( x = (int)(halfTextureSize - radius);
         x < halfTextureSize + radius; x++ ) {

        
        for( y = (int)(halfTextureSize - plusWidth);
             y < halfTextureSize + plusWidth; y++ ) {
            int pixelIndex = y * textureSize + x;
            
            channels[3][ pixelIndex ] = 1;
            }
        }

    // blur alpha

    textureImage->filter( &blur, 3 );
    

    commonPlusTexture = new SingleTextureGL( textureImage,
                                             // no wrap
                                             false );



    delete textureImage;
    }



void destroyCommonTextures() {
    if( commonCircleTexture != NULL ) {
        delete commonCircleTexture;
        commonCircleTexture = NULL;
        }
    if( commonPlusTexture != NULL ) {
        delete commonPlusTexture;
        commonPlusTexture = NULL;
        }
    }



void drawBlurCircle( Vector3D *inCenter, double inRadius ) {

    if( Features::drawNiceCircles ) {
        
        if( commonCircleTexture == NULL ) {
            initCommonTextures();
            }

        // circle in texture has radius of 0.666 of texture radius
        // compensate so that edge of circle is at inRadius
        double squareRadius = inRadius / 0.666;
        
        drawTextureQuad( commonCircleTexture, inCenter, squareRadius );
        }
    else {
        // draw simple line loops
        glBegin( GL_LINE_LOOP ); {
            drawCircle( inCenter, inRadius, 10 );
            }
        glEnd();
        }
    }



void drawBlurPlus( Vector3D *inCenter, double inRadius,
                   Angle3D *inRotation  ) {

    if( commonPlusTexture == NULL ) {
        initCommonTextures();
        }

    // circle in texture has radius of 0.666 of texture radius
    // compensate so that edge of circle is at inRadius
    double squareRadius = inRadius / 0.666;

    drawTextureQuad( commonPlusTexture, inCenter, squareRadius,
                     inRotation );
    }



void drawTextureQuad( SingleTextureGL *inTexture,
                      Vector3D *inCenter, double inRadius,
                      Angle3D *inRotation,
                      Color **inCornerColors ) {

    double squareRadius = inRadius;
    
    Vector3D corners[4];

    // first, set up corners relative to 0,0
    corners[0].mX = - squareRadius;
    corners[0].mY = - squareRadius;
    corners[0].mZ = 0;

    corners[1].mX = squareRadius;
    corners[1].mY = - squareRadius;
    corners[1].mZ = 0;

    corners[2].mX = squareRadius;
    corners[2].mY = squareRadius;
    corners[2].mZ = 0;

    corners[3].mX = - squareRadius;
    corners[3].mY = squareRadius;
    corners[3].mZ = 0;

    int i;
    
    // now add inCenter so that center is at inCenter
    for( i=0; i<4; i++ ) {
        // rotate if we have an angle
        if( inRotation != NULL ) {
            corners[i].rotate( inRotation );
            }
        
        corners[i].add( inCenter );
        }


    inTexture->enable();
    glBegin( GL_QUADS ); {

        if( inCornerColors != NULL ) {
            setGLColor( inCornerColors[0] );
            }
        glTexCoord2f( 0, 0 );
        glVertex3d( corners[0].mX, corners[0].mY, corners[0].mZ );

        if( inCornerColors != NULL ) {
            setGLColor( inCornerColors[1] );
            }
        glTexCoord2f( 1, 0 );
        glVertex3d( corners[1].mX, corners[1].mY, corners[1].mZ );

        if( inCornerColors != NULL ) {
            setGLColor( inCornerColors[2] );
            }
        glTexCoord2f( 1, 1 );
        glVertex3d( corners[2].mX, corners[2].mY, corners[2].mZ );

        if( inCornerColors != NULL ) {
            setGLColor( inCornerColors[3] );
            }
        glTexCoord2f( 0, 1 );
        glVertex3d( corners[3].mX, corners[3].mY, corners[3].mZ );
        }
    glEnd();
    inTexture->disable();
    }



void drawCircle( Vector3D *inCenter, double inRadius, int inNumSegments ) {

    for( int s=0; s<inNumSegments; s++ ) {

        double angle = (double)s / (double)inNumSegments;
        angle *= 2 * M_PI;
        
        double x = inCenter->mX +
            inRadius * cos( angle );

        double y = inCenter->mY +
            inRadius * sin( angle );
        
        glVertex3d( x, y, inCenter->mZ );
        }
    }



void setGLColor( Color *inColor ) {
    glColor4f( inColor->r, inColor->g, inColor->b, inColor->a );
    }

