/*
 * Modification History
 *
 * 2006-July-23   Jason Rohrer
 * Created.
 *
 * 2006-October-27   Jason Rohrer
 * Added support for poisoned fruit.
 */



#ifndef FRUIT_INCLUDED
#define FRUIT_INCLUDED



#include "minorGems/math/geometry/Vector3D.h"
#include "minorGems/graphics/Color.h"

#include "minorGems/graphics/openGL/SingleTextureGL.h"


#include "Storable.h"
#include "Seeds.h"



/**
 * A fruit.
 *
 * @author Jason Rohrer
 */
class Fruit : public Storable {

    public:


        
        /**
         * Constructs a fruit.
         *
         * @param inParentGenetics the genetics of this fruit's parent.
         *   Determines fruit shape and color.
         *   Destroyed by caller.
         * @param inSeeds the seeds this fruit should contain.
         *   Destroyed by caller.
         * @param inRipenRate the rate of fruit ripening.
         */
        Fruit( PlantGenetics *inParentGenetics, Seeds *inSeeds,
               double inRipenRate );


        
        ~Fruit();

        

        /**
         * Gets the nutrition provided by this fruit if eaten.
         *
         * @return nutrition stored as a color.  Destroyed by caller.
         */
        Color *getNutrition();


        
        /**
         * Gets whether this fruit is completely rotten.
         *
         * @return true if rotten.
         */
        char isRotten();


        
        /**
         * Gets whether this fruit is ripe.
         *
         * @return true if ripe.
         */
        char isRipe();



        /**
         * Get whether fruit is poisoned.
         *
         * @return true if poisoned.
         */
        char isPoisoned();

        

        /**
         * Poisons this fruit.
         */
        void poison();

        
        
        /**
         * Gets a copy of the seeds contained in this fruit.
         *
         * @return this fruit's seeds.  Must be destroyed by caller.
         */
        Seeds *getSeeds();

        
        
        /**
         * Passes time for this fruit.
         *
         * @param inTimeDeltaInSeconds the amount of time that has passed.
         */
        void passTime( double inTimeDeltaInSeconds );


        
        // implements the DrawableObject interface
        // (draws at a default rotation)
        void draw( Vector3D *inPosition, double inScale = 1 );

        // specifies a rotation
        void draw( Vector3D *inPosition, Angle3D *inRotation,
                   double inScale = 1 );

        

        // implements the Storable interface
        StorableType getType() {
            return fruitType;
            }


    private:
        PlantGenetics mParentGenetics;
        
        // 0-1 growing
        // 1-2 ripening
        // 2-4 ripe (stable)
        // 4-5 browning
        // 5-6 rotting        
        double mStatus;

        
        // true if poisoned
        char mPoisoned;

        
        Color mNutrition;
        Seeds mSeeds;

        double mRipenRate;
        
        SingleTextureGL *mShapeTexture;

        
        // the last rotation we were drawn at when a rotation was specified
        // use this as the default rotation when no rotation is specified
        Angle3D mLastRotation;
        
        
    };



#endif
