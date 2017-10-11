/*
 * Modification History
 *
 * 2006-July-7   Jason Rohrer
 * Created.
 *
 * 2006-September-12   Jason Rohrer
 * Fixed so that we don't try to mate with a pregnant partner.
 * Fixed the machine-gun mating bug.
 */



#include "GardenerAI.h"
#include "gameFunctions.h"


#include <math.h>
#include <float.h>


#include "minorGems/util/random/StdRandomSource.h"



extern StdRandomSource globalRandomSource;



GardenerAI::GardenerAI( Gardener *inGardener, World *inWorld )
    : mGardener( inGardener ), mWorld( inWorld ),
      mNextPlantingLocation( NULL ),
      mSecondsSinceLastGift( 0 ),
      mSecondsSinceLastRevenge( 0 ) {

    }



GardenerAI::~GardenerAI() {
    if( mNextPlantingLocation != NULL ) {
        delete mNextPlantingLocation;
        }
    }



void GardenerAI::passTime( double inTimeDeltaInSeconds ) {

    // before doing anything else, check if we are still following parent
    Gardener *parent = mGardener->getParentToFollow();

    if( parent != NULL &&
        ! parent->isDead() ) {

        // follow it if we get too far away
        Vector3D *destination;
        
        if( mWorld->getGardenerDistance( mGardener, parent )
            >
            10 ) {

            // move closer
            destination = mWorld->getGardenerPosition( parent );
            }
        else {
            // stay where we are
            destination = mWorld->getGardenerPosition( mGardener );
            }
        
        mGardener->setDesiredPosition( destination );

        delete destination;

        return;
        }

    
    
    mSecondsSinceLastGift += inTimeDeltaInSeconds;
    mSecondsSinceLastRevenge += inTimeDeltaInSeconds;
    
    // first check if hungry
    int lowIndex = -1;
    for( int i=0; i<3; i++ ) {
        if( mGardener->getNutrientLevel(i) == 0 ) {
            lowIndex = i;
            }
        }
    
    if( lowIndex != -1 ) {
        // low in at least one nutrient

        // try to find a fruit high in that nutrient
        int index = mGardener->getIndexOfFruitHighInNutrient( lowIndex );

        if( index != -1 ) {
            mGardener->setSelectedObjectIndex( index );
            // eat selected fruit
            mGardener->eat();
            }
        }
    

    // next deal with creating plot
    
    Vector3D *plotCenter = mWorld->getPlotCenter( mGardener );

    if( plotCenter == NULL ) {

        // need to pick a new plot

        // walk toward water until we hit it, or until we get
        // too close to other gardeners
        Vector3D *waterPoint = mWorld->getClosestWater( mGardener );

        double minPleasantDistance = 10;
        char tooCloseToOtherGardeners = false;
        
        
        Gardener *closestGardener = mWorld->getClosestGardener( mGardener );

        if( closestGardener != NULL ) {
            Vector3D *ourPosition = mWorld->getGardenerPosition( mGardener );
            Vector3D *closestGardenerPosition =
                mWorld->getGardenerPosition( closestGardener );

            double distance =
                ourPosition->getDistance( closestGardenerPosition );

            if( distance < minPleasantDistance ) {
                tooCloseToOtherGardeners = true;
                }

            delete ourPosition;
            delete closestGardenerPosition;
            }

        
        if( ! tooCloseToOtherGardeners &&
            ! mWorld->isInWater( mGardener ) ) {

            mGardener->setDesiredPosition( waterPoint );

            }
        else {
            // we just hit the water, or we came too close to other
            // gardeners

            // create our plot

            Vector3D *position = mWorld->getGardenerPosition( mGardener );

            Vector3D a( position );

            // vector pointing away from water center
            Vector3D rayFromWaterPoint( position );
            rayFromWaterPoint.subtract( waterPoint );

            rayFromWaterPoint.normalize();

            // plot diagonal of 20 world units
            rayFromWaterPoint.scale( 20 );
            
            // add this ray to our position
            position->add( &rayFromWaterPoint );

            Vector3D b( position );
            
            delete position;

            // thus, we pick a plot bordering the water that has a diagonal
            // length roughly equal to the water's radius

            // this can be a "skinny" rectangle, though, so widen it if
            // needed

            double diffX = fabs( a.mX - b.mX );
            double diffY = fabs( a.mY - b.mY );
            
            if( diffX < diffY ) {
                // taller than wide

                double increase = diffY - diffX;
                if( a.mX < b.mX ) {
                    b.mX += increase;
                    }
                else {
                    a.mX += increase;
                    }
                }
            if( diffY < diffX ) {
                // wider than tall

                double increase = diffX - diffY;
                if( a.mY < b.mY ) {
                    b.mY += increase;
                    }
                else {
                    a.mY += increase;
                    }
                }

            
            mWorld->setGardenerPlot( mGardener, &a, &b );
            }
        delete waterPoint;

        return;
        }


    // we have a plot

    // check that there are enough plants in it

    int targetPlantCount =
        (int)( mGardener->mGenetics.getParameter( desiredPlantCount ) );

               
    SimpleVector<Plant*> *plants = mWorld->getPlotPlants( mGardener );

    int numPlants = plants->size();

    SimpleVector<Seeds*> *seedsVector = mGardener->getAllSeeds();

    int numSeeds = seedsVector->size();
    delete seedsVector;
    
    
    if( numSeeds > 0 && numPlants < targetPlantCount ) {

        // plant more

        if( mNextPlantingLocation == NULL ) {
            // haven't picked a spot yet

            char foundPlantable = false;
            int numTries = 0;
            int maxNumTries = 10;

            while( !foundPlantable && numTries < maxNumTries ) {
                Vector3D *cornerA, *cornerB;
            
                mWorld->getGardenerPlot( mGardener, &cornerA, &cornerB );
                
                double x = globalRandomSource.getRandomBoundedDouble(
                    cornerA->mX, cornerB->mX );
                
                double y = globalRandomSource.getRandomBoundedDouble(
                    cornerA->mY, cornerB->mY );
                
            
                mNextPlantingLocation = new Vector3D( x, y, 0 );
                
                delete cornerA;
                delete cornerB;

                if( mWorld->canPlant( mNextPlantingLocation ) ) {
                    foundPlantable = true;
                    }
                else {
                    // try again
                    delete mNextPlantingLocation;
                    mNextPlantingLocation = NULL;
                    }
                numTries++;
                }
            }
                
                    
            

        if( mNextPlantingLocation != NULL ) {

            Vector3D *gardenerPosition =
                mWorld->getGardenerPosition( mGardener );
            
            if( ! gardenerPosition->equals( mNextPlantingLocation ) ) {
                // move to next plant location
                mGardener->setDesiredPosition( mNextPlantingLocation );
                }
            else {
                // at next location:

                // make sure we can still plant
                // else pick another location at next time step
                if( mWorld->canPlant( mNextPlantingLocation ) ) {
                    // plant here

                    double soilCondition =
                        mWorld->getSoilCondition( mNextPlantingLocation );


                    SimpleVector<Seeds*> *seedsVector =
                        mGardener->getAllSeeds();

                    // find best for this soil
                    Seeds *best = NULL;
                    double minSoilDistance = 2;
                    
                    for( int i=0; i<seedsVector->size(); i++ ) {
                        Seeds *seeds = *( seedsVector->getElement( i ) );

                        double distance =
                            fabs( seeds->mIdealSoilType - soilCondition );

                        if( distance < minSoilDistance ) {
                            minSoilDistance = distance;
                            best = seeds;
                            }
                        }

                    delete seedsVector;

                
                    if( best != NULL ) {
                        mWorld->addPlant( mGardener,
                                          new Plant( soilCondition, best ),
                                          mNextPlantingLocation );

                        mGardener->removeSeeds( best );
                        }
                    }

                delete mNextPlantingLocation;
                mNextPlantingLocation = NULL;
                }
            
            delete gardenerPosition;
            }
        else {
            // tried to pick a plantable location, but failed

            // expand plot

            Vector3D *a, *b;
            
            mWorld->getGardenerPlot( mGardener, &a, &b );
            
            // compute a vector stretching from b to a
            Vector3D b_to_a( a );

            b_to_a.subtract( b );

            // expand plot by 10% in each direction

            b_to_a.scale( 0.10 );

            // push a away from b
            a->add( &b_to_a );


            // also push b away from a
            // opposite direction
            b_to_a.scale( -1 );

            b->add( &b_to_a );


            mWorld->setGardenerPlot( mGardener, a, b );
            
            
            delete a;
            delete b;
            }
        
        delete plants;
        delete plotCenter;
        
        return;
        }


    // else we have enough plants (or no seeds left)

    
        
        
    // if any are ripe, harvest them
    Plant *ripePlant = NULL;

    int i;
    
    for( i=0; i<numPlants && ripePlant == NULL; i++ ) {
        Plant *thisPlant = *( plants->getElement( i ) );

        if( thisPlant->isRipe() ) {
            ripePlant = thisPlant;
            }
        }

    if( ripePlant != NULL ) {

        
        // move toward it
        
        Vector3D *plantPosition = mWorld->getPlantPosition( ripePlant );
        
        Vector3D *gardenerPosition =
            mWorld->getGardenerPosition( mGardener );
        
        if( ! gardenerPosition->equals( plantPosition ) ) {
            // move to plant
            mGardener->setDesiredPosition( plantPosition );
            }
        else {
            // already at plant
            
            // harvest it
            
            mWorld->harvestPlant( mGardener, ripePlant );
            }
        
        delete gardenerPosition;
        delete plantPosition;
        
        delete plants;
        
        delete plotCenter;
        
        return;
        }
    
    
    // else no ripe plants
    
    // water plants


    Plant *driestPlant = NULL;
    
    // ignore plants that have at least 1/4 water
    double driestWaterStatus = 0.25;
    
        
    for( int i=0; i<numPlants; i++ ) {
        Plant *thisPlant = *( plants->getElement( i ) );
        
        double waterStatus = thisPlant->getWaterStatus(); 
        if( waterStatus < driestWaterStatus ) {
            driestPlant = thisPlant;
            driestWaterStatus = waterStatus;
            }
        }


    
    if( mGardener->getCarryingWater() ) {
        // already carrying water

        // move to the driest plant

        if( driestPlant != NULL ) {

            // found driest

            // walk to it

            Vector3D *plantPosition = mWorld->getPlantPosition( driestPlant );

            Vector3D *gardenerPosition =
                mWorld->getGardenerPosition( mGardener );
            
            if( ! gardenerPosition->equals( plantPosition ) ) {
                // move to plant
                mGardener->setDesiredPosition( plantPosition );
                }
            else {
                // already at plant
                
                // dump water
                mGardener->setCarryingWater( false );

                mWorld->dumpWater( mGardener );
                }
            
            delete gardenerPosition;
            delete plantPosition;            
            }
        else {
            // else no dry plant found
            // wait and do nothing

            // head to plot center and wait
            mGardener->setDesiredPosition( plotCenter );
            }
        }
    else if( driestPlant != NULL ) {
        // there is a dry plant, and we're not carrying water
        // fetch water

        if( ! mWorld->isInWater( mGardener ) ) {

            Vector3D *waterPoint = mWorld->getClosestWater( mGardener );
            mGardener->setDesiredPosition( waterPoint );

            delete waterPoint;
            }
        else {
            // grab water
            mGardener->setCarryingWater( true );
            }
        }
    else {
        // no dry plant, and not carrying water

        char tryingToMate = false;


        // if not pregnant
        // and not target of another pregancy
        // hand have enough fruit to feel secure
        if( ! mGardener->isPregnant() &&
            ! mWorld->isTargetOfPregnancy( mGardener ) &&
            mGardener->getStoredFruitCount() >=
            mGardener->mGenetics.getParameter( storedFruitsBeforeMating ) ) {

            // we are not pregnant already
            
            // we have enough fruit stored
            // consider mating

            double ourThreshold =
                mGardener->mGenetics.getParameter( matingThreshold );

            
            Gardener *mostLiked = mGardener->getMostLikedGardener();

            if( mostLiked != NULL ) {

                double mostLikedMatingThreshold =
                    mostLiked->mGenetics.getParameter( matingThreshold );
                
                
                if( mGardener->getLikeMetric( mostLiked ) >=
                    ourThreshold
                    &&
                    mostLiked->getLikeMetric( mGardener ) >=
                    mostLikedMatingThreshold
                    &&
                    ! mostLiked->isPregnant()
                    &&
                    ! mWorld->isTargetOfPregnancy( mostLiked ) ) {

                    // we like them enough to mate
                    // and
                    // they like us enough to mate
                    // and
                    // they are not already pregnant
                    // and
                    // they are not already target of another pregnancy
                    
                    tryingToMate = true;


                    Vector3D *ourPosition =
                        mWorld->getGardenerPosition( mGardener );
                    Vector3D *otherPosition =
                        mWorld->getGardenerPosition( mostLiked );

                    double distance =
                        ourPosition->getDistance( otherPosition );

                    if( distance < getMaxDistanceForTransactions() ) {

                        mWorld->mateGardeners( mGardener, mostLiked );
                        }
                    else {
                        // move toward friend
                        mGardener->setDesiredPosition( otherPosition );
                        }

                    delete ourPosition;
                    delete otherPosition;                    
                    }
                }
            }


        
        // check if we should give fruit to a neighbor
        char tryingToGiveFruit = false;

        // wait five seconds between gifts to give them
        // time to arrive before we reasses the situation
        if( !tryingToMate && mSecondsSinceLastGift > 5 ) {
        
            Gardener *mostLiked = mGardener->getMostLikedGardener();

            if( mostLiked != NULL ) {

                // only give if we have 2+ more fruits than them
                // (to avoid back and forth giving when there is an
                //  odd number of fruits between the two of us)
                if( mGardener->getStoredFruitCount() >
                    mostLiked->getStoredFruitCount() + 1 ) {

                    // we have fruit to spare compared to our best friend
                    tryingToGiveFruit = true;
                

                    Vector3D *ourPosition =
                        mWorld->getGardenerPosition( mGardener );
                    Vector3D *otherPosition =
                        mWorld->getGardenerPosition( mostLiked );

                    double distance =
                        ourPosition->getDistance( otherPosition );

                    if( distance < getMaxDistanceForTransactions() ) {
                        // close enough to give

                        // find out which nutrient they are low in
                        int lowIndex = -1;
                        double lowValue = 2;
                        for( int i=0; i<3; i++ ) {
                            double value = mostLiked->getNutrientLevel(i); 
                            if( value < lowValue  ) {
                                lowIndex = i;
                                lowValue = value;
                                }
                            }
                        
                        // try to find a fruit high in that nutrient
                        int index = mGardener->getIndexOfFruitHighInNutrient(
                            lowIndex );

                        // we will always get a valid index here, because
                        // we have checked that we have stored fruit above
                        mGardener->setSelectedObjectIndex( index );
                        
                        mWorld->giveFruit(
                            mGardener, mostLiked,
                            // don't save seeds, but get any fruit, even
                            // if fruit not selected
                            mGardener->getSelectedFruit( false, true ) );

                        // reset timer
                        mSecondsSinceLastGift = 0;

                        // every time we give a gift,
                        // our friendliness toward this gardener is depleted
                        // a bit
                        
                        // Actually, don't do this for now, since for mating
                        // purposes, we want to retain our friendliness
                        
                        // mGardener->getAngry( mostLiked );
                        }
                    else {
                        // move toward friend
                        mGardener->setDesiredPosition( otherPosition );
                        }

                    delete ourPosition;
                    delete otherPosition;
                    }
                }
            }


        char gettingRevenge = false;
        
        if( !tryingToMate && !tryingToGiveFruit ) {

            // consider getting revenge
            if( mSecondsSinceLastRevenge > 5 ) {

                Gardener *leastLiked = mGardener->getLeastLikedGardener();

                if( leastLiked != NULL ) {
                    
                    Plant *plantToTake =
                        getClosestPlantInGardenerPlot( leastLiked );
                    
                    if( plantToTake != NULL ) {
                        expandOurPlotToContainPlant( plantToTake );
                        
                        gettingRevenge = true;

                        // reset timer
                        mSecondsSinceLastRevenge = 0;

                        // every time we take revenge, our anger
                        // toward this gardener lessens a bit
                        mGardener->getFriendly( leastLiked );
                        }
                    else {
                        // none to take (our plots overlap perfectly)
                        
                        // try looking for a plant to poison

                        Plant *plantToPoison =
                            mWorld->getTendedPlant( leastLiked );

                        if( plantToPoison != NULL ) {

                            // found candidate
                            gettingRevenge = true;
                            
                            // walk to it

                            Vector3D *plantPosition =
                                mWorld->getPlantPosition( plantToPoison );

                            Vector3D *gardenerPosition =
                                mWorld->getGardenerPosition( mGardener );
            
                            if( ! gardenerPosition->equals( plantPosition ) ) {
                                // move to plant
                                mGardener->setDesiredPosition( plantPosition );
                                }
                            else {
                                // already at plant
                                mWorld->dumpPoison( mGardener );

                                // reset timer
                                mSecondsSinceLastRevenge = 0;
                                
                                // every time we take revenge, our anger
                                // toward this gardener lessens a bit
                                mGardener->getFriendly( leastLiked );
                                }
            
                            delete gardenerPosition;
                            delete plantPosition;
                            }
                        }
                    }
                }
            }

        


        if( !tryingToMate && !tryingToGiveFruit && !gettingRevenge ) {
            // head to plot center and wait
            mGardener->setDesiredPosition( plotCenter );            
            }
        
        }


    
    if( plotCenter != NULL ) {
        delete plotCenter;
        }

    delete plants;
    
    }



