/*
 * Modification History
 *
 * 2006-July-27   Jason Rohrer
 * Created.
 */



#ifndef SEEDS_INCLUDED
#define SEEDS_INCLUDED



#include "minorGems/math/geometry/Vector3D.h"
#include "minorGems/graphics/openGL/SingleTextureGL.h"

#include "minorGems/graphics/Color.h"


#include "Storable.h"
#include "PlantGenetics.h"

#include "PlantLeaf.h"



/**
 * A inexhaustable "pile" of plant seeds.
 *
 * @author Jason Rohrer
 */
class Seeds : public Storable {

    public:

        // Constructs random seeds
        Seeds();
        

        
        /**
         * Constructs seeds by crossing seed characteristics from two
         * parents.
         *
         * Parent fruit color is always taken from parent A.
         *
         * @param inParentA, inParentB the parent seeds.  Destroyed by caller.
         */
        Seeds( Seeds *inParentA, Seeds *inParentB );


        
        /**
         * Copies seeds.
         *
         * @param inSeedsToCopy the seeds to copy.
         *   Destroyed by caller.
         */
        Seeds( Seeds *inSeedsToCopy );


        ~Seeds();

        

        PlantGenetics mGenetics;        
        
        double mIdealSoilType;

        Color mFruitNutrition;

        Color mParentFruitNutition;


        
        /**
         * Gets a sample leaf for this plant.
         *
         * @return a leaf.  Destroyed when this seed destroyed.
         */
        PlantLeaf *getSampleLeaf();

        
        
        // implements the DrawableObject interface
        void draw( Vector3D *inPosition, double inScale = 1 );


        
        // implements the Storable interface
        StorableType getType() {
            return seedsType;
            }


        
    protected:

        
        // map genetics to our parameters
        void getParametersFromGenetics();


        PlantLeaf *mSampleLeaf;

        
        SingleTextureGL *mSeedTexture;
    };



#endif
