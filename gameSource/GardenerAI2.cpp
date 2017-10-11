/*
 * Modification History
 *
 * 2006-September-28   Jason Rohrer
 * Created.
 *
 * 2006-October-4   Jason Rohrer
 * Fixed rapid-fire gift bug.
 * Changed to stop pursuit of desired position after completing a task.
 *
 * 2006-October-10   Jason Rohrer
 * Added a limit on maximum plot size.
 * Updated task_abandonPlot to work with new getLeastLikedGardener behavior.
 *
 * 2006-November-2   Jason Rohrer
 * Increased following distance for non-parent.
 */



#include "GardenerAI2.h"
#include "gameFunctions.h"


#include <math.h>
#include <float.h>


#include "minorGems/util/random/StdRandomSource.h"





extern StdRandomSource globalRandomSource;

extern double globalMaxPlotDimension;



/**
 * Gets the maximum dimension of a plot.
 */
double getMaximumDimension( Vector3D *inCornerA, Vector3D *inCornerB ) {
    double xDimension = fabs( inCornerB->mX - inCornerA->mX );
    double yDimension = fabs( inCornerB->mY - inCornerA->mY );

    if( xDimension > yDimension ) {
        return xDimension;
        }
    else {
        return yDimension;
        }    
    }



GardenerAI::GardenerAI( Gardener *inGardener, World *inWorld )
    : mGardener( inGardener ), mWorld( inWorld ),
      mNextPlantingLocation( NULL ),
      mNextPlotLocation( NULL ),
      mSecondsSinceLastGift( 0 ),
      mCurrentRestTime( 0 ),
      // three seconds
      mMaxRestTime( 3 ),
      mCurrentTask( task_none ) {

    }



GardenerAI::~GardenerAI() {
    if( mNextPlantingLocation != NULL ) {
        delete mNextPlantingLocation;
        mNextPlantingLocation = NULL;
        }
    if( mNextPlotLocation != NULL ) {
        delete mNextPlotLocation;
        mNextPlotLocation = NULL;
        }
    }


// an array of gene locators for behavior modifiers
// makes it easy to loop over gene locators below

GardenerGeneLocator taskModifierGeneLocators[ NUM_GARDENER_TASKS ] =
    { task_none_modifier,
      task_water_modifier,
      task_harvest_modifier,
      task_eat_modifier,
      task_createPlot_modifier,
      task_abandonPlot_modifier,
      task_plant_modifier,
      task_expandPlot_modifier,
      task_capturePlant_modifier,
      task_poisonPlant_modifier,
      task_giveGift_modifier,
      task_mate_modifier,
      task_rest_modifier };