Plant *GardenerAI::getClosestPlantInGardenerPlot( Gardener *inGardener ) {
    SimpleVector<Plant *> *ourPlants = mWorld->getPlotPlants( mGardener );
    SimpleVector<Plant *> *otherPlants = mWorld->getPlotPlants( inGardener );

    if( otherPlants == NULL ) {

        if( ourPlants != NULL ) {
            delete ourPlants;
            }
        
        return NULL;
        }
    
    // first, filter otherPlants to remove overlaps with ourPlants

    int i=0;
    while( i < otherPlants->size() ) {

        Plant *plant = *( otherPlants->getElement( i ) );

        if( ourPlants != NULL &&
            ourPlants->getElementIndex( plant ) != -1 ) {
            // overlap
            otherPlants->deleteElement( i );
            }
        else {
            // no overlap
            i++;
            }        
        }

    if( ourPlants != NULL ) {
        delete ourPlants;
        }


    Plant *returnPlant = NULL;


    int numPlants = otherPlants->size();
    
    if( numPlants > 0 ) {

        // look for closest to us

        Vector3D *ourPostion = mWorld->getGardenerPosition( mGardener );

        double closestDistance = DBL_MAX;
        
        for( i=0; i<numPlants; i++ ) {
            Plant *plant = *( otherPlants->getElement( i ) );
            
            Vector3D *plantPosition = mWorld->getPlantPosition( plant );

            double distance = plantPosition->getDistance( ourPostion );

            delete plantPosition;

            if( distance < closestDistance ) {
                closestDistance = distance;
                returnPlant = plant;
                }
            }

        delete ourPostion;
        }

    
    delete otherPlants;

    return returnPlant;
    }



