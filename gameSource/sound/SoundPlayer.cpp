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
 * Added lock in setMusicPlayer function.
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



#include "SoundPlayer.h"
#include "MusicPlayer.h"

#include <stdio.h>


// callback passed into portaudio
static int portaudioCallback( void *inputBuffer, void *outputBuffer,
                              unsigned long framesPerBuffer,
                              PaTimestamp outTime, void *userData ) {


    SoundPlayer *player = (SoundPlayer *)userData;

    player->getSamples( outputBuffer, framesPerBuffer );

    return 0;
    }



/**
 * Class that wraps SoundSamples in a PlayableSound.
 */
class SamplesPlayableSound : public PlayableSound {


    public:



        /**
         * Constructs a playable sound.
         *
         * @param inSamples the samples to play.
         *   Must be destroyed by caller.
         */
        SamplesPlayableSound( SoundSamples *inSamples );


        
        ~SamplesPlayableSound();

        
        // implements the PlayableSound interface
        virtual SoundSamples *getMoreSamples( unsigned long inNumSamples );
        virtual PlayableSound *copy();
        

    protected:

        SoundSamples *mRemainingSamples;

    };



SamplesPlayableSound::SamplesPlayableSound( SoundSamples *inSamples )
    : mRemainingSamples( new SoundSamples( inSamples ) ) {

    }



SamplesPlayableSound::~SamplesPlayableSound() {
    delete mRemainingSamples;
    }



SoundSamples *SamplesPlayableSound::getMoreSamples(
    unsigned long inNumSamples ) {

    SoundSamples *returnSamples = new SoundSamples( mRemainingSamples,
                                                    inNumSamples );
    mRemainingSamples->trim( returnSamples->mSampleCount );

    return returnSamples;
    }



PlayableSound *SamplesPlayableSound::copy() {
    return new SamplesPlayableSound( mRemainingSamples );
    }



SoundPlayer::SoundPlayer( int inSampleRate,
                          int inMaxSimultaneousRealtimeSounds,
                          void *inMusicPlayer,
                          double inMusicLoudness,
                          double inGlobalLoudness )
    : mLock( new MutexLock() ),
      mMusicPlayerLock( new MutexLock() ),
      mSampleRate( inSampleRate ),
      mMaxSimultaneousRealtimeSounds( inMaxSimultaneousRealtimeSounds ),
      mMusicPlayer( inMusicPlayer ),
      mMusicLoudness( inMusicLoudness ),
      mGlobalLoudness( inGlobalLoudness ),
      mFadingIn( false ),
      mNumFadeFramesRemaining( 0 ),
      mRealtimeSounds( new SimpleVector<PlayableSound *>() ),
      mPriorityFlags( new SimpleVector<char>() ),
      mSoundLoudnessModifiers( new SimpleVector<double>() ),      
      mSoundDroppedFlags( new SimpleVector<char>() ),
      mFilterChain( new SimpleVector<SoundFilter *>() ) {

    PaError error = Pa_Initialize();

    if( error == paNoError ) {
    
        error = Pa_OpenStream(
            &mAudioStream,
            paNoDevice,// default input device 
            0,              // no input 
            paFloat32,  // 32 bit floating point input 
            NULL,
            Pa_GetDefaultOutputDeviceID(),
            2,          // stereo output 
            paFloat32,      // 32 bit floating point output 
            NULL,
            mSampleRate,
            1024,   // frames per buffer
            0,    // number of buffers, if zero then use default minimum 
            paClipOff, // we won't output out of range samples so
                       // don't bother clipping them 
            portaudioCallback,
            (void *)this );  // pass self-pointer to callback function
        
        if( error == paNoError ) {

            error = Pa_StartStream( mAudioStream );
            
            if( error == paNoError ) {
                mAudioInitialized = true;
                }
            else {
                fprintf( stderr, "Error starting audio stream\n" );
                Pa_CloseStream( mAudioStream );
                }            
            }
        else {
            fprintf( stderr, "Error opening audio stream\n" );
            Pa_Terminate();
            }
        }
    else {
        fprintf( stderr, "Error initializing audio framework\n" );
        }

    
    if( error != paNoError ) {
        fprintf( stderr, "Error number: %d\n", error );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( error ) );
        mAudioInitialized = false;
        }
    }



