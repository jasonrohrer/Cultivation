/*
 * Modification History
 *
 * 2006-July-27   Jason Rohrer
 * Created.
 */



#include "SoilMap.h"
#include "landscape.h"
#include "features.h"
#include "glCommon.h"

#include "minorGems/graphics/Image.h"

#include "minorGems/graphics/filters/BoxBlurFilter.h"


#include "minorGems/util/random/StdRandomSource.h"

extern StdRandomSource globalRandomSource;

#include <time.h>

//#include "minorGems/io/file/FileOutputStream.h"
//#include "minorGems/graphics/converters/TGAImageConverter.h"


double baseCosFunction( double inX ) {
    return cos( inX * M_PI + M_PI ) * 0.5 + 0.5;
    }



double multiCosFunction( double inX ) {
    return
        baseCosFunction(
            baseCosFunction(
                baseCosFunction(
                    baseCosFunction( inX ) ) ) );
    }



double maxMultiCosFunction = multiCosFunction( 1 );



double imageSmothingFunction( double inX ) {

    return multiCosFunction( inX ) / maxMultiCosFunction;
    }



double islandTimeSeed = globalRandomSource.getRandomInt();

        

SoilMap::SoilMap( Vector3D *inCornerA, Vector3D *inCornerB )
    : mCornerA( inCornerA ), mCornerB( inCornerB ) {

    
    // pick a new seed each time a soil map is generated
    islandTimeSeed = globalRandomSource.getRandomInt();
    
    
    int textureSize = 64;

    Image *textureImage = new Image( textureSize, textureSize, 4, false );

    double *channels[4];

    
    for( int i=0; i<4; i++ ) {
        channels[i] = textureImage->getChannel( i );
        }

    // 0, 0 in images is at cornerA
    // (textureSize-1), (textureSize-1) in image is at cornerB

    double xSpan = mCornerB.mX - mCornerA.mX;
    double xStart = mCornerA.mX;
    
    double ySpan = mCornerB.mY - mCornerA.mY;
    double yStart = mCornerA.mY;
    
    
    Vector3D position( 0, 0, 0 );


    int numPixels = textureSize * textureSize;

    double *landscapeValues = new double[ numPixels ];

    // track max and min so that we can normalize values
    mMaxLandscapeValue = DBL_MIN;
    mMinLandscapeValue = DBL_MAX;

    int x;
    
    for( x=0; x<textureSize; x++ ) {

        double worldX =  xSpan * ( (double)x / (double)textureSize ) + xStart; 
        
        for( int y=0; y<textureSize; y++ ) {

            double worldY =
                ySpan * ( (double)y / (double)textureSize ) + yStart; 
            
            int pixelIndex = x + y * textureSize;

            position.setCoordinates( worldX, worldY, 0 );

            // don't normalize here, since we still need to find max
            // and min that will make normalization work
            double landscapeValue = getSoilCondition( &position, false );

            landscapeValues[ pixelIndex ] = landscapeValue;

            if( landscapeValue < mMinLandscapeValue ) {
                mMinLandscapeValue = landscapeValue;
                }
            if( landscapeValue > mMaxLandscapeValue ) {
                mMaxLandscapeValue = landscapeValue;
                }

            }
        }

    // normalize and map into image colors

    for( int pixelIndex = 0; pixelIndex < numPixels; pixelIndex++ ) {

        // normalize
        double landscapeValue = landscapeValues[ pixelIndex ];

        landscapeValue -= mMinLandscapeValue;
        landscapeValue =
            landscapeValue / ( mMaxLandscapeValue - mMinLandscapeValue );


        // change from continuous to binary

        // continuous is linear
        // binary is a step function
        // cos is like a smoothed step function (looks better)
        //   though the underlying landscape parameter is binary for ouside
        //   calls to getSoilCondition
        landscapeValue = imageSmothingFunction( landscapeValue );
        /*
        if( landscapeValue < 0.5 ) {
            landscapeValue = 0;
            }
        else {
            landscapeValue = 1;
            }
        */
        
        Color *blend = mapSoilToColor( landscapeValue );
            
        
        channels[0][ pixelIndex ] = blend->r;
        channels[1][ pixelIndex ] = blend->g;
        channels[2][ pixelIndex ] = blend->b;
        channels[3][ pixelIndex ] = 1;

        delete blend;
        }
        
    delete [] landscapeValues;


    int blurRadius = 1;
    BoxBlurFilter blur( blurRadius );

    // blur image (skip bluring alpha)
    textureImage->filter( &blur, 0 );
    textureImage->filter( &blur, 1 );
    textureImage->filter( &blur, 2 );
    
    
    mTexture = new SingleTextureGL( textureImage, false );


    // now generate grit

    // set all pixels to white 
    int p;
    for( p=0; p<numPixels; p++ ) { 
        channels[0][p] = 1;
        }
    // 1 and 2 same as channel zero
    memcpy( channels[1], channels[0], numPixels * sizeof( double ) );
    memcpy( channels[2], channels[0], numPixels * sizeof( double ) );

    // fill alpha with random noise
    for( int pixelIndex = 0; pixelIndex < numPixels; pixelIndex++ ) {

        channels[3][pixelIndex] =
            globalRandomSource.getRandomBoundedDouble( 0, 1 );
        }
    
    // wrap
    mGritTexture = new SingleTextureGL( textureImage, true );


    
    /*
    File outFileB( NULL, "landTerrain.tga" );
    FileOutputStream *outStreamB = new FileOutputStream( &outFileB );

    TGAImageConverter converter;
    
    converter.formatImage( textureImage, outStreamB );
    delete outStreamB;

    exit( 0 );
    */


    delete textureImage;

    // now compute boundary texture

    // higher rez
    textureSize = 128;

    textureImage = new Image( textureSize, textureSize, 4, false );

    for( int i=0; i<4; i++ ) {
        channels[i] = textureImage->getChannel( i );
        }

    numPixels = textureSize * textureSize;
    
    
    // set all pixels to white and solid
    for( p=0; p<numPixels; p++ ) { 
        channels[0][p] = 1;
        }
    // 1, 2, and 3 same as channel zero
    memcpy( channels[1], channels[0], numPixels * sizeof( double ) );
    memcpy( channels[2], channels[0], numPixels * sizeof( double ) );
    memcpy( channels[3], channels[0], numPixels * sizeof( double ) );


    double radius = textureSize / 3;

    double halfTextureSize = textureSize * 0.5;
    
    int y;
    for( y=0; y<textureSize; y++ ) {

        double yRelative = y - halfTextureSize;
        
        for( x=0; x<textureSize; x++ ) {

            double alphaValue;

            
            double xRelative = x - halfTextureSize;
 
            double pixelRadius = sqrt( xRelative * xRelative +
                                       yRelative * yRelative );

            int pixelIndex = y * textureSize + x;


            // call copied from getSoilCondition below
            double randomRadiusModifier = variableRoughnessLandscape(
                // try using x and y directly
                10 * xRelative, 10 * yRelative, islandTimeSeed,
                0.01, 0.001, 0.25, 0.65, 5 );

            randomRadiusModifier -= 0.25;
            
            if( pixelRadius <= radius + 20 * randomRadiusModifier ) {
                alphaValue = 0;
                }
            else {
                alphaValue = 1;
                }

            channels[3][ pixelIndex ] = alphaValue;
            }
        }
    
    // blur alpha
    
    blur.setRadius( 2 );
    textureImage->filter( &blur, 3 );
    

    // wrap to hide edge
    mWaterBoundaryTexture = new SingleTextureGL( textureImage, true );


    /*
    File outFileB( NULL, "boundary.tga" );
    FileOutputStream *outStreamB = new FileOutputStream( &outFileB );

    TGAImageConverter converter;
    
    converter.formatImage( textureImage, outStreamB );
    delete outStreamB;

    exit( 0 );
    */

    mWaterBoundaryImage = textureImage;
    mWaterBoundaryImageAlpha = channels[3];
    mWaterBoundaryImageNumPixels = numPixels;
    mWaterBoundaryImageSize = textureSize;
    }

        

