/*
 * Modification History
 *
 * 2006-October-26   Jason Rohrer
 * Created.
 *
 * 2006-December-25   Jason Rohrer
 * Added function for checking closed status of portal.
 *
 * 2007-July-30   Jason Rohrer
 * Smoothed linear segmentation of soul trails in larger images.
 */


#include "Portal.h"

#include "glCommon.h"
#include "features.h"

#include <stdio.h>


#include "minorGems/graphics/filters/BoxBlurFilter.h"

#include "minorGems/io/file/FileOutputStream.h"
// #include "minorGems/graphics/converters/TGAImageConverter.h"
#include "minorGems/graphics/converters/PNGImageConverter.h"



#include "minorGems/util/random/StdRandomSource.h"

extern StdRandomSource globalRandomSource;



Portal::Portal()
    : mMaxNumLevels( 5 ), mNumLevels( 0 ),
      mTopLevelZ( 8 ),
      mOpeningProgress( 0 ),
      mClosing( false ),
      mClosingProgress( 0 ),
      mTimePassed( 0 ) {

    mLayerTextures = new SingleTextureGL *[ mMaxNumLevels ];
    mLayerGenetics = new PortalLayerGenetics *[ mMaxNumLevels ];
    mLayerSpawnProgress = new double[ mMaxNumLevels ];
    
    
    // generate texture

    }



Portal::~Portal() {

    int i;
    for( i=0; i<mNumLevels; i++ ) {
        delete mLayerTextures[i];
        delete mLayerGenetics[i];
        }
    
    delete [] mLayerTextures;
    delete [] mLayerGenetics;
    delete [] mLayerSpawnProgress;

    int numImmortals = mImmortals.size();
    for( i=0; i<numImmortals; i++ ) {
        delete *( mImmortals.getElement( i ) );
        }
    }



/**
 * Draws a blurry circle using additive color in a square image.
 *
 * @param inImage the image to draw into.  Must be square.  Destroyed
 *  by caller.
 * @param inCircleColor the color of the circle.  Destroyed by caller.
 * @param inColorFade the fade factor, in [0..1].
 * @param inCenterX, inCenterY the center of the circle.
 * @param in Radius the radius of the circle.
 */
void drawBlurCircleInImage( Image *inImage, Color *inCircleColor,
                            double inColorFade,
                            double inCenterX, double inCenterY,
                            double inRadius ) {
    
    int imageSize = inImage->getWidth();

    int numChannels = inImage->getNumChannels();

    if( numChannels > 4 ) {
        // only consider first 4
        numChannels = 4;
        }
    
    double **channels = new double*[numChannels];
    int c;
    for( c=0; c<numChannels; c++ ) {
        channels[c] = inImage->getChannel( c );
        }

    // only consider pixels in a square around center
    int squareRadius = (int)( inRadius + 1 );
    
    for( int dX = -squareRadius; dX <= squareRadius; dX++ ) {
        int currentX = (int)( inCenterX + dX );

        // from circle center
        double distanceX = inCenterX - currentX;

        // optimization:  ignore pixels that are too far by x alone
        if( distanceX < inRadius ) {

            // wrap to index into image
            if( currentX < 0 ) {
                currentX += imageSize;
                }
            else if( currentX >= imageSize ) {
                currentX -= imageSize;
                }
        
            for( int dY = -squareRadius; dY <= squareRadius; dY++ ) {
                int currentY = (int)( inCenterY + dY );

                // dist from circle center
                double distanceY = inCenterY - currentY;
            
                double distance = sqrt( distanceY * distanceY +
                                        distanceX * distanceX );

                if( distance < inRadius ) {

                    // wrap for indexing into image
                    if( currentY < 0 ) {
                        currentY += imageSize;
                        }
                    else if( currentY >= imageSize ) {
                        currentY -= imageSize;
                        }
                
                    double brighness = ( inRadius - distance ) / inRadius; 

                
                    int pixelIndex = currentY * imageSize + currentX;

                    for( c=0; c<numChannels; c++ ) {
                        channels[c][ pixelIndex ] +=
                            inColorFade * brighness * (*inCircleColor)[c];
                    
                        // cap
                        if( channels[c][ pixelIndex ] > 1 ) {
                            channels[c][ pixelIndex ] = 1;
                            }
                        }
                    }
                }
            }
        }

    delete [] channels;
    }



