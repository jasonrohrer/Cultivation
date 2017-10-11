/*
 * Modification History
 *
 * 2004-August-22   Jason Rohrer
 * Created.
 *
 * 2004-August-31   Jason Rohrer
 * Added brief fade-in at note start to reduce clicks.
 *
 * 2006-October-3   Jason Rohrer
 * Optimized using profiler.
 */



#include "MusicNoteWaveTable.h"

#include "minorGems/util/SimpleVector.h"

#include <stdio.h>
#include <math.h>

extern double globalShortestNoteLength;
extern double globalLongestNoteLength;



MusicNote::MusicNote( int inFrequencyIndex, int inLengthIndex,
                      char inReversed )
    : mFrequencyIndex( inFrequencyIndex ),
      mLengthIndex( inLengthIndex ),
      mReversed( inReversed ) {

    
    }



MusicNote *MusicNote::copy() {
    return new MusicNote( mFrequencyIndex, mLengthIndex, mReversed );
    }



MusicNoteWaveTable::MusicNoteWaveTable( unsigned long inSamplesPerSecond ){


    // read frequencies and lengths from files
    

    SimpleVector<double> *frequencyVector = new SimpleVector<double>();
    SimpleVector<double> *lengthVector = new SimpleVector<double>();
    
    FILE *musicNotePitchesFILE = NULL;
    FILE *musicNoteLengthsFILE = NULL;
    

    if( musicNotePitchesFILE != NULL ) {

        double readValue;
        int numRead = 1;

        while( numRead == 1 ) {
            numRead = fscanf( musicNotePitchesFILE, "%lf", &readValue );
            
            if( numRead == 1 ) {
                frequencyVector->push_back( readValue );
                }
            }

        fclose( musicNotePitchesFILE );
        }
    else {
        // default pitches

        // Note  N,, means N two octaves down
        // N'' means two octaves up

        /*
        // This one sounds pretty good
        // but not enough notes, too bland
        
        // A,,
        frequencyVector->push_back( 110 );

        // D,
        frequencyVector->push_back( 146.832 );

        // A,
        frequencyVector->push_back( 220 );

        // D
        frequencyVector->push_back( 293.665 );

        // G
        frequencyVector->push_back( 391.995 );

        // C'
        frequencyVector->push_back( 523.251 );

        // E'
        frequencyVector->push_back( 659.255 );

        // G'
        frequencyVector->push_back( 783.991 );
        */


        // Instead, use entire two-octaves from c-major scale
        // Problem:  there can be some discords.
        // However:  much more interesting sounding than the two-chord version
        // above

        
        // base note:  C,
        double baseNote = 130.8127827;

        int majorScaleSteps[7] = {2,2,1,2,2,2,1};

        int scaleIndex = 0;
        int notePower = 0;

        // two octaves
        while( notePower < 25 ) {
            frequencyVector->push_back(
                baseNote * pow( 2, notePower / 12.0 ) );

            notePower += majorScaleSteps[ scaleIndex ];

            scaleIndex ++;
            if( scaleIndex >= 7 ) {
                // wrap around
                scaleIndex = 0;
                }
            }
        
                
        /*
          // These are the notes from Transcend level 001
        frequencyVector->push_back( 220 );
        frequencyVector->push_back( 277.183 );
        frequencyVector->push_back( 329.628 );
        frequencyVector->push_back( 440 );
        frequencyVector->push_back( 554.365 );
        frequencyVector->push_back( 659.255 );
        */
        }

    

    if( musicNoteLengthsFILE != NULL ) {

        double readValue;
        int numRead = 1;

        while( numRead == 1 ) {
            numRead = fscanf( musicNoteLengthsFILE, "%lf", &readValue );
            
            if( numRead == 1 ) {
                lengthVector->push_back( readValue );
                }
            }

        fclose( musicNoteLengthsFILE );
        }
    else {
        // default lengths
        lengthVector->push_back( globalLongestNoteLength );
        lengthVector->push_back( globalShortestNoteLength );
        }


    mFrequencyCount = frequencyVector->size();
    mLengthCount = lengthVector->size();

    double *frequencies = frequencyVector->getElementArray();
    mLengthsInSeconds = lengthVector->getElementArray();

    delete frequencyVector;
    delete lengthVector;
    
    
    
    
    mSampleTable = new float**[ mFrequencyCount ];
    mSampleCounts = new unsigned long[ mLengthCount ];

    for( int F=0; F<mFrequencyCount; F++ ) {

        mSampleTable[F] = new float*[ mLengthCount ];

        
        for( int L=0; L<mLengthCount; L++ ) {
            
            // construct a sample table for this freqency/length pair
            unsigned long lengthInSamples =
                (unsigned long)( mLengthsInSeconds[L] * inSamplesPerSecond );

            mSampleTable[F][L] = new float[ lengthInSamples ];


            // setting this inside a double-loop will set the same
            // value repeatedly with the same value, but this makes the code
            // cleaner (other options:  a separate loop to set this value, or
            //  an if statement to ensure that it is set only once)
            mSampleCounts[L] = lengthInSamples;

            

            // populate the sample table with a linearly decaying sine wave
            double frequencyInCyclesPerSecond = frequencies[F];


            double frequencyInCyclesPerSample =
                frequencyInCyclesPerSecond / inSamplesPerSecond;

            // sine function cycles every 2*pi
            // adjust so that it cycles according to our desired frequency
            double adjustedFrequency =
                frequencyInCyclesPerSample * ( 2 * M_PI );

            // try to fade in for 100 samples to avoid a click
            // at the start of the note
            unsigned long numFadeInSamples = 100;
            if( numFadeInSamples > lengthInSamples ) {
                numFadeInSamples = lengthInSamples / 2;
                }

            // optimizations (found with profiler)
            // pull these out of inner loop
            float lengthInSamplesMinusOne = (float)lengthInSamples - 1.0f;
            float inv_lengthInSamplesMinusOne =
                1.0f / lengthInSamplesMinusOne;
            float *theseSamples = mSampleTable[F][L];
            
            for( unsigned long i=0; i<lengthInSamples; i++ ) {

                // decay loudness linearly
                float loudness =
                    ( lengthInSamplesMinusOne - i )
                    * inv_lengthInSamplesMinusOne;
                    
                // fade in for the first 100 samples to avoid
                // a click

                if( i < numFadeInSamples ) {

                    float fadeInFactor =
                        (float)( i ) / (float)( numFadeInSamples - 1 );

                    // optimization:
                    // only do this extra multiplication for notes that
                    // are fading in
                    loudness *= fadeInFactor;
                    }
                
                theseSamples[i] =
                    loudness * (float)sin( i * adjustedFrequency );
                }
            }
        }

    
    delete [] frequencies;
    }



