/*
 * Modification History
 *
 * 2006-August-13   Jason Rohrer
 * Created.
 *
 * 2006-December-18   Jason Rohrer
 * Added seed width.
 */



#include "PlantGenetics.h"
#include <math.h>


int plantGeneLengths[28] = { 1, 1, 3, 3,
                             1, 1,
                             // fruit shape
                             1, 1,
                             // leaf color
                             3,
                             1,
                             // leaf shape
                             1, 1, 1, 1,
                             1, 1, 1,
                             // flower shape
                             1, 1, 1,
                             1, 1, 1,
                             // flower color
                             3, 3, 3, 3,
                             // flower timing
                             1 };



PlantGenetics::PlantGenetics()
    : Genetics( 28, plantGeneLengths ) {

    }



PlantGenetics::PlantGenetics( PlantGenetics *inParentA,
                              PlantGenetics *inParentB )
    : Genetics( inParentA, inParentB ) {

    }



PlantGenetics::PlantGenetics( PlantGenetics *inGeneticsToCopy )
    : Genetics( inGeneticsToCopy ) {

    }



double PlantGenetics::getParameter( PlantGeneLocator inLocator ) {
    double geneValue = mGenes[ inLocator ][0];
    
    switch( inLocator ) {
        case idealSoilType:
            if( geneValue < 0.5 ) {
                return 0;
                }
            else {
                return 1;
                }
        case seedWidth:
            // special case:
            // ignore seed width gene
            // instead, seed width depends on leavesPerJoint
            geneValue = mGenes[ leavesPerJoint ][0];
            
            return mapValueToRange( geneValue, -1, 0.5 );
        case fruitLobeRate:
            return mapValueToRange( geneValue, 0, 5 );
        case fruitLobeDepth:
            return mapValueToRange( geneValue, 0, 1 );
        case jointCount:
            // map from 2 to 5, integer
            return (int)( mapValueToRange( geneValue, 2, 5.999 ) );
        case leavesPerJoint:
            // map from 1 to 5, integer
            return (int)( mapValueToRange( geneValue, 1, 5.999 ) );
        case outerLeafGreen:
            return mapValueToRange( geneValue, 0.35, 1 );
        case leafWalkerDeltaAngle:
            return mapValueToRange( geneValue, -0.01, 0.01 );
        case leafWalkerDeltaDeltaAngle:
            return mapValueToRange( geneValue, -0.005, 0.005 );
        case leafWalkerSpawnIntervalFactor:
            return mapValueToRange( geneValue, 0.3333, 3 );
        case leafWalkerStartingSpawnInterval:
            return mapValueToRange( geneValue, 3, 20 );
        case leafWalkerSpawnAngle:
            return mapValueToRange( geneValue, -0.75 * M_PI, 0.75 * M_PI );
        case flowerPetalCount:
            return (int)( mapValueToRange( geneValue, 3, 10 ) );
        case flowerCenterRadius:
            return mapValueToRange( geneValue, 0.1, 1.0 );
        case flowerPetalAngle:
            return mapValueToRange( geneValue, -0.2 * M_PI, 0.2 * M_PI );
        case flowerPetalPointARadius:
            return mapValueToRange( geneValue, 0.3, 1.0 );
        case flowerPetalPointAAngle:
            return mapValueToRange( geneValue, 0.1 * M_PI, 0.2 * M_PI );
        case flowerPetalPointBRadius:
            return mapValueToRange( geneValue, 0.3, 1.0 );
        case flowerPetalPointBAngle:
            return mapValueToRange( geneValue, -0.2 * M_PI, -0.1 * M_PI );
        case timeBetweenFlowers:
            return mapValueToRange( geneValue, 1, 30 );
        // default to returning the gene itself
        default:
            return geneValue;
            break;
        }    
    }



Color *PlantGenetics::getColor( PlantGeneLocator inLocator ) {
    double *geneValue = mGenes[ inLocator ];

    int geneLength = mGeneLengths[ inLocator ];

    if( geneLength < 3 ) {
        return new Color( geneValue[0], geneValue[0], geneValue[0] );
        }
    else if ( inLocator == innerLeafColor ) {
        // red and blue between 0 and 0.5
        // green between 0.35 and 1
        return new Color( geneValue[0] * 0.5,
                          geneValue[1] * 0.65 + 0.35,
                          geneValue[2] * 0.5 );
        }
    else if ( inLocator == flowerCenterColor ||
              inLocator == flowerPetalPointAColor ||
              inLocator == flowerPetalPointBColor ||
              inLocator == flowerPetalPointCColor ) {
        
        // make sure green not the highest color component

        double colorValues[3];
        colorValues[0] = geneValue[0];
        colorValues[1] = geneValue[1];
        colorValues[2] = geneValue[2];
        
        if( colorValues[1] > colorValues[0] &&
            colorValues[1] > colorValues[2] ) {

            // lower it
            colorValues[1] = colorValues[0];
            }
            
        /*
        // map highest component to 1.0 and leave others alone

        // this avoids black flower centers
        
        int highestIndex = -1;
        double highestValue = 0;

        int i;
        for( i=0; i<3; i++ ) {
            if( geneValue[i] > highestValue ) {
                highestValue = geneValue[i];
                highestIndex = i;
                }
            }

        double colorValues[3];
        
        for( i=0; i<3; i++ ) {
            if( i == highestIndex ) {
                colorValues[i] = 1.0;
                }
            else {
                colorValues[i] = geneValue[i];
                }
            }
        */
        return new Color( colorValues[0], colorValues[1], colorValues[2] );
        }
    else if( inLocator == fruitNutrition ||
             inLocator == parentFruitNutrition ) {
        // map highest component to 1 and others to 0.25

        int highestIndex = -1;
        double highestValue = 0;

        int i;
        for( i=0; i<3; i++ ) {
            if( geneValue[i] > highestValue ) {
                highestValue = geneValue[i];
                highestIndex = i;
                }
            }

        double colorValues[3];
        
        for( i=0; i<3; i++ ) {
            if( i == highestIndex ) {
                colorValues[i] = 1.0;
                }
            else {
                colorValues[i] = 0.25;
                }
            }

        return new Color( colorValues[0], colorValues[1], colorValues[2] );
        }
    else {
        return new Color( geneValue[0], geneValue[1], geneValue[2] );
        }
    }