void Portal::upgrade( Gardener *inLevelOpener ) {
    if( mNumLevels < mMaxNumLevels ) {

        if( inLevelOpener != NULL ) {
            GardenerGenetics *baseGenetics = &( inLevelOpener->mGenetics );
            
            mLayerGenetics[ mNumLevels ] =
                new PortalLayerGenetics( baseGenetics );
            }
        else {
            // no opener
            // use random base genetics
            mLayerGenetics[ mNumLevels ] = new PortalLayerGenetics();
            }
        
        mLayerSpawnProgress[ mNumLevels ] = 0;
    

        // texture for this layer

        // a blurry, wire-frame glyph
        int textureSize = 32;
    
    
        Image *textureImage = new Image( textureSize, textureSize, 4, false );

        double *channels[4];

        int pixelsPerChannel = textureSize * textureSize;
    
        int i;
        int p;
        for( i=0; i<4; i++ ) {
            channels[i] = textureImage->getChannel( i );
            }

        // start all pixels as white with alpha of zero
        for( p=0; p<pixelsPerChannel; p++ ) {
            channels[0][p] = 1;
            channels[1][p] = 1;
            channels[2][p] = 1;
            channels[3][p] = 0;
            }


        double xPosition, yPosition;

        xPosition =
            mLayerGenetics[ mNumLevels ]->getParameter( glyphStartPositionX );
        yPosition =
            mLayerGenetics[ mNumLevels ]->getParameter( glyphStartPositionY );

        double startAngle =
            mLayerGenetics[ mNumLevels ]->getParameter( glyphStartAngle );

        double deltaAngle =
            mLayerGenetics[ mNumLevels ]->getParameter( glyphDeltaAngle );

        // up by one pixel
        Vector3D currentDirection( 0, 1.0 / textureSize, 0 );

        // rotate
        Angle3D angle( 0, 0, startAngle );
        currentDirection.rotate( &angle );
        
        int stepsBeforeDirectionChange = 
            (int)( mLayerGenetics[ mNumLevels ]->getParameter(
                glyphStepsBeforeDirectionChange ) );
        
        int numSteps = 0;

        int stepsSinceDirectionChange = 0;

        // all white with full alpha
        Color drawColor( 1, 1, 1, 1 );
        
        while( numSteps < 100 ) {

            // white pixel
            double x = xPosition * (textureSize - 1);
            double y = yPosition * (textureSize - 1);

            drawBlurCircleInImage( textureImage, &drawColor,
                                   0.5,
                                   x, y, 1.5 );
            
            if( stepsSinceDirectionChange > stepsBeforeDirectionChange ) {
                // change direction
                stepsSinceDirectionChange = 0;

                Angle3D angle( 0, 0, deltaAngle );
                
                currentDirection.rotate( &angle );
                }
                

            // bounce off boundaries
			// don't let them get too close to edge
            xPosition += currentDirection.mX;
            if( xPosition < 0.1 ) {
                xPosition = 0.1;
                currentDirection.mX = -currentDirection.mX;
                }
            if( xPosition > 0.9 ) {
                xPosition = 0.9;
                currentDirection.mX = -currentDirection.mX;
                }

            yPosition += currentDirection.mY;
            if( yPosition < 0.1 ) {
                yPosition = 0.1;
                currentDirection.mY = -currentDirection.mY;
                }
            if( yPosition > 0.9 ) {
                yPosition = 0.9;
                currentDirection.mY = -currentDirection.mY;
                }
            
            
            numSteps++;
            stepsSinceDirectionChange++;
            }
        

        
        // blur alpha
        BoxBlurFilter blur( 1 );

        textureImage->filter( &blur, 3 );
    

        /*
        char *fileName = autoSprintf( "glyph_%d.tga", mNumLevels );
        
        File outFileB( NULL, fileName );
        delete [] fileName;

        FileOutputStream *outStreamB = new FileOutputStream( &outFileB );

        TGAImageConverter converter;
        
        converter.formatImage( textureImage, outStreamB );
        delete outStreamB;
        */
        
        
        mLayerTextures[ mNumLevels ] = new SingleTextureGL( textureImage,
                                                            // no wrap
                                                            false );
        delete textureImage;    

        
        mNumLevels++;
        }
    }



