/*
 * Modification History
 *
 * 2006-July-23   Jason Rohrer
 * Created.
 *
 * 2006-October-27   Jason Rohrer
 * Added support for poisoned fruit.
 */



#include "Fruit.h"
#include "glCommon.h"

#include <GL/gl.h>

#include "minorGems/graphics/filters/BoxBlurFilter.h"


//#include "minorGems/io/file/FileOutputStream.h"
//#include "minorGems/graphics/converters/TGAImageConverter.h"


Fruit::Fruit( PlantGenetics *inParentGenetics, Seeds *inSeeds,
              double inRipenRate )
    : mParentGenetics( inParentGenetics ),
      mStatus( 0 ),
      mPoisoned( false ),
      mSeeds( inSeeds ),
      mRipenRate( inRipenRate ),
      mLastRotation( 0, 0, 0 ) {

    Color *nutritionColor = mParentGenetics.getColor( fruitNutrition );
    
    mNutrition.setValues( nutritionColor );

    delete nutritionColor;



    // generate the texture


    // a blurry white circle that is gray at edges
    int textureSize = 32;
    
    
    Image *textureImage = new Image( textureSize, textureSize, 4, false );

    double *channels[4];

    int pixelsPerChannel = textureSize * textureSize;
    
    int i;
    int p;
    for( i=0; i<4; i++ ) {
        channels[i] = textureImage->getChannel( i );
        }

    // start all pixels as gray and transparent
    for( p=0; p<pixelsPerChannel; p++ ) {
        channels[0][p] = 0.5;
        channels[1][p] = 0.5;
        channels[2][p] = 0.5;
        channels[3][p] = 0;
        }

    int blurRadius = 1;

    double lobeRate = mParentGenetics.getParameter( fruitLobeRate );
    double lobeDepth = mParentGenetics.getParameter( fruitLobeDepth );
    
    double maxRadius = textureSize / 2 -  blurRadius - 1;

    // idea:  maxRadius = baseRadius + lobeDepth * baseRadius;
    // maxRadius = baseRadius * ( 1 + lobeDepth );
    // baseRadius = maxRadius /  ( 1 + lobeDepth );
    double baseRadius = maxRadius / ( 1 + lobeDepth );
    
    
    int y;
    int x;
    for( y=0; y<textureSize; y++ ) {
        for( x=0; x<textureSize; x++ ) {

            double alphaValue;

            double xRelative = x - (0.5 *textureSize );
            double yRelative = y - (0.5 *textureSize );

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

            // compute function at this angle to determine radius
            double functionRadius =
                baseRadius * lobeDepth * sin( lobeRate * angle )
                + baseRadius;
            
            if( pixelRadius <= functionRadius ) {
                alphaValue = 1;

                // color based on radius

                double grayWeight = pixelRadius / functionRadius;

                double colorLevel = ( 1 - grayWeight ) * 1 +
                    grayWeight * 0.5;

                channels[0][ pixelIndex ] = colorLevel;
                channels[1][ pixelIndex ] = colorLevel;
                channels[2][ pixelIndex ] = colorLevel;
                }
            else {
                alphaValue = 0;
                }

            channels[3][ pixelIndex ] = alphaValue;
            }
        }

    // blur alpha
    BoxBlurFilter blur( blurRadius );

    textureImage->filter( &blur, 3 );
    

    mShapeTexture = new SingleTextureGL( textureImage,
                                         // no wrap
                                         false );


    /*
    File outFileB( NULL, "fruit.tga" );
    FileOutputStream *outStreamB = new FileOutputStream( &outFileB );

    TGAImageConverter converter;
    
    converter.formatImage( textureImage, outStreamB );
    delete outStreamB;

    exit( 0 );
    */
    
    delete textureImage;
    }



Fruit::~Fruit() {
    delete mShapeTexture;
    }



Color *Fruit::getNutrition() {
    return mNutrition.copy();
    }



char Fruit::isRotten() {

    return ( mStatus >= 6 );
    }



char Fruit::isRipe() {
    return ( mStatus >= 2 );
    }



char Fruit::isPoisoned() {
    return mPoisoned;
    }



void Fruit::poison() {
    mPoisoned = true;
    }



Seeds *Fruit::getSeeds() {
    return new Seeds( &mSeeds );
    }
        


