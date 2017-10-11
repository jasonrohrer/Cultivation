/*
 * Modification History
 *
 * 2004-July-17   Jason Rohrer
 * Created.
 *
 * 2004-July-21   Jason Rohrer
 * Changed to use a callback to reduce latency.
 *
 * 2004-August-9   Jason Rohrer
 * Added a limit on the number of simultaneous sounds.
 *
 * 2004-August-12   Jason Rohrer
 * Parameterized the sample rate.
 *
 * 2004-August-13   Jason Rohrer
 * Added Mutex to protect members that are accessed by multiple threads.
 *
 * 2004-August-14   Jason Rohrer
 * Changed to fade sounds out before dropping them.
 *
 * 2004-August-15   Jason Rohrer
 * Added a function for getting the sample rate.
 * Added volume modifier parameter to playSoundNow function.
 *
 * 2004-August-20   Jason Rohrer
 * Added priority flags.
 *
 * 2004-August-23   Jason Rohrer
 * Added music.
 *
 * 2004-August-25   Jason Rohrer
 * Added function for setting music loudness.
 *
 * 2004-August-31   Jason Rohrer
 * Added function for removing filters.
 *
 * 2006-September-19   Jason Rohrer
 * Added global loudness and fade-in.
 *
 * 2006-October-2   Jason Rohrer
 * Added a separate lock for music player to deal with deadlock.
 */



#ifndef SOUND_PLAYER_INCLUDED
#define SOUND_PLAYER_INCLUDED


#include "SoundSamples.h"
#include "SoundFilter.h"
#include "PlayableSound.h"

#include "minorGems/sound/portaudio/pa_common/portaudio.h"
#include "minorGems/sound/portaudio/pablio/pablio.h"


#include "minorGems/util/SimpleVector.h"

#include "minorGems/system/MutexLock.h"



/**
 * Class that plays both running background music and realtime sounds.
 *
 * @author Jason Rohrer
 */
class SoundPlayer {


        
    public:


        
        /**
         * Constructs a sound player.
         *
         * @param inSampleRate the number of samples per second.
         *   Should be a "standard" rate, like 44100, 22050, etc.
         * @param inMaxSimultaneousRealtimeSounds the number of simultaneous
         *   realtime sounds to allow.  When this limit is reached,
         *   older sounds are silenced prematurely to make way for
         *   newer sounds.
         * @param inMusicPlayer the player to get music from, or NULL
         *   to disable music.  Defaults to NULL.
         *   Typed as (void*) to avoid an include loop.
         *   Must be destroyed by caller after this class is destroyed.
         * @param inMusicLoudness an adjustment for music loudness in the
         *   range [0,1].  Defaults to 1.
         * @param inGlobalLoudness the global adjustment for loudness.
         *   In the range [0,1].  Defaults to 1.
         */
        SoundPlayer( int inSampleRate,
                     int inMaxSimultaneousRealtimeSounds,
                     void *inMusicPlayer = NULL,
                     double inMusicLoudness = 1,
                     double inGlobalLoudness = 1 );

        ~SoundPlayer();



        /**
         * Sets the music player.
         *
         * @param inMusicPlayer the player to get music from, or NULL
         *   to disable music.  Typed as (void*) to avoid an include loop.
         *   Must be destroyed by caller after this class is destroyed.
         */
        void setMusicPlayer( void *inMusicPlayer );

        

        /**
         * Sets the loudess of the music.
         *
         * @param inMusicLoudness the loudness in the range [0,1].
         */
        void setMusicLoudness( double inMusicLoudness );


        
        /**
         * Sets the global loudness.
         *
         * @param inLoudness the loudness in the range [0,1].
         */
        void setGlobalLoudness( double inLoudness );



        /**
         * Triggers global loudness to fade up to 1 over time.
         *
         * @param inFadeTimeInSeconds the time until a loudness of 1
         *   is reached.
         */
        void fadeIn( double inFadeTimeInSeconds );

        
        
        /**
         * Mixes a sound to the speakers as quickly as possible.
         *
         * This call does not adjust the volume level of the samples
         * before mixing them with other realtime sounds or the background
         * music.
         *
         * @param inSamples the samples to play.
         *   Must be destroyed by caller.
         * @param inPriorityFlag true if this sound should have
         *   high priority, or false for low priority.  Defaults to false.
         * @param inLoudnessModifier the value to adjust the sound's
         *   loudness by, in [0,1], when playing.  Defaults to 1.
         */
        void playSoundNow( SoundSamples *inSamples,
                           char inPriorityFlag = false,
                           double inLoudnessModifier = 1.0 );


        
        /**
         * Same as earlier playSoundNow, except that it takes a PlayableSound
         * that must be destroyed by the caller.
         */
        void playSoundNow( PlayableSound *inSound,
                           char inPriorityFlag = false,
                           double inLoudnessModifier = 1.0 );
        
               
        
        /**
         * Add the next section of music to be played.
         *
         * This call drives the sound player to send audio to the speakers.
         *
         * @param inSamples the samples to play.
         *   Must be destroyed by caller.
         */
        void addMoreMusic( SoundSamples *inSamples );



        /**
         * Called by the internal portaudio callback function.
         */
        void getSamples( void *outputBuffer, unsigned long inFramesInBuffer );


        
        /**
         * Adds a filter to the end of the chain that will process
         * samples before sending them to the speakers.
         *
         * @param inFilter the filter to add.
         *   Will be destroyed when this class is destroyed.
         */
        void addFilter( SoundFilter *inFilter );



        /**
         * Removes and destroys all filters.
         */
        void removeAllFilters();
        


        /**
         * Gets the current sample rate.
         *
         * @return the sample rate, in samples per second.
         */
        unsigned long getSampleRate();

        
        
    protected:

        MutexLock *mLock;

        // separate lock for setting music player
        // deals with a deadlock issue
        MutexLock *mMusicPlayerLock;
        
        unsigned long mSampleRate;
        
        char mAudioInitialized;

        int mMaxSimultaneousRealtimeSounds;

        // Typed as (void*) to avoid an include loop.
        void *mMusicPlayer;
        double mMusicLoudness;

        double mGlobalLoudness;
        char mFadingIn;
        int mNumFadeFramesRemaining;
        
        PortAudioStream *mAudioStream;

        // realtime sounds that should be mixed into the next to-speaker call
        SimpleVector<PlayableSound *> *mRealtimeSounds;

        SimpleVector<char> *mPriorityFlags;
        
        // one modifier for each realtime sound
        SimpleVector<double> *mSoundLoudnessModifiers;
        
        // one flag for each realtime sound, indicating whether it should
        // be dropped (faded out) during the next frame 
        SimpleVector<char> *mSoundDroppedFlags;

        
        SimpleVector<SoundFilter *> *mFilterChain;


        
        /**
         * Checks the sound set to ensure that our max simultaneous limit
         * is being met.
         *
         * Not thread-safe:  should be called with mLock already locked.
         */
        void checkForExcessSounds();
        
        
        
    };



#endif