char Portal::isOpen() {
    if( mClosing ) {
        return false;
        }
    
    if( mOpeningProgress == 1 ) {
        return true;
        }
    else {
        return false;
        }
    }



char Portal::isClosed() {
    if( mClosing && mClosingProgress == 1 ) {
        return true;
        }
    else {
        return false;
        }
    }



void Portal::sendGardener( Gardener *inGardener,
                           Vector3D *inStartingPosition,
                           Angle3D *inStartingRotation ) {

    inGardener->setFrozen( true );

    mPassengers.push_back( inGardener );
    mPassengerPositions.push_back( new Vector3D( inStartingPosition ) );
    mPassengerRotations.push_back( new Angle3D( inStartingRotation ) );
    mPassengerFades.push_back( 1.0 );
    }


        
void Portal::passTime( double inTimeDeltaInSeconds ) {

    mTimePassed += inTimeDeltaInSeconds;

    for( int i=0; i<mNumLevels; i++ ) {
        // full spawn takes 1 second
        mLayerSpawnProgress[i] += inTimeDeltaInSeconds;

        // cap
        if( mLayerSpawnProgress[i] > 1 ) {
            mLayerSpawnProgress[i] = 1;
            }
        }

    // if reached max and last layer done spawning
    if( mNumLevels == mMaxNumLevels &&
        mLayerSpawnProgress[ mNumLevels - 1 ] == 1 ) {
        
        // open portal over course of 2 seconds
        mOpeningProgress += inTimeDeltaInSeconds / 2;

        // cap
        if( mOpeningProgress > 1 ) {
            mOpeningProgress = 1;
            }
        }

    if( mOpeningProgress == 1 ) {
        // open

        // cause any passengers to rise up
        
        // in units per second
        double portalRiseRate = 3;

        double fadeZStart = mTopLevelZ / 2;
        // fade so that we hit zero at top
        // but we only start fading half way from top
        double portalFadeRate = 1.0 / ( fadeZStart / portalRiseRate );
        
        int numPassengers = mPassengers.size();
        for( int i=0; i<numPassengers; i++ ) {
            
            Vector3D *position = *( mPassengerPositions.getElement( i ) );
            
            position->mZ -= portalRiseRate * inTimeDeltaInSeconds;

            if( (- position->mZ) >= fadeZStart ) {
                *( mPassengerFades.getElement( i ) ) -=
                    portalFadeRate * inTimeDeltaInSeconds;

                if( *( mPassengerFades.getElement( i ) ) < 0 ) {
                    *( mPassengerFades.getElement( i ) ) = 0;
                    }
                }
            }


        // now remove any that are done
        char foundDone = true;
        
        while( foundDone ) {
            foundDone = false;

            int numPassengers = mPassengers.size();
            for( int i=0; i<numPassengers && !foundDone; i++ ) {
            
                Vector3D *position = *( mPassengerPositions.getElement( i ) );
            
                if( (- position->mZ) >= mTopLevelZ ) {
                    // done passing through

                    Gardener *gardener = *( mPassengers.getElement( i ) );
                    Angle3D *rotation =
                        *( mPassengerRotations.getElement( i ) );

                    delete position;
                    delete rotation;
                    mPassengerPositions.deleteElement( i );
                    mPassengerRotations.deleteElement( i );

                    mPassengers.deleteElement( i );
                    mPassengerFades.deleteElement( i );
                    
                    gardener->setFrozen( false );
                    gardener->forceDead();

                    // save copy of genetics to immortalize gardener
                    mImmortals.push_back(
                        new ImmortalGenetics( &( gardener->mGenetics ) ) );
                    
                    foundDone = true;

                    if( gardener->mUserControlling ) {
                        // start closing now
                        mClosing = true;
                        }
                    }
                }
            }

        if( mClosing && mClosingProgress < 1 ) {
            // close portal over course of 2 seconds
            mClosingProgress += inTimeDeltaInSeconds / 2;

            // cap
            if( mClosingProgress > 1 ) {
                mClosingProgress = 1;

                // actually close the portal
                close();
                }
            }
        }
    }