SoilMap::~SoilMap() {
    delete mTexture;
    delete mWaterBoundaryTexture;
    delete mWaterBoundaryImage;

    delete mGritTexture;
    }



double SoilMap::getSoilCondition( Vector3D *inPosition, char inNormalize ) {
    
    // call copied from customHighDetailLandscape in
    // subreal forever
    double landscapeValue = variableRoughnessLandscape(
        10*inPosition->mX, 10*inPosition->mY, islandTimeSeed,
        0.01, 0.001, 0.25, 0.65, 5 );

    if( inNormalize ) {
        landscapeValue -= mMinLandscapeValue;
        landscapeValue =
            landscapeValue / ( mMaxLandscapeValue - mMinLandscapeValue );

        // clip
        if( landscapeValue < 0 ) {
            landscapeValue = 0;
            }
        if( landscapeValue > 1 ) {
            landscapeValue = 1;
            }

        // change from continous to binary

        if( landscapeValue < 0.5 ) {
            landscapeValue = 0;
            }
        else {
            landscapeValue = 1;
            }
        }
    
    return landscapeValue;
    }



Color *SoilMap::mapSoilToColor( double inSoilCondition ) {
    Color greenColor( 0.5, 0.5, 0.2 );
    Color brownColor( 0.3, 0.2, 0 );
    
    return Color::linearSum( &greenColor,
                             &brownColor,
                             inSoilCondition );        
    }



