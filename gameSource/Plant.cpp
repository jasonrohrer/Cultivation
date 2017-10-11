/*
 * Modification History
 *
 * 2006-July-2   Jason Rohrer
 * Created.
 *
 * 2006-September-25   Jason Rohrer
 * Fixed fruit highlighting bug.
 */



#include "Plant.h"

#include "features.h"
#include "glCommon.h"


#include <GL/gl.h>
#include <math.h>


#include "minorGems/util/random/StdRandomSource.h"
#include "World.h"



extern StdRandomSource globalRandomSource;
extern World *globalWorld;


#include "minorGems/graphics/filters/BoxBlurFilter.h"



Plant::Plant( double inSoilType, Seeds *inSeeds )
    : mSeeds( inSeeds ),
      mLeaf( &( inSeeds->mGenetics ) ),
      mFlower( &( inSeeds->mGenetics ) ),
      mLeafTerminiiSet( false ),
      mTimeSinceLastFlower( 0 ),
      mHighlightRipeFruit( false ),
      mWaterStatus( 0.0 ),
      mJustWatered( false ),
      mPoisonStatus( 0.0 ),
      mPoisoned( false ),
      mGrowth( 0 ),
      mTimeSinceLastFruit( 0 ),
      // 30 seconds
      mTimeBetweenFruits( 30 ),
      mWaterConsumptionRate( 0.10 ),
      mGrowthRate( 0.05 ),
      mCurrentSoilType( inSoilType ),
      mStartZAngle(
          globalRandomSource.getRandomBoundedDouble( 0, 2 * M_PI ) ) {


    // water consumption based on leaf area

    // An area fraction of 0.25 results in the "standard" consumption rate
    // Larger areas increase the rate, smaller areas decrease it
    double leafAreaFraction = mLeaf.getLeafAreaFraction();

    double adjustedFraction = ( leafAreaFraction + 0.75 );

    // raise to 4th power to increase effect
    mWaterConsumptionRate *= pow( adjustedFraction, 4 );

    

    // generate texture for leaf joint cap

    // a blurry circle of outer leaf color
    int textureSize = 32;
    
    
    Image *textureImage = new Image( textureSize, textureSize, 4, false );

    double *channels[4];

    int pixelsPerChannel = textureSize * textureSize;
    
    int i;
    int p;
    for( i=0; i<4; i++ ) {
        channels[i] = textureImage->getChannel( i );
        }

    // start all pixels as outer leaf color with alpha of zero
    double leafGreen = mSeeds.mGenetics.getParameter( outerLeafGreen );
    for( p=0; p<pixelsPerChannel; p++ ) {
        channels[0][p] = 0;
        channels[1][p] = leafGreen;
        channels[2][p] = 0;
        channels[3][p] = 0;
        }

    
    double radius = textureSize / 3;
    
    for( int y=0; y<textureSize; y++ ) {
        for( int x=0; x<textureSize; x++ ) {

            double value;

            double xRelative = x - (0.5 *textureSize );
            double yRelative = y - (0.5 *textureSize );
            
            if( sqrt( xRelative * xRelative + yRelative * yRelative )
                <= radius ) {
                value = 1;
                }
            else {
                value = 0;
                }

            channels[3][ y * textureSize + x ] = value;
            }
        }

    // blur alpha
    BoxBlurFilter blur( 3 );

    textureImage->filter( &blur, 3 );
    

    mJointCapTexture = new SingleTextureGL( textureImage,
                                            // no wrap
                                            false );

    delete textureImage; 
    }



Plant::~Plant() {

    delete mJointCapTexture;

    int numFlowers = mFlowerTerminusIndicies.size();

    int f;
    for( f=0; f<numFlowers; f++ ) {
        delete *( mFlowerAngles.getElement( f ) );
        }

    int numTerminii = mLeafTerminii.size();
        
    for( int t=0; t<numTerminii; t++ ) {
        delete *( mLeafTerminii.getElement( t ) );
        }

    int numFruit = mFruit.size();
    
    for( f=0; f<numFruit; f++ ) {
        delete *( mFruit.getElement( f ) );
        delete *( mFruitAngles.getElement( f ) );
        }

    }



