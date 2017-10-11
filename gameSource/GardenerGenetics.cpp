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



#include "GardenerGenetics.h"


extern int globalNumNotesPerMelody;
extern double globalMusicSongLength;
extern double globalShortestNoteLength;



int gardenerGeneLengths[48] = { // behavior modifiers
                                1, 1, 1, 1,
                                1, 1, 1, 1,
                                1, 1, 1, 1,
                                1,
                                // behavior thresholds
                                1, 1, 1, 1, 1, 1, 1,
                                // body color
                                3,
                                // bones
                                1, 1, 1, 1,
                                // scale corner colors
                                3, 3, 3, 3,
                                // scale corner color deltas
                                3, 3, 3, 3,
                                // scale spacing and size
                                1, 1, 1, 1,
                                // eyes
                                1, 1, 1,
                                // melodies
                                globalNumNotesPerMelody,
                                globalNumNotesPerMelody,
                                globalNumNotesPerMelody,
                                globalNumNotesPerMelody,
                                globalNumNotesPerMelody,
                                globalNumNotesPerMelody,
                                // arrangement
                                // how many sections?
                                // compute how many notes would be in song
                                // if all short notes were used
                                // then compute how many melody sections
                                // would be needed to produce that many notes
                                (int)( 
                                  ( globalMusicSongLength /
                                       globalShortestNoteLength ) /
                                  globalNumNotesPerMelody
                                  // add extra sections just to be safe
                                  + 2 * globalNumNotesPerMelody ),
                                // chance of reverse note
                                1 };



GardenerGenetics::GardenerGenetics()
    : Genetics( 48, gardenerGeneLengths ) {

    }



GardenerGenetics::GardenerGenetics( GardenerGenetics *inGenetics )
    : Genetics( inGenetics ) {

    }



GardenerGenetics::GardenerGenetics( GardenerGenetics *inParentA,
                                    GardenerGenetics *inParentB )
    : Genetics( inParentA, inParentB ) {

    }



double GardenerGenetics::getParameter( GardenerGeneLocator inLocator ) {

    double geneValue = mGenes[ inLocator ][0];
    
    switch( inLocator ) {
        // special cases where we map gene into parameter space
        case matingThreshold: {
            double friendship = getParameter( friendshipThreshold );

            // gene determines how much more we add to frienship threshold
            // to get mating threshold
            double matingIncrement = (1.0 - friendship) * geneValue;

            return friendship + matingIncrement;
            }
        case followingThreshold: {
            double mating = getParameter( matingThreshold );

            // gene determines how much more we add to mating threshold
            // to get following threshold
            double followingIncrement = (1.0 - mating) * geneValue;

            return mating + followingIncrement;
            }
        case storedFruitsBeforeMating:
            // min of 0, max of 6
            return (int)( mapValueToRange( geneValue, 0, 6.9 ) );
        case desiredPlantCount:
            // min of 1 plant, max of 6
            return (int)( mapValueToRange( geneValue, 1, 6.9 ) );
        case overlapTolerance:
            // 25% of gardeners can tolerate an overlap fraction of 100%
            return mapValueToRange( geneValue, 0, 1.33333 );
        case pregnancyProgressRate:
            return mapValueToRange( geneValue, 0.01, 0.02 );
        case boneStartSpacing:
            return mapValueToRange( geneValue, 3, 10 );
        case boneSpacingDelta:
            return mapValueToRange( geneValue, -2, 2 );
        case boneCurvature:
            return mapValueToRange( geneValue, 3, 10 );
        case boneCurveFrequency:
            return mapValueToRange( geneValue, 0.75, 1.75 );
        case scaleStepSize:
            return mapValueToRange( geneValue, 0.14, 0.2 );
        case scaleSize:
            return mapValueToRange( geneValue, 0.15, 0.2 );
        case scaleWaveStrength:
            return mapValueToRange( geneValue, 0, 0.2 );
        case scaleWaveFrequency:
            return mapValueToRange( geneValue, 0, 10 );
        case eyeSize:
            return mapValueToRange( geneValue, 0.5, 1.5 );
        case pupilSize:
            return mapValueToRange( geneValue, 0.5, 1.5 );
        case eyeSeparation:
            return mapValueToRange( geneValue, 1, 1.5 );
        case chanceOfReverseNote:
            return mapValueToRange( geneValue, 0, 0.1 );
        case task_none_modifier:
            // never select task_none
            return 0;
        // all other modifiers mapped to range [0.5, 2]
        case task_water_modifier:
        case task_harvest_modifier:
        case task_eat_modifier:
        case task_createPlot_modifier:
        case task_plant_modifier:
        case task_expandPlot_modifier:
        case task_capturePlant_modifier:
        case task_poisonPlant_modifier:
        case task_giveGift_modifier:
        case task_mate_modifier:
        case task_rest_modifier:
            return mapValueToRange( geneValue, 0.5, 2 );
            
        // default to returning the gene itself
        default:
            return geneValue;
        }    
    }



Color *GardenerGenetics::getColor( GardenerGeneLocator inLocator ) {
    double *geneValue = mGenes[ inLocator ];

    int geneLength = mGeneLengths[ inLocator ];

    if( geneLength < 3 ) {
        return new Color( geneValue[0], geneValue[0], geneValue[0] );
        }
    else {
        if( inLocator == scaleCornerAColorDelta ||
            inLocator == scaleCornerBColorDelta ||
            inLocator == scaleCornerCColorDelta ||
            inLocator == scaleCornerDColorDelta ) {
            // limit range of delta colors

            double deltaR, deltaG, deltaB;

            deltaR = mapValueToRange( geneValue[0], -0.1, 0.1 );
            deltaG = mapValueToRange( geneValue[1], -0.1, 0.1 );
            deltaB = mapValueToRange( geneValue[2], -0.1, 0.1 );

            // delta for alpha always zero
            return new Color( deltaR, deltaG, deltaB, 0 );
            }
        
        return new Color( geneValue[0], geneValue[1], geneValue[2] );
        }
    }

