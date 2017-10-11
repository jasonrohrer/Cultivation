/*
 * Modification History
 *
 * 2006-August-13   Jason Rohrer
 * Created.
 *
 * 2006-December-18   Jason Rohrer
 * Added seed width.
 */



#ifndef PLANT_GENETICS_INCLUDED
#define PLANT_GENETICS_INCLUDED


#include "Genetics.h"

#include "minorGems/graphics/Color.h"



enum PlantGeneLocator{
    idealSoilType = 0,
    seedWidth,
    fruitNutrition,
    parentFruitNutrition,
    fruitLobeRate,
    fruitLobeDepth,
    jointCount,
    leavesPerJoint,
    innerLeafColor,
    outerLeafGreen,
    leafWalkerDeltaAngle,
    leafWalkerDeltaDeltaAngle,
    leafWalkerSpawnIntervalFactor,
    leafWalkerStartingSpawnInterval,
    leafWalkerSpawnAngle,
    leafWalkerSpawnDouble,
    flowerPetalCount,
    flowerCenterRadius,
    flowerPetalAngle,
    flowerPetalPointARadius,
    flowerPetalPointAAngle,
    flowerPetalPointBRadius,
    flowerPetalPointBAngle,
    flowerCenterColor,
    flowerPetalPointAColor,
    flowerPetalPointBColor,
    flowerPetalPointCColor,
    timeBetweenFlowers };
    


/**
 * Genetics for a plant.
 *
 * @author Jason Rohrer
 */
class PlantGenetics : public Genetics {

    public:


        
        /**
         * Constructs random genetics
         */
        PlantGenetics();


        
        /**
         * Constructs genetics by crossing two parent genetics.
         *
         * @param inParentA, inParentB the parent genetics.
         *   Destroyed by caller.
         */         
        PlantGenetics( PlantGenetics *inParentA,
                       PlantGenetics *inParentB );



        /**
         * Constructs genetics by copying another.
         *
         * @param inGeneticsToCopy the genetics to copy.
         *   Destroyed by caller.
         */         
        PlantGenetics( PlantGenetics *inGeneticsToCopy );


        
        /**
         * Maps a given gene to a parameter value.
         *
         * @param inLocator a gene locator.
         *
         * @return a parameter value for the specified gene.
         */
        double getParameter( PlantGeneLocator inLocator );


        
        /**
         * Maps a given gene to a color.
         *
         * @param inLocator a gene locator.
         *
         * @return a new color.  Destroyed by caller.
         */
        Color *getColor( PlantGeneLocator inLocator );
        

        
    };



#endif