void Plant::water() {
    mJustWatered = true;
    }



void Plant::poison() {
    mPoisoned = true;
    }



double Plant::getPoisonStatus() {
    return mPoisonStatus;
    }



char Plant::isRipe() {
    int numFruit = mFruit.size();
    
    for( int f=0; f<numFruit; f++ ) {

        Fruit *thisFruit = *( mFruit.getElement( f ) );

        if( thisFruit->isRipe() ) {
            return true;
            }
        }
                
    return false;
    }



void Plant::highlightFirstRipeFruit( char inHighlight ) {
    mHighlightRipeFruit = inHighlight;
    }



Fruit *Plant::harvest( Vector3D *outFruitPosition ) {
    // harvest first fruit that is ripe
    int numFruit = mFruit.size();
    
    for( int f=0; f<numFruit; f++ ) {

        Fruit *thisFruit = *( mFruit.getElement( f ) );

        if( thisFruit->isRipe() ) {
            int terminusIndex =
                *( mFruitTerminusIndices.getElement( f ) );
                
            Vector3D *terminus =
                *( mLeafTerminii.getElement( terminusIndex ) );
            
            

            mFruit.deleteElement( f );
            mFruitTerminusIndices.deleteElement( f );

            delete *( mFruitAngles.getElement( f ) );
            mFruitAngles.deleteElement( f );


            outFruitPosition->setCoordinates( terminus );
            return thisFruit;
            }
        }

    return NULL;
    }



Fruit *Plant::peekAtRipeFruit() {
    int numFruit = mFruit.size();
    
    for( int f=0; f<numFruit; f++ ) {

        Fruit *thisFruit = *( mFruit.getElement( f ) );

        if( thisFruit->isRipe() ) {
            return thisFruit;
            }
        }

    return NULL;
    }



double Plant::getWaterStatus() {
    if( mGrowth < 1 ) {
        return mWaterStatus;
        }
    else {
        // done growing
        // no longer needs to be watered
        return 1;
        }
    }



char Plant::isFullGrown() {
    if( mGrowth < 1 ) {
        return false;
        }
    else {
        return true;
        }
    }