void SoilMap::mapPointToBoundaryPixel( Vector3D *inPoint,
                                       int *outX, int *outY ) {

    // first, map inPoint coordinates to 0,1

    double pointX = inPoint->mX;
    double pointY = inPoint->mY;


    pointX = (pointX - mCornerA.mX) / ( mCornerB.mX - mCornerA.mX );
    pointY = (pointY - mCornerA.mY) / ( mCornerB.mY - mCornerA.mY );

    *outX = (int)( pointX * ( mWaterBoundaryImageSize - 1 ) );
    *outY = (int)( pointY * ( mWaterBoundaryImageSize - 1 ) );
    }



char SoilMap::isInBounds( Vector3D *inPosition ) {
    int x, y;

    mapPointToBoundaryPixel( inPosition, &x, &y );

    if( y < 0 || y >= mWaterBoundaryImageSize ||
        x < 0 || x >= mWaterBoundaryImageSize ) {

        // completely outside image
        return false;
        }
        
    double alphaValue =
        mWaterBoundaryImageAlpha[ y * mWaterBoundaryImageSize + x ];

    if( alphaValue == 0 ) {
        return true;
        }
    else {
        return false;
        }
    }


        
Vector3D *SoilMap::getClosestBoundaryPoint( Vector3D *inPosition ) {
    int x, y;


    char onLand = isInBounds( inPosition );

    
    mapPointToBoundaryPixel( inPosition, &x, &y );

    // for now, examine all pixels
    int closestX = -1;
    int closestY = -1;
    double closestDistance = DBL_MAX;

    for( int yIndex=0; yIndex<mWaterBoundaryImageSize; yIndex++ ) {

        // optimization:
        // distance to x,y is at least as big as distance between
        // y and yIndex

        int yDistance = y - yIndex;
        if( yDistance < 0 ) {
            yDistance = -yDistance;
            }

        if( yDistance < closestDistance ) {

            // can't rule out this entire row based on y distance
            // compute xDistance for each pixel in row
            
            for( int xIndex=0; xIndex<mWaterBoundaryImageSize; xIndex++ ) {

                int xDistance = x - xIndex;
                if( xDistance < 0 ) {
                    xDistance = -xDistance;
                    }

                if( xDistance < closestDistance ) {
                    // can't rule out based on xDistance alone

                    // compute true distance
                    double distance = sqrt( xDistance * xDistance +
                                            yDistance * yDistance );

                    if( distance < closestDistance ) {

                        int pixelIndex =
                            yIndex * mWaterBoundaryImageSize
                            + xIndex;

                        // note:  use 0.5 as water threshold to make
                        // sure we return a point that is *really* in
                        // the water, even with round-off errors (and not
                        // just a point right on the edge of the water)
                        
                        if( // closest water point if on land
                            onLand &&
                            mWaterBoundaryImageAlpha[ pixelIndex ] > 0.5
                            ||  // OR
                            // closest land point if on water
                            !onLand &&
                            mWaterBoundaryImageAlpha[ pixelIndex ] == 0 ) {

                            
                            closestDistance = distance;

                            closestY = yIndex;
                            closestX = xIndex;
                            }
                        }

                    }
                
                }
            }
        }


    // have closest x and y

    // map back into world
    double worldX = (double)closestX / (double)mWaterBoundaryImageSize;
    worldX = worldX * (mCornerB.mX - mCornerA.mX) + mCornerA.mX;

    double worldY = (double)closestY / (double)mWaterBoundaryImageSize;
    worldY = worldY * (mCornerB.mY - mCornerA.mY) + mCornerA.mY;

    return new Vector3D( worldX, worldY, 0 );
    }



