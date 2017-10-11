/*
 * Modification History
 *
 * 2006-July-7   Jason Rohrer
 * Created.
 */



#ifndef GARDENER_AI_INCLUDED
#define GARDENER_AI_INCLUDED


#include "Gardener.h"
#include "World.h"




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


        double mSecondsSinceLastGift;
        double mSecondsSinceLastRevenge;


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
         * Expands our plot in the world so that inPlant is contained.
         *
         * @param inPlant the plant to contain.  Managed by world.
         */
        void expandOurPlotToContainPlant( Plant *inPlant );

        
        
    };



#endif
