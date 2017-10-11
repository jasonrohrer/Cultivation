/*
 * Modification History
 *
 * 2006-July-2   Jason Rohrer
 * Created.
 *
 * 2006-September-12   Jason Rohrer
 * Fixed the machine-gun mating bug.
 *
 * 2006-September-26   Jason Rohrer
 * Improved cloud appearance.
 *
 * 2006-October-4   Jason Rohrer
 * Fixed a crash when a baby's parent dies and is removed.
 *
 * 2006-October-27   Jason Rohrer
 * Added portal.
 *
 * 2006-November-2   Jason Rohrer
 * Made portal active area smaller.
 *
 * 2006-November-20   Jason Rohrer
 * Fixed so gardener stops moving as it passes through portal.
 *
 * 2006-December-25   Jason Rohrer
 * Added function for checking if portal is open.
 * Added function for checking closed status of portal.
 * Changed so that gardeners get angry if you mate with their most liked.
 *
 * 2007-November-17   Jason Rohrer
 * Added ignoreGardener function to fix bug in ghost mode.
 * Fixed bug with dead gardener getting angry about mating.
 */


#include "features.h"
#include "World.h"
#include "emotionIcons.h"
#include "glCommon.h"
#include "gameFunctions.h"
// used to generate cloud texture
#include "landscape.h"

#include "sound/SoundEffectsBank.h"



#include <GL/gl.h>
#include <float.h>

#include "minorGems/util/random/StdRandomSource.h"

#include "minorGems/graphics/Image.h"
#include "minorGems/graphics/filters/SeamlessFilter.h"


extern StdRandomSource globalRandomSource;

extern SoundEffectsBank *globalSoundEffectsBank;


extern World *globalWorld;


// #include "minorGems/io/file/FileOutputStream.h"
// #include "minorGems/graphics/converters/TGAImageConverter.h"


World::World( Vector3D *inCornerA, Vector3D *inCornerB )
    : mCornerA( inCornerA ), mCornerB( inCornerB ),
      mSoilMap( inCornerA, inCornerB ),
      mCloudPosition( 0 ),
      mCloudMotionRate( 0.005 ),
      mGardenerVelocity( 10 ),
      mGardenerRotationVelocity( 3 ),
      mPortalPosition( 0, 0, 0 ),
      mPortal( NULL ),
      mMinPlantingDistance( 5 ),
      mMaxDistanceThatCountsAsClose( 10 ),
      mGardenerZ( -2.5 ),
      mHighlightPosition( NULL ) {


    // for testing
    // mPortal = new Portal();
    // mPortal->upgrade();
    
        
    // generate cloud texture

    int textureSize = 64;
    int numPixels = textureSize * textureSize;
    Image *textureImage = new Image( textureSize, textureSize, 4, false );

    double *channels[4];

    
    for( int i=0; i<4; i++ ) {
        channels[i] = textureImage->getChannel( i );
        }

    // white
    // leave alpha uninitialized
    int p;
    for( p=0; p<numPixels; p++ ) {
        channels[0][p] = 1;
        }
    memcpy( channels[1], channels[0], numPixels * sizeof( double ) );
    memcpy( channels[2], channels[0], numPixels * sizeof( double ) );

    double cloudTimeSeed = time( NULL );

    double minValue = DBL_MAX;
    double maxValue = DBL_MIN;
    
    for( int y=0; y<textureSize; y++ ) {
        for( int x=0; x<textureSize; x++ ) {
            
            double value = landscape(
                20*x, 20*y, cloudTimeSeed,
                0.01, 0.5, 5 );
            
            channels[3][ y * textureSize + x ] = value;

            if( value < minValue ) {
                minValue = value;
                }
            if( value > maxValue ) {
                maxValue = value;
                }
            }
        }

    // now scale by max and min
    double range = maxValue - minValue;
    for( p=0; p<numPixels; p++ ) {
        channels[3][p] -= minValue;
        channels[3][p] /= range;
        }

    
    // lower brightness
    double brightness = 0.85;
    // increase contrast
    double contrast = 4;
    
    for( p=0; p<numPixels; p++ ) {
        double value = channels[3][p];

        value *= brightness;
        
        value -= 0.5;
        value *= contrast;
        value += 0.5;

        // clamp
        if( value > 1 ) {
            value = 1;
            }
        else if( value < 0 ) {
            value = 0;
            }
        channels[3][p] = value;
        }
        
    /*
    File outFileA( NULL, "clouds.tga" );
    FileOutputStream *outStreamA = new FileOutputStream( &outFileA );

    TGAImageConverter converter;
    
    converter.formatImage( textureImage, outStreamA );
    delete outStreamA;
    */
    
    // make seamless
    SeamlessFilter seamless( 2 );
    textureImage->filter( &seamless, 3 );

    
    /*    
    File outFileB( NULL, "clouds_seamless.tga" );
    FileOutputStream *outStreamB = new FileOutputStream( &outFileB );

    converter.formatImage( textureImage, outStreamB );
    delete outStreamB;
    */
    
    
    // wrap
    mCloudTexture = new SingleTextureGL( textureImage, true );

    delete textureImage;
    }



World::~World() {
    int i;
    
    int numPlants = mPlants.size();

    for( i=0; i<numPlants; i++ ) {
        delete *( mPlants.getElement( i ) );
        delete *( mPlantPositions.getElement( i ) );
        }

    int numGardeners = mGardeners.size();

    // walk backward through vector so we can remove
    // gardeners from it as we go
    for( i=numGardeners-1; i>=0; i-- ) {
        removeGardener( i );
        }


    int numFlyingObjects = mFlyingObjects.size();

    for( i=0; i<numFlyingObjects; i++ ) {
        delete *( mFlyingObjects.getElement( i ) );
        }

    if( mHighlightPosition != NULL ) {
        delete mHighlightPosition;
        mHighlightPosition = NULL;
        }

    delete mCloudTexture;

    if( mPortal != NULL ) {
        delete mPortal;
        mPortal = NULL;
        }
    }



void World::addPlant( Gardener *inGardener,
                      Plant *inPlant, Vector3D *inPosition ) {

    mPlants.push_back( inPlant );

    mPlantPositions.push_back( new Vector3D( inPosition ) );

    mLastPlantTender.push_back( inGardener );
    
    angerOthers( inGardener, inPosition );
    }



void World::addGardener( Gardener *inGardener, Vector3D *inPosition,
                         Angle3D *inRotation ) {

    // tell others to track
    int numGardeners = mGardeners.size();
    for( int i=0; i<numGardeners; i++ ) {
        Gardener *other = *( mGardeners.getElement( i ) );

        other->trackOtherGardener( inGardener );

        inGardener->trackOtherGardener( other );
        }

    
    mGardeners.push_back( inGardener );

    mGardenerPositions.push_back( new Vector3D( inPosition ) );

    Angle3D *rotation;
    if( inRotation == NULL ) {
        rotation = new Angle3D( 0, 0, 0 );
        }
    else {
        rotation = new Angle3D( inRotation );
        }
    
    mGardenerRotations.push_back( rotation );
    mGardenerRotationsComplete.push_back( false );
    mGardenerDestinationForRotation.push_back( new Vector3D( inPosition ) );
    
    mGardenerPlotsCornerA.push_back( NULL );
    mGardenerPlotsCornerB.push_back( NULL );

    mGardenerTargetOfPregnancy.push_back( false );


    numGardeners = mGardeners.size();
    setNumGardeners( numGardeners );
    }