void SoilMap::draw( Vector3D *inPosition, double inScale ) {

    glColor4f( 1, 1, 1, 1 );

    if( Features::drawSoil ) {
        mTexture->enable();
        glBegin( GL_QUADS ); {
            
            glTexCoord2f( 0, 0 );
            glVertex2d( mCornerA.mX, mCornerA.mY );
            
            glTexCoord2f( 1, 0 );
            glVertex2d( mCornerB.mX, mCornerA.mY );

            glTexCoord2f( 1, 1 );
            glVertex2d( mCornerB.mX, mCornerB.mY );

            glTexCoord2f( 0, 1 );
            glVertex2d( mCornerA.mX, mCornerB.mY );
            }
        glEnd();
        mTexture->disable();
        }
    else {
        // draw some grid points for motion reference

        glPointSize( 2 );

        double spacing = 3;
        
        glColor4f( 1, 0.5, 0, 0.25 );

        double xStart, xEnd, yStart, yEnd;

        xStart = mCornerA.mX;
        yStart = mCornerA.mY;

        xEnd = mCornerB.mX;
        yEnd = mCornerB.mY;

        Vector3D position( 0, 0, 0 );
        
        glBegin( GL_POINTS ); {
            for( double y=yStart; y<=yEnd; y+=spacing ) {
                for( double x=xStart; x<=xEnd; x+=spacing ) {

                    position.setCoordinates( x, y, 0 );

                    // skip drawing out-of-bounds points (in water)
                    if( isInBounds( &position ) ) {
                        Color *drawColor =
                            mapSoilToColor( getSoilCondition( &position ) );

                    
                        
                        setGLColor( drawColor );
                        glVertex2d( x, y );

                        delete drawColor;
                        }

                    }
                }
            }
        glEnd();
        }

    
    if( Features::drawWater ) {
        // cover with water
        glColor4f( 0, 0, 1, 1 );
        mWaterBoundaryTexture->enable();
        glBegin( GL_QUADS ); {

            glTexCoord2f( 0, 0 );
            glVertex2d( mCornerA.mX, mCornerA.mY );

            glTexCoord2f( 1, 0 );
            glVertex2d( mCornerB.mX, mCornerA.mY );

            glTexCoord2f( 1, 1 );
            glVertex2d( mCornerB.mX, mCornerB.mY );

            glTexCoord2f( 0, 1 );
            glVertex2d( mCornerA.mX, mCornerB.mY );
            }
        glEnd();
        mWaterBoundaryTexture->disable();
        }
    

    if( Features::drawSurfaceNoise ) {
        // cover with high-res grit
        double textureMappingRadius = 12;

        // stretch twice as large as underlying textures
    
        glColor4f( 0, 0, 0, 0.1 );
        mGritTexture->enable();
        glBegin( GL_QUADS ); {

            glTexCoord2f( 0, 0 );
            glVertex2d( 2 * mCornerA.mX, 2 * mCornerA.mY );

            glTexCoord2f( textureMappingRadius, 0 );
            glVertex2d( 2 * mCornerB.mX, 2 * mCornerA.mY );

            glTexCoord2f( textureMappingRadius, textureMappingRadius );
            glVertex2d( 2 * mCornerB.mX, 2 * mCornerB.mY );

            glTexCoord2f( 0, textureMappingRadius );
            glVertex2d( 2 * mCornerA.mX, 2 * mCornerB.mY );
            }
        glEnd();
        mGritTexture->disable();
        }
    }


