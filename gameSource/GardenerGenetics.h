/*
 * Modification History
 *
 * 2006-August-11   Jason Rohrer
 * Created.
 *
 * 2006-October-30   Jason Rohrer
 * Added a following threshold gene.
 *
 * 2006-November-8   Jason Rohrer
 * Added a copy constructor.
 */



#ifndef GARDENER_GENETICS_INCLUDED
#define GARDENER_GENETICS_INCLUDED


#include "Genetics.h"

#include "minorGems/graphics/Color.h"



enum GardenerGeneLocator{
    // behavior modifiers
    task_none_modifier = 0,
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
    task_rest_modifier,
    // threshold of friendship, in range [0,1]
    friendshipThreshold,
    // always at or above frienship threshold 
    matingThreshold,
    // always at or above mating threshold 
    followingThreshold,
    storedFruitsBeforeMating,
    desiredPlantCount,
    overlapTolerance,
    pregnancyProgressRate,
    bodyColor,
    boneStartSpacing,
    boneSpacingDelta,
    boneCurvature,
    boneCurveFrequency,
    scaleCornerAColor,
    scaleCornerBColor,
    scaleCornerCColor,
    scaleCornerDColor,
    scaleCornerAColorDelta,
    scaleCornerBColorDelta,
    scaleCornerCColorDelta,
    scaleCornerDColorDelta,
    scaleStepSize,
    scaleSize,
    scaleWaveStrength,
    scaleWaveFrequency,
    eyeSize,
    pupilSize,
    eyeSeparation,
    melodyA,
    melodyB,
    melodyC,
    melodyD,
    melodyE,
    melodyF,
    songArrangement,
    chanceOfReverseNote };
    


/**
 * Genetics for a gardener.
 *
 * @author Jason Rohrer
 */
class GardenerGenetics : public Genetics {

    public:


        
        /**
         * Constructs random genetics
         */
        GardenerGenetics();


        
        /**
         * Constructs genetics by crossing two parent genetics.
         *
         * @param inParentA, inParentB the parent genetics.
         *   Destroyed by caller.
         */         
        GardenerGenetics( GardenerGenetics *inParentA,
                          GardenerGenetics *inParentB );



        /**
         * Maps a given gene to a parameter value.
         *
         * @param inLocator a gene locator.
         *
         * @return a parameter value for the specified gene.
         */
        double getParameter( GardenerGeneLocator inLocator );



        /**
         * Maps a given gene to a color.
         *
         * @param inLocator a gene locator.
         *
         * @return a new color.  Destroyed by caller.
         */
        Color *getColor( GardenerGeneLocator inLocator );

        
        
    protected:

        
        /**
         * Constructs a copy of another genetics.
         *
         * @param inGenetics the genetics to copy.  Destroyed by caller.
         */
        GardenerGenetics( GardenerGenetics *inGenetics );

        
    };



#endif