void Plant::passTime( double inTimeDeltaInSeconds ) {

    // become poisoned gradually
    if( mPoisoned ) {
        // 1 second to shrivel
        mPoisonStatus += inTimeDeltaInSeconds;

        if( mPoisonStatus >= 1 ) {
            mPoisonStatus = 1;
            }
        // do nothing else
        return;
        }

    
    // become watered gradually if just watered
    if( mJustWatered ) {

        // take 1 second to fill up
        mWaterStatus += inTimeDeltaInSeconds;

        if( mWaterStatus >= 1 ) {
            mWaterStatus = 1;
            mJustWatered = false;
            }
        
        }
    
    // growth rate reduced as soil gets further from our ideal soil type.
    // if completely opposite (disance between ideal and actual is 1),
    // then growth goes to half it's normal rate.
    double localGrowthRate =
        mGrowthRate *
        ( 1 - 0.5 * fabs( mCurrentSoilType - mSeeds.mIdealSoilType ) );

    
    if( mWaterStatus > 0.0 && mGrowth < 1 ) {
        // growing
        mGrowth += inTimeDeltaInSeconds * localGrowthRate;

        // don't consume water if just watered to avoid
        // race condition
        if( !mJustWatered ) {
            // consume water
            mWaterStatus -= inTimeDeltaInSeconds * mWaterConsumptionRate;
            }
        
        if( mWaterStatus < 0 ) {
            mWaterStatus = 0;
            }

        
        if( mGrowth >= 1.0 ) {
            mGrowth = 1.0;

            // done growing
            // remove all extra water
            mWaterStatus = 0;
            }
        }
    else if( mGrowth >= 1.0 ) {
        // done growing

        mTimeSinceLastFlower += inTimeDeltaInSeconds;


        // advance each flower
        int numFlowers = mFlowerTerminusIndicies.size();

        // bloom rate from leaf area
        // larger leaves results in faster bloom
        double bloomRate = mWaterConsumptionRate;

        int f;
        for( int f=0; f<numFlowers; f++ ) {
            double stage = *( mFlowerStages.getElement( f ) );

            stage += inTimeDeltaInSeconds * bloomRate;

            if( stage > 4 ) {
                stage = 4;
                }

            *( mFlowerStages.getElement( f ) ) = stage;            

            if( stage == 4 ) {
                    
                // start growing fruit

                Seeds *offspringSeeds;
            
                Plant *closestPlant = globalWorld->getClosestPlant( this );
            
                if( closestPlant != NULL ) {
                    // cross
                    offspringSeeds =
                        new Seeds( &( mSeeds ),
                                   &( closestPlant->mSeeds ) );
                        
                    }
                else {
                    // duplicate self
                    offspringSeeds = new Seeds( &mSeeds );
                    }

                // ripen rate depends on water consumption
                Fruit *newFruit = new Fruit( &( mSeeds.mGenetics ),
                                             offspringSeeds,
                                             mWaterConsumptionRate );

                delete offspringSeeds;

                mFruit.push_back( newFruit );
                mFruitTerminusIndices.push_back(
                    *( mFlowerTerminusIndicies.getElement( f ) ) );
                Angle3D *flowerAngle =
                    *( mFlowerAngles.getElement( f ) );

                mFruitAngles.push_back( new Angle3D( flowerAngle ) );


                // remove flower
                mFlowerTerminusIndicies.deleteElement( f );
                mFlowerStages.deleteElement( f );
                mFlowerAngles.deleteElement( f );
                delete flowerAngle;

                // reset boundary
                numFlowers = mFlowerTerminusIndicies.size();

                // step f back so we don't miss one
                f--;
                }
                
            }


        int numFruit = mFruit.size();
    
        for( f=0; f<numFruit; f++ ) {
            Fruit *thisFruit = *( mFruit.getElement( f ) );

            thisFruit->passTime( inTimeDeltaInSeconds );

            if( thisFruit->isRotten() ) {
                // remove it

                delete thisFruit;
                delete *( mFruitAngles.getElement( f ) );

                mFruit.deleteElement( f );
                mFruitTerminusIndices.deleteElement( f );
                mFruitAngles.deleteElement( f );

                numFruit = mFruit.size();
                // back up in loop
                f--;
                }
            }

        /*
        if( mRipeFruit == NULL ) {
            // ripening fruit
            
            mTimeSinceLastFruit += inTimeDeltaInSeconds;

            if( mTimeSinceLastFruit >= mTimeBetweenFruits ) {

                // fruit ripe
                Seeds *offspringSeeds;
            
                Plant *closestPlant = globalWorld->getClosestPlant( this );
            
                if( closestPlant != NULL ) {
                    // cross
                    offspringSeeds = new Seeds( &( mSeeds ),
                                                &( closestPlant->mSeeds ) );
                
                    }
                else {
                    // duplicate self
                    offspringSeeds = new Seeds( &mSeeds );
                    }
            
                mRipeFruit = new Fruit( &( mSeeds.mFruitNutrition ),
                                        offspringSeeds );

                delete offspringSeeds;
                }
            }
        */
        }
    }



