/*
 * Modification History
 *
 * 2006-August-29   Jason Rohrer
 * Created.
 */



#ifndef PLANT_FLOWER_INCLUDED
#define PLANT_FLOWER_INCLUDED



#include "minorGems/math/geometry/Vector3D.h"
#include "minorGems/math/geometry/Angle3D.h"
#include "minorGems/graphics/openGL/SingleTextureGL.h"
#include "minorGems/graphics/Color.h"

#include "PlantGenetics.h"



/**
 * A drawable plant flower.
 *
 * @author Jason Rohrer
 */
class PlantFlower {

    public:

        
        /**
         * Constructs a flower.
         *
         * @param inGenetics the genetics for the shape of this flower.
         *   Destroyed by caller.
         */
        PlantFlower( PlantGenetics *inGenetics );

        

        ~PlantFlower();


        
        /**
         * Passes time for this flower.
         *
         * @param inTimeDeltaInSeconds the amount of time that has passed.
         */
        void passTime( double inTimeDeltaInSeconds );
        

        
        /**
         * Draws this flower.
         *
         * @param inPosition the position of this flower's base tip.
         *   Destroyed by caller.
         * @param inRotation the rotation of this flower, around its base
         *   tip.  Destroyed by caller.
         * @param inScale the scale of this flower.
         * @param inStage ([0,1] bud growth, [1,2] bud bloom, [2,3] hold,
         *   [3,4] fade)
         */
        void draw( Vector3D *inPosition, Angle3D *inRotation, double inScale,
                   double inStage );


    private:
        PlantGenetics mGenetics;
        
        SingleTextureGL *mCenterTexture;
        SingleTextureGL *mPetalTexture;

        
    };



#endif
