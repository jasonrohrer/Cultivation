/*
 * Modification History
 *
 * 2006-July-27   Jason Rohrer
 * Created.
 */



#ifndef SOIL_MAP_INCLUDED
#define SOIL_MAP_INCLUDED



#include "minorGems/math/geometry/Vector3D.h"
#include "minorGems/graphics/openGL/SingleTextureGL.h"
#include "minorGems/graphics/Color.h"


#include "DrawableObject.h"



/**
 * A map of soil conditions.
 *
 * @author Jason Rohrer
 */
class SoilMap : public DrawableObject {

    public:

        /**
         * Constructs a map.
         *
         * @param inCornerA, inCornerB the corners of the map in world
         *   coordinates.  Destroyed by caller.
         */
        SoilMap( Vector3D *inCornerA, Vector3D *inCornerB );

        

        virtual ~SoilMap();


        
        /**
         * Gets the soil condition at a given location.
         *
         * @param inPosition the position to test the soil at.
         *   Destroyed by caller.
         * @param inNormalize set to true to normalize result to [0,1].
         *   Defaults to true.
         *
         * @return a value in [0..1], where 0 represents brown soil
         *   and 1 represents green soil.
         */
        double getSoilCondition( Vector3D *inPosition,
                                 char inNormalize = true );

        

        /**
         * Maps a soil condition to a color.
         *
         * @param inSoilCondition the condition in [0,1].
         *
         * @return a new Color mapped from inSoilCondition.
         *   Destroyed by caller.
         */
        static Color *mapSoilToColor( double inSoilCondition );



        /**
         * Gets whether a world point is in bounds of land (not in water).
         *
         * @param inPosition the poisition to check.  Destroyed by caller.
         *
         * @return true if in bounds.
         */
        char isInBounds( Vector3D *inPosition );



        /**
         * Gets the closest water boundary point to a given point.
         *
         * @param inPosition the poisition to check.  Destroyed by caller.
         *   
         *
         * @return a boundary point.  Destroyed by caller.
         *   If inPosition is on land, closest water point returned.
         *   If inPosition is on water, closest land point returned.
         */
        Vector3D *getClosestBoundaryPoint( Vector3D *inPosition );

        
        
        // implements the DrawableObject interface
        // inPosition and inScale are ignored
        void draw( Vector3D *inPosition, double inScale = 1 );


    private:

        Vector3D mCornerA, mCornerB;

        SingleTextureGL *mTexture;

        SingleTextureGL *mWaterBoundaryTexture;

        SingleTextureGL *mGritTexture;
        
        Image *mWaterBoundaryImage;
        double *mWaterBoundaryImageAlpha;
        int mWaterBoundaryImageNumPixels;
        int mWaterBoundaryImageSize;
        
        double mMinLandscapeValue, mMaxLandscapeValue;


        
        /**
         * Maps a point in world space to pixel coordinates in the
         * water boundary image.
         *
         * @param inPoint the point in the world.  Destroyed by caller.
         * @param outX, outY pointers to where pixel coordinates should be
         *   returned.
         */
        void mapPointToBoundaryPixel( Vector3D *inPoint,
                                      int *outX, int *outY );


        
    };



#endif
