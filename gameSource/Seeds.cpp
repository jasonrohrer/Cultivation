/*
 * Modification History
 *
 * 2006-July-27   Jason Rohrer
 * Created.
 */


#include "Seeds.h"
#include "SoilMap.h"
#include "glCommon.h"

#include "minorGems/graphics/filters/BoxBlurFilter.h"

#include <stdio.h>


#include "minorGems/util/random/StdRandomSource.h"

extern StdRandomSource globalRandomSource;



Seeds::Seeds()
    : mGenetics(), mSampleLeaf( NULL ) {

    getParametersFromGenetics();
    }



Seeds::Seeds( Seeds *inParentA, Seeds *inParentB )
    : mGenetics( &( inParentA->mGenetics ),
                 &( inParentB->mGenetics ) ),
    mSampleLeaf( NULL ) {

    getParametersFromGenetics();
    }



Seeds::Seeds( Seeds *inSeedsToCopy )
    : mGenetics( &( inSeedsToCopy->mGenetics ) ),
      mSampleLeaf( NULL ) {
        
    getParametersFromGenetics();
    }



Seeds::~Seeds() {
    if( mSampleLeaf != NULL ) {
        delete mSampleLeaf;
        mSampleLeaf = NULL;
        }

    delete mSeedTexture;
    }


    
void Seeds::getParametersFromGenetics() {
    mIdealSoilType = mGenetics.getParameter( idealSoilType );


    Color *nutrition = mGenetics.getColor( fruitNutrition );
    Color *parentNutrition = mGenetics.getColor( parentFruitNutrition );

    mFruitNutrition.setValues( nutrition );
    mParentFruitNutition.setValues( parentNutrition );


    delete nutrition;
    delete parentNutrition;


    double seedWidthParameter = mGenetics.getParameter( seedWidth );
    

    // generate texture 

    // a blurry white seed shape (point down)
    int textureSize = 32;
    
    Image *textureImage = new Image( textureSize, textureSize, 4, false );

    double *channels[4];

    int pixelsPerChannel = textureSize * textureSize;
    
    int i;
    int p;
    for( i=0; i<4; i++ ) {
        channels[i] = textureImage->getChannel( i );
        }

    
    double radius = textureSize / 3;

    int y;
    int x;


    // all white, transparent
    for( p=0; p<pixelsPerChannel; p++ ) {
        channels[0][p] = 1;
        channels[1][p] = 1;
        channels[2][p] = 1;
        channels[3][p] = 0;
        }

    // alpha in seed shape


    // seed shape similar to single petal from a rose curve

    int yStart = textureSize / 6;
    int yEnd = textureSize - textureSize / 6;
    
    for( y=yStart; y<=yEnd; y++ ) {
        for( x=0; x<textureSize; x++ ) {

            double alphaValue;

            double xRelative = x - (0.5 *textureSize );
            //double yRelative = y - (0.5 *textureSize );
            double yRelative = y;

            // make seed image wider
            xRelative *= 0.5 - 0.25 * seedWidthParameter;

            // center vertically (instead of having seed tip at very bottom
            // of image)
            yRelative -= yStart;
            
            double pixelRadius = sqrt( xRelative * xRelative +
                                       yRelative * yRelative );

            int pixelIndex = y * textureSize + x;

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

            angle += M_PI;
            
            // compute rose function at this angle to determine radius
            double functionRadius = 2 * radius * sin( 3 * angle );
            
            if( pixelRadius <= functionRadius ) {
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
    
    mSeedTexture = new SingleTextureGL( textureImage,
                                        // no wrap
                                        false );

    delete textureImage;    
    }



PlantLeaf *Seeds::getSampleLeaf() {
    if( mSampleLeaf == NULL ) {
        mSampleLeaf = new PlantLeaf( &mGenetics );
        }

    return mSampleLeaf;
    }


void Seeds::draw( Vector3D *inPosition, double inScale ) {

    //double radius = inScale * 0.6;
    double radius = inScale;

    
    Color ripeColor;
    
    if( mFruitNutrition.r == 1 ) {
        ripeColor.setValues( 1, 0, 0, 1 );
        }
    else if( mFruitNutrition.g == 1 ) {
        // show g nutrient as yellow, not green
        ripeColor.setValues( 1, 1, 0, 1 );
        }
    else if( mFruitNutrition.b == 1 ) {
        // show b nutrient as purple, not blue
        ripeColor.setValues( 0.5, 0, 1, 1 );
        }

    
    Vector3D nutritionCenter( inPosition );
    Vector3D seedCenter( inPosition );

    // draw nutrition in bottom corner
    nutritionCenter.mX += radius / 2;
    nutritionCenter.mY -= radius / 2;

    
    setGLColor( &ripeColor );
    drawBlurCircle( &nutritionCenter, radius / 4 );

    
    
    // seed in center of rectangle
    Color *seedColor = SoilMap::mapSoilToColor( mIdealSoilType );

    Color shadowColor;
    if( mIdealSoilType == 0 ) {
        shadowColor.setValues( 1, 1, 1, 1 );
        }
    else {
        shadowColor.setValues( 0, 0, 0, 1 );
        }

    // draw shadow

    glColor4f( shadowColor.r, shadowColor.g, shadowColor.b, 1 );

    drawTextureQuad( mSeedTexture, &seedCenter, 1.1 * radius );


    // draw seed
    glColor4f( seedColor->r, seedColor->g, seedColor->b, 1 );

    drawTextureQuad( mSeedTexture, &seedCenter, radius );
        
    delete seedColor;    
    
    }

