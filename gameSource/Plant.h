/*
 * Modification History
 *
 * 2006-July-2   Jason Rohrer
 * Created.
 */



#ifndef PLANT_INCLUDED
#define PLANT_INCLUDED



#include "minorGems/math/geometry/Vector3D.h"
#include "minorGems/graphics/openGL/SingleTextureGL.h"

#include "minorGems/util/SimpleVector.h"


#include "DrawableObject.h"


#include "PlantLeaf.h"
#include "PlantFlower.h"
#include "Fruit.h"
#include "Seeds.h"



/**
 * A plant.
 *
 * @author Jason Rohrer
 */
class Plant {

    public:

        /**
         * Constructs a plant.
         *
         * @param inSoilType the soil type where the plant is growing.
         * @param inSeeds the seeds to grow this plant from.
         *   Destroyed by caller.
         */
        Plant( double inSoilType, Seeds *inSeeds );


        virtual ~Plant();

        

        /**
         * Waters this plant.
         */
        void water();


        
        /**
         * Poisons this plant.
         */
        void poison();

        

        /**
         * Gets the poisoned level of this plant.
         *
         * @return a value in [0,1], where 1 is fully poisoned.
         */
        double getPoisonStatus();
        

        
        /**
         * Gets whether this plant has ripe fruit that can be harvested.
         */
        char isRipe();

        

        /**
         * Turns on highlighting of whichever fruit will be returned
         * by a call to harvest.
         *
         * Only lasts until some fruit is drawn with a highlight.
         * After that, highlight is switched back off.
         *
         * Thus, to draw a continuous highlight, the highlight should
         * be turned on again before each drawing of this plant.
         *
         * @param inHighlight true to turn on, false to turn off.
         */
        void highlightFirstRipeFruit( char inHighlight );


        
        /**
         * Picks a ripe fruit from this plant.
         *
         * @param outFruitPosition pointer to vector where fruit position
         *   should be returned.  Destroyed by caller.
         *
         * @return the ripe fruit, or NULL if no ripe fruit present.
         *   Destroyed by caller.
         */
        Fruit *harvest( Vector3D *outFruitPosition );


        /**
         * Returns a pointer to the next ripe fruit.
         *
         * @return the next ripe fruit, or NULL if none ripe.
         *   Managed by this class.
         */
        Fruit *peekAtRipeFruit();


        
        /**
         * Gets the water status of this plant.
         *
         * @return a value in [0,1], where 0 is dry and 1 is wet.
         */
        double getWaterStatus();


        
        /**
         * Gets whether plant is done growing.
         *
         * @return true if done growing.
         */
        char isFullGrown();

        
        
        /**
         * Passes time for this plant.
         *
         * @param inTimeDeltaInSeconds the amount of time that has passed.
         */
        void passTime( double inTimeDeltaInSeconds );


        
        /**
         * Draws this plant.
         *
         * @param inPosition the center position of this plant.
         * @param inScale the scale of this plant, defaults to 1.
         * @param inMaxZ the maximum z location of plant parts to draw.
         * @param inMinZ the minimum z location of plant parts to draw.
         *  Parts outside the inMaxZ/inMinZ window are skipped.
         */
        void draw( Vector3D *inPosition, double inScale = 1,
                   double inMaxZ = 0, double inMinZ = -100 );


        
        // the seeds that this plant was grown from
        Seeds mSeeds;

        

    private:
        PlantLeaf mLeaf;
        PlantFlower mFlower;

        // which leaf terminus each flower is on
        SimpleVector<long> mFlowerTerminusIndicies;
        SimpleVector<double> mFlowerStages;
        SimpleVector<Angle3D *> mFlowerAngles;

        char mLeafTerminiiSet;
        SimpleVector<Vector3D *> mLeafTerminii;
        
        double mTimeSinceLastFlower;

        SimpleVector<Fruit *> mFruit;
        // fruit attatched to leaf terminus after flower fades
        SimpleVector<long> mFruitTerminusIndices;
        SimpleVector<Angle3D *> mFruitAngles;


        char mHighlightRipeFruit;
        
        
        double mWaterStatus;
        char mJustWatered;

        // become poisoned gradually
        double mPoisonStatus;
        char mPoisoned;

        
        double mGrowth;

        double mTimeSinceLastFruit;
        double mTimeBetweenFruits;
        
        double mWaterConsumptionRate;
        double mGrowthRate;
        
        double mCurrentSoilType;

        double mStartZAngle;

        SingleTextureGL *mJointCapTexture;
        
        
    };



#endif
