/*
 * Modification History
 *
 * 2004-August-22   Jason Rohrer
 * Created.
 */



#ifndef MUSIC_NOTE_WAVE_TABLE_INCLUDED
#define MUSIC_NOTE_WAVE_TABLE_INCLUDED



/**
 * Class representing a note that can be mapped into the wave table.
 */
class MusicNote {

    public:


        /**
         * Constructs a note.
         *
         * @param inFrequencyIndex the frequency parameter in the wave table.
         * @param inLengthIndex the length parameter in the wave table.
         * @param inReversed true if this note's samples should be played
         *   in reverse.  Defaults to false.
         */
        MusicNote( int inFrequencyIndex, int inLengthIndex,
                   char inReversed = false );

        

        /**
         * Makes a copy of this note.
         *
         * @return a copy.
         *   Must be destroyed by caller.
         */
        MusicNote *copy();

        
        
        int mFrequencyIndex;
        int mLengthIndex;

        char mReversed;
        
    };



/**
 * Class that contains pre-rendered sample table for each possible
 * note waveform.
 *
 * @author Jason Rohrer
 */
class MusicNoteWaveTable {


    public:


        
        /**
         * Constructs a wave table.
         * Reads configuration using the LevelDirectoryManager.
         *
         * @param inSamplesPerSecond the sample rate.
         */
        MusicNoteWaveTable( unsigned long inSamplesPerSecond );


        
        ~MusicNoteWaveTable();



        /**
         * Gets the number of frequencies contained in this table.
         *
         * @return the number of frequencies.
         */
        int getFrequencyCount();



        /**
         * Gets the number of lengths contained in this table.
         *
         * @return the number of lengths.
         */
        int getLengthCount();


        
        /**
         * Gets the length associated with a length index.
         *
         * @param inLengthIndex the index to map.
         *
         * @return the length in seconds.
         */
        double getLengthInSeconds( int inLengthIndex );

        

        /**
         * Maps freqency and length index parameters to wave samples.
         *
         * @param inFrequencyIndex the frequency index, in the range
         *   [ 0, getFrequencyCount() ).
         * @param inLengthIndex the length index, in the range
         *   [ 0, getLengthCount() ).
         * @param outNumSamples pointer to where the number of samples
         *   should be returned (the size of the returned array).
         *
         * @return an array of samples.  Will be destroyed by this
         *  class.  Should not be modified by caller.
         */
        float *mapParametersToSamples( int inFrequencyIndex,
                                       int inLengthIndex,
                                       unsigned long *outNumSamples );



        /**
         * Maps a note to wave samples.
         *
         * @param inNote the note to map.
         *   Must be destroyed by caller.
         * @param outNumSamples pointer to where the number of samples
         *   should be returned (the size of the returned array).
         *
         * @return an array of samples.  Will be destroyed by this
         *  class.  Should not be modified by caller.
         */
        float *mapNoteToSamples( MusicNote *inNote,
                                 unsigned long *outNumSamples );


    protected:


        int mFrequencyCount;
        int mLengthCount;

        
        float ***mSampleTable;
        unsigned long *mSampleCounts;
        double *mLengthsInSeconds;
        
    };



#endif





