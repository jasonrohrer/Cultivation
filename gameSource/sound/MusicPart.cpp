/*
 * Modification History
 *
 * 2004-August-22   Jason Rohrer
 * Created.
 *
 * 2004-August-26   Jason Rohrer
 * Added parameter to control character of part.
 */



#include "MusicPart.h"

#include <math.h>


#include "minorGems/util/random/StdRandomSource.h"

extern StdRandomSource globalRandomSource;



MusicPart::MusicPart( MusicNoteWaveTable *inWaveTable,
                      double inPartLengthInSeconds,
                      double *inMelody,
                      int inNumNotes,
                      char inHighNotes,
                      char inShortNotes,
                      double inChanceOfReverseNote )
    : mWaveTable( inWaveTable ),
      mNotes( new SimpleVector<MusicNote *>() ) {

    int frequencyCount = inWaveTable->getFrequencyCount();
    int lengthCount = inWaveTable->getLengthCount();


    // populate our note vector with notes, using inMelody to select pitches

    // decide note length using our parameter
    int lengthIndex;

    if( inShortNotes ) {
        lengthIndex = lengthCount - 1;
        }
    else {
        lengthIndex = 0;
        }


    // pick a frequency range
    int rangeStart, rangeEnd;

    if( inHighNotes ) {
        rangeEnd = frequencyCount - 1;

        rangeStart = rangeEnd - frequencyCount / 2;
        }
    else {
        rangeStart = 0;
        rangeEnd = rangeStart + frequencyCount / 2;
        }
             
    
    double totalLength = 0;

    int melodyIndex = 0;
    
    while( totalLength < inPartLengthInSeconds ) {

        // add another note

        double thisNoteMelody = inMelody[ melodyIndex ];

        // put into range
        int frequencyIndex =
            (int)( thisNoteMelody * ( rangeEnd - rangeStart ) + rangeStart );

        char noteReversed;
        
        // flip a weighted coin to determine if this note should be played
        // in reverse
        if( globalRandomSource.getRandomDouble() < inChanceOfReverseNote ) {
            noteReversed = true;
            }
        else {            
            noteReversed = false;
            }

        mNotes->push_back(
            new MusicNote( frequencyIndex, lengthIndex, noteReversed ) );

        // add this note's length to our total
        totalLength += inWaveTable->getLengthInSeconds( lengthIndex );


        // next melody note
        melodyIndex ++;

        if( melodyIndex >= inNumNotes ) {
            // wrap back to first note
            melodyIndex = 0;
            }
        }


    // note lengths sum to a total length that may be beyond the limit

    if( totalLength > inPartLengthInSeconds ) {
        // drop the last note

        int lastNoteIndex = mNotes->size() - 1;
        
        delete *( mNotes->getElement( lastNoteIndex ) );
        mNotes->deleteElement( lastNoteIndex );


        // could do something more intelligent here...
        // like drop the note that results in a total length that
        // is closest to the inPartLengthInSeconds        
        }
    

    }



MusicPart::~MusicPart() {

    int numNotes = mNotes->size();

    for( int i=0; i<numNotes; i++ ) {
        delete *( mNotes->getElement( i ) );
        }
    delete mNotes;
    }



double MusicPart::getPartLengthInSeconds() {

    return mPartLengthInSeconds;
    }



int MusicPart::getNotesStartingInInterval( 
    double inStartTimeInSeconds, 
    double inLengthInSeconds,
    MusicNote ***outNotes,
    double **outNoteStartOffsetsInSeconds ) {


    SimpleVector<MusicNote*> *returnNotes = new SimpleVector<MusicNote*>();
    SimpleVector<double> *returnStartOffsets = new SimpleVector<double>();
    

    double endTimeInSeconds = inStartTimeInSeconds + inLengthInSeconds;

    // walk through notes looking for those that start in the interval
    int numNotes = mNotes->size();
    double currentNoteStartTime = 0;
    
    for( int i=0;
         i<numNotes && currentNoteStartTime <= endTimeInSeconds;
         i++ ) {


        MusicNote *note = *( mNotes->getElement( i ) );
        
        double noteLength =
            mWaveTable->getLengthInSeconds( note->mLengthIndex );
        
        if( currentNoteStartTime >= inStartTimeInSeconds ) {
            // add the note

            returnNotes->push_back( note->copy() );

            returnStartOffsets->push_back(
                currentNoteStartTime - inStartTimeInSeconds );
            }
        // else skip the note


        currentNoteStartTime += noteLength;
        }
    
    
    int numNotesReturned = returnNotes->size();
    
    *outNotes = returnNotes->getElementArray();
    *outNoteStartOffsetsInSeconds = returnStartOffsets->getElementArray();

    
    delete returnNotes;
    delete returnStartOffsets;
    
    return numNotesReturned;
    }