void GardenerAI::passTime( double inTimeDeltaInSeconds ) {
    
    // before doing anything else, check if we are still following parent
    Gardener *parent = mGardener->getParentToFollow();

    // or following another leader
    Gardener *leader = parent;

    double followDistance = 10;
    
    if( parent == NULL ||
        parent->isDead() ) {

        leader = mGardener->getLeader();
        
        // a bit longer if leader not parent
        followDistance = 15;
        }

    
    if( leader != NULL &&
        ! leader->isDead() ) {

        // follow it if we get too far away
        Vector3D *destination;
        
        if( mWorld->getGardenerDistance( mGardener, leader )
            >
            followDistance ) {

            // move closer
            destination = mWorld->getGardenerPosition( leader );
            }
        else {
            // stay where we are
            destination = mWorld->getGardenerPosition( mGardener );
            }
        
        mGardener->setDesiredPosition( destination );

        delete destination;

        return;
        }



    // if we got here, we're not following a parent
    mSecondsSinceLastGift += inTimeDeltaInSeconds;


    SimpleVector<Plant*> *plants = mWorld->getPlotPlants( mGardener );

    int numPlants = plants->size();
    

    // if we don't have a plot, we must always switch to that task
    Vector3D *plotCenter = mWorld->getPlotCenter( mGardener );

    if( plotCenter == NULL ) {
        mCurrentTask = task_createPlot;
        }
    delete plotCenter;

    
    if( mCurrentTask == task_none ) {
        // need to pick a task

        // we first compute weights for each task

        // The general idea is to compute a [0,1] weight for each.
        // In some cases, this is binary, simply indicating whether the task
        // is sensible or not (example:  no sense in eating if we aren't
        // hungry).
        // In other cases, this weight is continuous and represents urgency.

        // However, these [0,1] weights are deterministic and the same
        // for all individuals.

        // Individual differences stem from the application of genetic
        // modifiers that adjust these [0,1] weights.
        // After the application of modifiers, weights may exceed 1.
        

        
        double weights[ NUM_GARDENER_TASKS ];
        GardenerTask taskLabels[ NUM_GARDENER_TASKS ];

        // used to normalize weights
        double weightSum = 0;

        int t;
        for( t=0; t<NUM_GARDENER_TASKS; t++ ) {
            // hack:  enum type is actually a series of integers
            taskLabels[t] = (GardenerTask) t;

            switch( taskLabels[t] ) {
                case task_none: {
                    // never intentionally pick no task
                    weights[t] = 0;
                    break;
                    }
                case task_water: {
                    Plant *driestPlant = getDriestPlant();
                    if( driestPlant == NULL ) {
                        weights[t] = 0;
                        }
                    else {
                        double driestWaterStatus =
                            driestPlant->getWaterStatus();

                        // approaches 1 as driest plant approaches no water
                        weights[t] = ( 0.25 - driestWaterStatus ) / 0.25;
                        }
                    
                    break;
                    }
                case task_harvest: {
                    
                    Plant *ripePlant = NULL;

                    int i;
                    
                    for( i=0; i<numPlants && ripePlant == NULL; i++ ) {
                        Plant *thisPlant = *( plants->getElement( i ) );

                        if( thisPlant->isRipe() ) {
                            ripePlant = thisPlant;
                            }
                        }
                    
                    if( ripePlant != NULL ) {
                        weights[t] = 1;
                        }
                    else {
                        weights[t] = 0;
                        }
                    break;
                    }
                case task_eat: {
                    weights[t] = 0;
                    
                    // first check if hungry
                    double lowestLevel = 0.25;
                    int lowIndex = -1;
                    for( int i=0; i<3; i++ ) {
                        double level = mGardener->getNutrientLevel(i);
                        if( level < lowestLevel ) {
                            lowIndex = i;
                            lowestLevel = level; 
                            }
                        }
                    
                    if( lowIndex != -1 ) {
                        // make sure we have fruit to eat
                        int index =
                            mGardener->getIndexOfFruitHighInNutrient(
                                lowIndex );

                        if( index != -1 ) {
                            // grows closer to 1 as level approaches zero
                            weights[t] = (0.25 - lowestLevel) / 0.25;
                            }
                        }
                    break;
                    }
                case task_createPlot: {
                    // we only create a plot if we don't have one above
                    // we never select this task probabalistically
                    weights[t] = 0;
                    break;
                    }
                case task_abandonPlot: {

                    weights[t] = 0;
                    
                    double ourPlotArea =
                        mWorld->getPlotArea( mGardener );

                    if( ourPlotArea > 0 ) {
                        // sum overlaps with other gardners
                        double overlapAreaSum = 0;
                        
                        int numGardeners;
                        Gardener **gardeners =
                            mWorld->getAllGardeners( &numGardeners );

                        for( int i=0; i<numGardeners; i++ ) {
                            
                            Gardener *g = gardeners[i];
                            
                            // ignore self
                            if( g != mGardener ) {

                                // only consider gardeners for whom we are
                                // the least liked gardener.
                                // We're making them angry, should we abandon
                                // our plot and seek land elsewhere?

                                // Compare their like metric for us to their
                                // metric for the gardener that they like
                                // the least.
                                // Thus, we can tell if we are on their
                                // "least liked" list.
                                
                                double theirLikeMetricForUs =
                                    g->getLikeMetric( mGardener );

                                Gardener *theirLeastLiked = 
                                    g->getLeastLikedGardener();

                                double theirLowestLikeMetric =
                                    g->getLikeMetric( theirLeastLiked );

                                
                                if( theirLikeMetricForUs <=
                                    theirLowestLikeMetric ) {

                                    // they hate us

                                    // add in their overlap
                                    overlapAreaSum +=
                                        mWorld->getPlotIntersectionArea(
                                            mGardener,
                                            g );
                                    }
                                }
                            }
                        delete [] gardeners;
                        
                        double overlapFraction = overlapAreaSum / ourPlotArea;
                        if( overlapFraction > 1 ) {
                            overlapFraction = 1;
                            }
                    
                        double threshold = mGardener->mGenetics.getParameter(
                            overlapTolerance );

                        if( overlapFraction >= threshold ) {
                            // our sum of overlap with gardeners that hate us
                            // is greater than our threshold

                            
                            // weight depends on how much overlap we have
                            // above our threshold
                            double range = 1 - threshold;

                            if( range > 0 ) {
                                weights[t] =
                                    ( overlapFraction - threshold ) / range;
                                }
                            else {
                                weights[t] = 1;
                                }
                            }
                        }
                    break;
                    }
                case task_plant: {
                    int targetPlantCount =
                        (int)( mGardener->mGenetics.getParameter(
                            desiredPlantCount ) );

                    int numPlants = plants->size();
                    
                    SimpleVector<Seeds*> *seedsVector =
                        mGardener->getAllSeeds();

                    int numSeeds = seedsVector->size();
                    delete seedsVector;
    
    
                    if( numSeeds > 0 && numPlants < targetPlantCount ) {
                        // weight shrinks as we near our target count
                        weights[t] =
                            (double)(targetPlantCount - numPlants) /
                            targetPlantCount;
                        }
                    else {
                        weights[t] = 0;
                        }
                    break;
                    }
                case task_expandPlot: {
                    // we only try to expand the plot if our planting
                    // task fails, so we never select this task
                    // probabalistically
                    weights[t] = 0;
                    break;
                    }
                case task_capturePlant: {
                    weights[t] = 0;

                    Gardener *leastLiked =
                        mGardener->getLeastLikedGardener();

                    if( leastLiked != NULL ) {
                        
                        Plant *plantToTake =
                            getClosestPlantInGardenerPlot( leastLiked );
                        
                        if( plantToTake != NULL ) {
                            
                            double likeMetric =
                                mGardener->getLikeMetric( leastLiked );

                            // weight depending on our like metric
                            weights[t] = 1 - likeMetric;
                            }
                        }
                    break;
                    }
                case task_poisonPlant: {
                    weights[t] = 0;

                    Gardener *leastLiked =
                        mGardener->getLeastLikedGardener();
                    
                    if( leastLiked != NULL ) {
                        
                        Plant *plantToPoison =
                            mWorld->getTendedPlant( leastLiked );
                        
                        if( plantToPoison != NULL ) {
                            
                            double likeMetric =
                                mGardener->getLikeMetric( leastLiked );
                            
                            // don't consider poisoning unless
                            // we really don't like them
                            if( likeMetric < 0.25 ) {
                                // weight depending on our like metric
                                weights[t] = (0.25 - likeMetric) / 0.25;
                                }
                            }
                        }
                    break;
                    }
                case task_giveGift: {
                    weights[t] = 0;

                    if( mSecondsSinceLastGift > 5 ) {
                        Gardener *mostLiked =
                            mGardener->getMostLikedGardener();

                        if( mostLiked != NULL ) {

                            // only give if we have 2+ more fruits than them
                            // (to avoid back and forth giving when there is an
                            //  odd number of fruits between the two of us)
                            if( mGardener->getStoredFruitCount() >
                                mostLiked->getStoredFruitCount() + 1 ) {

                                double likeMetric =
                                    mGardener->getLikeMetric( mostLiked );

                                // weight based on like metric
                                weights[t] = likeMetric;
                                }
                            }
                        }
                    break;
                    }
                case task_mate: {
                    
                    weights[t] = 0;

                    // if not pregnant
                    // and not target of another pregancy
                    // hand have enough fruit to feel secure
                    if( ! mGardener->isPregnant() &&
                        ! mWorld->isTargetOfPregnancy( mGardener ) &&
                        mGardener->getStoredFruitCount() >=
                        mGardener->mGenetics.getParameter(
                            storedFruitsBeforeMating ) ) {

                        // we are not pregnant already
                        
                        // we have enough fruit stored
                        // consider mating

                        double ourThreshold =
                            mGardener->mGenetics.getParameter(
                                matingThreshold );

            
                        Gardener *mostLiked =
                            mGardener->getMostLikedGardener();

                        if( mostLiked != NULL ) {

                            double mostLikedMatingThreshold =
                                mostLiked->mGenetics.getParameter(
                                    matingThreshold );

                            double likeMetric =
                                mGardener->getLikeMetric( mostLiked );
                
                            if( likeMetric >= ourThreshold
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
                                // they are not already target of
                                // another pregnancy

                                
                                // weight depends on like metric

                                // how much over our mating threshold?
                                double range = 1 - ourThreshold;

                                if( range > 0 ) {
                                
                                    weights[t] =
                                        ( likeMetric - ourThreshold ) / range;
                                    }
                                else {
                                    weights[t] = 1;
                                    }
                                }
                            }
                        }
                    break;
                    }                    
                case task_rest: {
                    // never pick resting probabalistically

                    // instead, select it as default when all other tasks have
                    // no weight, below
                    weights[t] = 0;
                    break;
                    }                    
                }

            // apply genetic modifiers (skip task_none)
            if( t > 0 ) {
                weights[t] *=
                    mGardener->mGenetics.getParameter(
                        taskModifierGeneLocators[t] );
                }
            
            weightSum += weights[t];
            }

        // now we have a weight for each task

        // normalize
        
        if( weightSum == 0 ) {
            // avoid divide by zero
            weightSum = 1;
            }
        
        for( t=0; t<NUM_GARDENER_TASKS; t++ ) {
            weights[t] /= weightSum;
            }

        // now weights are a probability distribution

        // pick a random variable to sample from the probability distribution
        double randomVariable = globalRandomSource.getRandomDouble();

        // to sample from a probability distribution using a [0,1] random
        // variable, walk through weights, summing them, until weight
        // sum passes random variable value
        
        char hit = false;
        double weightAccumulation = 0;
        
        for( t=0; t<NUM_GARDENER_TASKS && !hit; t++ ) {

            weightAccumulation += weights[t];

            if( weightAccumulation >= randomVariable ) {
                // found the task hit by our random variable
                hit = true;

                mCurrentTask = taskLabels[t];
                }
            }

        if( mCurrentTask == task_none ) {
            // this means that all task weights were zero, or that our
            // random variable was zero

            // default to resting
            mCurrentTask = task_rest;
            }
        }


    // we have a task
    // execute it
    
    switch( mCurrentTask ) {
        case task_none: {
            printf( "Error:  task_none selected by GardenerAI2.\n" );
            break;
            }
        case task_water: {
            Plant *driestPlant = getDriestPlant();

            if( driestPlant != NULL ) {
                if( mGardener->getCarryingWater() ) {
                    // already carrying water

                    // move to the driest plant

                    
                    Vector3D *plantPosition =
                        mWorld->getPlantPosition( driestPlant );

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

                        // finished
                        mCurrentTask = task_none;
                        }
            
                    delete gardenerPosition;
                    delete plantPosition;            
                    }
                else {
                    // there is a dry plant, and we're not carrying water
                    // fetch water

                    if( ! mWorld->isInWater( mGardener ) ) {
                        
                        Vector3D *waterPoint =
                            mWorld->getClosestWater( mGardener );
                        
                        mGardener->setDesiredPosition( waterPoint );
                        
                        delete waterPoint;
                        }
                    else {
                        // grab water
                        mGardener->setCarryingWater( true );
                        }
                    }
                }
            else {
                // no driest plant

                // make sure we're not still carrying water after
                // a plant finished growing
                mGardener->setCarryingWater( false );

                // finished
                mCurrentTask = task_none;
                }

            break;
            }
        case task_harvest: {
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
        
                Vector3D *plantPosition =
                    mWorld->getPlantPosition( ripePlant );
        
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

                    // finished
                    mCurrentTask = task_none;
                    }
                
                
                delete gardenerPosition;
                delete plantPosition;        
                }
            else {
                // nothing to harvest

                // finished
                mCurrentTask = task_none;
                }    

            break;
            }
        case task_eat: {
            // first find what nutrient we are lowest in
            double lowestLevel = 0.25;
            int lowIndex = -1;
            for( int i=0; i<3; i++ ) {
                double level = mGardener->getNutrientLevel(i); 
                if( level < lowestLevel ) {
                    lowIndex = i;
                    lowestLevel = level;
                    }
                }
    
            if( lowIndex != -1 ) {
                // low in at least one nutrient

                // try to find a fruit high in that nutrient
                int index =
                    mGardener->getIndexOfFruitHighInNutrient( lowIndex );

                if( index != -1 ) {
                    mGardener->setSelectedObjectIndex( index );
                    // eat selected fruit
                    mGardener->eat();
                    }
                }

            // always finish, since this is an instant action
            mCurrentTask = task_none;
            
            break;
            }
        case task_createPlot: {
            // need to pick a new plot

            // first pick a random land point in the world
            if( mNextPlotLocation == NULL ) {
                mNextPlotLocation = mWorld->getRandomLandPoint();
                }
            
            Vector3D *gardenerPosition =
                mWorld->getGardenerPosition( mGardener );
            
            if( ! gardenerPosition->equals( mNextPlotLocation ) ) {
                // move to next plot location
                mGardener->setDesiredPosition( mNextPlotLocation );
                }
            else {
                // there already


                // once there, create a plot

                Vector3D a( mNextPlotLocation );

                Vector3D b( mNextPlotLocation );

                // 20 x 20

                a.mX -= 10;
                a.mY -= 10;

                b.mX += 10;
                b.mY += 10;
                

                delete mNextPlotLocation;
                mNextPlotLocation = NULL;

                // don't check if plot meets our criteria

                // we will do that in the behavior selection (since
                // we might decide to abandon our plot)

                mWorld->setGardenerPlot( mGardener, &a, &b );

                // finished
                mCurrentTask = task_none;
                }
            delete gardenerPosition;
            break;
            }
        case task_abandonPlot: {
            mWorld->setGardenerPlot( mGardener, NULL, NULL );
            break;
            }           
        case task_plant: {
            if( mNextPlantingLocation == NULL ) {
                // haven't picked a spot yet

                char foundPlantable = false;
                int numTries = 0;
                int maxNumTries = 100;

                Vector3D *cornerA, *cornerB;
            
                mWorld->getGardenerPlot( mGardener, &cornerA, &cornerB );

                
                while( !foundPlantable && numTries < maxNumTries ) {
                    
                
                    double x = globalRandomSource.getRandomBoundedDouble(
                        cornerA->mX, cornerB->mX );
                
                    double y = globalRandomSource.getRandomBoundedDouble(
                        cornerA->mY, cornerB->mY );
                
            
                    mNextPlantingLocation = new Vector3D( x, y, 0 );
                
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
                
                delete cornerA;
                delete cornerB;
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

                        // finished
                        mCurrentTask = task_none;
                        }

                    delete mNextPlantingLocation;
                    mNextPlantingLocation = NULL;
                    }
            
                delete gardenerPosition;
                }
            else {
                // tried to pick a plantable location, but failed

                // switch to expand task
                mCurrentTask = task_expandPlot;
                }
            break;
            }
        case task_expandPlot: {
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


            // make sure it's not too big

            double maxDimension = getMaximumDimension( a, b );

            if( maxDimension <= globalMaxPlotDimension ) {
            
                mWorld->setGardenerPlot( mGardener, a, b );

                // finished
                mCurrentTask = task_none;
                }
            else {
                // can't expand, though we need to

                // abandon plot
                mCurrentTask = task_abandonPlot;
                }
            
            delete a;
            delete b;

            
            break;
            }
        case task_capturePlant: {
            Gardener *leastLiked = mGardener->getLeastLikedGardener();

            if( leastLiked != NULL ) {
                    
                Plant *plantToTake =
                    getClosestPlantInGardenerPlot( leastLiked );
                
                if( plantToTake != NULL ) {
                    char expanded = expandOurPlotToContainPlant( plantToTake );

                    if( expanded ) {
                        // every time we successfully take revenge, our anger
                        // toward this gardener lessens a bit
                        mGardener->getFriendly( leastLiked );
                        }
                    // else our plot cannot get any bigger
                    
                    }
                }

            // always finish (this task executes in one timestep)
            mCurrentTask = task_none;            
            break;
            }
        case task_poisonPlant: {
            Gardener *leastLiked = mGardener->getLeastLikedGardener();

            if( leastLiked != NULL ) {
            
                Plant *plantToPoison =
                    mWorld->getTendedPlant( leastLiked );

                if( plantToPoison != NULL ) {

                    // found candidate

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
                                
                        // every time we take revenge, our anger
                        // toward this gardener lessens a bit
                        mGardener->getFriendly( leastLiked );

                        // finished
                        mCurrentTask = task_none;
                        }
            
                    delete gardenerPosition;
                    delete plantPosition;
                    }
                else {
                    // nothing to poison
                    mCurrentTask = task_none;
                    }
                }
            else {
                // nothing to poison
                mCurrentTask = task_none;
                }
            break;
            }
        case task_giveGift: {
            Gardener *mostLiked = mGardener->getMostLikedGardener();

            if( mostLiked != NULL ) {

                // only give if we have 2+ more fruits than them
                // (to avoid back and forth giving when there is an
                //  odd number of fruits between the two of us)
                if( mGardener->getStoredFruitCount() >
                    mostLiked->getStoredFruitCount() + 1 ) {

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

                        // reset our gift timer so that we don't send
                        // rapid-fire gifts before our first one arrives

                        // the reason we need a gift timer, and not
                        // a revenge timer or any other kind of timer,
                        // is because our decision to give a gift is
                        // based on the state of the other gardener, and not
                        // just on our own state.  Their state changes
                        // in response to our gift only after they receive
                        // it, so we must wait until they receive it
                        // before we decide to give again.

                        // for revenge actions, and other actions (like eating,
                        // watering, etc.), our decision is based only
                        // on our internal state, which changes as soon
                        // as we execute the action.
                        
                        mSecondsSinceLastGift = 0;
                        
                        
                        // every time we give a gift,
                        // our friendliness toward this gardener is depleted
                        // a bit
                        
                        // Actually, don't do this for now, since for mating
                        // purposes, we want to retain our friendliness
                        
                        // mGardener->getAngry( mostLiked );

                        // finished
                        mCurrentTask = task_none;
                        }
                    else {
                        // move toward friend
                        mGardener->setDesiredPosition( otherPosition );
                        }

                    delete ourPosition;
                    delete otherPosition;
                    }
                else {
                    // they have enough fruit now, cancel
                    mCurrentTask = task_none;
                    }
                }
            else {
                // we no longer have a most liked
                mCurrentTask = task_none;
                }
            
            break;
            }
        case task_mate: {
            // if not pregnant
            // and not target of another pregancy
            // don't check fruit supply (we've decided to mate, so let's do it)
            if( ! mGardener->isPregnant() &&
                ! mWorld->isTargetOfPregnancy( mGardener ) ) {

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
                    

                        Vector3D *ourPosition =
                            mWorld->getGardenerPosition( mGardener );
                        Vector3D *otherPosition =
                            mWorld->getGardenerPosition( mostLiked );

                        double distance =
                            ourPosition->getDistance( otherPosition );

                        if( distance < getMaxDistanceForTransactions() ) {

                            mWorld->mateGardeners( mGardener, mostLiked );

                            // finished
                            mCurrentTask = task_none;
                            }
                        else {
                            // move toward friend
                            mGardener->setDesiredPosition( otherPosition );
                            }

                        delete ourPosition;
                        delete otherPosition;                    
                        }
                    else {
                        // something changed since we decided to mate
                        mCurrentTask = task_none;
                        }
                    }
                else {
                    // we no longer have a most liked
                    mCurrentTask = task_none;
                    }
                }
            else {
                // we are already pregnant or target of another pregnancy
                mCurrentTask = task_none;
                }
            break;
            }
        case task_rest: {
            mCurrentRestTime += inTimeDeltaInSeconds;
            if( mCurrentRestTime >= mMaxRestTime ) {
                // finished
                mCurrentRestTime = 0;
                mCurrentTask = task_none;
                }
            break;
            }
        }


    
    if( mCurrentTask == task_none ) {

        // just finished a task

        // Make sure we stop moving and don't continue chasing down
        // the desired position we set as part of our last task.

        // For example, when giving a gift, we set our desired position
        // at the gardener, but we can stop moving closer to the gardener
        // once we have gotten close enough and given the gift.
        
        Vector3D *gardenerPosition =
            mWorld->getGardenerPosition( mGardener );

        mGardener->setDesiredPosition( gardenerPosition );

        delete gardenerPosition;        
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



Plant *GardenerAI::getDriestPlant() {

    SimpleVector<Plant*> *plants = mWorld->getPlotPlants( mGardener );

    int numPlants = plants->size();

    
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

    delete plants;

    return driestPlant;
    }



char GardenerAI::expandOurPlotToContainPlant( Plant *inPlant ) {

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

    char returnValue;

    if( getMaximumDimension( cornerA, cornerB ) <= globalMaxPlotDimension ) {
    
        mWorld->setGardenerPlot( mGardener, cornerA, cornerB );

        returnValue = true;
        }
    else {
        // new plot exceeds limit
        returnValue = false;
        }

    
    
    delete plantPosition;

    delete cornerA;
    delete cornerB;    

    return returnValue;
    }