void World::removeGardener( Gardener *inGardener ) {

    int i = mGardeners.getElementIndex( inGardener );

    if( i != -1 ) {
        removeGardener( i );
        }
    }



void World::ignoreGardener( Gardener *inGardener ) {

    int i = mGardeners.getElementIndex( inGardener );

    if( i != -1 ) {
        ignoreGardener( i );
        }
    }



void World::ignoreGardener( int inGardenerIndex ) {

    int i = inGardenerIndex;


    Gardener *gardener = *( mGardeners.getElement( i ) );

    // remove any flying objects that are heading toward this gardener
    // since there will no longer be a destination for them
    int j=0;
    while( j < mFlyingObjects.size() ) {
        FlyingObject *object = *( mFlyingObjects.getElement( j ) );

        if( object->mDestinationGardener == gardener ) {
            delete object;
            mFlyingObjects.deleteElement( j );
            }
        else {
            j++;
            }
        }

    
    // all others should untrack this gardener
    
    int numGardeners = mGardeners.size();

    for( j=0; j<numGardeners; j++ ) {
        if( j != i ) {
            Gardener *other = *( mGardeners.getElement( j ) );

            other->untrackOtherGardener( gardener );

            
            // also, remove this gardener as a parent from any baby offspring
            Gardener *parentOfOther = other->getParentToFollow();

            if( parentOfOther == gardener ) {
                // this is our offspring and it's still following us

                // sorry, goodbye
                other->forgetParent();
                }

            // tell others to stop following us
            Gardener *leaderOfOther = other->getLeader();

            if( leaderOfOther == gardener ) {
                other->setLeader( NULL );
                }
            }
        }

    
    // tell our leader to drop us
    Gardener *ourLeader = gardener->getLeader();
    if( ourLeader != NULL ) {
        ourLeader->dropFollower( gardener );
        }

    // tell our parent to drop us
    Gardener *ourParent = gardener->getParentToFollow();
    if( ourParent != NULL ) {
        ourParent->dropOutsideOffspring( gardener );
        }
    

    // remove as plant tender
    int numPlants = mPlants.size();
    for( j=0; j<numPlants; j++ ) {
        if( gardener == *( mLastPlantTender.getElement( j ) ) ) {
            *( mLastPlantTender.getElement( j ) ) = NULL;            
            }
        }

    // clear plot
    setGardenerPlot( gardener, NULL, NULL );
    }



    
void World::removeGardener( int inGardenerIndex ) {
    int i = inGardenerIndex;
    
    ignoreGardener( i );
    
    delete *( mGardeners.getElement( i ) );
    delete *( mGardenerPositions.getElement( i ) );
    delete *( mGardenerRotations.getElement( i ) );
    delete *( mGardenerDestinationForRotation.getElement( i ) );
    

    mGardeners.deleteElement( i );
    mGardenerPositions.deleteElement( i );
    mGardenerRotations.deleteElement( i );
    mGardenerRotationsComplete.deleteElement( i );
    mGardenerDestinationForRotation.deleteElement( i );

    mGardenerPlotsCornerA.deleteElement( i );
    mGardenerPlotsCornerB.deleteElement( i );

    int numGardeners = mGardeners.size();
    setNumGardeners( numGardeners );
    }



Vector3D *World::getGardenerPosition( Gardener *inGardener ) {
    int index = mGardeners.getElementIndex( inGardener );

    if( index != -1 ) {
        return new Vector3D( *( mGardenerPositions.getElement( index ) ) );
        }
    else {
        return NULL;
        }    
    }



Vector3D **World::getAllGardenerPositions( int *outNumGardeners ) {
    int numGardeners = mGardenerPositions.size();

    Vector3D **returnArray = new Vector3D*[ numGardeners ];

    for( int i=0; i<numGardeners; i++ ) {
        returnArray[i] =
            new Vector3D( *( mGardenerPositions.getElement( i ) ) );
        }

    *outNumGardeners = numGardeners;

    return returnArray;
    }



Gardener **World::getAllGardeners( int *outNumGardeners ) {
    int numGardeners = mGardeners.size();

    Gardener **returnArray = new Gardener*[ numGardeners ];

    for( int i=0; i<numGardeners; i++ ) {
        returnArray[i] = *( mGardeners.getElement( i ) );
        }

    *outNumGardeners = numGardeners;

    return returnArray;    
    }



MusicPart **World::getAllGardenerMusicParts( int *outNumGardeners ) {
    int numGardeners = mGardenerPositions.size();

    MusicPart **returnArray = new MusicPart*[ numGardeners ];

    for( int i=0; i<numGardeners; i++ ) {
        Gardener *thisGardener = *( mGardeners.getElement( i ) );
        
        returnArray[i] = thisGardener->getMusicPart();
        }

    *outNumGardeners = numGardeners;

    return returnArray;    
    }



double *World::getAllGardenerMusicVolumeModifiers( int *outNumGardeners ) {
    int numGardeners = mGardenerPositions.size();

    double *returnArray = new double[ numGardeners ];

    for( int i=0; i<numGardeners; i++ ) {
        Gardener *thisGardener = *( mGardeners.getElement( i ) );
        
        returnArray[i] = thisGardener->getLife();
        }

    *outNumGardeners = numGardeners;

    return returnArray;
    }



Vector3D *World::getPlantPosition( Plant *inPlant ) {
    int index = mPlants.getElementIndex( inPlant );

    if( index != -1 ) {
        return new Vector3D( *( mPlantPositions.getElement( index ) ) );
        }
    else {
        return NULL;
        }    
    }



void World::setGardenerPlot( Gardener *inGardener,
                             Vector3D *inCornerA, Vector3D *inCornerB ) {
    
    int index = mGardeners.getElementIndex( inGardener );

    if( index != -1 ) {

        // delete any old corner values
        
        Vector3D *cornerA, *cornerB;
        cornerA = *( mGardenerPlotsCornerA.getElement( index ) );
        cornerB = *( mGardenerPlotsCornerB.getElement( index ) );

        if( cornerA != NULL ) {
            delete cornerA;
            }
        if( cornerB != NULL ) {
            delete cornerB;
            }


        if( inCornerA != NULL && inCornerB != NULL ) {
            // set new values
        
            *( mGardenerPlotsCornerA.getElement( index ) ) =
                new Vector3D( inCornerA );
            
            *( mGardenerPlotsCornerB.getElement( index ) ) =
                new Vector3D( inCornerB );
            }
        else {
            // leave NULL values
            *( mGardenerPlotsCornerA.getElement( index ) ) = NULL;
            
            *( mGardenerPlotsCornerB.getElement( index ) ) = NULL;
            }
                 
        }
    }



char World::isInPlot( Gardener *inGardener ) {
    
    int index = mGardeners.getElementIndex( inGardener );

    if( index != -1 ) {

        Vector3D *position = *( mGardenerPositions.getElement( index ) ); 

        return isInPlot( inGardener, position );
        }
    else {
        return false;
        }
    }



char World::isInPlot( Gardener *inGardener, Vector3D *inPosition ) {

    if( inGardener->isDead() ) {
        return false;
        }
    
    int index = mGardeners.getElementIndex( inGardener );

    if( index != -1 ) {

        Vector3D *cornerA, *cornerB;
        cornerA = *( mGardenerPlotsCornerA.getElement( index ) );
        cornerB = *( mGardenerPlotsCornerB.getElement( index ) );

        if( cornerA != NULL && cornerB != NULL ) {

            return isInRectangle( inPosition, cornerA, cornerB );            
            }
        else {
            // no plot set
            return false;
            }        
        }
    else {
        // not our gardener
        return false;
        }
    }