void Fruit::passTime( double inTimeDeltaInSeconds ) {
    double progressRate = mRipenRate;

    mStatus += inTimeDeltaInSeconds * progressRate;

    if( mStatus > 6 ) {
        mStatus = 6;
        }
    }



void Fruit::draw( Vector3D *inPosition, double inScale ) {
    // default rotation
    draw( inPosition, &mLastRotation, inScale );
    }



void Fruit::draw( Vector3D *inPosition, Angle3D *inRotation,
                  double inScale ) {

    double radius;

    Color drawColor;

    double leafGreen = mParentGenetics.getParameter( outerLeafGreen );

    Color ripeColor;

    if( mNutrition.r == 1 ) {
        ripeColor.setValues( 1, 0, 0, 1 );
        }
    else if( mNutrition.g == 1 ) {
        // show g nutrient as yellow, not green
        ripeColor.setValues( 1, 1, 0, 1 );
        }
    else if( mNutrition.b == 1 ) {
        // show b nutrient as purple, not blue
        ripeColor.setValues( 0.5, 0, 1, 1 );
        }

    // turn light grey when rotted
    Color rotColor( 0.8, 0.8, 0.8, 1 );
    
    
    if( mStatus < 1 ) {
        // growing
        radius = inScale * mStatus;

        
        drawColor.setValues( 0, leafGreen, 0, 1 );
        }
    else {
        radius = inScale;

        if( mStatus >= 2 && mStatus <=4 ) {
            // ripe/stable
            drawColor.setValues( &ripeColor );
            }
        else if( mStatus < 2 ) {
            // ripening 

            Color unripe( 0, leafGreen, 0, 1 );

            Color *mix = Color::linearSum( &ripeColor, &unripe, mStatus - 1 );

            drawColor.setValues( mix );
            delete mix;
            }
        else if( mStatus < 5 ){
            // browning
            

            Color *mix = Color::linearSum( &rotColor,
                                           &ripeColor, mStatus - 4 );

            drawColor.setValues( mix );
            delete mix;
            }
        else {
            // rotting
            drawColor.setValues( &rotColor );
            
            // shrink down to half size as we rot
            radius *=
                0.5 * ( 6 - mStatus ) + 0.5;
            }
        }

    

    // fruit black if poisoned
    if( mPoisoned ) {
        drawColor.setValues( 0, 0, 0, 1 );
        }

    
    
    Vector3D corners[4];

    // first, set up corners relative to 0,0
    corners[0].mX = - radius;
    corners[0].mY = - radius;
    corners[0].mZ = 0;

    corners[1].mX = radius;
    corners[1].mY = - radius;
    corners[1].mZ = 0;

    corners[2].mX = radius;
    corners[2].mY = radius;
    corners[2].mZ = 0;

    corners[3].mX = - radius;
    corners[3].mY = radius;
    corners[3].mZ = 0;

    int i;
    
    // now rotate around center
    // then add inPosition so that center is at inPosition
    for( i=0; i<4; i++ ) {
        corners[i].rotate( inRotation );
        corners[i].add( inPosition );
        }

    
    mShapeTexture->enable();
    glBegin( GL_QUADS ); {
        
        setGLColor( &drawColor );

        glTexCoord2f( 0, 0 );
        glVertex3d( corners[0].mX, corners[0].mY, corners[0].mZ );

        glTexCoord2f( 1, 0 );
        glVertex3d( corners[1].mX, corners[1].mY, corners[1].mZ );

        glTexCoord2f( 1, 1 );
        glVertex3d( corners[2].mX, corners[2].mY, corners[2].mZ );

        glTexCoord2f( 0, 1 );
        glVertex3d( corners[3].mX, corners[3].mY, corners[3].mZ );
        }
    glEnd();
    mShapeTexture->disable();

    /*
    // shrinks vertically as it rots
    double heightRadius = inScale * mRotStatus;

    double widthRadius = inScale;

    glColor4f( mNutrition.r, mNutrition.g, mNutrition.b, 1 );
        
    glBegin( GL_TRIANGLES ); {
        glVertex2d( inPosition->mX - widthRadius,
                    inPosition->mY - heightRadius );
        glVertex2d( inPosition->mX, inPosition->mY + heightRadius );
        glVertex2d( inPosition->mX + widthRadius,
                    inPosition->mY - heightRadius );
            }
    glEnd();
    */

    // save a new default rotation
    mLastRotation.setComponents( inRotation );
    }