void Plant::draw( Vector3D *inPosition, double inScale,
                  double inMaxZ, double inMinZ ) {


    if( mPoisoned && mPoisonStatus >= 1) {
        // draw nothing
        return;
        }

    
    double drawScale = inScale;

    if( mPoisoned ) {
        // shrink with poisoning
        
        drawScale *= ( 1 - mPoisonStatus );
        }

    
    double radius = drawScale * ( mGrowth * 0.8 + 0.2 );


    // leaves become black with poisoning
    // (shades of white to allow texture color to dominate) 
    Color leafColor( 1 - mPoisonStatus,
                     1 - mPoisonStatus,
                     1 - mPoisonStatus, 1 );

    
    if( ! Features::drawNicePlantLeaves ) {
        // set color to shades of green green for leaves if we're drawing
        // simple boxes, since there's no texture color

        leafColor.setValues( 0, 1 - mPoisonStatus, 0, 1 );
        }

    
    Angle3D zeroAngle( 0, 0, 0 );
    

    PlantGenetics *genetics = &( mSeeds.mGenetics );

    
    
    int maxNumJoints = (int)( genetics->getParameter( jointCount ) );

    double growthFactor = mGrowth * 0.8 + 0.2;
    
    int numFullJoints = (int)( growthFactor * maxNumJoints );

    double partialJoint = growthFactor * maxNumJoints - numFullJoints;

    int numLeavesPerJoint = (int)( genetics->getParameter( leavesPerJoint ) );

    Angle3D angleIncrement( 0, 0, 2 * M_PI / numLeavesPerJoint );
    Angle3D startAngle( 0, 0, mStartZAngle );

    double currentScale = 1;

    double scaleDecrement = currentScale / ( maxNumJoints + 1 );

    Vector3D leafPosition( inPosition );

    Vector3D positionIncrement( 0, 0, -0.5 );

    Vector3D leafWalkerTerminus;

    SimpleVector<Vector3D *> thisLayerLeafTerminii;
    
    
    for( int j=0; j<numFullJoints; j++ ) {

        // lower leaves are darker
        double colorScaleFactor = (double)(j+1) / (double)maxNumJoints; 
        // min scaling of 0.5
        colorScaleFactor = colorScaleFactor * 0.5 + 0.5;
        
        
        Color thisLevelColor;
        thisLevelColor.setValues( &leafColor );
        thisLevelColor.weightColor( colorScaleFactor );
        
        
        
        
        Angle3D currentAngle( &startAngle );

        double zValue = leafPosition.mZ;

        if( zValue <= inMaxZ && zValue >= inMaxZ ) {
            // draw this joint
            for( int g=0; g<numLeavesPerJoint; g++ ) {

                if( Features::drawShadows ) {
                    // draw shadow
                    glColor4f( 0, 0, 0, 0.5 );
                    mLeaf.draw( &leafPosition, &currentAngle,
                                currentScale * radius * 1.05 );
                    }
                
                // draw leaf
                setGLColor( &thisLevelColor );
                
                mLeaf.draw( &leafPosition, &currentAngle,
                            currentScale * radius,
                            &leafWalkerTerminus );


                thisLayerLeafTerminii.push_back(
                    new Vector3D( &leafWalkerTerminus ) );

                
                currentAngle.add( &angleIncrement );
                }

            // finally cap this joint
            setGLColor( &thisLevelColor );
            mJointCapTexture->enable();
            glBegin( GL_QUADS ); {

                double capRadius = currentScale * radius * 0.1;
                double capZ = leafPosition.mZ;
                
                glTexCoord2f( 0, 0 );
                glVertex3d( leafPosition.mX - capRadius,
                            leafPosition.mY - capRadius, capZ );

                glTexCoord2f( 1, 0 );
                glVertex3d( leafPosition.mX + capRadius,
                            leafPosition.mY - capRadius, capZ );
                
                glTexCoord2f( 1, 1 );
                glVertex3d( leafPosition.mX + capRadius,
                            leafPosition.mY + capRadius, capZ );

                glTexCoord2f( 0, 1 );
                glVertex3d( leafPosition.mX - capRadius,
                            leafPosition.mY + capRadius, capZ );
                }
            glEnd();
            mJointCapTexture->disable();        
            }
        
        
        Angle3D angleToNextJoint( &angleIncrement );

        angleToNextJoint.scale( 0.5 );

        currentAngle.add( &angleToNextJoint );

        // start next joint at our current angle
        startAngle.setComponents( &currentAngle );

        currentScale -= scaleDecrement;

        leafPosition.add( &positionIncrement );
        }

    if( partialJoint > 0 ) {
        Angle3D currentAngle( &startAngle );

        // darker as growing completes


        // lower leaves are darker
        double colorScaleFactor =
            (double)(numFullJoints+1) / (double)maxNumJoints; 

        // min scaling of 0.5
        colorScaleFactor = colorScaleFactor * 0.5 + 0.5;

        // scale factor comes into effect as partial joint reaches 1
        colorScaleFactor = (1 - partialJoint) +
            colorScaleFactor * partialJoint;
        
        Color thisLevelColor;
        thisLevelColor.setValues( &leafColor );
        thisLevelColor.weightColor( colorScaleFactor );
        
        

        double zValue = leafPosition.mZ;

        if( zValue <= inMaxZ && zValue >= inMaxZ ) {
            // draw this joint
        
            for( int g=0; g<numLeavesPerJoint; g++ ) {

                if( Features::drawShadows ) {
                    // draw shadow
                    glColor4f( 0, 0, 0, 0.5 );
                    mLeaf.draw( &leafPosition, &currentAngle,
                                partialJoint * currentScale * radius * 1.05 );
                    }
                
                setGLColor( &thisLevelColor );

                mLeaf.draw( &leafPosition, &currentAngle,
                            // scale down further by partial fraction
                            partialJoint * currentScale * radius );

                currentAngle.add( &angleIncrement );
                }

            // finally cap this joint
            setGLColor( &thisLevelColor );
            mJointCapTexture->enable();
            glBegin( GL_QUADS ); {

                double capRadius = currentScale * radius * 0.1;
                double capZ = leafPosition.mZ;
                
                glTexCoord2f( 0, 0 );
                glVertex3d( leafPosition.mX - capRadius,
                            leafPosition.mY - capRadius, capZ );

                glTexCoord2f( 1, 0 );
                glVertex3d( leafPosition.mX + capRadius,
                            leafPosition.mY - capRadius, capZ );
                
                glTexCoord2f( 1, 1 );
                glVertex3d( leafPosition.mX + capRadius,
                            leafPosition.mY + capRadius, capZ );

                glTexCoord2f( 0, 1 );
                glVertex3d( leafPosition.mX - capRadius,
                            leafPosition.mY + capRadius, capZ );
                }
            glEnd();
            mJointCapTexture->disable();
            }
        }

    int numTerminii = thisLayerLeafTerminii.size();
    int t;
    
    if( mGrowth >= 1 ) {

        // NOTE:
        // This method of collecting all leaf terminii for the plant ASSUMES
        // that each terminus is at a unique location
        // This seems like a safe assumption, given the way leaves are
        // arranged now, but it is not safe in the general case.
        
        // If two terminii are at the same location, the terminus collection
        // would finish before collecting all terminii
        
        
        if( !mLeafTerminiiSet ) {
            // not done collecting leaf terminii for full-growth plant

            int numExisting = mLeafTerminii.size();
            char collision = false;
            
            for( int t=0; t<numTerminii && !collision; t++ ) {
                Vector3D *newTerminus =
                    *( thisLayerLeafTerminii.getElement( t ) );
                
                // make sure not the same as existing
                char same = false;
                for( int e=0; e<numExisting && !same; e++ ) {
                    Vector3D *existing = *( mLeafTerminii.getElement( e ) );

                    if( existing->equals( newTerminus ) ) {
                        same = true;
                        collision = true;
                        }
                    }

                if( !same ) {
                    // add to list of all terminii
                    mLeafTerminii.push_back( new Vector3D( newTerminus ) );
                    }                
                }

            if( collision ) {
                // we are back to drawing a layer that we've already drawn
                // before

                // so we're not gathering new leaf terminii anymore

                mLeafTerminiiSet = true;
                }
            }
        else {
        
            // don't try adding flowers if we already have more than
            // numTerminii
            // flowers
            int numTotalTerminii = mLeafTerminii.size();
            int numFlowers = mFlowerTerminusIndicies.size();
            int numFruit = mFruitTerminusIndices.size();
            
            if( numFlowers < numTotalTerminii &&
                mTimeSinceLastFlower >=
                genetics->getParameter( timeBetweenFlowers ) ) {
                // new flower

                // pick random, unflowered, unfruited terminus
            

                int numTries = 0;
                char found = false;
                int foundIndex = -1;
                while( ! found && numTries < 100 ) {
                    foundIndex =
                        globalRandomSource.getRandomBoundedInt(
                            0,
                            numTotalTerminii - 1 );
                    found = true;
                    int f;
                    for( f=0; f<numFlowers && found; f++ ) {
                        if( *( mFlowerTerminusIndicies.getElement( f ) )
                            ==
                            foundIndex ) {
                            // collision with existing flower location
                            found = false;
                            }
                        }
                    for( f=0; f<numFruit && found; f++ ) {
                        if( *( mFruitTerminusIndices.getElement( f ) )
                            ==
                            foundIndex ) {
                            // collision with existing fruit location
                            found = false;
                            }
                        }
                    numTries++;
                    }

                if( found ) {
                    mFlowerTerminusIndicies.push_back( foundIndex );
                    mFlowerStages.push_back( 0 );
                    mFlowerAngles.push_back(
                        new Angle3D(
                            0, 0,
                            globalRandomSource.getRandomBoundedDouble(
                                0, 2 * M_PI ) ) );
                    }
            
                mTimeSinceLastFlower = 0;
                }

        
            // recount, since we may have added some
            numFlowers = mFlowerTerminusIndicies.size();
            
            for( int f=0; f<numFlowers; f++ ) {
                int terminusIndex =
                    *( mFlowerTerminusIndicies.getElement( f ) );
                
                Vector3D *terminus =
                    *( mLeafTerminii.getElement( terminusIndex ) );
            
                double zValue = terminus->mZ;
                
                if( zValue <= inMaxZ && zValue >= inMaxZ ) {
                    
                    Angle3D *flowerAngle = *( mFlowerAngles.getElement( f ) );
                    
                    double flowerStage = *( mFlowerStages.getElement( f ) );
                    
                    mFlower.draw( terminus,
                                  flowerAngle, drawScale, flowerStage );
                    }
                }
            }

        // draw fruit
        int numFruit = mFruit.size();

        
        for( int f=0; f<numFruit; f++ ) {
            int terminusIndex =
                *( mFruitTerminusIndices.getElement( f ) );
                
            Vector3D *terminus =
                *( mLeafTerminii.getElement( terminusIndex ) );
            
            double zValue = terminus->mZ;
            
            if( zValue <= inMaxZ && zValue >= inMaxZ ) {
                Angle3D *fruitAngle = *( mFruitAngles.getElement( f ) );
                Fruit *thisFruit = *( mFruit.getElement( f ) );

                double fruitScale = drawScale * 0.2;
                
                thisFruit->draw( terminus,
                                 fruitAngle, fruitScale );

                if( mHighlightRipeFruit && thisFruit->isRipe() ) {

                    // make sure this is the fruit that we will harvest
                    // next
                    // (the z-range drawing can screw us
                    // up here, since we might draw fruits out-of-order)
                    // thus, the first-drawn ripe fruit is not necessarily
                    // the fruit that will be next harvested
                    Fruit *fruitNextHarvested = peekAtRipeFruit();

                    
                    if( thisFruit == fruitNextHarvested ) {
                        // this fruit will be harvested next
                                            
                        glColor4f( 1, 1, 1, 0.25 );
                        
                        // highlight brightens only
                        glBlendFunc( GL_SRC_ALPHA, GL_ONE );

                        drawBlurCircle( terminus, fruitScale );

                        // back to normal blend function
                        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

                        // only highlight one
                        mHighlightRipeFruit = false;
                        }
                    }
                }
            }
        }

    // delete this layer's terminus points
    for( t=0; t<numTerminii; t++ ) {
        delete *( thisLayerLeafTerminii.getElement( t ) );
        }
    }