char World::isInWater( Gardener *inGardener ) {

    int index = mGardeners.getElementIndex( inGardener );

    if( index != -1 ) {

        Vector3D *position = *( mGardenerPositions.getElement( index ) ); 

        return ! mSoilMap.isInBounds( position );
        }
    else {
        // not our gardener
        return false;
        }        
    }



char World::isInWater( Vector3D *inPosition ) {
    return ! mSoilMap.isInBounds( inPosition );
    }



Plant *World::getClosestPlotPlant( Gardener *inGardener,
                                   char inGetEvenIfTooFar,
                                   char inGetOnlyRipe ) {
    
        
    int index = mGardeners.getElementIndex( inGardener );

    if( index == -1 || inGardener->isDead() ) {
        // we do not manage this gardener or it's already dead
        return NULL;
        }
    
    
    Vector3D *position = *( mGardenerPositions.getElement( index ) );
    
    Vector3D *cornerA = *( mGardenerPlotsCornerA.getElement( index ) );
    Vector3D *cornerB = *( mGardenerPlotsCornerB.getElement( index ) );

    if( cornerA == NULL || cornerB == NULL ) {
        // no plot for this gardener
        
        return NULL;
        }
    // else gardener has a plot
    
    
    // check each plant to find closest plant in gardener's plot

    double closestDistance;
    if( ! inGetEvenIfTooFar ) {
        // only consider plants that are close to inGardener
        closestDistance = mMaxDistanceThatCountsAsClose;
        }
    else {
        // consider all plants
        closestDistance = DBL_MAX;
        }
    
    Plant *closestPlant = NULL;


    
    int numPlants = mPlants.size();
    
    for( int i=0; i<numPlants; i++ ) {
        Plant *plant = *( mPlants.getElement( i ) );

        // ignore poisoned plants
        if( plant->getPoisonStatus() == 0 ) {
            Vector3D *plantPosition = *( mPlantPositions.getElement( i ) );

            // ignore those outside of plot
            if( isInRectangle( plantPosition, cornerA, cornerB ) ) {
                double distance = position->getDistance( plantPosition );

                // ignore those that are to far away to count as close
                if( distance <= closestDistance ) {
                    
                    // ignore all but ripe plants if caller requests it
                    if( !inGetOnlyRipe ||
                        plant->isRipe() ) {
                        
                        closestDistance = distance;
                        closestPlant = plant;
                        }
                    }
                }
            }
        }

    
    return closestPlant;
    }



SimpleVector<Plant*> *World::getPlotPlants( Gardener *inGardener ) {
    SimpleVector<Plant*> *resultVector = new SimpleVector<Plant*>;


    int index = mGardeners.getElementIndex( inGardener );

    if( index == -1 || inGardener->isDead() ) {
        // we do not manage this gardener or it's already dead

        // empty vector
        return resultVector;
        }
    
    Vector3D *cornerA = *( mGardenerPlotsCornerA.getElement( index ) );
    Vector3D *cornerB = *( mGardenerPlotsCornerB.getElement( index ) );

    if( cornerA == NULL || cornerB == NULL ) {
        // no plot for this gardener

        // return empty vector
        return resultVector;
        }
    // else gardener has a plot


    int numPlants = mPlants.size();
    
    for( int i=0; i<numPlants; i++ ) {

        Plant *plant = *( mPlants.getElement( i ) );

        // ignore poisoned plants
        if( plant->getPoisonStatus() == 0 ) {
        
            Vector3D *plantPosition = *( mPlantPositions.getElement( i ) );

            // ignore those outside of plot
            if( isInRectangle( plantPosition, cornerA, cornerB ) ) {


                resultVector->push_back( plant );
                }
            }
        }
    
    return resultVector;
    }



Plant *World::getTendedPlant( Gardener *inGardener ) {
    if( inGardener->isDead() ) {
        return NULL;
        }

    int numPlants = mLastPlantTender.size();
    
    for( int i=0; i<numPlants; i++ ) {
        Gardener *lastTender = *( mLastPlantTender.getElement( i ) );

        if( lastTender == inGardener ) {
            Plant *plant = *( mPlants.getElement( i ) );

            // ignore poisoned plants
            if( plant->getPoisonStatus() == 0 ) {
                return plant;
                }
            }
        }

    return NULL;
    }



/**
 * Special FlyingObject implementation for fruit exchange (or harvest).
 *
 * When fruit reaches destination, it is added to storage of recieving
 * gardener.
 */
class FruitFlyingObject : public FlyingObject {

    public:

        /**
         * Same as constructor for FlyingObject, except:
         *
         * @param inGiver the gardener that is giving inDestinationGardener
         *   fruit.  NULL if there is no giver (harvest case).
         */
        FruitFlyingObject( Gardener *inGiver,
                           DrawableObject *inObject,
                           char inDestroyObject,
                           Gardener *inDestinationGardener,
                           Vector3D *inStartPosition )
            : FlyingObject( inObject, inDestroyObject, inDestinationGardener,
                            inStartPosition ),
            mGiver( inGiver ) {

            }
        

        virtual void reachedDestination() {
            // no longer need to destroy object, since we're passing
            // it to destination gardener
            mDestroyObject = false;

            mDestinationGardener->storeItem( (Fruit*) mObject );            

            
            if( mGiver != NULL ) {

                // make sure giver still in world
                Vector3D *giverPosition =
                    globalWorld->getGardenerPosition( mGiver );

                if( giverPosition != NULL ) {
                    mDestinationGardener->getFriendly( mGiver );
                    globalWorld->flyEmotionIcon( mDestinationGardener, mGiver,
                                                 new LikeIcon() );
                    delete giverPosition;
                    }
                }
            }

    protected:
        Gardener *mGiver;
    };



void World::harvestPlant( Gardener *inGardener, Plant *inPlant ) {

    Vector3D *plantPosition = getPlantPosition( inPlant );

    Vector3D *gardenerPosition = getGardenerPosition( inGardener );

    if( plantPosition != NULL && gardenerPosition != NULL ) {

        if( isInPlot( inGardener, plantPosition ) ) {

            double distance = gardenerPosition->getDistance( plantPosition );

            if( distance <= mMaxDistanceThatCountsAsClose ) {

                // anger other gardeners (if this plant in their plots)
                // with this action
                angerOthers( inGardener, plantPosition );

                // count harvesting as tending
                int index = mPlants.getElementIndex( inPlant );

                *( mLastPlantTender.getElement( index ) ) = inGardener;

                
                Vector3D fruitPosition;
                
                Fruit *fruit = inPlant->harvest( &fruitPosition );


                
                
                if( fruit != NULL ) {

                    // true in case it doesn't reach its destination
                    char destroyObject = true;

                    FruitFlyingObject *flyingObject
                        = new FruitFlyingObject( NULL,
                                 fruit, destroyObject, inGardener,
                                 &fruitPosition );


                    mFlyingObjects.push_back( flyingObject );
                    }
                
                }
            }
        
        }

    if( plantPosition != NULL ) {
        delete plantPosition;
        }
    if( gardenerPosition != NULL ) {
        delete gardenerPosition;
        }   
    }



