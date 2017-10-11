/*
 * Modification History
 *
 * 2006-August-11   Jason Rohrer
 * Created.
 */



#include "Genetics.h"


#include <string.h>
#include <stdio.h>


#include "minorGems/util/random/StdRandomSource.h"

extern StdRandomSource globalRandomSource;



Genetics::~Genetics() {
    for( int i=0; i<mNumGenes; i++ ) {
        delete [] mGenes[i];
        }
    delete [] mGenes;

    delete [] mGeneLengths;
    }



Genetics::Genetics( int inNumGenes, int *inGeneLengths )
    : mNumGenes( inNumGenes ),
      mGeneLengths( new int[ inNumGenes ] ),
      mGenes( new double*[ inNumGenes ] ) {

    for( int i=0; i<mNumGenes; i++ ) {
        int length = inGeneLengths[i];
        mGeneLengths[i] = length;
        mGenes[i] = new double[ length ];

        for( int j=0; j<length; j++ ) {
            mGenes[i][j] = globalRandomSource.getRandomDouble();
            }
        }
    }



Genetics::Genetics( Genetics *inParentA,
                    Genetics *inParentB ) {

    int minNumGenes = inParentA->getNumGenes();

    int bNumGenes = inParentB->getNumGenes();

    if( bNumGenes < minNumGenes ) {
        minNumGenes = bNumGenes;
        }

    mNumGenes = minNumGenes;

    mGenes = new double*[ mNumGenes ];
    mGeneLengths = new int[ mNumGenes ];

    for( int i=0; i<mNumGenes; i++ ) {
        Genetics *chosenParent = NULL;
        
        if( globalRandomSource.getRandomBoolean() ) {
            chosenParent = inParentA;
            }
        else {
            chosenParent = inParentB;
            }

        mGenes[i] = chosenParent->getGene( i );
        mGeneLengths[i] = chosenParent->getGeneLength( i );
        }    
    }



Genetics::Genetics( Genetics *inGeneticsToCopy )
    : mNumGenes( inGeneticsToCopy->getNumGenes() ) {

    mGeneLengths = new int[ mNumGenes ];
    mGenes = new double*[ mNumGenes ];
    
    for( int i=0; i<mNumGenes; i++ ) {
        mGeneLengths[i] = inGeneticsToCopy->getGeneLength( i );
        mGenes[i] = inGeneticsToCopy->getGene( i );
        } 
    }



double Genetics::mapValueToRange( double inValue, double inRangeStart,
                                  double inRangeEnd ) {
    return inValue * ( inRangeEnd - inRangeStart ) + inRangeStart;
    }



int Genetics::getNumGenes() {
    return mNumGenes;
    }


int Genetics::getGeneLength( int inIndex ) {
    return mGeneLengths[ inIndex ];
    }
    


double *Genetics::getGene( int inIndex ) {
    int length = getGeneLength( inIndex );
    
    double *returnArray = new double[ length ];
                                    
    memcpy( returnArray,
            mGenes[ inIndex ],
            sizeof( double ) * length );

    return returnArray;
    }



void Genetics::printGenetics() {

    for( int i=0; i<mNumGenes; i++ ) {

        for( int j=0; j<mGeneLengths[i]; j++ ) {
            printf( "%f, ", mGenes[i][j] );
            }
        printf( "\n" );
        }
    printf( "\n" );
    }



char Genetics::equals( Genetics *inGenetics ) {

    if( mNumGenes != inGenetics->getNumGenes() ) {
        return false;
        }

    int i;
    for( i=0; i<mNumGenes; i++ ) {
        if( mGeneLengths[i] != inGenetics->getGeneLength( i ) ) {
            return false;
            }
        }

    // same length

    for( i=0; i<mNumGenes; i++ ) {
        double *otherGene = inGenetics->getGene( i );

        for( int j=0; j<mGeneLengths[i]; j++ ) {
            if( otherGene[j] != mGenes[i][j] ) {
                return false;
                }
            }
        }

    
    // all tests passed
    return true;
    }