void GardenerAI::expandOurPlotToContainPlant( Plant *inPlant ) {

    Vector3D *cornerA, *cornerB;

    mWorld->getGardenerPlot( mGardener, &cornerA, &cornerB );

    Vector3D *plantPosition = mWorld->getPlantPosition( inPlant );

    Vector3D *closestCornerX;
    double xDistance;
    Vector3D *closestCornerY;
    double yDistance;
    
    double cornerAXDistance = plantPosition->mX - cornerA->mX;
    double cornerBXDistance = plantPosition->mX - cornerB->mX;

    double cornerAYDistance = plantPosition->mY - cornerA->mY;
    double cornerBYDistance = plantPosition->mY - cornerB->mY;

    if( fabs( cornerAXDistance ) < fabs( cornerBXDistance ) ) {
        closestCornerX = cornerA;
        xDistance = cornerAXDistance;
        }
    else {
        closestCornerX = cornerB;
        xDistance = cornerBXDistance;
        }

    
    if( fabs( cornerAYDistance ) < fabs( cornerBYDistance ) ) {
        closestCornerY = cornerA;
        yDistance = cornerAYDistance;
        }
    else {
        closestCornerY = cornerB;
        yDistance = cornerBYDistance;
        }

    // make distances a little bit larger (1 unit) to contain plant
    double extra = 1;
    if( xDistance < 0 ) {
        xDistance -= extra;
        }
    if( xDistance > 0 ) {
        xDistance += extra;
        }
    if( yDistance < 0 ) {
        yDistance -= extra;
        }
    if( yDistance > 0 ) {
        yDistance += extra;
        }
    
        
    
    // only want to change corners in way that makes plot bigger
    // thus, to contain inPlant, we may need to expand in only one direction
    // (in other words, x or y)

    // we can use the plot area to detect plot contraction
    
    double oldArea =
        fabs( cornerA->mX - cornerB->mX ) *
        fabs( cornerA->mY - cornerB->mY );
        
    
    closestCornerX->mX += xDistance;

    double newArea =
        fabs( cornerA->mX - cornerB->mX ) *
        fabs( cornerA->mY - cornerB->mY );

    if( newArea < oldArea ) {
        // contraction

        // reverse it
        closestCornerX->mX -= xDistance;
        }


    oldArea =
        fabs( cornerA->mX - cornerB->mX ) *
        fabs( cornerA->mY - cornerB->mY );
        
    
    closestCornerY->mY += yDistance;
    
    newArea =
        fabs( cornerA->mX - cornerB->mX ) *
        fabs( cornerA->mY - cornerB->mY );

    if( newArea < oldArea ) {
        // contraction

        // reverse it
        closestCornerY->mY -= yDistance;
        }

    mWorld->setGardenerPlot( mGardener, cornerA, cornerB );

    
    
    delete plantPosition;

    delete cornerA;
    delete cornerB;    
    }
