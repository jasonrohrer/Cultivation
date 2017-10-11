/*
 * Modification History
 *
 * 2004-August-22   Jason Rohrer
 * Created.
 *
 * 2004-August-26   Jason Rohrer
 * Added parameter to control character of part.
 *
 * 2006-September-18   Jason Rohrer
 * Changed to accept a fixed melody in constructor.
 */



#ifndef MUSIC_PART_INCLUDED
#define MUSIC_PART_INCLUDED



#include "MusicNoteWaveTable.h"

#include "minorGems/util/SimpleVector.h"



/**
 * A sequence of music notes.
 *
 * @author Jason Rohrer
 */
class MusicPart {


    public:


        
        /**
         * Constructs a randomized part.
         * Reads configuration using the LevelDirectoryManager.
         *
         * @param inWaveTable the wave table to pick random notes from.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inPartLengthInSeconds the length of this music part.
         * @param inMelody an array of values in [0,1] for selecting melody
         *   notes.  Must be destroyed by caller.
         * @param inNumNotes the number of elements in inMelody.
         * @param inHighNotes true to use high end of scale, false to use
         *   low end.
         * @param inShortNotes true to use short (fast) notes, false to use
         *   long (slow) notes.
         * @param inChanceOfReverseNote value in [0,1] specifying probability
         *   of each note being played in reverse.
         */
        MusicPart( MusicNoteWaveTable *inWaveTable,
                   double inPartLengthInSeconds,
                   double *inMelody,
                   int inNumNotes,
                   char inHighNotes,
                   char inShortNotes,
                   double inChanceOfReverseNote );


        
        ~MusicPart();


        
        /**
         * Gets this part's length.
         *
         * @return the length in seconds.
         */
        double getPartLengthInSeconds();

        

        /**
         * Gets the notes that should start playing in a given time interval.
         *
         * Note that only notes that *start* in the interval are returned.
         * Notes that are playing during the interval, but that start before
         * the interval, are ignored.
         *
         * @param inStartTimeInSeconds the start of the interval
         *   Must be in the range [ 0, getPartLengthInSeconds() ].
         * @param inLengthInSeconds the length of the interval.
         * @param outNotes pointer to where an array of notes
         *   should be returned.
         *   The returned array and the notes must be destroyed by caller.
         * @param outNoteStartOffsetsInSeconds pointer to where an array of
         *   note start offsets should be returned.
         *   Offsets are in seconds beyond inStartTimeInSeconds.
         *   The returned array must be destroyed by caller.
         *
         * @return the number of notes that start in the interval
         *   (in other words, the length of the returned arrays).
         */
        int getNotesStartingInInterval( 
            double inStartTimeInSeconds, 
            double inLengthInSeconds,
            MusicNote ***outNotes,
            double **outNoteStartOffsetsInSeconds );

        
    protected:
        MusicNoteWaveTable *mWaveTable;
        
        SimpleVector<MusicNote *> *mNotes;

        
        double mPartLengthInSeconds;

        
    };



#endif