void Portal::draw( Vector3D *inPosition, double inScale,
                   double inMaxZ, double inMinZ ) {

    if( mClosing && mClosingProgress >= 1.0 ) {
        // draw nothing
        return;
        }
    
    // portal brightens only
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    
    Vector3D levelPosition( inPosition );

    double zStep = mTopLevelZ / ( mMaxNumLevels - 1 );
    
    for( int i=0; i<mNumLevels; i++ ) {

        if( levelPosition.mZ >= inMinZ &&
            levelPosition.mZ <= inMaxZ ) {


            if( i == 0 ) {
                // draw all passing gardeners below the lowest level

                // back to normal blend function when drawing gardeners
                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            
                int numPassengers = mPassengers.size();
                for( int i=0; i<numPassengers; i++ ) {
            
                    Gardener *gardener = *( mPassengers.getElement( i ) );
                    Vector3D *position =
                        *( mPassengerPositions.getElement( i ) );
                    Angle3D *rotation =
                        *( mPassengerRotations.getElement( i ) );
                    double fade = *( mPassengerFades.getElement( i ) );


                    gardener->draw( position,
                                    rotation,
                                    // scale
                                    4,
                                    fade);
                
                    }

                // back to portal blend mode:  brightens only
                glBlendFunc( GL_SRC_ALPHA, GL_ONE );
                }
            

            double param_quadRotationSpeed =
                mLayerGenetics[i]->getParameter( quadRotationSpeed );

            double param_quadRotationDelta =
                mLayerGenetics[i]->getParameter( quadRotationDelta );

            double param_offsetRadius =
                mLayerGenetics[i]->getParameter( offsetRadius );

            double param_offsetSineWeight =
                mLayerGenetics[i]->getParameter( offsetSineWeight );

            double param_offsetSineSpeed =
                mLayerGenetics[i]->getParameter( offsetSineSpeed );

            double param_offsetSineSineSpeed =
                mLayerGenetics[i]->getParameter( offsetSineSineSpeed );

            double param_quadDensity =
                mLayerGenetics[i]->getParameter( quadDensity );

            double param_quadScale =
                mLayerGenetics[i]->getParameter( quadScale );

            double param_quadScaleSineWeight =
                mLayerGenetics[i]->getParameter( quadScaleSineWeight );

            double param_quadScaleSineSpeed =
                mLayerGenetics[i]->getParameter( quadScaleSineSpeed );

            double param_quadScaleSineSineSpeed =
                mLayerGenetics[i]->getParameter( quadScaleSineSineSpeed );


            // modify parameters if portal not open yet
            /*
            param_quadRotationSpeed =
                mOpeningProgress * param_quadRotationSpeed
                +
                ( 1 - mOpeningProgress ) * 0;
            
            param_quadRotationDelta =
                mOpeningProgress * param_quadRotationDelta
                +
                ( 1 - mOpeningProgress ) * 0;
            */
                        
            // draw a circle of quads

            Angle3D quadRotation( 0, 0,
                                  param_quadRotationSpeed * mTimePassed );
            Angle3D quadRotationDelta( 0, 0, param_quadRotationDelta );


            // use scale colors from underlying gardener genetics as starting
            // colors
            Color *cornerColors[4];
            cornerColors[0] = mLayerGenetics[i]->getColor( scaleCornerAColor );
            cornerColors[1] = mLayerGenetics[i]->getColor( scaleCornerBColor );
            cornerColors[2] = mLayerGenetics[i]->getColor( scaleCornerCColor );
            cornerColors[3] = mLayerGenetics[i]->getColor( scaleCornerDColor );
            
            cornerColors[0]->a =
                mLayerGenetics[i]->getParameter( cornerAAlpha );
            cornerColors[1]->a =
                mLayerGenetics[i]->getParameter( cornerBAlpha );
            cornerColors[2]->a =
                mLayerGenetics[i]->getParameter( cornerCAlpha );
            cornerColors[3]->a =
                mLayerGenetics[i]->getParameter( cornerDAlpha );

            // force cornerA alpha to be 1 so that portal layer is always
            // visible
            cornerColors[0]->a = 1.0;
            
            
            Color *cornerDeltaColors[4];
            cornerDeltaColors[0] =
                mLayerGenetics[i]->getColor( scaleCornerAColorDelta );
            cornerDeltaColors[1] =
                mLayerGenetics[i]->getColor( scaleCornerBColorDelta );
            cornerDeltaColors[2] =
                mLayerGenetics[i]->getColor( scaleCornerCColorDelta );
            cornerDeltaColors[3] =
                mLayerGenetics[i]->getColor( scaleCornerDColorDelta );
            
            
            float colorDeltas[4][3];
            int p, c;
            for( p=0; p<4; p++ ) {
                // modify alpha for each corner color
                // fade in during spawn
                cornerColors[p]->a *= mLayerSpawnProgress[i];

                
                // next compute animated color deltas
                Color *color = cornerDeltaColors[p];
                float redComponent = color->r;
                
                for( c=0; c<3; c++ ) {
                    float componentValue = (*color)[c];
                                            
                    colorDeltas[p][c] = componentValue +
                        componentValue *
                        // use red component as color cycle speed
                        sin( redComponent * mTimePassed * 5 );
                    }
                }


            
            double aStep = 0.1 / param_quadDensity;

            if( ! Features::drawComplexPortal ) {
                // 3-times fewer glyphs in each layer
                aStep *= 3;
                }

            // round so that aStep is a fraction of the form (2*PI)/n
            double invAStep = (2 * M_PI) / aStep;

            aStep = (2 * M_PI) / ( (int)invAStep );
            
            
            for( double a=0; a< 2 * M_PI; a += aStep ) {

                double xOffset =
                    param_offsetRadius * inScale
                    +
                    param_offsetSineWeight * inScale *
                    sin( param_offsetSineSpeed * a +
                         sin( param_offsetSineSineSpeed * mTimePassed ) );

                xOffset += 10 * inScale * ( 1 - mLayerSpawnProgress[i] );

                if( mClosing ) {
                    xOffset *= ( 1.0 - mClosingProgress );
                    }
                
                Vector3D offset( xOffset, 0, 0 );
                Angle3D angle( 0, 0, a );

                offset.rotate( &angle );

                offset.add( &levelPosition );

                double quadSize =
                    param_quadScale * inScale +
                    ( inScale * param_quadScaleSineWeight ) *
                    sin( param_quadScaleSineSpeed * a +
                         sin( param_quadScaleSineSineSpeed * mTimePassed ) );

                
                // shrink quads if not open
                quadSize = mOpeningProgress * quadSize +
                    ( 1 - mOpeningProgress ) * 0.4 * quadSize;
                
                if( mClosing ) {
                    quadSize *= ( 1.0 - mClosingProgress );
                    }
                
                drawTextureQuad( mLayerTextures[i],
                                 &offset, quadSize, &quadRotation,
                                 cornerColors );

                quadRotation.add( &quadRotationDelta );

                // bounce off value boundaries at 0 and 1
                for( p=0; p<4; p++ ) {
                    Color *cornerColor = cornerColors[p];
                    
                    for( c=0; c<3; c++ ) {
                        (*cornerColor)[c] += (*colorDeltas)[c];
                        
                        if( (*cornerColor)[c] > 1 ) {
                            (*cornerColor)[c] = 1 - ( (*cornerColor)[c] - 1 );
                            colorDeltas[p][c] *= -1;
                            }
                        else if( (*cornerColor)[c] < 0 ) {
                            (*cornerColor)[c] = 0 + ( 0 - (*cornerColor)[c]);
                            colorDeltas[p][c] *= -1;
                            }
                        }
                    }
                }
            
            for( p=0; p<4; p++ ) {
                delete cornerColors[p];
                delete cornerDeltaColors[p];
                }
            }
        levelPosition.mZ -= zStep;
        }

    // back to normal blend function
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }



void Portal::close() {


    int baseImageSize = 40;

    int numImmortals = mImmortals.size();
    
    int imageSize = baseImageSize * numImmortals;


    // image with all black pixels
    Image *image = new Image( imageSize, imageSize, 3, true );

    double *channels[3];

    //int pixelsPerChannel = imageSize * imageSize;
    
    int c;
    //int p;
    for( c=0; c<3; c++ ) {
        channels[c] = image->getChannel( c );
        }



    double *currentX = new double[ numImmortals ];
    double *currentY = new double[ numImmortals ];
    double *deltaAngle = new double[ numImmortals ];
    double *deltaDeltaAngle = new double[ numImmortals ];
    double *deltaDeltaDeltaAngle = new double[ numImmortals ];
    double *deltaAngleSineWeight = new double[ numImmortals ];
    double *deltaAngleSineSpeed = new double[ numImmortals ];

    Color *colors = new Color[ numImmortals ];
    Color **cornerColors = new Color*[ numImmortals ]; 
    double **cornerSpreads = new double*[ numImmortals ];

    
    Vector3D *currentDirection = new Vector3D[ numImmortals ];
    
    ImmortalGenetics **immortalGenes = mImmortals.getElementArray();
    
    int i;
    for( i=0; i<numImmortals; i++ ) {
        currentX[i] = immortalGenes[i]->getParameter( soulStartX );
        currentY[i] = immortalGenes[i]->getParameter( soulStartY );
        
        deltaAngle[i] = immortalGenes[i]->getParameter( soulDeltaAngle );
        deltaDeltaAngle[i] =
            immortalGenes[i]->getParameter( soulDeltaDeltaAngle );
        deltaDeltaDeltaAngle[i] =
            immortalGenes[i]->getParameter( soulDeltaDeltaDeltaAngle );
        deltaAngleSineWeight[i] =
            immortalGenes[i]->getParameter( soulDeltaAngleSineWeight );
        deltaAngleSineSpeed[i] =
            immortalGenes[i]->getParameter( soulDeltaAngleSineSpeed );
        double startAngle = immortalGenes[i]->getParameter( soulStartAngle );

        // up by one pixel
        currentDirection[i].setCoordinates( 0, 1.0 / imageSize, 0 );

        // rotate
        Angle3D angle( 0, 0, startAngle );
        currentDirection[i].rotate( &angle );

        Color *tempColor = immortalGenes[i]->getColor( bodyColor ); 
        colors[i].setValues( tempColor );
        delete tempColor;

        cornerColors[i] = new Color[4];
        cornerSpreads[i] = new double[4];
        
        for( int v=0; v<4; v++ ) {
            // hack:  can loop over scale colors A through G because they
            // are in order in the enum
            tempColor =
                immortalGenes[i]->getColor(
                    (GardenerGeneLocator)( scaleCornerAColor + v ) ); 

            cornerColors[i][v].setValues( tempColor );
            delete tempColor;

            // use same hack here
            cornerSpreads[i][v] = immortalGenes[i]->getParameter(
                (ImmortalGeneLocator)( scaleCornerAColor + v ) );
            }
        }

    delete [] immortalGenes;

    // execute steps
    for( int s=0; s<1000; s++ ) {

        for( i=0; i<numImmortals; i++ ) {
            

            // take extra sub-steps between rotations if image is bigger
            // so that trails fill image better
            double subStepFraction = 1.0 / numImmortals;
            
            for( int ss=0; ss<numImmortals; ss++ ) {
                double x = currentX[i] * (imageSize - 1);
                double y = currentY[i] * (imageSize - 1);
                
                // corners spread farther with more immortals
                int cornerSpread = numImmortals;
                
                double cornerPixelX[4] =
                    { x + cornerSpread * cornerSpreads[i][0],
                      x + cornerSpread * cornerSpreads[i][1],
                      x - cornerSpread * cornerSpreads[i][2],
                      x - cornerSpread * cornerSpreads[i][3] };
                
                double cornerPixelY[4] =
                    { y + cornerSpread * cornerSpreads[i][0],
                      y - cornerSpread * cornerSpreads[i][0],
                      y + cornerSpread * cornerSpreads[i][0],
                      y - cornerSpread * cornerSpreads[i][0] };
                
                int v;
                for( v=0; v<4; v++ ) {
                    if( cornerPixelX[v] < 0 ) {
                        cornerPixelX[v] += imageSize;
                        }
                    if( cornerPixelY[v] < 0 ) {
                        cornerPixelY[v] += imageSize;
                        }
                    if( cornerPixelX[v] >= imageSize ) {
                        cornerPixelX[v] -= imageSize;
                        }
                    if( cornerPixelY[v] >= imageSize ) {
                        cornerPixelY[v] -= imageSize;
                        }
                    }
            
                // draw center
                drawBlurCircleInImage( image, &(colors[i]),
                                       0.33,
                                       x, y, 1.5 );
                                
                // draw corners
                for( v=0; v<4; v++ ) {
                    drawBlurCircleInImage( image, &(cornerColors[i][v]),
                                           0.066,
                                           cornerPixelX[v], cornerPixelY[v],
                                           1.5 );
                    }


                // take a sub-step

                // sub-step rotation

                double fractional_s = 
                    s + (double)ss / (double)( numImmortals - 1 );
                
                // rotate
                Angle3D angle( 0, 0,
                               subStepFraction * (
                                   deltaAngle[i] +
                                   deltaAngleSineWeight[i] *
                                   sin( fractional_s * 
                                        deltaAngleSineSpeed[i] ) ) );
            
                currentDirection[i].rotate( &angle );

                // sub-step the angle
                deltaAngle[i] += subStepFraction * deltaDeltaAngle[i];
                
                // sub-step the deltaDelta angle
                deltaDeltaAngle[i] += 
                    subStepFraction * deltaDeltaDeltaAngle[i];



                // sub-step step
                currentX[i] += currentDirection[i].mX;
                currentY[i] += currentDirection[i].mY;

                // wrap
                if( currentX[i] > 1 ) {
                    currentX[i] -= 1;
                    }
                else if( currentX[i] < 0 ) {
                    currentX[i] += 1;
                    }
                if( currentY[i] > 1 ) {
                    currentY[i] -= 1;
                    }
                else if( currentY[i] < 0 ) {
                    currentY[i] += 1;
                    }
                }
            
            
            
            /*
            // old delta code, before we moved this stuff into 
            // the sub-steps
            
            // problem:  because we rotated only after each batch of substeps,
            // each batch of substeps resulted in a straight line segment
            // these line segments were highly visible in larger images (for
            // example, when there are 16 immortals).

            // moving these rotation steps into the substeps (and breaking them
            // up into rotation substeps) fixed the problem

            // rotate
            Angle3D angle( 0, 0,
                           deltaAngle[i] +
                           deltaAngleSineWeight[i] *
                           sin( s * deltaAngleSineSpeed[i] ) );
            
            currentDirection[i].rotate( &angle );

            // step angle
            deltaAngle[i] += deltaDeltaAngle[i];

            // step deltaDelta
            deltaDeltaAngle[i] += deltaDeltaDeltaAngle[i];
            */
            }

        }


    for( i=0; i<numImmortals; i++ ) {
        delete [] cornerColors[i];
        delete [] cornerSpreads[i];
        }
    
    delete [] currentX;
    delete [] currentY;
    delete [] deltaAngle;
    delete [] deltaDeltaAngle;
    delete [] deltaDeltaDeltaAngle;
    delete [] deltaAngleSineWeight;
    delete [] deltaAngleSineSpeed;
    delete [] currentDirection;
    delete [] colors;
    delete [] cornerColors;
    delete [] cornerSpreads;

    // send result out to file
    
    char fileExists = true;
    int fileCounter = 0;

    // search for a new file name
    
    while( fileExists ) {
        fileExists = false;
        char *fileName = autoSprintf( "immortal_%d.png", fileCounter );
        
        File outFile( NULL, fileName );
        delete [] fileName;

        if( outFile.exists() ) {
            fileExists = true;
            fileCounter++;
            }        
        }

    char *fileName = autoSprintf( "immortal_%d.png", fileCounter );
    
    File outFile( NULL, fileName );
    delete [] fileName;

    
    FileOutputStream *outStream = new FileOutputStream( &outFile );
    
    PNGImageConverter converter;
        
    converter.formatImage( image, outStream );
    delete outStream;

    delete image;
    }

