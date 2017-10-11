/*
 * Modification History
 *
 * 2006-September-28   Jason Rohrer
 * Created.
 *
 * 2006-October-4   Jason Rohrer
 * Fixed rapid-fire gift bug.
 *
 * 2006-October-10   Jason Rohrer
 * Added a limit on maximum plot size.
 */



#ifndef GARDENER_AI_2_INCLUDED
#define GARDENER_AI_2_INCLUDED


#include "Gardener.h"
#include "World.h"


#define NUM_GARDENER_TASKS   13

enum GardenerTask{
    task_none = 0,  // need to pick a new task
    task_water,
    task_harvest,
    task_eat,
    task_createPlot,
    task_abandonPlot,
    task_plant,
    task_expandPlot,
    task_capturePlant,
    task_poisonPlant,
    task_giveGift,
    task_mate,
    task_rest };



/**
 * Artificial intelligence control for a gardener.
 *
 * @author Jason Rohrer
 */
class GardenerAI {

    public:



        /**
         * Constructs an AI.
         *
         * All params must be destroyed by caller after this AI is destroyed.
         *
         * @param inGardener the gardener to control
         * @param inWorld the world the gardener is in.
         */
        GardenerAI( Gardener *inGardener, World *inWorld );



        ~GardenerAI();


        
        /**
         * Passes time for this AI.
         *
         * @param inTimeDeltaInSeconds the amount of time that has passed.
         */
        void passTime( double inTimeDeltaInSeconds );



    private:

        Gardener *mGardener;
        World *mWorld;


        Vector3D *mNextPlantingLocation;
        Vector3D *mNextPlotLocation;
        

        double mSecondsSinceLastGift;

        double mCurrentRestTime;
        double mMaxRestTime;
        

        GardenerTask mCurrentTask;
        

        /**
         * Gets the closest plant in inGardener's plot that is not
         * on our gardener's plot.
         *
         * Both parameter and return value are managed and destroyed by
         * mWorld.
         *
         * @param inGardener the gardener to look at.
         *
         * @return the plant in inGardener's plot, or NULL if no
         *   such plant exists.
         */
        Plant *getClosestPlantInGardenerPlot( Gardener *inGardener );



        /**
         * Gets the driest plant in inGardener's plot.
         *
         * @return the plant in inGardener's plot, or NULL if no
         *   such plant exists.  Managed by mWorld
         */
        Plant *getDriestPlant();
        
        

        /**
         * Expands our plot in the world so that inPlant is contained.
         *
         * @param inPlant the plant to contain.  Managed by world.
         *
         * @return true if expansion successful, or false if expanding
         *   our plot to contain inPlant would make our plot too big.
         */
        char expandOurPlotToContainPlant( Plant *inPlant );

        
        
    };



#endif