MusicNoteWaveTable::~MusicNoteWaveTable(){

    for( int F=0; F<mFrequencyCount; F++ ) {
        for( int L=0; L<mLengthCount; L++ ) {

            delete [] mSampleTable[F][L];

            }
        delete [] mSampleTable[F];
        }
    
    delete [] mSampleTable;
    delete [] mSampleCounts;
    delete [] mLengthsInSeconds;
    }



int MusicNoteWaveTable::getFrequencyCount(){
    return mFrequencyCount;
    }



int MusicNoteWaveTable::getLengthCount(){
    return mLengthCount;
    }



double MusicNoteWaveTable::getLengthInSeconds( int inLengthIndex ) {
    return mLengthsInSeconds[ inLengthIndex ];
    }



float *MusicNoteWaveTable::mapParametersToSamples(
    int inFrequencyIndex,
    int inLengthIndex,
    unsigned long *outNumSamples ){

    *outNumSamples = mSampleCounts[inLengthIndex];

    return mSampleTable[inFrequencyIndex][inLengthIndex];
    }



float *MusicNoteWaveTable::mapNoteToSamples(
    MusicNote *inNote,
    unsigned long *outNumSamples ){


    return mapParametersToSamples( inNote->mFrequencyIndex,
                                   inNote->mLengthIndex,
                                   outNumSamples );
    }