SoundPlayer::~SoundPlayer() {
    if( mAudioInitialized ) {
        PaError error = Pa_StopStream( mAudioStream );

        if( error == paNoError ) {
            error = Pa_CloseStream( mAudioStream );

            if( error != paNoError ) {
                fprintf( stderr, "Error closingaudio stream\n" );
                }
            }
        else {
            fprintf( stderr, "Error stopping audio stream\n" );
            }

        Pa_Terminate();
        
        if( error != paNoError ) {
            fprintf( stderr, "Error number: %d\n", error);
            fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( error) );
            }
        }

    delete mLock;
    delete mMusicPlayerLock;
    
    int i;
    
    int numSounds = mRealtimeSounds->size();

    for( i=0; i<numSounds; i++ ) {
        delete *( mRealtimeSounds->getElement( i ) );
        }
    delete mRealtimeSounds;

    delete mPriorityFlags;
    delete mSoundLoudnessModifiers;
    delete mSoundDroppedFlags;
    
    int numFilters = mFilterChain->size();

    for( i=0; i<numFilters; i++ ) {
        delete *( mFilterChain->getElement( i ) );
        }
    delete mFilterChain;
    }



void SoundPlayer::setMusicPlayer( void *inMusicPlayer ) {
    mMusicPlayerLock->lock();
    mMusicPlayer = inMusicPlayer;
    mMusicPlayerLock->unlock();
    }



void SoundPlayer::setMusicLoudness( double inMusicLoudness ) {
    mLock->lock();
    mMusicLoudness = inMusicLoudness;
    mLock->unlock();
    }



void SoundPlayer::setGlobalLoudness( double inLoudness ) {
    mLock->lock();
    mGlobalLoudness = inLoudness;
    mLock->unlock();
    }




void SoundPlayer::fadeIn( double inFadeTimeInSeconds ) {
    mLock->lock();
    mFadingIn = true;
    mNumFadeFramesRemaining = (int)( mSampleRate * inFadeTimeInSeconds );
    mLock->unlock();
    }



void SoundPlayer::getSamples( void *outputBuffer,
                              unsigned long inFramesInBuffer ) {

    SoundSamples *mixingBuffer = new SoundSamples( inFramesInBuffer );

    unsigned long bufferLength = inFramesInBuffer;

    
    mLock->lock();
    // add each pending realtime sound to the buffer

    int i = 0;
    
    // we may be removing sounds from the buffer as we use them up
    // i is adjusted inside the while loop
    while( i<mRealtimeSounds->size() ) {
        
        PlayableSound *realtimeSound = *( mRealtimeSounds->getElement( i ) );

        double loudnessModifier =
            *( mSoundLoudnessModifiers->getElement( i ) );

        SoundSamples *samples = realtimeSound->getMoreSamples( bufferLength );

        unsigned long mixLength = samples->mSampleCount;


        char shouldDrop = *( mSoundDroppedFlags->getElement( i ) ); 

        // fade out if we should drop
        float fadeFactor = 1;

        unsigned long maxJ = mixLength - 1;

        for( unsigned long j=0; j<mixLength; j++ ) {
            mixingBuffer->mLeftChannel[j] +=
                loudnessModifier * fadeFactor * samples->mLeftChannel[j];
            mixingBuffer->mRightChannel[j] +=
                loudnessModifier * fadeFactor * samples->mRightChannel[j];

            if( shouldDrop ) {
                fadeFactor = ( maxJ - j ) / (float)maxJ;
                }
            }

        delete samples;
        
        
        if( mixLength < bufferLength || shouldDrop ) {

            // we have used up all samples of this sound or
            // it is flagged to be dropped
            delete realtimeSound;
            mRealtimeSounds->deleteElement( i );
            mPriorityFlags->deleteElement( i );
            mSoundLoudnessModifiers->deleteElement( i );
            mSoundDroppedFlags->deleteElement( i );
            
            // don't increment i, since the next element drops into the current
            // index
            }
        else {    
            // increment i to move on to the next sound
            i++;
            }
        }


    // Deadlock problem:
    // MusicPlayer locks the globalLock while it accesses music data from world
    // However, world thread, while it has globalLock locked, tries to set
    // the music volume, which requires locking our main lock mLock
    //
    // Bad order of events:
    // World locks globalLock
    // SoundPlayer locks mLock
    // SoundPlayer calls MusicPlayer to get samples
    // MusicPlayer waits for globalLock to get notes
    // World calls setMusicLoudness, waiting for SoundPlayer's mLock
    // => DEADLOCK 

    // grab a local copy of music loudness before unlocking main lock
    double localMusicLoundless = mMusicLoudness;
    
    // unlock main lock while we access the music player    
    mLock->unlock();

    // lock the specific lock
    mMusicPlayerLock->lock();
    
    if( mMusicPlayer != NULL ) {
        // cast out of void *
        MusicPlayer *player = (MusicPlayer *)mMusicPlayer;

        // mix in the music
        SoundSamples *musicSamples =
            player->getMoreMusic( inFramesInBuffer );

        for( unsigned long j=0; j<inFramesInBuffer; j++ ) {
            mixingBuffer->mLeftChannel[j] +=
                musicSamples->mLeftChannel[j] * localMusicLoundless;
            mixingBuffer->mRightChannel[j] +=
                musicSamples->mRightChannel[j] * localMusicLoundless;
            }
        delete musicSamples;
        }        
    mMusicPlayerLock->unlock();


    // re-lock the main lock
    mLock->lock();

    
    
    // filter the samples
    int numFilters = mFilterChain->size();
    SoundSamples *filteredSamples = new SoundSamples( mixingBuffer );
    delete mixingBuffer;
    
    for( i=0; i<numFilters; i++ ) {
        SoundFilter *filter = *( mFilterChain->getElement( i ) );
        
        SoundSamples *nextOutput = filter->filterSamples( filteredSamples );

        delete filteredSamples;

        filteredSamples = nextOutput;
        }

    mLock->unlock();
    
    
    float *samples = (float *)outputBuffer;

    unsigned long numSamples = 2 * bufferLength;
    unsigned long j;
    unsigned long frameNumber = 0;

    // if fading in, adjust global volume as we go
    double globalLoudnessDelta = 0;

    if( mFadingIn ) {
        // want to reach loudness of 1 after mNumFadeFramesRemaining

        double loudnessChangeLeft = 1 - mGlobalLoudness;
        
        globalLoudnessDelta = loudnessChangeLeft / mNumFadeFramesRemaining;
        }

    
    for( j=0; j<numSamples; j+=2 ) {

        samples[j] =
            mGlobalLoudness * filteredSamples->mLeftChannel[frameNumber];
        samples[j+1] =
            mGlobalLoudness * filteredSamples->mRightChannel[frameNumber];

        frameNumber++;

        if( mFadingIn ) {
            mGlobalLoudness += globalLoudnessDelta;
            if( mGlobalLoudness >= 1 ) {
                // done
                mGlobalLoudness = 1;
                mFadingIn = false;
                mNumFadeFramesRemaining = 0;
                }
            }
        }

    
    if( mFadingIn ) {
        // not done fading in yet

        // update frames remaining
        mNumFadeFramesRemaining -= inFramesInBuffer;
        }

    
    delete filteredSamples;        
    }


