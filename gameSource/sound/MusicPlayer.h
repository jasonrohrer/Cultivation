/*
 * Modification History
 *
 * 2004-August-22   Jason Rohrer
 * Created.
 */



#ifndef MUSIC_PLAYER_INCLUDED
#define MUSIC_PLAYER_INCLUDED



#include "SoundSamples.h"
#include "MusicNoteWaveTable.h"

#include "minorGems/util/SimpleVector.h"



/**
 * Class that plays music notes
 *
 * @author Jason Rohrer
 */
class MusicPlayer {


    public:


        
        /**
         * Constructs a player.
         * Reads configuration using the LevelDirectoryManager.
         *
         * @param inSamplesPerSecond the sample rate.
         * @param inWaveTable the wave table to use when rendering notes.
         *   Must be destroyed by caller after this class is destroyed.
		 * @param inPartLength the length of music parts in seconds.
         */
        MusicPlayer( unsigned long inSamplesPerSecond,
					 MusicNoteWaveTable *inWaveTable,
					 double inPartLength );

        
        ~MusicPlayer();


        
        /**
         * Gets more samples of music from this player.
         *
         * @param inNumSamples the number of samples to get.
         *
         * @return a buffer of samples.  Must be destroyed by caller.
         */
        SoundSamples *getMoreMusic( unsigned long inNumSamples );

        
        
    protected:
        MusicNoteWaveTable *mWaveTable;
        
        SimpleVector<SoundSamples*> *mActiveNotes;
        SimpleVector<unsigned long> *mNotePositions;
        

        
        double mPartLengthInSeconds;

        double mCurrentPlayTime;

        double mSampleRate;
        
    };



#endif