void World::dumpWater( Gardener *inGardener ) {

    Plant *plant = getClosestPlotPlant( inGardener );

    if( plant != NULL ) {
        plant->water();

        int index = mPlants.getElementIndex( plant );

        *( mLastPlantTender.getElement( index ) ) = inGardener;
        }
    }



void World::dumpPoison( Gardener *inGardener ) {
    Vector3D *position = getGardenerPosition( inGardener );

    if( position != NULL ) {
        Plant *plant = getClosestPlant( position );

        if( plant != NULL ) {
            plant->poison();

            // FIXME:  flying object?

            Vector3D *plantPosition = getPlantPosition( plant );
            angerOthers( inGardener, plantPosition );
            delete plantPosition;
            }
        }
    
    delete position;
    }



void World::getGardenerPlot( Gardener *inGardener,
                             Vector3D **inCornerA, Vector3D **inCornerB ) {

    // default to returning NULL
    *inCornerA = NULL;
    *inCornerB = NULL;


    if( inGardener->isDead() ) {
        // ignore plots of dead gardeners
        return;
        }
        
    
    int index = mGardeners.getElementIndex( inGardener );

    if( index != -1 ) {
    
        Vector3D *cornerA, *cornerB;
        cornerA = *( mGardenerPlotsCornerA.getElement( index ) );
        cornerB = *( mGardenerPlotsCornerB.getElement( index ) );

        if( cornerA != NULL ) {
            *inCornerA = new Vector3D( cornerA );
            }
        
        if( cornerB != NULL ) {
            *inCornerB = new Vector3D( cornerB );
            }
        }
    }



double World::getPlotArea( Gardener *inGardener ) {
    Vector3D *cornerA, *cornerB;

    getGardenerPlot( inGardener, &cornerA, &cornerB );

    double area = 0;
    
    if( cornerA != NULL ) {
        area = getRectangleArea( cornerA, cornerB );

        delete cornerA;
        delete cornerB;
        }

    return area;
    }



double World::getPlotIntersectionArea( Gardener *inGardenerA,
                                       Gardener *inGardenerB ) {
    Vector3D *cornerFirstA, *cornerFirstB;

    getGardenerPlot( inGardenerA, &cornerFirstA, &cornerFirstB );

    if( cornerFirstA == NULL ) {
        // A has no plot, no intersection
        return 0;
        }

    
    Vector3D *cornerSecondA, *cornerSecondB;

    getGardenerPlot( inGardenerB, &cornerSecondA, &cornerSecondB );

    if( cornerSecondA == NULL ) {
        delete cornerFirstA;
        delete cornerFirstB;

        return 0;
        }

    Vector3D *intersectionA, *intersectionB;

    getRectangleIntersection( cornerFirstA, cornerFirstB,
                              cornerSecondA, cornerSecondB,
                              &intersectionA, &intersectionB );

    delete cornerFirstA;
    delete cornerFirstB;
    delete cornerSecondA;
    delete cornerSecondB;

    if( intersectionA == NULL ) {
        return 0;
        }

    double area = getRectangleArea( intersectionA, intersectionB );

    delete intersectionA;
    delete intersectionB;

    return area;
    }



Gardener *World::getClosestGardener( Gardener *inGardener ) {
    int index = mGardeners.getElementIndex( inGardener );
    
    if( index == -1 ) {
        return NULL;
        }
    else {
        Vector3D *position = *( mGardenerPositions.getElement( index ) );
                                
        
        Gardener *closestGardener = NULL;
        double minDistance = DBL_MAX;

        int numGardeners = mGardeners.size();

        for( int i=0; i<numGardeners; i++ ) {

            Gardener *otherGardener = *( mGardeners.getElement( i ) );

            // ignore self
            // ignore dead gardeners
            // ignore offspring that are following us
            if( otherGardener != inGardener &&
                ! otherGardener->isDead() &&
                otherGardener->getParentToFollow() != inGardener ) {

                Vector3D *otherPosition =
                    *( mGardenerPositions.getElement( i ) );

                double distance = position->getDistance( otherPosition );

                if( distance < minDistance ) {
                    minDistance = distance;
                    closestGardener = otherGardener;
                    }                
                }
            }

        return closestGardener;
        }
    }



double World::getGardenerDistance( Gardener *inGardenerA,
                                   Gardener *inGardenerB ) {

    Vector3D *positionA = getGardenerPosition( inGardenerA );
    Vector3D *positionB = getGardenerPosition( inGardenerB );
    
    
    double distance = positionA->getDistance( positionB );
    
    delete positionA;
    delete positionB;

    return distance;
    }



Plant *World::getClosestPlant( Plant *inPlant ) {
    int index = mPlants.getElementIndex( inPlant );
    
    if( index == -1 ) {
        return NULL;
        }
    else {
        Vector3D *position = *( mPlantPositions.getElement( index ) );


        return getClosestPlant( position, inPlant );
        }
    }
        


Plant *World::getClosestPlant( Vector3D *inPosition, Plant *inPlantToIgnore,
                               char inGetEvenIfTooFar,
                               char inGetEvenIfPoisoned ) {
        
    Plant *closestPlant = NULL;
    double minDistance = DBL_MAX;

    if( ! inGetEvenIfTooFar ) {
        // reduce starting min distance to ignore those that are
        // farther away
        minDistance = mMaxDistanceThatCountsAsClose;
        }
    
    
    int numPlants = mPlants.size();

    for( int i=0; i<numPlants; i++ ) {

        Plant *otherPlant = *( mPlants.getElement( i ) );

        // skip ignored plants
        // skip poisoned plants if flag set to false
        if( otherPlant != inPlantToIgnore &&
            ( inGetEvenIfPoisoned ||
              otherPlant->getPoisonStatus() == 0 ) ) {

            Vector3D *otherPosition =
                *( mPlantPositions.getElement( i ) );

            double distance = inPosition->getDistance( otherPosition );

            if( distance <= minDistance ) {
                minDistance = distance;
                closestPlant = otherPlant;
                }                
            }
        }

    return closestPlant;
    }



char World::canPlant( Vector3D *inPosition ) {

    if( ! mSoilMap.isInBounds( inPosition ) ) {
        // can't plant in water
        return false;
        }

    // do not ignore poisoned plants,
    // since we can't plant too close to them
    Plant *plant = getClosestPlant( inPosition, NULL, true, true );

    if( plant == NULL ) {
        return true;
        }
    else {
        Vector3D *position = getPlantPosition( plant );

        double distance = position->getDistance( inPosition );
        delete position;

        if( distance >= mMinPlantingDistance ) {
            return true;
            }
        }

    return false;
    }



Vector3D *World::getClosestWater( Gardener *inGardener ) {
    Vector3D *position = getGardenerPosition( inGardener );

    Vector3D *returnValue = mSoilMap.getClosestBoundaryPoint( position );

    delete position;

    return returnValue;
    }



Vector3D *World::getClosestLand( Vector3D *inPosition ) {

    if( mSoilMap.isInBounds( inPosition ) ) {
        // this is a land point
        return new Vector3D( inPosition );
        }
    else {
        // closest land to a water point
        return mSoilMap.getClosestBoundaryPoint( inPosition );
        }
    }



Vector3D *World::getRandomLandPoint() {
    double x = globalRandomSource.getRandomBoundedDouble( mCornerA.mX,
                                                          mCornerB.mX );
    double y = globalRandomSource.getRandomBoundedDouble( mCornerA.mY,
                                                          mCornerB.mY );

    Vector3D *returnValue = new Vector3D( x, y, 0 );

    if( isInWater( returnValue ) ) {
        
        // find a land point near this water point
        Vector3D *newReturnValue =
            mSoilMap.getClosestBoundaryPoint( returnValue );

        delete returnValue;
        returnValue = newReturnValue;
        }
    // else we picked a land point
    
    return returnValue;
    }