void SoundPlayer::checkForExcessSounds() {
    
    int numSounds = mRealtimeSounds->size();
    if( numSounds > mMaxSimultaneousRealtimeSounds ) {
        // flag the oldest unflagged, low-priority sound
        
        // skip sounds that are already flagged or are priority sounds
        int i=0;
        while( i<numSounds &&
               ( *( mSoundDroppedFlags->getElement( i ) )  ||
                 *( mPriorityFlags->getElement( i ) ) ) ) {
            i++;
            }

        if( i<numSounds ) {
            *( mSoundDroppedFlags->getElement( i ) ) = true;
            }
        else {
            // else all low-priority sounds are already flagged

            // try again, ignoring priority flags
            i=0;
            while( i<numSounds && *( mSoundDroppedFlags->getElement( i ) ) ) {
                i++;
                }

            if( i<numSounds ) {
                *( mSoundDroppedFlags->getElement( i ) ) = true;
                }

            // else all sounds are already flagged
            
            }
        
        }
    }



void SoundPlayer::playSoundNow( SoundSamples *inSamples,
                                char inPriorityFlag,
                                double inLoudnessModifier ) {
    mLock->lock();
    
    mRealtimeSounds->push_back( new SamplesPlayableSound( inSamples ) );
    mPriorityFlags->push_back( inPriorityFlag );
    mSoundLoudnessModifiers->push_back( inLoudnessModifier );
    mSoundDroppedFlags->push_back( false );

    checkForExcessSounds();
    
    mLock->unlock();
    }



void SoundPlayer::playSoundNow( PlayableSound *inSound,
                                char inPriorityFlag,
                                double inLoudnessModifier ) {
    mLock->lock();
    
    mRealtimeSounds->push_back( inSound->copy() );
    mPriorityFlags->push_back( inPriorityFlag );
    mSoundLoudnessModifiers->push_back( inLoudnessModifier );
    mSoundDroppedFlags->push_back( false );

    checkForExcessSounds();

    mLock->unlock();
    }


        
void SoundPlayer::addMoreMusic( SoundSamples *inSamples ) {

    }



void SoundPlayer::addFilter( SoundFilter *inFilter ) {

    mLock->lock();
    
    mFilterChain->push_back( inFilter );

    mLock->unlock();
    }



void SoundPlayer::removeAllFilters() {
    mLock->lock();

    int numFilters = mFilterChain->size();

    for( int i=0; i<numFilters; i++ ) {
        delete *( mFilterChain->getElement( i ) );
        }

    mFilterChain->deleteAll();

    mLock->unlock();
    }



unsigned long SoundPlayer::getSampleRate() {
    return mSampleRate;
    }

