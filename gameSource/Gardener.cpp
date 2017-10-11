/*
 * Modification History
 *
 * 2006-July-2   Jason Rohrer
 * Created.
 *
 * 2006-October-4   Jason Rohrer
 * Fixed a crash when a baby's parent dies and is removed.
 * Made water pickup and dumping smoother.
 * Fixed an eye drawing bug.
 *
 * 2006-October-10   Jason Rohrer
 * Fixed crashing bug.
 * Improved selection of least/most-liked gardeners when there is a tie.
 *
 * 2006-October-26   Jason Rohrer
 * Added a limit of 10 stored items.
 *
 * 2006-October-27   Jason Rohrer
 * Added forceDead function.  Added poisonFruit function.
 * Changed to ignore poisoned fruits in getIndexOfFruitHighInNutrient.
 *
 * 2006-October-30   Jason Rohrer
 * Added a following threshold gene.
 *
 * 2006-November-2   Jason Rohrer
 * Fixed so that mLife drops to 0 when forced dead.
 *
 * 2006-November-13   Jason Rohrer
 * Added a frozen state and a manual fade factor for drawing.
 *
 * 2006-November-24   Jason Rohrer
 * Made most/least liked gardener consistent across multiple function calls.
 * Fixed birth position bug when parent moving.
 *
 * 2006-November-25   Jason Rohrer
 * Added feeding of followers.
 *
 * 2006-November-25   Jason Rohrer
 * Fixed so that nutrients pass through to babies of followers.
 *
 * 2006-December-25   Jason Rohrer
 * Added function to check if any stored fruit poisoned.
 */



#include "Gardener.h"

#include "features.h"
#include "gameFunctions.h"
#include "glCommon.h"

#include "minorGems/graphics/filters/BoxBlurFilter.h"

#include "minorGems/util/random/StdRandomSource.h"

extern StdRandomSource globalRandomSource;

extern MusicNoteWaveTable globalWaveTable;

extern double globalMusicSongLength;
extern int globalNumNotesPerMelody;
extern double globalShortestNoteLength;


#include "World.h"

extern World *globalWorld;


#include <GL/gl.h>
#include <math.h>


//#include "minorGems/io/file/FileOutputStream.h"
//#include "minorGems/graphics/converters/TGAImageConverter.h"



ScaleCornerColors *ScaleCornerColors::copy() {

    ScaleCornerColors *returnValue = new ScaleCornerColors();

    for( int i=0; i<4; i++ ) {
        
        returnValue->mColors[i].setValues( &( mColors[i] ) );
        }
    
    return returnValue;
    }



Gardener::Gardener( Vector3D *inStartPosition )
    : mGenetics(),
      mDesiredPosition( inStartPosition ),
      mLastDrawAngle( 0, 0, 0 ),
      mParent( NULL ), mGrowthProgress( 1 ) {

    setUpStartingState();    
    }



Gardener::Gardener( Gardener *inParentA, Gardener *inParentB )
    : mGenetics( &( inParentA->mGenetics ), &( inParentB->mGenetics ) ),
      mLastDrawAngle( 0, 0, 0 ),
      mParent( inParentA ), mGrowthProgress( 0.35 ) {

    // set desired position, to start, to parentA's position
    Vector3D *position = inParentA->getDesiredPosition();

    mDesiredPosition.setCoordinates( position );
    delete position;
    
    
    setUpStartingState();
    }

int gardenerLargeTextureSize = 64;
Image gardenerLargeTextureImage( gardenerLargeTextureSize,
                                 gardenerLargeTextureSize, 4, false );

int gardenerSmallTextureSize = 32;
Image gardenerSmallTextureImage( gardenerSmallTextureSize,
                                 gardenerSmallTextureSize, 4, false );