Vector3D *World::getPlotCenter( Gardener *inGardener ) {

    Vector3D *a, *b;
    
    getGardenerPlot( inGardener, &a, &b );

    Vector3D *returnValue = NULL;

    if( a != NULL && b != NULL ) {
        returnValue = getRectangleCenter( a, b );

        delete a;
        delete b;
        }

    return returnValue;
    }



void World::removePlant( Plant *inPlant ) {
    int index = mPlants.getElementIndex( inPlant );

    if( index != -1 ) {
        mPlants.deleteElement( index );

        delete *( mPlantPositions.getElement( index ) );
        mPlantPositions.deleteElement( index );

        mLastPlantTender.deleteElement( index );
        
        delete inPlant;
        }
    
    // else this plant not managed by this world
    }



void World::giveFruit( Gardener *inGiver, Gardener *inReceiver,
                       Fruit *inFruit ) {

    Vector3D *startPosition = getGardenerPosition( inGiver );

    // true in case it doesn't reach its destination
    char destroyObject = true;

    FruitFlyingObject *flyingObject
        = new FruitFlyingObject( inGiver,
                                 inFruit, destroyObject, inReceiver,
                                 startPosition );

    delete startPosition;

    mFlyingObjects.push_back( flyingObject );    
    }



/**
 * Special FlyingObject implementation for mating.
 *
 * When object reaches destination, that gardener becomes pregnant.
 */
class MatingFlyingObject : public FlyingObject {

    public:

        /**
         * Same as constructor for FlyingObject, except:
         *
         * @param inSourceGardener the source of the offspring object.
         */
        MatingFlyingObject( Gardener *inSourceGardener,
                            DrawableObject *inObject,
                            char inDestroyObject,
                            Gardener *inDestinationGardener,
                            Vector3D *inStartPosition )
            : FlyingObject( inObject, inDestroyObject, inDestinationGardener,
                            inStartPosition ),
            mSourceGardener( inSourceGardener ) {

            }
        

        virtual void reachedDestination() {
            // no longer need to destroy object, since we're passing
            // it to destination gardener
            mDestroyObject = false;

            mDestinationGardener->setPregnant( (Gardener*) mObject );

            // no longer target of pregnancy---actually pregnant
            globalWorld->cancelTargetOfPregnancy( mDestinationGardener );

            // check if other gardeners should get mad at source gardener

            // first, make sure source gardener still exists
            Vector3D *sourceGardenerPosition =
                globalWorld->getGardenerPosition( mSourceGardener );

            if( sourceGardenerPosition != NULL ) {
                // source gardener still in world

                int numGardeners;
                Gardener **allGardeners =
                    globalWorld->getAllGardeners( &numGardeners );

                for( int g=0; g<numGardeners; g++ ) {
                    Gardener *gardener = allGardeners[g];

                    if( gardener != mDestinationGardener &&
                        gardener != mSourceGardener &&
                        ! gardener->isDead() ) {

                        // another gardener (a third party to this mating)

                        // who is its most liked?
                        Gardener *mostLiked = gardener->getMostLikedGardener();

                        if( mostLiked == mDestinationGardener ) {

                            // mSourceGardener mated with gardener's most liked

                            // this makes gardener angry
                            gardener->getAngry( mSourceGardener );

                            Vector3D *gardenerPosition =
                                globalWorld->getGardenerPosition( gardener );
                            
                            globalWorld->flyEmotionIcon( gardener,
                                                         mSourceGardener,
                                                         new DislikeIcon(),
                                                         // normal size icon
                                                         // for angry
                                                         false );

                            delete gardenerPosition;
                            }
                        }
                    }
                
                delete [] allGardeners;
                
                delete sourceGardenerPosition;
                }
            }

    protected:
        Gardener *mSourceGardener;
        
    };



void World::mateGardeners( Gardener *inParentA, Gardener *inParentB ) {

    Gardener *offspringOfA = new Gardener( inParentA, inParentB );
    Gardener *offspringOfB = new Gardener( inParentB, inParentA );


    Vector3D *startPositionOffspringOfA = getGardenerPosition( inParentB );
    Vector3D *startPositionOffspringOfB = getGardenerPosition( inParentA );

    // true in case it doesn't reach its destination
    char destroyObject = true;

    MatingFlyingObject *flyingObjectToA
        = new MatingFlyingObject( inParentB,
                                  offspringOfA, destroyObject, inParentA,
                                  startPositionOffspringOfA );

    delete startPositionOffspringOfA;

    mFlyingObjects.push_back( flyingObjectToA );


    MatingFlyingObject *flyingObjectToB
        = new MatingFlyingObject( inParentA,
                                  offspringOfB, destroyObject, inParentB,
                                  startPositionOffspringOfB );

    delete startPositionOffspringOfB;

    mFlyingObjects.push_back( flyingObjectToB );


    // track that these are targets
    int indexA = mGardeners.getElementIndex( inParentA );
    int indexB = mGardeners.getElementIndex( inParentB );

    *( mGardenerTargetOfPregnancy.getElement( indexA ) ) = true;
    *( mGardenerTargetOfPregnancy.getElement( indexB ) ) = true;    
    }



char World::isTargetOfPregnancy( Gardener *inGardener ) {
    int index = mGardeners.getElementIndex( inGardener );

    return *( mGardenerTargetOfPregnancy.getElement( index ) );
    }

        

void World::cancelTargetOfPregnancy( Gardener *inGardener ) {
    int index = mGardeners.getElementIndex( inGardener );

    *( mGardenerTargetOfPregnancy.getElement( index ) ) = false;    
    }



Gardener *World::getNextUserControllableGardener(
    Gardener *inGardenerToSkip ) {

    int i;
    
    // move each moving gardener
    int numGardeners = mGardeners.size();

    for( i=0; i<numGardeners; i++ ) {
        Gardener *gardener = *( mGardeners.getElement( i ) );

        if( !gardener->isDead() && gardener != inGardenerToSkip ) {
            if( gardener->mUserCanControl ) {
                return gardener;
                }
            }
        }

    return NULL;
    }


// for testing
// double timeSinceLastPortalUpgrade = 0;


