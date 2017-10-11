/*
 * Modification History
 *
 * 2006-August-20   Jason Rohrer
 * Created.
 */



#ifndef PLANT_LEAF_INCLUDED
#define PLANT_LEAF_INCLUDED



#include "minorGems/math/geometry/Vector3D.h"
#include "minorGems/math/geometry/Angle3D.h"
#include "minorGems/graphics/openGL/SingleTextureGL.h"
#include "minorGems/graphics/Color.h"

#include "PlantGenetics.h"
#include "DrawableObject.h"



/**
 * A drawable plant leaf.
 *
 * @author Jason Rohrer
 */
class PlantLeaf : public DrawableObject {

    public:

        
        /**
         * Constructs a leaf.
         *
         * @param inGenetics the genetics for the shape of this leaf.
         *   Destroyed by caller.
         */
        PlantLeaf( PlantGenetics *inGenetics );

        

        virtual ~PlantLeaf();


        
        double getLeafAreaFraction();

        
        
        /**
         * Draws this leaf.
         *
         * @param inPosition the position of this leaf's base tip.
         *   Destroyed by caller.
         * @param inRotation the rotation of this leaf, around its base
         *   tip.  Destroyed by caller.
         * @param inScale the scale of this leaf.
         * @param outLeafWalkerTerminus pointer to vector where
         *   walker terminal location should be returned.  Set to NULL
         *   to ignore terminal location.  Defaults to NULL.
         */
        void draw( Vector3D *inPosition, Angle3D *inRotation,
                   double inScale, Vector3D *outLeafWalkerTerminus = NULL );


        
        // implements DrawableObject interface
        virtual void draw( Vector3D *inPosition, double inScale = 1 );
        
            
            
    private:

        SingleTextureGL *mTexture;

        double mLeafAreaFraction;

        Vector3D mLeafWalkerTerminus;
        
    };



#endif