void Gardener::setUpStartingState() {
    
    mSecondsSinceLastFruitPoisoning = 0;
    mLeader = NULL;

    mMostLiked = NULL;
    mLeastLiked = NULL;
    
    mUserCanControl = false;
    mUserControlling = false;
    mMoving = false;
    mCarryingWater = false;
    mWaterPickupFraction = 0;
    mRedNutrient = 1;
    mGreenNutrient = 1;
    mBlueNutrient = 1;
    mPoisoned = false;
    mLife = 1.0;
    mBaseAgeRate = 0.0005;
    mBaseEnergyConsumedPerSecond = 0.01;
    mDead = false;
    mGhostMode = false;
    mFrozen = false;
    mEmotionDisplay = 0;
    mPlotHidden = true;
    mSelectedStorageIndex = -1;
    mOffspring = NULL;
    mPregnancyProgress = 0;
    // start fading out at 5% life
    mNearDeathPoint = 0.05;
    
    Color *c = mGenetics.getColor( bodyColor );
    
    mColor.setValues( c );

    delete c;

    // generate notes from genetic melody parts
    int numNotes =
        (int)( globalMusicSongLength / globalShortestNoteLength )
        // extra to be safe
        + 10;
    
    double *notes = new double[numNotes];

    double *arrangement = mGenetics.getGene( songArrangement );
    int numArrangementParts = mGenetics.getGeneLength( songArrangement );


    double *melodies[6];

    melodies[0] = mGenetics.getGene( melodyA );
    melodies[1] = mGenetics.getGene( melodyB );
    melodies[2] = mGenetics.getGene( melodyC );
    melodies[3] = mGenetics.getGene( melodyD );
    melodies[4] = mGenetics.getGene( melodyE );
    melodies[5] = mGenetics.getGene( melodyF );


    // only have a repertoire of a subset of melodies
    // gardeners with larger eyes and pupils have more melodies to choose
    // from and thus more complex songs
    double eyeFactor =
        mGenetics.getParameter( pupilSize ) +
        mGenetics.getParameter( eyeSize );

    // eye factor is in range 1 to 3, map to [0,1];
    eyeFactor = (eyeFactor -  1) / 2;

    // map to int from 1 to 6
    int numMelodiesAvailable = (int)( eyeFactor * 5.99 ) + 1;

    
    int currentSection = 0;
    int currentSectionNote = 0;
    
    for( int n=0; n<numNotes; n++ ) {
        double melodySelector = arrangement[ currentSection ];

        // 6 melodies
        // int melodyNumber = (int)( melodySelector * 5.99 );
        int melodyNumber =
            (int)( melodySelector * ( numMelodiesAvailable - 0.01 ) );
        
        notes[n] = melodies[ melodyNumber ][ currentSectionNote ];

        currentSectionNote ++;

        if( currentSectionNote >= globalNumNotesPerMelody ) {
            // on to next section
            currentSectionNote = 0;
            currentSection++;

            if( currentSection >= numArrangementParts ) {
                // wrap back
                currentSection = 0;
                }
            }
        }

    delete [] arrangement;

    for( int m=0; m<6; m++ ) {
        delete [] melodies[m];
        }

    double reverseNote = mGenetics.getParameter( chanceOfReverseNote );
    
    mStillMusicPart = new MusicPart(
        &globalWaveTable,
        globalMusicSongLength,
        notes,
        numNotes,
        false,
        false,
        reverseNote );

    // high notes when holding water
    mHoldingMusicPart = new MusicPart(
        &globalWaveTable,
        globalMusicSongLength,
        notes,
        numNotes,
        true,
        false,
        reverseNote );

    // low fast notes
    mMovingMusicPart = new MusicPart(
        &globalWaveTable,
        globalMusicSongLength,
        notes,
        numNotes,
        false,
        true,
        reverseNote );

    // high fast notes
    mMovingHoldingMusicPart = new MusicPart(
        &globalWaveTable,
        globalMusicSongLength,
        notes,
        numNotes,
        true,
        true,
        reverseNote );

    delete [] notes;

    

    // generate texture for gardener base
    
    // a blurry white circle that is gray at edges
    int textureSize = gardenerSmallTextureSize;
    double halfTextureSize = 0.5 * textureSize;
    
    Image *textureImage = &gardenerSmallTextureImage;

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
    int blurRadius = 1;
    BoxBlurFilter blur( blurRadius );

    textureImage->filter( &blur, 3 );
    

    mBaseTexture = new SingleTextureGL( textureImage,
                                        // no wrap
                                        false );


    // now scale texture

    // a simple square with blurred edges
    
    // start all pixels as white and transparent
    for( p=0; p<pixelsPerChannel; p++ ) {
        channels[0][p] = 1;
        channels[3][p] = 0;
        }
    // 1 and 2 same as channel zero
    memcpy( channels[1], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[2], channels[0], pixelsPerChannel * sizeof( double ) );

    
    for( y=0; y<textureSize; y++ ) {

        double yRelative = y - halfTextureSize;

        for( x=0; x<textureSize; x++ ) {

            double alphaValue;

            double xRelative = x - halfTextureSize;

            int pixelIndex = y * textureSize + x;
            
            if( fabs( xRelative ) <= radius
                &&
                fabs( yRelative ) <= radius ) {

                alphaValue = 1;
                }
            else {
                alphaValue = 0;
                }

            channels[3][ pixelIndex ] = alphaValue;
            }
        }

    // blur alpha
    blur.setRadius( 3 );
    textureImage->filter( &blur, 3 );
    

    mScaleTexture = new SingleTextureGL( textureImage,
                                         // no wrap
                                         false );


    

    
    // now build overlay bone texture


    // higher res    
    textureSize = gardenerLargeTextureSize;
    
    halfTextureSize = 0.5 * textureSize;
    
    
    textureImage = &gardenerLargeTextureImage;

    
    pixelsPerChannel = textureSize * textureSize;
    
    for( i=0; i<4; i++ ) {
        channels[i] = textureImage->getChannel( i );
        }
    
    radius = textureSize / 3;

    
    // start all pixels as white and transparent
    for( p=0; p<pixelsPerChannel; p++ ) {
        channels[0][p] = 1;
        channels[3][p] = 0;
        }
    // 1 and 2 same as channel zero
    memcpy( channels[1], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[2], channels[0], pixelsPerChannel * sizeof( double ) );


    double yStep = mGenetics.getParameter( boneStartSpacing );
    double stepSizeDelta = mGenetics.getParameter( boneSpacingDelta );

    double boneCurve = mGenetics.getParameter( boneCurvature );
    double curveFrequency = mGenetics.getParameter( boneCurveFrequency );
    
    int stripeWidth = 2;
    
    y=0;
    int stepsSinceLastBone = 0;
    
    while( y<textureSize ) {

        if( stepsSinceLastBone >= yStep ) {
            // place bone here
            
            for( int stripeLine = 0; stripeLine < stripeWidth; stripeLine++ ) {
                double yRelative = (y + stripeLine) - halfTextureSize;
        
                for( x=0; x<textureSize; x++ ) {
                    double xRelative = x - halfTextureSize;
            

                    // adjust y to make curved lines
                    double adjustedY = yRelative + boneCurve *
                        cos( 2 * (xRelative / textureSize)
                             * M_PI  * curveFrequency );
                    
                    double pixelRadius = sqrt( xRelative * xRelative +
                                               adjustedY * adjustedY );

                    int adjustedIndexY =
                        (int)( adjustedY + halfTextureSize );
                    
                    int pixelIndex = adjustedIndexY * textureSize + x;
                    
                    if( pixelRadius <= radius ) {
                        channels[3][ pixelIndex ] = 1;
                        }            
                    }
                }

            
            yStep += stepSizeDelta;

            stepsSinceLastBone = 0;
            }
        else {
            stepsSinceLastBone ++;
            }

        double yRelative = y - halfTextureSize;
        if( fabs( yRelative ) <= radius ) {
            // center line
            channels[3][ y * textureSize + textureSize / 2 ] = 1;
            channels[3][ y * textureSize + textureSize / 2 - 1 ] = 1;
            channels[3][ y * textureSize + textureSize / 2 + 1] = 1;
            }

        y++;
        }

    // blur alpha
    blur.setRadius( 1 );
    textureImage->filter( &blur, 3 );
    

    mBonesTexture = new SingleTextureGL( textureImage,
                                         // no wrap
                                         false );




    // now build skin texture
    
    // start all pixels as white and transparent
    for( p=0; p<pixelsPerChannel; p++ ) {
        channels[0][p] = 1;
        channels[3][p] = 0;
        }
    // 1 and 2 same as channel zero
    memcpy( channels[1], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[2], channels[0], pixelsPerChannel * sizeof( double ) );


    
    for( y=0; y<textureSize; y++ ) {
        
        double yRelative = y - halfTextureSize;
        
        for( x=0; x<textureSize; x++ ) {

            double alphaValue;

            double xRelative = x - halfTextureSize;

            double pixelRadius = sqrt( xRelative * xRelative +
                                       yRelative * yRelative );

            int pixelIndex = y * textureSize + x;
            
            if( pixelRadius <= radius ) {
                // random value
                alphaValue =
                    globalRandomSource.getRandomBoundedDouble( 0, 1 );
                }
            else {
                alphaValue = 0;
                }

            channels[3][ pixelIndex ] = alphaValue;
            }
        }

    // blur alpha
    //textureImage->filter( &blur, 3 );
    

    mSkinTexture = new SingleTextureGL( textureImage,
                                        // no wrap
                                        false );




    // eye texture

    // start all pixels as black and transparent
    for( p=0; p<pixelsPerChannel; p++ ) {
        channels[0][p] = 0;
        }
    // 1, 2 and alpha same as channel zero
    memcpy( channels[1], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[2], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[3], channels[0], pixelsPerChannel * sizeof( double ) );


    double rimRadius = textureSize / 3;
    double innerRimRadius = textureSize / 3.5;

    double pupilRadius = textureSize / 5;
    pupilRadius *= mGenetics.getParameter( pupilSize );
    
    double pupilYOffset = pupilRadius;

    for( y=0; y<textureSize; y++ ) {

        double yRelative = y - halfTextureSize;

        for( x=0; x<textureSize; x++ ) {
            
            double xRelative = x - halfTextureSize;

            double yPupilRelative = yRelative - pupilYOffset;
            
            
            double pixelRadius = sqrt( xRelative * xRelative +
                                       yRelative * yRelative );

            double pixelPupilRadius = sqrt( xRelative * xRelative +
                                            yPupilRelative * yPupilRelative );

            int pixelIndex = y * textureSize + x;
            
            if( pixelRadius <= rimRadius ) {
                double colorValue = 0;
                
                if( pixelRadius <= innerRimRadius ) {
                    colorValue = 1;
                    }
                if( pixelPupilRadius <= pupilRadius ) {
                    colorValue = 0;
                    }
                
                channels[0][ pixelIndex ] = colorValue;
                channels[1][ pixelIndex ] = colorValue;
                channels[2][ pixelIndex ] = colorValue;
                channels[3][ pixelIndex ] = 1;
                }
            }
        }

    // blur all channels

    // optimization (found with profiler)
    // blur only alpha and first channel, since R, G, and B channels are
    // identical.  Then copy  R into G and B.
    blur.setRadius( 2 );
    textureImage->filter( &blur, 0 );
    textureImage->filter( &blur, 3 );

    // copy blurred color data
    memcpy( channels[1], channels[0], pixelsPerChannel * sizeof( double ) );
    memcpy( channels[2], channels[0], pixelsPerChannel * sizeof( double ) );
    
    
    mEyeTexture = new SingleTextureGL( textureImage,
                                       // no wrap
                                       false );

    


    
    /*
    File outFileB( NULL, "scale.tga" );
    FileOutputStream *outStreamB = new FileOutputStream( &outFileB );

    TGAImageConverter converter;
    
    converter.formatImage( textureImage, outStreamB );
    delete outStreamB;

    exit( 0 );
    */



    // now create scale locations


    // walker goes back and forth across gardener, laying rows of scales

    // scales only laid inside the round radius of the gardener

    double scaleBoundaryRadius = 0.6666;

    double scaleStep = mGenetics.getParameter( scaleStepSize );

    ScaleCornerColors currentColors;
    Color *aColor = mGenetics.getColor( scaleCornerAColor );
    Color *bColor = mGenetics.getColor( scaleCornerBColor );
    Color *cColor = mGenetics.getColor( scaleCornerCColor );
    Color *dColor = mGenetics.getColor( scaleCornerDColor );
    
    currentColors.mColors[0].setValues( aColor );
    currentColors.mColors[1].setValues( bColor );
    currentColors.mColors[2].setValues( cColor );
    currentColors.mColors[3].setValues( dColor );

    delete aColor;
    delete bColor;
    delete cColor;
    delete dColor;


    ScaleCornerColors deltaColors;
    Color *aColorDelta = mGenetics.getColor( scaleCornerAColorDelta );
    Color *bColorDelta = mGenetics.getColor( scaleCornerBColorDelta );
    Color *cColorDelta = mGenetics.getColor( scaleCornerCColorDelta );
    Color *dColorDelta = mGenetics.getColor( scaleCornerDColorDelta );
    
    deltaColors.mColors[0].setValues( aColorDelta );
    deltaColors.mColors[1].setValues( bColorDelta );
    deltaColors.mColors[2].setValues( cColorDelta );
    deltaColors.mColors[3].setValues( dColorDelta );

    delete aColorDelta;
    delete bColorDelta;
    delete cColorDelta;
    delete dColorDelta;
    

    double scaleAngle = 0;
    double scaleDeltaAngle = 0.1;
    double ourScaleSize = mGenetics.getParameter( scaleSize );

    double sineStrength = mGenetics.getParameter( scaleWaveStrength );
    double sineFrequency = mGenetics.getParameter( scaleWaveFrequency );
    
    
    // take square shape of scale into account
    double maxScaleRadius = sqrt( 2 * ourScaleSize * ourScaleSize );


    // make sure scale positions are centered (we want a scale to be at 0,0)
    double limit = (int)(1 / scaleStep) * scaleStep;
    
    for( double bodyY = -limit; bodyY <= limit; bodyY += scaleStep ) {
        
        for( double bodyX = -limit; bodyX <= limit; bodyX += scaleStep ) {

            double adjustedBodyY =
                bodyY +
                sineStrength * sin( sineFrequency * bodyX );

            
            double pointRadius = sqrt( bodyX * bodyX +
                                       adjustedBodyY * adjustedBodyY );
            
            if( pointRadius <= (scaleBoundaryRadius - maxScaleRadius) ) {
                // place a scale

                Vector3D *scalePosition =
                    new Vector3D( bodyX, adjustedBodyY, 0 );
                mScalePositions.push_back( scalePosition );
                mScaleRotations.push_back( scaleAngle );
                mScaleSizes.push_back( ourScaleSize );
                mScaleColors.push_back( currentColors.copy() );


                // adjust angle
                scaleAngle += scaleDeltaAngle;
                
                // adjust colors
                for( int c=0; c<4; c++ ) {
                    Color *thisColor = &( currentColors.mColors[c] );
                    Color *thisDeltaColor = &( deltaColors.mColors[c] );
                    
                    // loop over r, g, b, and a components
                    for( int v=0; v<4; v++ ) {
                        (*thisColor)[v] += (*thisDeltaColor)[v];

                        // wrap around
                        if( (*thisColor)[v] < 0 ) {
                            (*thisColor)[v] = 1;
                            }
                        else if( (*thisColor)[v] > 1 ) {
                            (*thisColor)[v] = 0;
                            }
                        }
                    }

                
                }

            
            }
        }


    
    /*
    // walker starts in center of gardener, walks up, lays first scale

    // lays scale at every step

    double walkerX = 0;
    double walkerY = 0;

    double walkerDeltaX = 0;
    double walkerDeltaY = 0.2;
    
    double walkerDeltaAngle = 0;
    double walkerDeltaDeltaAngle = 0.05;

    ScaleCornerColors currentColors;
    currentColors.mColors[0].setValues( 1, 0, 0, 1 );
    currentColors.mColors[1].setValues( 1, 1, 0, 1 );
    currentColors.mColors[2].setValues( 0, 1, 0, 1 );
    currentColors.mColors[3].setValues( 0, 0, 1, 1 );

    ScaleCornerColors deltaColors;
    deltaColors.mColors[0].setValues( -0.1, 0, 0.1, 0 );
    deltaColors.mColors[1].setValues( -0.1, 1, 0.1, 0 );
    deltaColors.mColors[2].setValues( 0, 0, 0, 0 );
    deltaColors.mColors[3].setValues( 0, 0, 0, 0 );

    double scaleAngle = 0;
    double scaleDeltaAngle = 0.1;

    double scaleSize = 0.1;
    double scaleDeltaSize = 0.01;
    
    
    for( int s=0; s<20; s++ ) {
        // add a scale

        Vector3D *scalePosition = new Vector3D( walkerX, walkerY, 0 );
        mScalePositions.push_back( scalePosition );
        mScaleRotations.push_back( scaleAngle );
        mScaleSizes.push_back( scaleSize );
        mScaleColors.push_back( currentColors.copy() );


        // advance walker
        walkerX += walkerDeltaX;
        walkerY += walkerDeltaY;

        
        if( walkerX < -0.66 || walkerX > 0.66 ) {
            // bounce, reversing direction
            walkerDeltaX *= -1;

            // reset delta angle
            walkerDeltaAngle = 0;
            }
        if( walkerY < -0.66 || walkerY > 0.66 ) {
            // bounce, reversing direction
            walkerDeltaY *= -1;

            // reset delta angle
            walkerDeltaAngle = 0;
            }
        
        
        // adjust direction
        Vector3D direction( walkerDeltaX, walkerDeltaY, 0 );
        Angle3D deltaAngle( 0, 0, walkerDeltaAngle );
        direction.rotate( &deltaAngle );

        walkerDeltaX = direction.mX;
        walkerDeltaY = direction.mY;


        // adjust angles
        walkerDeltaAngle += walkerDeltaDeltaAngle;        
        
        scaleAngle += scaleDeltaAngle;

        // adjust size
        scaleSize += scaleDeltaSize;
        
        // adjust colors
        for( int c=0; c<4; c++ ) {
            Color *thisColor = &( currentColors.mColors[c] );
            Color *thisDeltaColor = &( deltaColors.mColors[c] );

            // loop over r, g, b, and a components
            for( int v=0; v<4; v++ ) {
                (*thisColor)[v] += (*thisDeltaColor)[v];

                // wrap around
                if( (*thisColor)[v] < 0 ) {
                    (*thisColor)[v] = 1;
                    }
                else if( (*thisColor)[v] > 1 ) {
                    (*thisColor)[v] = 0;
                    }
                }
            }
        
        }  // end loop over all scales to place
    
    */
    mGlobalScaleAngle.setComponents( 0, 0, 0 );
    }