void World::passTime( double inTimeDeltaInSeconds ) {

    int i;

    Vector3D upVector( 0, 1, 0 );

    
    // move each moving gardener
    int numGardeners = mGardeners.size();

    for( i=0; i<numGardeners; i++ ) {
        Gardener *gardener = *( mGardeners.getElement( i ) );

        // frozen gardeners don't move
        // dead gardeners don't move, unless in ghost mode
        if( !gardener->isFrozen() &&
            ( !gardener->isDead() || gardener->isGhost() ) ) {
        
            Vector3D *position = *( mGardenerPositions.getElement( i ) );
            
            Vector3D *desiredPosition = gardener->getDesiredPosition();

            // check the destination that our rotation was set for
            Vector3D *lastDesiredPosition =
                *( mGardenerDestinationForRotation.getElement( i ) );

            if( ! lastDesiredPosition->equals( desiredPosition ) ) {

                // new destination

                // our rotation not complete
                // we need to rotate again
                *( mGardenerRotationsComplete.getElement( i ) ) = false;
                }

            
            if( ! position->equals( desiredPosition ) ) {
                // not there yet
                
                // turn desired position into a scaled motion vector

                // vector representing the full path from current position to
                // desired position
                Vector3D travelVector( desiredPosition );
                travelVector.subtract( position );
                
                
                Vector3D motionVector( travelVector );
                motionVector.normalize();

                Angle3D *currentAngle =
                    *( mGardenerRotations.getElement( i ) );

                // don't worry about angle once we are done rotating
                // and have started moving
                char rotationAlreadyComplete =
                    *( mGardenerRotationsComplete.getElement( i ) );

                
                Angle3D *desiredAngle =
                    upVector.getZAngleTo( &motionVector );

                
                motionVector.scale( inTimeDeltaInSeconds * mGardenerVelocity );
                
                if( motionVector.getLength() > travelVector.getLength() ) {
                    // overshooting
                    
                    // truncate
                    motionVector.setCoordinates( &travelVector );
                    }

                if( !rotationAlreadyComplete &&
                    currentAngle->mZ != desiredAngle->mZ ) {

                    // rotate more

                    Angle3D rotation( desiredAngle );
                    rotation.subtract( currentAngle );

                    if( fabs( rotation.mZ ) > M_PI ) {
                        // more than 180

                        // rotate in other direction instead (shorter)

                        if( rotation.mZ > 0 ) {
                            rotation.mZ -= 2 * M_PI;
                            }
                        else if( rotation.mZ < 0 ) {
                            rotation.mZ += 2 * M_PI;
                            }
                        }
                    
                    double maxRotationThisStep = inTimeDeltaInSeconds *
                        mGardenerRotationVelocity;

                    if( rotation.mZ < 0 ) {
                        maxRotationThisStep = -maxRotationThisStep;
                        }

                    if( fabs( rotation.mZ ) > fabs( maxRotationThisStep ) ) {
                        // truncate
                        rotation.mZ = maxRotationThisStep;
                        }
                    
                    currentAngle->add( &rotation );
                    if( fabs( currentAngle->mZ ) > M_PI ) {
                        if( currentAngle->mZ < 0 ) {
                            currentAngle->mZ += 2 * M_PI;
                            }
                        else if( currentAngle->mZ > 0 ) {
                            currentAngle->mZ -= 2 * M_PI;
                            }
                        }
                    }

                if( currentAngle->mZ == desiredAngle->mZ ||
                    rotationAlreadyComplete ) {

                    
                    // facing right direction
                    // move

                    // first flag rotation as complete
                    // thus, we avoid correcting our rotation once
                    // we have started moving
                    // This avoids problems with round-off errors
                    // toward the end of our trip when our remaining motion
                    // vector is very short
                    *( mGardenerRotationsComplete.getElement( i ) ) = true;

                    // remember the destination that this rotation
                    // was set for (incase the destination changes mid-move)
                    lastDesiredPosition->setCoordinates( desiredPosition );
                    
                    
                    // skip this for now
                    if( false ) {
                        // make sure move doesn't take us out of bounds

                        Vector3D newPosition( position );
                        newPosition.add( &motionVector );
                        
                        if( mSoilMap.isInBounds( &newPosition ) ) {
                            // move there
                            position->add( &motionVector );
                            }
                        else {
                            // switch new position to closest in-bound position
                            Vector3D *closePoint =
                                mSoilMap.getClosestBoundaryPoint(
                                    &newPosition );
                            
                            // gardener->setDesiredPosition( closePoint );
                            
                            position->setCoordinates( closePoint );
                            
                            delete closePoint;
                            }
                        }
                    else {
                        // allowed to move anywhere
                        position->add( &motionVector );
                        }
                    }

                delete desiredAngle;
                
                gardener->setMoving( true );
                }
            else {
                gardener->setMoving( false );

                // once we reach our destination, we prepare for the next
                // rotation
                *( mGardenerRotationsComplete.getElement( i ) ) = false;
                }
            
            delete desiredPosition;

            // set emotions based on closest gardener

            // first, set default
            gardener->setEmotionDisplay( 0 );
            
            Gardener *closest = getClosestGardener( gardener );

            if( closest != NULL ) {

                double likeMetric = gardener->getLikeMetric( closest );


                // convert to -1, 1 range
                likeMetric = (likeMetric - 0.5 ) * 2;

                // adjust according to distance

                double distance = getGardenerDistance( gardener, closest );

                double maxDistance = 20;

                if( distance > maxDistance ) {
                    likeMetric = 0;
                    }
                else {
                    likeMetric *= ( maxDistance - distance ) / maxDistance;
                    }

                gardener->setEmotionDisplay( likeMetric );                
                }
            
            
            gardener->passTime( inTimeDeltaInSeconds );

            
            // check if gardener passing through portal

            if( !gardener->isDead() &&
                mPortal != NULL &&
                mPortal->isOpen() &&
                position->getDistance( &mPortalPosition ) <=
                mMinPlantingDistance ) {

                // tell gardener to stop moving
                gardener->setDesiredPosition( position );
                
                
                // match drawing z position for gardeners
                Vector3D startPosition( position );
                startPosition.mZ = mGardenerZ;
                
                mPortal->sendGardener(
                    gardener, &startPosition,
                    *( mGardenerRotations.getElement( i ) ) );

                }

            // check if closest plant is poisoned
            // in other words, gardener standing in poison
            Plant *plant = getClosestPlant( position, NULL, true, true );

            if( plant != NULL &&
                plant->getPoisonStatus() >= 1 ) {

                Vector3D *plantPosition = getPlantPosition( plant );

                if( plantPosition->getDistance( position ) <=
                    mMinPlantingDistance ) {

                    // gardener standing in poison circle
                
                    gardener->poisonFruit();
                    }

                delete plantPosition;
                }
            }
        }

    
    int numPlants = mPlants.size();

    for( i=0; i<numPlants; i++ ) {
        Plant *plant = *( mPlants.getElement( i ) );

        plant->passTime( inTimeDeltaInSeconds );

        if( mPortal != NULL &&
            plant->getPoisonStatus() >= 1 ) {
            
            // check if this poisoned plant should destroy the portal
            Vector3D *plantPosition = *( mPlantPositions.getElement( i ) );

            if( plantPosition->getDistance( &mPortalPosition )
                <=
                mMinPlantingDistance ) {

                // portal center in poison

                delete mPortal;
                mPortal = NULL;
                }
            }
        }


    

    

    // move flying objects
    double flyingObjectVelocity = 1.5 * mGardenerVelocity;
    
    i=0;

    while( i < mFlyingObjects.size() ) {
        FlyingObject *object = *( mFlyingObjects.getElement( i ) );

        
        Vector3D *motionDirection =
            getGardenerPosition( object->mDestinationGardener );

        // consider gardener height
        motionDirection->mZ = mGardenerZ;
        
        double distanceToDestination =
            motionDirection->getDistance( object->mCurrentPosition );

        
        motionDirection->subtract( object->mCurrentPosition );

        motionDirection->normalize();

        motionDirection->scale( flyingObjectVelocity * inTimeDeltaInSeconds );


        if( motionDirection->getLength() >= distanceToDestination ) {
            // this last hop takes us there

            // call destination hook
            object->reachedDestination();
            
            // get rid of this flying object
            delete object;
            mFlyingObjects.deleteElement( i );

            // don't increase i
            }
        else {
            // this move will not pass destination
            object->mCurrentPosition->add( motionDirection );
            
            // move on to next index i
            i++;
            }
        
        delete motionDirection;
        }


    // move clouds
    mCloudPosition += mCloudMotionRate * inTimeDeltaInSeconds;

    if( mPortal != NULL ) {
        mPortal->passTime( inTimeDeltaInSeconds );

		/*
		// for testing
        timeSinceLastPortalUpgrade += inTimeDeltaInSeconds;

        if( timeSinceLastPortalUpgrade >= 10 ) {
            mPortal->upgrade();
            timeSinceLastPortalUpgrade = 0;
            }
		*/
        }
    }



