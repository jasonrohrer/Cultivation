/*
 * Modification History
 *
 * 2004-August-6   Jason Rohrer
 * Created.
 */



#ifndef PLAYABLE_SOUND_INCLUDED
#define PLAYABLE_SOUND_INCLUDED



#include "SoundSamples.h"



/**
 * Interface for a sound source that can produce more samples on demand.
 *
 * @author Jason Rohrer
 */
class PlayableSound {


    public:

        

        /**
         * Gets more samples from this sound.
         *
         * @param inNumSamples the number of samples to get.
         *
         * @return the sound samples.  Must be destroyed by caller.
         *   At most inNumSamples are returned.  If less than inNumSamples
         *   are returned, this indicates that the end of the sound
         *   has been reached.
         */
        virtual SoundSamples *getMoreSamples( unsigned long inNumSamples ) = 0;


        
        /**
         * Makes a copy of this sound.
         *
         * @return a copy.
         *   Must be destroyed by caller.
         */
        virtual PlayableSound *copy() = 0;


        
        virtual ~PlayableSound();


        
    };



// to make the compilers happy
inline PlayableSound::~PlayableSound() {
    }



#endif