Gardener::~Gardener() {
    delete mBaseTexture;
    delete mScaleTexture;
    delete mBonesTexture;
    delete mSkinTexture;
    delete mEyeTexture;
    
    int i;

    int numScales = mScaleSizes.size();

    for( i=0; i<numScales; i++ ) {
        delete *( mScalePositions.getElement( i ) );
        delete *( mScaleColors.getElement( i ) );
        }

    
    int numItems = mStoredItems.size();

    for( i=0; i<numItems; i++ ) {
        delete *( mStoredItems.getElement( i ) );
        }

    if( mOffspring != NULL ) {
        delete mOffspring;
        mOffspring = NULL;
        }


    delete mMovingMusicPart;
    delete mMovingHoldingMusicPart;
    delete mHoldingMusicPart;
    delete mStillMusicPart;
    }



Color *Gardener::getColor() {
    return new Color( mColor );
    }



MusicPart *Gardener::getMusicPart() {
    if( mCarryingWater ) {
        if( mMoving ) {
            return mMovingHoldingMusicPart;
            }
        else {
            return mHoldingMusicPart;
            }
        }
    else {
        if( mMoving ) {
            return mMovingMusicPart;
            }
        else {
            return mStillMusicPart;
            }
        }
    }



void Gardener::setDesiredPosition( Vector3D *inPosition ) {
    mDesiredPosition.setCoordinates( inPosition );
    }



Vector3D *Gardener::getDesiredPosition() {
    return new Vector3D( &mDesiredPosition );
    }



void Gardener::setMoving( char inMoving ) {
    mMoving = inMoving;
    }



char Gardener::isMoving() {
    return mMoving;
    }