void World::draw() {

    // draw soil map
    Vector3D dummy( 0, 0, 0 );
    mSoilMap.draw( &dummy, 1 );

    /*
    // draw some grid lines for motion reference

    glLineWidth( 2 );    
    glColor4f( 1, 0.5, 0, 0.25 );

    double xStart, xEnd, yStart, yEnd;

    xStart = mCornerA.mX;
    yStart = mCornerA.mY;

    xEnd = mCornerB.mX;
    yEnd = mCornerB.mY;

    
    for( double x=xStart; x<=xEnd; x+=10 ) {
        glBegin( GL_LINES ); {
            glVertex2d( x, yStart );
            glVertex2d( x, yEnd );
            }
        glEnd();
        }
    for( double y=yStart; y<=yEnd; y+=10 ) {
        glBegin( GL_LINES ); {
            glVertex2d( xStart, y );
            glVertex2d( xEnd, y );
            }
        glEnd();
        }
    */




    // draw any visible plots
    glLineWidth( 1 );
    int numGardeners = mGardeners.size();

    int i;

    for( i=0; i<numGardeners; i++ ) {
        Gardener *gardener = *( mGardeners.getElement( i ) );

        if( ! gardener->isPlotHidden() && ! gardener->isDead() ) {
            Vector3D *cornerA, *cornerB;

            cornerA = *( mGardenerPlotsCornerA.getElement( i ) );
            cornerB = *( mGardenerPlotsCornerB.getElement( i ) );

        
        
            if( cornerA != NULL && cornerB != NULL ) {

                Color *color = gardener->getColor();

            
                // draw a box for plot
                glBegin( GL_LINE_LOOP ); {

                    glColor4f( color->r, color->g, color->b, 1 );
                    glVertex2d( cornerA->mX, cornerA->mY );
                    glVertex2d( cornerA->mX, cornerB->mY );
                    glVertex2d( cornerB->mX, cornerB->mY );
                    glVertex2d( cornerB->mX, cornerA->mY );
                    }
                glEnd();
            
                glBegin( GL_QUADS ); {

                    glColor4f( color->r, color->g, color->b, 0.1 );
                    glVertex2d( cornerA->mX, cornerA->mY );
                    glVertex2d( cornerA->mX, cornerB->mY );
                    glVertex2d( cornerB->mX, cornerB->mY );
                    glVertex2d( cornerB->mX, cornerA->mY );
                    }
                glEnd();

                delete color;
                }
            }
        }



    
    

    // draw plants
    int numPlants = mPlants.size();

    // set color of ground according to water status
    Color dryColor( 0, 0, 0, 0.3 );
    Color wetColor( 0, 0, 1, 0.5 );
    // factor in poison status
    Color poisonedColor( 0, 0, 0, 0.9 );  
    
    for( i=0; i<numPlants; i++ ) {
        Vector3D *position = *( mPlantPositions.getElement( i ) ); 

        // draw a circle around each plant to show unplantable area

        Plant *plant = *( mPlants.getElement( i ) );

        double waterLevel = plant->getWaterStatus();

        if( plant->isFullGrown() ) {
            // don't show water for full grown plants
            waterLevel = 0;
            }
        
        Color *blendedColor = Color::linearSum( &wetColor, &dryColor,
                                                waterLevel );

        double poisonLevel = plant->getPoisonStatus();

        Color *blendedColor2 = Color::linearSum( &poisonedColor, blendedColor,
                                                 poisonLevel );
        delete blendedColor;
        
        setGLColor( blendedColor2 );
        delete blendedColor2;
        
        drawBlurCircle( position, mMinPlantingDistance );
        }
    
    
    // draw plant parts at various z depths for proper overlapping
    // (cannot use z-buffering here because plant leaves use transparent
    // textures)
    for( double z=0.5; z>-5; z-=0.5 ) {
    
        for( i=0; i<numPlants; i++ ) {
            Plant *plant = *( mPlants.getElement( i ) );

            Vector3D *position = *( mPlantPositions.getElement( i ) ); 
        
            plant->draw( position, 8, z, z-0.499 );
            }
        }


    // finally, draw brown cicles over plants that are out of water

    for( i=0; i<numPlants; i++ ) {
        Vector3D *position = *( mPlantPositions.getElement( i ) ); 

        // draw a circle around each plant to show unplantable area

        Plant *plant = *( mPlants.getElement( i ) );

        double waterLevel = plant->getWaterStatus();

        // factor in poison level
        double poisonLevel = plant->getPoisonStatus();
        
        
        // don't draw at all for full grown plants
        // or for plants that have more than 50% water
        // or for fully-poisoned plants
        if( ! plant->isFullGrown() &&
            waterLevel < 0.5 &&
            poisonLevel < 1 ) {

            // becomes visible as plant needs water
            glColor4f( 0.5, 0.5, 0.1,
                       ( 0.5 - waterLevel ) * 2 * 0.75 );

            // gets bigger as plant needs more water
            double radius = mMinPlantingDistance * ( 0.5 - waterLevel );

            // shrinks with poisoning
            radius *= ( 1 - poisonLevel );
            
            drawBlurCircle( position, radius );
            }
        }

    // draw portal 
    
    
    // draw gardeners above everything else
    for( i=0; i<numGardeners; i++ ) {
        Gardener *gardener = *( mGardeners.getElement( i ) );

        if( ! gardener->isFrozen() && ! gardener->isDead() ) {
        
            Vector3D drawPosition( *( mGardenerPositions.getElement( i ) ) );
            drawPosition.mZ = mGardenerZ;
        
            gardener->draw( &drawPosition,
                            *( mGardenerRotations.getElement( i ) ),
                            4 );
            }
        }

    
    if( mPortal != NULL ) {
        // draw all layers of portal, except part above clouds
        mPortal->draw( &mPortalPosition, 8, 1, -4.9 ); 
        }
    

    // flying objects
    int numFlyingObjects = mFlyingObjects.size();

    for( i=0; i<numFlyingObjects; i++ ) {
        FlyingObject *flyingObject = *( mFlyingObjects.getElement( i ) );

        DrawableObject *drawObject = flyingObject->mObject;

        double scale = 1;

        if( flyingObject->isLarge() ) {
            scale = 3;
            }
        
        drawObject->draw( flyingObject->mCurrentPosition, scale );
        }

    // last, draw highlight

    if( mHighlightPosition != NULL ) {
        glColor4f( 1, 0, 0, 0.25 );

        // highlight brightens only
        //glBlendFunc( GL_SRC_ALPHA, GL_ONE );

        drawBlurCircle( mHighlightPosition, mMinPlantingDistance );

        // back to normal blend function
        //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        }


    if( Features::drawClouds ) {
        // last, cover with clouds

        // stretch twice as large as underlying world

        // put above everything
        double height = -5;

        // multiple layers to disguise tiling but still give some detail
        int numLayers = 2;
        double textureMappingRadii[2];
        textureMappingRadii[0] = 7;
        textureMappingRadii[1] = 3;

        double textureAlphas[2];
        textureAlphas[0] = 0.125;
        textureAlphas[1] = 0.25;
    
        mCloudTexture->enable();

    
        for( i=0; i<numLayers; i++ ) {
            double textureMappingRadius = textureMappingRadii[i];

            double offset = mCloudPosition * textureMappingRadius;
        
            glColor4f( 1, 1, 1, textureAlphas[i] );
            glBegin( GL_QUADS ); {

                glTexCoord2f( offset, offset );
                glVertex3d( 2 * mCornerA.mX, 2 * mCornerA.mY, height );
            
                glTexCoord2f( offset + textureMappingRadius, offset );
                glVertex3d( 2 * mCornerB.mX, 2 * mCornerA.mY, height );
            
                glTexCoord2f( offset + textureMappingRadius,
                              offset + textureMappingRadius );
                glVertex3d( 2 * mCornerB.mX, 2 * mCornerB.mY, height );
            
                glTexCoord2f( offset, offset + textureMappingRadius );
                glVertex3d( 2 * mCornerA.mX, 2 * mCornerB.mY, height );
                }
            glEnd();
            }
    
        mCloudTexture->disable();
        }


    if( mPortal != NULL ) {
        // draw portal portions above clouds
        mPortal->draw( &mPortalPosition, 8, -4.91, -30 ); 
        }
 
    }



char World::isInRectangle( Vector3D *inPosition,
                           Vector3D *inCornerA, Vector3D *inCornerB ) {

    if( // x between a and b
        ( inPosition->mX <= inCornerA->mX
          && inPosition->mX >= inCornerB->mX
          ||
          inPosition->mX >= inCornerA->mX
          && inPosition->mX <= inCornerB->mX ) 
        &&
        // y between a and b
        ( inPosition->mY <= inCornerA->mY
          && inPosition->mY >= inCornerB->mY
          ||
          inPosition->mY >= inCornerA->mY
          && inPosition->mY <= inCornerB->mY ) ) { 

        return true;
        }
    else {
        // outside
        return false;
        }
    }



char World::getRangeIntersection( double inStartA, double inEndA,
                                  double inStartB, double inEndB,
                                  double *outStart, double *outEnd ) {
    // make sure start <= end for each range

    if( inStartA > inEndA ) {
        // swap
        double temp = inStartA;
        inStartA = inEndA;
        inEndA = temp;
        }

    if( inStartB > inEndB ) {
        // swap
        double temp = inStartB;
        inStartB = inEndB;
        inEndB = temp;
        }
    
    
    // pick biggest start
    if( inStartA > inStartB ) {
        *outStart = inStartA;
        }
    else {
        *outStart = inStartB;
        }

    // pick smallest end
    if( inEndA < inEndB ) {
        *outEnd = inEndA;
        }
    else {
        *outEnd = inEndB;
        }

    
    if( *outStart < *outEnd ) {
        return true;
        }
    else {
        // no intersection
        return false;
        }
    }



void World::getRectangleIntersection( Vector3D *inFirstCornerA,
                                      Vector3D *inFirstCornerB,
                                      Vector3D *inSecondCornerA,
                                      Vector3D *inSecondCornerB,
                                      Vector3D **outCornerA,
                                      Vector3D **outCornerB ) {

    *outCornerA = NULL;
    *outCornerB = NULL;

    
    // first, find x intersection
    double firstXStart = inFirstCornerA->mX;
    double firstXEnd = inFirstCornerB->mX;

    double secondXStart = inSecondCornerA->mX;
    double secondXEnd = inSecondCornerB->mX;
    
    double xStart, xEnd;

    char xIntersects = getRangeIntersection( firstXStart, firstXEnd,
                                             secondXStart, secondXEnd,
                                             &xStart, &xEnd );

    if( ! xIntersects ) {
        return;
        }

    // else x intersects


    // get y intersection

    double firstYStart = inFirstCornerA->mY;
    double firstYEnd = inFirstCornerB->mY;

    double secondYStart = inSecondCornerA->mY;
    double secondYEnd = inSecondCornerB->mY;
    
    double yStart, yEnd;

    char yIntersects = getRangeIntersection( firstYStart, firstYEnd,
                                             secondYStart, secondYEnd,
                                             &yStart, &yEnd );

    if( ! yIntersects ) {
        return;
        }

    // both x and y intersect

    *outCornerA = new Vector3D( xStart, yStart, 0 );
    *outCornerB = new Vector3D( xEnd, yEnd, 0 );
    }



double World::getRectangleArea( Vector3D *inCornerA,
                                Vector3D *inCornerB ) {

    return
        fabs( inCornerA->mX - inCornerB->mX ) *
        fabs( inCornerA->mY - inCornerB->mY );
    }



Vector3D *World::getRectangleCenter( Vector3D *inCornerA,
                                     Vector3D *inCornerB ) {

    return new Vector3D( 0.5 * ( inCornerA->mX + inCornerB->mX ),
                         0.5 * ( inCornerA->mY + inCornerB->mY ),
                         0 );
    }



void World::angerOthers( Gardener *inGardener, Vector3D *inPosition ) {
    // first check if this is in any other gardener's plot
    int numGardeners = mGardeners.size();

    for( int i=0; i<numGardeners; i++ ) {
        Gardener *otherGardener = *( mGardeners.getElement( i ) );

        // ignore self
        if( otherGardener != inGardener ) {

            if( isInPlot( otherGardener, inPosition ) ) {
                // this plant in other gardener's plot too
                
                // eating it makes them angry
                
                otherGardener->getAngry( inGardener );
                flyEmotionIcon( otherGardener, inGardener,
                                new DislikeIcon() );
                }
            }
        }
    }



void World::flyEmotionIcon( Gardener *inSource, Gardener *inTarget,
                            DrawableObject *inIcon,
                            char inLarge ) {

    Vector3D *startPosition = getGardenerPosition( inSource );
    
    FlyingObject *object = new FlyingObject( inIcon,
                                             true,
                                             inTarget,
                                             startPosition,
                                             inLarge );

    delete startPosition;

    mFlyingObjects.push_back( object );    
    }
        


void World::setHighlightPosition( Vector3D *inPosition ) {
    if( inPosition == NULL ) {
        if( mHighlightPosition != NULL ) {
            delete mHighlightPosition;
            }
        mHighlightPosition = NULL;
        }
    else {
        if( mHighlightPosition != NULL ) {
            mHighlightPosition->setCoordinates( inPosition );
            }
        else {
            mHighlightPosition = new Vector3D( inPosition );
            }
        }            
    }



void World::augmentPortal( Vector3D *inPosition, Gardener *inAugmenter ) {
    if( mPortal != NULL ) {
        if( inPosition->getDistance( &mPortalPosition ) >
            mMinPlantingDistance ) {
		  
		    // too far away to augment existing portal
            }
        else {
            // close enough to augment
            mPortal->upgrade( inAugmenter );
            }
        }

    if( mPortal == NULL ) {
        // open a new portal and augment it
        mPortal = new Portal();
        mPortalPosition.setCoordinates( inPosition );

        mPortal->upgrade( inAugmenter );
        }            


    }



char World::isPortalOpen() {

    if( mPortal == NULL ) {
        return false;
        }
    else {
        return mPortal->isOpen();
        }
    }



char World::isPortalClosed() {

    if( mPortal == NULL ) {
        return false;
        }
    else {
        return mPortal->isClosed();
        }
    }

