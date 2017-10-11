/*
 * Modification History
 *
 * 2006-August-29   Jason Rohrer
 * Created.
 *
 * 2006-September-13   Jason Rohrer
 * Fixed bug that caused flower petals to be too narrow.
 */


#include "PlantFlower.h"

#include "features.h"
#include "glCommon.h"

#include "minorGems/graphics/Image.h"
#include "minorGems/graphics/filters/BoxBlurFilter.h"

#include <GL/gl.h>


//#include "minorGems/io/file/FileOutputStream.h"
//#include "minorGems/graphics/converters/TGAImageConverter.h"


PlantFlower::PlantFlower( PlantGenetics *inGenetics )
    : mGenetics( inGenetics ) {



    // generate texture for flower center

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

    
    double radius = textureSize / 3;

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
            
            if( pixelRadius <= radius ) {
                alphaValue = 1;

                // color based on radius

                double grayWeight = pixelRadius / radius;

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
    int blurRadius = 2;
    BoxBlurFilter blur( blurRadius );

    textureImage->filter( &blur, 3 );
    

    mCenterTexture = new SingleTextureGL( textureImage,
                                          // no wrap
                                          false );


    // petal texture contains a blurred triangle

        
    // all white, transparent
    for( p=0; p<pixelsPerChannel; p++ ) {
        channels[0][p] = 1;
        channels[1][p] = 1;
        channels[2][p] = 1;
        channels[3][p] = 0;
        }

    // alpha in triangle shape

    double endRadius = (textureSize / 2) - blurRadius - 1;

    // number of lines occupied by triangle
    int triangleHeight = textureSize - 2 * (blurRadius + 1);
    
    double radiusGrowFactorPerLine = endRadius / triangleHeight;

    int startRow = blurRadius + 1;
    int endRow = textureSize - (blurRadius + 1);

    double currentRadius = 0;

    int centerX = textureSize / 2;
    
    for( y=startRow; y<endRow; y++ ) {

        int startX = (int)( centerX - currentRadius );
        int endX = (int)( centerX + currentRadius );

        for( x=startX; x<endX; x++ ) {
            channels[3][ y * textureSize + x ] = 1;
            }

        currentRadius += radiusGrowFactorPerLine;
        }
    
    

    // blur alpha
    textureImage->filter( &blur, 3 );
    
    mPetalTexture = new SingleTextureGL( textureImage,
                                         // no wrap
                                         false );
    
    
    /*
    File outFileB( NULL, "flowerPetal.tga" );
    FileOutputStream *outStreamB = new FileOutputStream( &outFileB );

    TGAImageConverter converter;
    
    converter.formatImage( textureImage, outStreamB );
    delete outStreamB;

    exit( 0 );
    */
    
    delete textureImage; 
    }

        

PlantFlower::~PlantFlower() {
    delete mCenterTexture;
    delete mPetalTexture;
    }


        
void PlantFlower::passTime( double inTimeDeltaInSeconds ) {

    }



void PlantFlower::draw( Vector3D *inPosition,
                        Angle3D *inRotation, double inScale,
                        double inStage ) {

    int numPetals = (int)( mGenetics.getParameter( flowerPetalCount ) );

    double centerRadius =
        inScale
        * mGenetics.getParameter( flowerCenterRadius )
        * 0.2;


    Color budColor( 0, mGenetics.getParameter( outerLeafGreen ), 0 );

    double budColorWeight = 2 - inStage;

    if( budColorWeight > 1 ) {
        budColorWeight = 1;
        }

    Color fadeColor( 0.4, 0.2, 0 );
    double fadeColorWeight = inStage - 3;
    if( fadeColorWeight > 1 ) {
        fadeColorWeight = 1;
        }
    
    
    if( inStage < 1 ) {
        // bud growing from zero radius up to full radius
        centerRadius *= inStage;
        }
    else if( inStage > 3 ) {
        // fading
        centerRadius *= (4 - inStage);
        }
    
    Angle3D angleIncrement( 0, 0, 2 * M_PI / numPetals );

    Angle3D startAngle( inRotation );


    double petalRadiusFactor;
    
    if( inStage < 1 ) {
        petalRadiusFactor = 0;
        }
    else if( inStage < 2 ){
        // petals growing
        petalRadiusFactor = inStage - 1;
        }
    else if( inStage < 3 ) {
        // holding
        petalRadiusFactor = 1;
        }
    else {
        // fading
        petalRadiusFactor = 4 - inStage;
        }
    

    // create points for a single petal and then rotate it to draw each petal
    Vector3D petalPoints[3];

    // point c, in center
    petalPoints[2].setCoordinates( 0, 0, 0 );

    Angle3D petalAngle( 0, 0, mGenetics.getParameter( flowerPetalAngle ) );
    
    // straight up from center
    double pointARadius =
        mGenetics.getParameter( flowerPetalPointARadius )
        * inScale * 0.2
        + centerRadius;
    
    pointARadius *= petalRadiusFactor;
    
    
    petalPoints[0].setCoordinates( 0, pointARadius, 0 );

    // now rotate
    petalPoints[0].rotate( &petalAngle );
    // rotation relative to petal angle
    Angle3D pointAAngle( 0, 0,
                         mGenetics.getParameter( flowerPetalPointAAngle ) );
    petalPoints[0].rotate( &pointAAngle );


    // straight up from center
    double pointBRadius =
        mGenetics.getParameter( flowerPetalPointBRadius )
        * inScale * 0.2
        + centerRadius;

    pointBRadius *= petalRadiusFactor;
    
    
    petalPoints[1].setCoordinates( 0, pointBRadius, 0 );

    // now rotate
    petalPoints[1].rotate( &petalAngle );
    // rotation relative to petal angle
    Angle3D pointBAngle( 0, 0,
                         mGenetics.getParameter( flowerPetalPointBAngle ) ); 
    petalPoints[1].rotate( &pointBAngle );

    
    
    Color *petalPointColors[3];

    petalPointColors[0] = mGenetics.getColor( flowerPetalPointAColor );
    petalPointColors[1] = mGenetics.getColor( flowerPetalPointBColor );
    petalPointColors[2] = mGenetics.getColor( flowerPetalPointCColor );

    int i;

    
    if( inStage >= 1 && inStage < 2 ) {
        // blend petal colors with bud color
        
        for( i=0; i<3; i++ ) {
            Color *drawColor = Color::linearSum( &budColor,
                                                 petalPointColors[i],
                                                 budColorWeight );
            delete petalPointColors[i];
            petalPointColors[i] = drawColor;
            }
        }
    else if( inStage >= 3 && inStage <= 4 ) {
        // blend with fade color
        for( i=0; i<3; i++ ) {
            Color *drawColor = Color::linearSum( &fadeColor,
                                                 petalPointColors[i],
                                                 fadeColorWeight );
            delete petalPointColors[i];
            petalPointColors[i] = drawColor;
            }
        }

    
    double maxRadius = pointARadius;
    if( maxRadius < pointBRadius ) {
        maxRadius = pointBRadius;
        }
    if( maxRadius < centerRadius ) {
        maxRadius = centerRadius;
        }
        
    
    if( Features::drawShadows ) {
        // draw shadow under flower using center texture

        mCenterTexture->enable();
        glBegin( GL_QUADS ); {
        
            double centerZ = inPosition->mZ;

            glColor4f( 0, 0, 0, 0.25 );
        
            glTexCoord2f( 0, 0 );
            glVertex3d( inPosition->mX - maxRadius,
                        inPosition->mY - maxRadius, centerZ );
        
            glTexCoord2f( 1, 0 );
            glVertex3d( inPosition->mX + maxRadius,
                        inPosition->mY - maxRadius, centerZ );
        
            glTexCoord2f( 1, 1 );
            glVertex3d( inPosition->mX + maxRadius,
                        inPosition->mY + maxRadius, centerZ );
        
            glTexCoord2f( 0, 1 );
            glVertex3d( inPosition->mX - maxRadius,
                        inPosition->mY + maxRadius, centerZ );
            }
        glEnd();
        mCenterTexture->disable();

        }
    

    
    // rotate all by start angle for first petal
    for( i=0; i<3; i++ ) {
        petalPoints[i].rotate( &startAngle );
        }

    if( inStage >= 1 && inStage < 4 ) {
        // draw petals
        mPetalTexture->enable();
        glBegin( GL_TRIANGLES ); {
            for( int p=0; p<numPetals; p++ ) {
                
                for( i=0; i<3; i++ ) {
                    // draw
                    setGLColor( petalPointColors[i] );

                    switch( i ) {
                        case 0:
                            glTexCoord2f( 1, 1 );
                            break;
                        case 1:
                            glTexCoord2f( 0, 1 );
                            break;
                        case 2:
                            glTexCoord2f( 0.5, 0 );
                        }
                            
                    glVertex3d( inPosition->mX + petalPoints[i].mX,
                                inPosition->mY + petalPoints[i].mY,
                                inPosition->mZ + petalPoints[i].mZ );

                    
                
                    // rotate for next petal
                    petalPoints[i].rotate( &angleIncrement );
                    }
                }
            }
        glEnd();
        mCenterTexture->disable();

        }
    
    for( i=0; i<3; i++ ) {
        delete petalPointColors[i];
        }
    
    

    mCenterTexture->enable();
    glBegin( GL_QUADS ); {
        
        double centerZ = inPosition->mZ;

        Color *centerColor = mGenetics.getColor( flowerCenterColor );

        
        Color *drawColor;

        if( inStage <= 2 ) {
            drawColor = Color::linearSum( &budColor, centerColor,
                                          budColorWeight ); 
            }
        else if( inStage > 3 && inStage <= 4 ) {
            drawColor = Color::linearSum( &fadeColor, centerColor,
                                          fadeColorWeight ); 
            }
        else {
            drawColor = new Color();
            drawColor->setValues( centerColor );
            }

        
        setGLColor( drawColor );
        delete centerColor;
        delete drawColor;
                   
        glTexCoord2f( 0, 0 );
        glVertex3d( inPosition->mX - centerRadius,
                    inPosition->mY - centerRadius, centerZ );
        
        glTexCoord2f( 1, 0 );
        glVertex3d( inPosition->mX + centerRadius,
                    inPosition->mY - centerRadius, centerZ );
        
        glTexCoord2f( 1, 1 );
        glVertex3d( inPosition->mX + centerRadius,
                    inPosition->mY + centerRadius, centerZ );
        
        glTexCoord2f( 0, 1 );
        glVertex3d( inPosition->mX - centerRadius,
                    inPosition->mY + centerRadius, centerZ );
        }
    glEnd();
    mCenterTexture->disable();
    }