void Gardener::setCarryingWater( char inCarryingWater ) {
    mCarryingWater = inCarryingWater;
    }



char Gardener::getCarryingWater() {
    return mCarryingWater;
    }



double Gardener::getNutrientLevel( int inNutrientIndex ) {
    switch( inNutrientIndex ) {
        case 0:
            return mRedNutrient;
        case 1:
            return mGreenNutrient;
        case 2:
            return mBlueNutrient;
        default:
            return 0;
        }
    }



char Gardener::isPregnant() {

    if( mOffspring != NULL ) {
        return true;
        }
    else {
        return false;
        }
    }



double Gardener::getLife() {
    return mLife;
    }



char Gardener::isDead() {
    return mDead;
    }



void Gardener::forceDead() {
    mLife = 0;
    mDead = true;
    }



void Gardener::setGhostMode( char inMode ) {

    mGhostMode = inMode;
    }



char Gardener::isGhost() {
    return mGhostMode;
    }



void Gardener::setFrozen( char inFrozen ) {
    mFrozen = inFrozen;
    }



char Gardener::isFrozen() {
    return mFrozen;
    }



void Gardener::poisonFruit() {

    // one new piece of fruit poisoned every five seconds
    if( mSecondsSinceLastFruitPoisoning > 5 ) {

        int numItems = mStoredItems.size();

        char found = false;
        for( int i=0; i<numItems && !found; i++ ) {
            Storable *item = *( mStoredItems.getElement( i ) );
            
            if( item->getType() == fruitType ) {

                Fruit *fruit = (Fruit *)item;

                if( ! fruit->isPoisoned() ) {

                    fruit->poison();
                    found = true;
                    mSecondsSinceLastFruitPoisoning = 0;
                    }
                }
            }
        }
    }



void Gardener::eat() {

    // get fruit, save seeds in storage, and get any fruit if none selected
    Fruit *fruit = getSelectedFruit( true, true );
    
    if( fruit != NULL ) {


        if( ! fruit->isPoisoned() ) { 
            Color *nutrition = fruit->getNutrition();

            this->feed( nutrition->r, nutrition->g, nutrition->b );

            delete nutrition;
            }
        else {
            // eating a poisoned fruit

            // jump to near death state
            mLife = mNearDeathPoint;
			mPoisoned = true;
            }
        
        delete fruit;
        }
    }



void Gardener::trackOtherGardener( Gardener *inGardener ) {
    mOtherGardeners.push_back( inGardener );

    // start off luke warm toward everyone
    // but liking them a little bit
    mLikeMetrics.push_back( 0.6 );

    // reset
    mMostLiked = NULL;
    mLeastLiked = NULL;
    }



void Gardener::untrackOtherGardener( Gardener *inGardener ) {
    int index = mOtherGardeners.getElementIndex( inGardener );

    if( index != -1 ) {
        mOtherGardeners.deleteElement( index );
        mLikeMetrics.deleteElement( index );
        }
    
    // reset
    mMostLiked = NULL;
    mLeastLiked = NULL;
    }



void Gardener::getAngry( Gardener *inGardener ) {
    int index = mOtherGardeners.getElementIndex( inGardener );

    if( index != -1 ) {
        // decrease like metric
        double metric = *( mLikeMetrics.getElement( index ) );

        metric -= 0.1;

        if( metric < 0 ) {
            metric = 0;
            }
        
        *( mLikeMetrics.getElement( index ) ) = metric;
        }

    // reset
    mMostLiked = NULL;
    mLeastLiked = NULL;
    }



void Gardener::getMaxAngry( Gardener *inGardener ) {
    int index = mOtherGardeners.getElementIndex( inGardener );

    if( index != -1 ) {
        // decrease like metric all the way down
        *( mLikeMetrics.getElement( index ) ) = 0;
        }

    // reset
    mMostLiked = NULL;
    mLeastLiked = NULL;
    }



void Gardener::getFriendly( Gardener *inGardener ) {
    int index = mOtherGardeners.getElementIndex( inGardener );

    if( index != -1 ) {
        // increase like metric
        double metric = *( mLikeMetrics.getElement( index ) );

        metric += 0.1;

        if( metric > 1 ) {
            metric = 1;
            }
        
        *( mLikeMetrics.getElement( index ) ) = metric;
        }

    // reset
    mMostLiked = NULL;
    mLeastLiked = NULL;
    }



double Gardener::getLikeMetric( Gardener *inGardener ) {
    int index = mOtherGardeners.getElementIndex( inGardener );

    if( index != -1 ) {
        return  *( mLikeMetrics.getElement( index ) );
        }
    else {
        return -1;
        }
    }



char Gardener::likeEnoughToMate( Gardener *inGardener ) {
    int index = mOtherGardeners.getElementIndex( inGardener );

    if( index != -1 ) {
        double likeMetric = *( mLikeMetrics.getElement( index ) );

        double matingLevel = mGenetics.getParameter( matingThreshold );

        if( likeMetric >= matingLevel ) {
            return true;
            }
        }

    // default
    return false;
    }



char Gardener::likeEnoughToFollow( Gardener *inGardener ) {
    int index = mOtherGardeners.getElementIndex( inGardener );

    if( index != -1 ) {
        double likeMetric = *( mLikeMetrics.getElement( index ) );

        double followLevel = mGenetics.getParameter( followingThreshold );

        if( likeMetric >= followLevel ) {
            return true;
            }
        }

    // default
    return false;
    }



Gardener *Gardener::getMostLikedGardener() {
    if( mMostLiked != NULL ) {
        return mMostLiked;
        }
    
    int numOthers = mOtherGardeners.size();

    // ignore gardeners that are at or below our friendship threshold
    double highestMetric =
        mGenetics.getParameter( friendshipThreshold ) + 0.01;

    // track all gardeners that are equally high
    SimpleVector<Gardener*> highestGardeners;


    for( int i=0; i<numOthers; i++ ) {
        double metric = *( mLikeMetrics.getElement( i ) );

        if( metric >= highestMetric ) {
            Gardener *other = *( mOtherGardeners.getElement( i ) ); 

            // ignore offspring that are following us
            // ignore already dead
            if( other->getParentToFollow() != this &&
                ! other->isDead() ) {
        
                if( metric == highestMetric ) {
                    // add another to list
 
                    highestGardeners.push_back( other );
                    }            
                else if( metric > highestMetric ) {
                    // a new high
            
                    // clear the list
                    highestGardeners.deleteAll();
                    
                    // start with a new list
                    highestGardeners.push_back( other );
                
                    highestMetric = metric;
                    }
                }
            }        
        }

    int numHigh = highestGardeners.size();

    if( numHigh == 0 ) {
        return NULL;
        }
    else {
        // pick one at random
        int pick = globalRandomSource.getRandomBoundedInt( 0, numHigh - 1 );

        // save for future calls
        mMostLiked = *( highestGardeners.getElement( pick ) );
        
        return mMostLiked;
        }
    }



Gardener *Gardener::getLeastLikedGardener() {
    if( mLeastLiked != NULL ) {
        return mLeastLiked;
        }

    int numOthers = mOtherGardeners.size();

    // ignore gardeners that are at or above our friendship threshold
    double lowestMetric =
        mGenetics.getParameter( friendshipThreshold ) - 0.01;

    // track all gardeners that are equally low
    SimpleVector<Gardener*> lowestGardeners;

    
    for( int i=0; i<numOthers; i++ ) {
        double metric = *( mLikeMetrics.getElement( i ) );

        if( metric <= lowestMetric ) {
            Gardener *other = *( mOtherGardeners.getElement( i ) ); 

            // ignore offspring that are following us
            // ignore already dead
            if( other->getParentToFollow() != this &&
                ! other->isDead() ) {
        
                if( metric == lowestMetric ) {
                    // add another to list
 
                    lowestGardeners.push_back( other );
                    }            
                else if( metric < lowestMetric ) {
                    // a new low
            
                    // clear the list
                    lowestGardeners.deleteAll();
                    
                    // start with a new list
                    lowestGardeners.push_back( other );
                
                    lowestMetric = metric;
                    }
                }
            }
        }

    int numLow = lowestGardeners.size();

    if( numLow == 0 ) {
        return NULL;
        }
    else {
        // pick one at random
        int pick = globalRandomSource.getRandomBoundedInt( 0, numLow - 1 );

        // save for future calls
        mLeastLiked = *( lowestGardeners.getElement( pick ) );
        
        return mLeastLiked;
        }
    }



void Gardener::setEmotionDisplay( double inEmotion ) {
    mEmotionDisplay = inEmotion;
    }



void Gardener::setPlotHidden( char inHidden ) {
    mPlotHidden = inHidden;
    }



char Gardener::isPlotHidden() {
    return mPlotHidden;
    }



void Gardener::storeItem( Storable *inItem ) {
    mStoredItems.push_back( inItem );

    // limit of 10 items
    while( mStoredItems.size() > 10 ) {

        // drop oldest item
        delete *( mStoredItems.getElement( 0 ) );
        mStoredItems.deleteElement( 0 );
        }
    }



int Gardener::getStoredFruitCount() {
    int numItems = mStoredItems.size();

    int count = 0;
    
    for( int i=0; i<numItems; i++ ) {
        Storable *item = *( mStoredItems.getElement( i ) );

        if( item->getType() == fruitType ) {
		    Fruit *fruit = (Fruit *)item;
			
			// ignore poisoned fruit
			if( !fruit->isPoisoned() ) {
			    count ++;
			    }
            }
        }
        
    return count;
    }



char Gardener::isStoredFruitPoisoned() {
    int numItems = mStoredItems.size();

    for( int i=0; i<numItems; i++ ) {
        Storable *item = *( mStoredItems.getElement( i ) );

        if( item->getType() == fruitType ) {
		    Fruit *fruit = (Fruit *)item;

			if( fruit->isPoisoned() ) {
			    return true;
			    }
            }
        }
    
    return false;
    }



Fruit *Gardener::getSelectedFruit( char inSaveSeeds, char inGetAnyFruit ) {

    Storable *item = getSelectedStorable();

    if( item != NULL ) {

        if( inGetAnyFruit && item->getType() != fruitType ) {
            // fruit not selected

            // search for any fruit

            // search through storage for first fruit encountered
            // and set our selection to that
            char found = false;

            int numItems = mStoredItems.size();

            for( int i=0; i<numItems && !found; i++ ) {
                Storable *item = *( mStoredItems.getElement( i ) );

                if( item->getType() == fruitType ) {
                    mSelectedStorageIndex = i;
                    found = true;
                    }
                }

            item = getSelectedStorable();
            }


        if( item->getType() == fruitType ) {
            if( inSaveSeeds ) {
                // replace fruit with seeds
                Fruit *fruit = (Fruit *)item;
                
                Seeds *seeds = fruit->getSeeds();

                *( mStoredItems.getElement( mSelectedStorageIndex ) ) =
                    seeds;
                }
            else {
                // simply remove fruit from storage
                mStoredItems.deleteElement( mSelectedStorageIndex );
                
                if( mSelectedStorageIndex >= mStoredItems.size() ) {
                    mSelectedStorageIndex = mStoredItems.size() - 1;
                    }
                }

            return (Fruit *)item;
            }
        }

    return NULL;
    }



int Gardener::getIndexOfFruitHighInNutrient( int inNutrientIndex ) {

    int numItems = mStoredItems.size();

    int fruitIndex = -1;
    char foundHigh = false;
    
    
    for( int i=0; i<numItems && !foundHigh; i++ ) {
        Storable *item = *( mStoredItems.getElement( i ) );

        if( item->getType() == fruitType ) {

            Fruit *fruit = (Fruit *)item;

            if( ! fruit->isPoisoned() ) { 
                Color *nutrition = fruit->getNutrition();

                double nutrientLevel = (*nutrition)[ inNutrientIndex ];

                delete nutrition;

                if( !foundHigh ) {
                    // found a fruit
                    // take note of it, but keep searching
                    fruitIndex = i;
                    }
            
                if( nutrientLevel == 1 ) {
                    // found high-nutrient fruit, so stop searching
                    foundHigh = true;
                    }
                }
            }
        }
    
    return fruitIndex;
    }



Seeds *Gardener::getSelectedSeeds() {
    Storable *item = getSelectedStorable();

    if( item != NULL ) {
        if( item->getType() == seedsType ) {
            return (Seeds *)item;
            }
        }

    return NULL;
    }



void Gardener::removeSeeds( Seeds *inSeeds ) {
    int numItems = mStoredItems.size();

    for( int i=0; i<numItems; i++ ) {
        Storable *item = *( mStoredItems.getElement( i ) );

        if( item == inSeeds ) {
            delete item;
            mStoredItems.deleteElement( i );

            int numLeft = mStoredItems.size();

            if( mSelectedStorageIndex >= numLeft ) {
                mSelectedStorageIndex = numLeft - 1;
                }
            
            return;
            }
        }
    }



SimpleVector<Seeds *> *Gardener::getAllSeeds() {
    SimpleVector<Seeds *> *seedsVector = new SimpleVector<Seeds *>();
    
    int numItems = mStoredItems.size();

    for( int i=0; i<numItems; i++ ) {
        Storable *item = *( mStoredItems.getElement( i ) );

        if( item->getType() == seedsType ) {
            seedsVector->push_back( (Seeds*)item );
            }
        }    

    return seedsVector;
    }



Storable *Gardener::getSelectedStorable() {
    if( mSelectedStorageIndex == -1 ) {
        if( mStoredItems.size() > 0 ) {
            // default to first fruit

            mSelectedStorageIndex = 0;
            }
        }

    if( mSelectedStorageIndex != -1 ) {
        
        Storable *item = *( mStoredItems.getElement( mSelectedStorageIndex ) );

        return item;
        }
    else {
        return NULL;
        }
    }



Storable *Gardener::getStorable( int inIndex ) {
    return *( mStoredItems.getElement( inIndex ) ); 
    }



void Gardener::deleteSelectedStorable() {
    if( mSelectedStorageIndex != -1 ) {
        
        Storable *item = *( mStoredItems.getElement( mSelectedStorageIndex ) );

        delete item;

        mStoredItems.deleteElement( mSelectedStorageIndex );

        int newItemCount = mStoredItems.size();
        
        if( mSelectedStorageIndex >= newItemCount ) {
            mSelectedStorageIndex = newItemCount - 1;
            }
        }
    }


void Gardener::passTime( double inTimeDeltaInSeconds ) {

    if( mDead ) {
        return;
        }

    
    double energyRate = mBaseEnergyConsumedPerSecond;

    
    double energyConsumed = energyRate * inTimeDeltaInSeconds;

    double energyRateFactor = 1;
    
    if( mMoving ) {
        energyRateFactor *= 2;
        }
    
    if( mCarryingWater ) {
        energyRateFactor *= 2;
        }

    if( mOffspring != NULL ) {
        // pregnant
        energyRateFactor *= 2;
        }

    if( mGrowthProgress < 1 ) {
        // still growing

        // growing consumes energy faster than other activities
        energyRateFactor *= 4;
        }
    

    energyConsumed *= energyRateFactor;
    
    // energy usage quadruples if moving and carrying water at same time
    // energy rate octuples if moving, carrying water, and pregnant
    
    mRedNutrient -= energyConsumed;
    mGreenNutrient -= energyConsumed;
    mBlueNutrient -= energyConsumed;


    // pregnancy cannot progress if low in any nutrient
    char pregnancyCanProgress = true;
    
    double agingFactor = 1;

    // age faster if we're low on nutrients
    if( mRedNutrient <= 0 ) {
        agingFactor *= 2;

        // never negative
        mRedNutrient = 0;
        pregnancyCanProgress = false;
        }
    if( mGreenNutrient <= 0 ) {
        agingFactor *= 2;
        mGreenNutrient = 0;
        pregnancyCanProgress = false;
        }
    if( mBlueNutrient <= 0 ) {
        agingFactor *= 2;
        mBlueNutrient = 0;
        pregnancyCanProgress = false;
        }

	if( mPoisoned ) {
	    agingFactor *= 8;
	    }

    // for testing only
    // pregnancyCanProgress = true;
    
    mLife -= mBaseAgeRate * inTimeDeltaInSeconds * agingFactor;


    if( mLife <= 0 ) {
        mLife = 0;
        mDead = true;

        if( mOffspring != NULL ) {
            // destroy offspring (end pregnancy)
            delete mOffspring;
            mOffspring = NULL;
            }
        }

    
    // age fruit
    int numItems = mStoredItems.size();

    for( int i=0; i<numItems; i++ ) {
        Storable *item = *( mStoredItems.getElement( i ) );

        if( item->getType() == fruitType ) {
            Fruit *fruit = (Fruit *)item;
            fruit->passTime( inTimeDeltaInSeconds );

            if( fruit->isRotten() ) {

                // remove
                delete fruit;

                mStoredItems.deleteElement( i );

                // back up in loop
                numItems = mStoredItems.size();
                i--;

                if( mSelectedStorageIndex >= numItems ) {
                    mSelectedStorageIndex = numItems - 1;
                    }
                }                
            }
        }

    if( mOffspring != NULL &&
        pregnancyCanProgress ) {

        double rate = mGenetics.getParameter( pregnancyProgressRate );

        mPregnancyProgress += rate * inTimeDeltaInSeconds;

        if( mPregnancyProgress >= 1 ) {
            // done

            // give birth

            // stick baby into world at our current position
            Vector3D *position = globalWorld->getGardenerPosition( this );

            // give some seeds
            SimpleVector<Seeds *> *allSeeds = getAllSeeds();

            int numSeeds = allSeeds->size();
            if( numSeeds > 0 ) {
                // give last seeds to offspring to give them a start
                Seeds *ourLastSeeds =
                    *( allSeeds->getElement( numSeeds - 1 ) );

                // give copy
                mOffspring->storeItem( new Seeds( ourLastSeeds ) );
                }
            delete allSeeds;

            // user can control offspring if user can control us
            mOffspring->mUserCanControl = mUserCanControl;


            // use our rotation for offspring start
            addGardenerToGame( mOffspring, position, &mLastDrawAngle );

            delete position;

            // save pointer so that we can feed it
            mOutsideOffspring.push_back( mOffspring );
            
            mOffspring = NULL;
            mPregnancyProgress = 0;
            }
        }

    // can grow if same conditions for pregnancy possible
    if( mGrowthProgress < 1
        && pregnancyCanProgress ) {

        // grow at pregnancy progress rate
        double rate = mGenetics.getParameter( pregnancyProgressRate );
        
        mGrowthProgress += rate * inTimeDeltaInSeconds;
        
        if( mGrowthProgress >= 1 ) {
            mGrowthProgress = 1;

            if( mParent != NULL ) {
                // tell parent to stop feeding us
                mParent->dropOutsideOffspring( this );
            
                // forget parent
                forgetParent();
                }
            }
        }
    
    // water pickup animations
    // animations take 1 second
    if( mCarryingWater && mWaterPickupFraction < 1 ) {
        mWaterPickupFraction += inTimeDeltaInSeconds;

        if( mWaterPickupFraction > 1 ) {
            mWaterPickupFraction = 1;
            }
        }
    else if( !mCarryingWater && mWaterPickupFraction > 0 ) {
        mWaterPickupFraction -= inTimeDeltaInSeconds;

        if( mWaterPickupFraction < 0 ) {
            mWaterPickupFraction = 0;
            }
        }
    
    // rotate scales
    mGlobalScaleAngle.mZ += 0.5 * inTimeDeltaInSeconds * energyRateFactor;

    
    mSecondsSinceLastFruitPoisoning += inTimeDeltaInSeconds;
    }



void Gardener::draw( Vector3D *inPosition, double inScale ) {

    Angle3D defaultAngle( 0, 0, 0 );
    
    draw( inPosition, &defaultAngle, inScale );
    }



void Gardener::draw( Vector3D *inPosition, Angle3D *inRotation,
                     double inScale, double inFadeFactor ) {

    // smooth transition to full transparent at end of life
    double endOfLifeTransparentFactor = 1;
    if( mLife <= mNearDeathPoint ) {
        endOfLifeTransparentFactor = mLife / mNearDeathPoint;
        }

    // add caller's fade factor
    endOfLifeTransparentFactor *= inFadeFactor;

    
    
    mLastDrawAngle.setComponents( inRotation );
    
    double drawScale = inScale * mGrowthProgress;
        

    double radius = drawScale;

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


    if( Features::drawShadows ) {
        // first draw shadow at z = 0

        glColor4f( 0, 0, 0, 0.25 * endOfLifeTransparentFactor );

        mBaseTexture->enable();
        glBegin( GL_QUADS ); {
        
            glTexCoord2f( 0, 0 );
            glVertex3d( corners[0].mX, corners[0].mY, 0 );

            glTexCoord2f( 1, 0 );
            glVertex3d( corners[1].mX, corners[1].mY, 0 );

            glTexCoord2f( 1, 1 );
            glVertex3d( corners[2].mX, corners[2].mY, 0 );

            glTexCoord2f( 0, 1 );
            glVertex3d( corners[3].mX, corners[3].mY, 0 );
            }
        glEnd();
        mBaseTexture->disable();
        }

    
    // base becomes transparent with age
    glColor4f( mColor.r, mColor.g, mColor.b,
               mLife * endOfLifeTransparentFactor);

    mBaseTexture->enable();
    glBegin( GL_QUADS ); {
        
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
    mBaseTexture->disable();


    if( Features::drawComplexGardeners ) {
        // now draw scales
        Angle3D scaleRotation( 0, 0, 0 );

    
        int numScales = mScaleSizes.size();


        // scale patch shrinks as we age
        double scalePatchSizeFactor = mLife;

    
        mScaleTexture->enable();
        glBegin( GL_QUADS ); {

            Vector3D scaleCorners[4];

            for( i=0; i<numScales; i++ ) {
                Vector3D *scalePosition = *( mScalePositions.getElement( i ) );
                scaleRotation.mZ = *( mScaleRotations.getElement( i ) );
                double scaleSize = *( mScaleSizes.getElement( i ) );
                ScaleCornerColors *colors = *( mScaleColors.getElement( i ) );


                scaleRotation.add( &mGlobalScaleAngle );
            
                // first, set up corners relative to 0,0
                scaleCorners[0].mX = - 1;
                scaleCorners[0].mY = - 1;
                scaleCorners[0].mZ = 0;
            
                scaleCorners[1].mX = 1;
                scaleCorners[1].mY = - 1;
                scaleCorners[1].mZ = 0;
            
                scaleCorners[2].mX = 1;
                scaleCorners[2].mY = 1;
                scaleCorners[2].mZ = 0;
            
                scaleCorners[3].mX = - 1;
                scaleCorners[3].mY = 1;
                scaleCorners[3].mZ = 0;

                int c;
    
                // now rotate around center
                // then add inPosition so that center is at inPosition
                for( c=0; c<4; c++ ) {
                    scaleCorners[c].rotate( &scaleRotation );
                    scaleCorners[c].scale( scaleSize );
                    scaleCorners[c].add( scalePosition );

                    // scale entire patch
                    scaleCorners[c].scale( scalePatchSizeFactor );
                
                    scaleCorners[c].scale( drawScale );
                    scaleCorners[c].rotate( inRotation );
                    scaleCorners[c].add( inPosition );
                    }

                // set transparency
                colors->mColors[0].a = endOfLifeTransparentFactor;
                colors->mColors[1].a = endOfLifeTransparentFactor;
                colors->mColors[2].a = endOfLifeTransparentFactor;
                colors->mColors[3].a = endOfLifeTransparentFactor;

                setGLColor( &( colors->mColors[0] ) );
                glTexCoord2f( 0, 0 );
                glVertex3d( scaleCorners[0].mX, scaleCorners[0].mY,
                            scaleCorners[0].mZ );

                setGLColor( &( colors->mColors[1] ) );
                glTexCoord2f( 1, 0 );
                glVertex3d( scaleCorners[1].mX, scaleCorners[1].mY,
                            scaleCorners[1].mZ );

                setGLColor( &( colors->mColors[2] ) );
                glTexCoord2f( 1, 1 );
                glVertex3d( scaleCorners[2].mX, scaleCorners[2].mY,
                            scaleCorners[2].mZ );

                setGLColor( &( colors->mColors[3] ) );
                glTexCoord2f( 0, 1 );
                glVertex3d( scaleCorners[3].mX, scaleCorners[3].mY,
                            scaleCorners[3].mZ );
                }
            }
        glEnd();
        mScaleTexture->disable();


        // draw bones


        // bones become white with age
        Color white( 1, 1, 1, 1 );

        Color *boneColor = Color::linearSum( &mColor, &white, mLife );
    
        glColor4f( boneColor->r, boneColor->g, boneColor->b,
                   0.5 * endOfLifeTransparentFactor );

        delete boneColor;
    
        // bones brighten only
        glBlendFunc( GL_SRC_ALPHA, GL_ONE );

        
        mBonesTexture->enable();
        glBegin( GL_QUADS ); {
        
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
        mBonesTexture->disable();


        // back to normal blend function
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );


        }

    // draw markers

    if( mWaterPickupFraction > 0 ) {

        Vector3D waterCorners[4];

        double waterRadius = radius * 0.5 * mWaterPickupFraction;
        
        // first, set up corners relative to 0,0
        waterCorners[0].mX = - waterRadius;
        waterCorners[0].mY = - waterRadius;
        waterCorners[0].mZ = 0;
        
        waterCorners[1].mX = waterRadius;
        waterCorners[1].mY = - waterRadius;
        waterCorners[1].mZ = 0;
        
        waterCorners[2].mX = waterRadius;
        waterCorners[2].mY = waterRadius;
        waterCorners[2].mZ = 0;
        
        waterCorners[3].mX = - waterRadius;
        waterCorners[3].mY = waterRadius;
        waterCorners[3].mZ = 0;
    
        // now rotate around center
        // then add inPosition so that center is at inPosition
        for( i=0; i<4; i++ ) {
            waterCorners[i].rotate( inRotation );
            waterCorners[i].add( inPosition );
            }

        glColor4f( 0, 0, 1,
                   0.75 * endOfLifeTransparentFactor);
        mBaseTexture->enable();
        glBegin( GL_QUADS ); {
        
            glTexCoord2f( 0, 0 );
            glVertex3d( waterCorners[0].mX, waterCorners[0].mY,
                        waterCorners[0].mZ );
            
            glTexCoord2f( 1, 0 );
            glVertex3d( waterCorners[1].mX, waterCorners[1].mY,
                        waterCorners[1].mZ );
            
            glTexCoord2f( 1, 1 );
            glVertex3d( waterCorners[2].mX, waterCorners[2].mY,
                        waterCorners[2].mZ );
            
            glTexCoord2f( 0, 1 );
            glVertex3d( waterCorners[3].mX, waterCorners[3].mY,
                        waterCorners[3].mZ );
            }
        glEnd();
        mBaseTexture->disable();
        
        }




    // now draw nutrition indicators
    double maxNutrientRadius = radius * 0.25;

    // arrange them in a semi-circle
    
    // "up" direction on gardener body
    Vector3D nutrientPosition( 0, maxNutrientRadius * 2, 0 );

    // first nutrient on rear left of gardener
    Angle3D nutrientAngle( 0, 0, M_PI - M_PI / 5 );
    Angle3D nextNutrientOffsetAngle( 0, 0, M_PI / 5 );

    
    double nutrientAlpha = 1 * endOfLifeTransparentFactor;
    Color nutrientColors[3];
    nutrientColors[0].setValues( 1, 0, 0, nutrientAlpha );
    nutrientColors[1].setValues( 1, 1, 0, nutrientAlpha );
    nutrientColors[2].setValues( 0.5, 0, 1, nutrientAlpha );

    double nutrientLevels[3];
    nutrientLevels[0] = mRedNutrient;
    nutrientLevels[1] = mGreenNutrient;
    nutrientLevels[2] = mBlueNutrient;


    Vector3D nutrientCorners[4];

        
    // first, set up corners relative to 0,0
    nutrientCorners[0].mX = - maxNutrientRadius;
    nutrientCorners[0].mY = - maxNutrientRadius;
    nutrientCorners[0].mZ = 0;
    
    nutrientCorners[1].mX = maxNutrientRadius;
    nutrientCorners[1].mY = - maxNutrientRadius;
    nutrientCorners[1].mZ = 0;
    
    nutrientCorners[2].mX = maxNutrientRadius;
    nutrientCorners[2].mY = maxNutrientRadius;
    nutrientCorners[2].mZ = 0;
        
    nutrientCorners[3].mX = - maxNutrientRadius;
    nutrientCorners[3].mY = maxNutrientRadius;
    nutrientCorners[3].mZ = 0;

    
    
    for( int n=0; n<3; n++ ) {
        Vector3D thisNutrientCenterCorners[4];
        Vector3D thisNutrientBorderCorners[4];
   
        for( i=0; i<4; i++ ) {
            thisNutrientCenterCorners[i].setCoordinates(
                &( nutrientCorners[i] ) );
            thisNutrientBorderCorners[i].setCoordinates(
                &( nutrientCorners[i] ) );

            // shrink center only
            thisNutrientCenterCorners[i].scale( nutrientLevels[n] );

            // rotate and position both center and border
            thisNutrientCenterCorners[i].add( &nutrientPosition );
            thisNutrientCenterCorners[i].rotate( &nutrientAngle ); 
            
            thisNutrientCenterCorners[i].rotate( inRotation );
            thisNutrientCenterCorners[i].add( inPosition );

            thisNutrientBorderCorners[i].add( &nutrientPosition );
            thisNutrientBorderCorners[i].rotate( &nutrientAngle ); 
            
            thisNutrientBorderCorners[i].rotate( inRotation );
            thisNutrientBorderCorners[i].add( inPosition );
            }

        // draw shadow first
        glColor4f( 0, 0, 0,
                   0.5 * endOfLifeTransparentFactor );
        mBaseTexture->enable();
        glBegin( GL_QUADS ); {
        
            glTexCoord2f( 0, 0 );
            glVertex3d( thisNutrientBorderCorners[0].mX,
                        thisNutrientBorderCorners[0].mY,
                        thisNutrientBorderCorners[0].mZ );
            
            glTexCoord2f( 1, 0 );
            glVertex3d( thisNutrientBorderCorners[1].mX,
                        thisNutrientBorderCorners[1].mY,
                        thisNutrientBorderCorners[1].mZ );
            
            glTexCoord2f( 1, 1 );
            glVertex3d( thisNutrientBorderCorners[2].mX,
                        thisNutrientBorderCorners[2].mY,
                        thisNutrientBorderCorners[2].mZ );
            
            glTexCoord2f( 0, 1 );
            glVertex3d( thisNutrientBorderCorners[3].mX,
                        thisNutrientBorderCorners[3].mY,
                        thisNutrientBorderCorners[3].mZ );
            }
        glEnd();


        // now draw center
        setGLColor( &( nutrientColors[n] ) );
        mBaseTexture->enable();
        glBegin( GL_QUADS ); {
        
            glTexCoord2f( 0, 0 );
            glVertex3d( thisNutrientCenterCorners[0].mX,
                        thisNutrientCenterCorners[0].mY,
                        thisNutrientCenterCorners[0].mZ );
            
            glTexCoord2f( 1, 0 );
            glVertex3d( thisNutrientCenterCorners[1].mX,
                        thisNutrientCenterCorners[1].mY,
                        thisNutrientCenterCorners[1].mZ );
            
            glTexCoord2f( 1, 1 );
            glVertex3d( thisNutrientCenterCorners[2].mX,
                        thisNutrientCenterCorners[2].mY,
                        thisNutrientCenterCorners[2].mZ );
            
            glTexCoord2f( 0, 1 );
            glVertex3d( thisNutrientCenterCorners[3].mX,
                        thisNutrientCenterCorners[3].mY,
                        thisNutrientCenterCorners[3].mZ );
            }
        glEnd();
        mBaseTexture->disable();


        // rotate for next nutrient
        nutrientAngle.add( &nextNutrientOffsetAngle );
        }
    


    if( mOffspring != NULL ) {
        // draw pregnancy

        double pregnancyScale = (0.7 * mPregnancyProgress + 0.3) * drawScale;

        mOffspring->draw( inPosition, inRotation, pregnancyScale );
        }


    

    // draw skin
    glColor4f( mColor.r, mColor.g, mColor.b,
               0.5 * endOfLifeTransparentFactor );

        
    mSkinTexture->enable();
    glBegin( GL_QUADS ); {
        
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
    mSkinTexture->disable();




    

    


    // now draw eyes

    double eyeRadius = radius / 5;
    eyeRadius *= mGenetics.getParameter( eyeSize );

    double separation =
        2 * eyeRadius * mGenetics.getParameter( eyeSeparation );

    // go up until eyes would hit top edge
    double bodyRadius = 0.66 * radius - eyeRadius;
    double halfSeparation = separation / 2;

    double eyeHeight = 0;
    
    if( halfSeparation >= bodyRadius ) {
        separation = 2 * bodyRadius;
        halfSeparation = bodyRadius;

        // leave eye height at zero
        }
    else {
        // height is opposite leg of triangle to keep eyes inside bodyRadius

        // safe to take sqrt because bodyRadius is greater than halfSeparation

        // avoid taking sqrt when bodyRadius eqals halfSeparation because
        // round-off errors cause sqrt to return a NAN error sometimes (like
        // when compiler optimizations are on)
        
        eyeHeight = sqrt( bodyRadius * bodyRadius -
                          halfSeparation * halfSeparation );
        }

    
    double squishFactor = 1;
    Angle3D eyeAngle( 0, 0, 0 );
    Color eyeColor( 1, 1, 1,
                    1 * endOfLifeTransparentFactor );
    if( mEmotionDisplay < 0 ) {
        squishFactor += mEmotionDisplay * 0.5;

        eyeAngle.mZ = - mEmotionDisplay * M_PI * 0.5;

        // eyes become red with anger
        eyeColor.g += mEmotionDisplay;
        eyeColor.b += mEmotionDisplay;
        }
    else {
        // eyes become light blue with love
        eyeColor.r -= mEmotionDisplay * 0.5;
        eyeColor.g -= mEmotionDisplay * 0.5;
        }

    
    
    Vector3D leftCorners[4];
    
    // first, set up corners relative to 0,0
    leftCorners[0].mX = - eyeRadius;
    leftCorners[0].mY = - eyeRadius * squishFactor;
    leftCorners[0].mZ = 0;

    leftCorners[1].mX = eyeRadius;
    leftCorners[1].mY = - eyeRadius * squishFactor;
    leftCorners[1].mZ = 0;

    leftCorners[2].mX = eyeRadius;
    leftCorners[2].mY = eyeRadius * squishFactor;
    leftCorners[2].mZ = 0;

    leftCorners[3].mX = - eyeRadius;
    leftCorners[3].mY = eyeRadius * squishFactor;
    leftCorners[3].mZ = 0;

    // right corners are copy of left
    Vector3D rightCorners[4];
    for( i=0; i<4; i++ ) {
        rightCorners[i].setCoordinates( &( leftCorners[i] ) );
        }

    
    // move eye into position relative to gardener center
    // then rotate around center
    // then add inPosition so that center is at inPosition
    for( i=0; i<4; i++ ) {
        leftCorners[i].rotate( &eyeAngle );
        
        leftCorners[i].mY += eyeHeight;

        // go out from center for left eye
        leftCorners[i].mX -= separation / 2;

        leftCorners[i].rotate( inRotation );
        leftCorners[i].add( inPosition );
        }

    
    mEyeTexture->enable();

    setGLColor( &eyeColor );
    
    glBegin( GL_QUADS ); {
        
        glTexCoord2f( 0, 0 );
        glVertex3d( leftCorners[0].mX, leftCorners[0].mY, leftCorners[0].mZ );

        glTexCoord2f( 1, 0 );
        glVertex3d( leftCorners[1].mX, leftCorners[1].mY, leftCorners[1].mZ );

        glTexCoord2f( 1, 1 );
        glVertex3d( leftCorners[2].mX, leftCorners[2].mY, leftCorners[2].mZ );

        glTexCoord2f( 0, 1 );
        glVertex3d( leftCorners[3].mX, leftCorners[3].mY, leftCorners[3].mZ );
        }
    glEnd();
    mEyeTexture->disable();


    // opposite for right
    eyeAngle.scale( -1 );
    for( i=0; i<4; i++ ) {
        rightCorners[i].rotate( &eyeAngle );
        
        rightCorners[i].mY += eyeHeight;

        // go out from center for right eye
        rightCorners[i].mX += separation / 2;

        rightCorners[i].rotate( inRotation );
        rightCorners[i].add( inPosition );
        }



    mEyeTexture->enable();
    setGLColor( &eyeColor );
    glBegin( GL_QUADS ); {
        
        glTexCoord2f( 0, 0 );
        glVertex3d( rightCorners[0].mX, rightCorners[0].mY,
                    rightCorners[0].mZ );

        glTexCoord2f( 1, 0 );
        glVertex3d( rightCorners[1].mX, rightCorners[1].mY,
                    rightCorners[1].mZ );

        glTexCoord2f( 1, 1 );
        glVertex3d( rightCorners[2].mX, rightCorners[2].mY,
                    rightCorners[2].mZ );

        glTexCoord2f( 0, 1 );
        glVertex3d( rightCorners[3].mX, rightCorners[3].mY,
                    rightCorners[3].mZ );
        }
    glEnd();
    mEyeTexture->disable();
    
    }



int Gardener::getStoredObjects( DrawableObject ***outObjects ) {

    if( outObjects != NULL ) {
        *outObjects = (DrawableObject **)( mStoredItems.getElementArray() );
        }
    
    return mStoredItems.size();
    }



int Gardener::getSelectedObjectIndex() {
    return mSelectedStorageIndex;
    }



void Gardener::setSelectedObjectIndex( int inIndex ) {

    mSelectedStorageIndex = inIndex;
    }



void Gardener::setPregnant( Gardener *inOffspring ) {
    if( mOffspring != NULL ) {
        delete mOffspring;
        }
    mOffspring = inOffspring;

    mPregnancyProgress = 0;
    }



Gardener *Gardener::getParentToFollow() {
    return mParent;
    }



void Gardener::forgetParent() {
    mParent = NULL;
    }



void Gardener::dropOutsideOffspring( Gardener *inOffspring ) {
    mOutsideOffspring.deleteElementEqualTo( inOffspring );
    }



void Gardener::feed( double inRedNutrient, double inGreenNutrient,
                     double inBlueNutrient ) {

    Color nutrition( inRedNutrient, inGreenNutrient, inBlueNutrient );
    
    // share with outside offspring and followers
    int numOffspring = mOutsideOffspring.size();
    int numFollowers = mFollowers.size();

    int numToFeed = numFollowers + numOffspring;
            
    // split evenly among self and offspring
    nutrition.weightColor( 1.0 / ( numToFeed + 1 ) );
        
    mRedNutrient += nutrition.r;
    if( mRedNutrient > 1 ) {
        mRedNutrient = 1;
        }
    mGreenNutrient += nutrition.g;
    if( mGreenNutrient > 1 ) {
        mGreenNutrient = 1;
        }
    mBlueNutrient += nutrition.b;
    if( mBlueNutrient > 1 ) {
        mBlueNutrient = 1;
        }

    int i;
    for( i=0; i<numOffspring; i++ ) {
        Gardener *offspring = *( mOutsideOffspring.getElement( i ) );
        
        offspring->feed( nutrition.r, nutrition.g, nutrition.b );
        }
    
    for( i=0; i<numFollowers; i++ ) {
        Gardener *follower = *( mFollowers.getElement( i ) );
        
        follower->feed( nutrition.r, nutrition.g, nutrition.b );
        }
    }



void Gardener::setLeader( Gardener *inLeader ) {
    mLeader = inLeader;
    }



Gardener *Gardener::getLeader() {
    return mLeader;
    }



void Gardener::addFollower( Gardener *inFollower ) {
    mFollowers.push_back( inFollower );
    }


void Gardener::dropFollower( Gardener *inFollower ) {
    mFollowers.deleteElementEqualTo( inFollower );
    }


        
