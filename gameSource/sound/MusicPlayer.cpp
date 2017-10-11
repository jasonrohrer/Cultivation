/*
 * Modification History
 *
 * 2004-August-22   Jason Rohrer
 * Created.
 *
 * 2004-August-23   Jason Rohrer
 * Fixed bug in note offsets.
 *
 * 2004-August-24   Jason Rohrer
 * Added missing support for reversed notes and stereo split.
 * Changed to use constant power panning.
 */



#include "MusicPlayer.h"
#include "MusicPart.h"

#include "minorGems/math/geometry/Vector3D.h"



#include "../World.h"

extern World *globalWorld;

extern Vector3D globalPlayerCurrentPosition;


// needed to synchronize between GL thread and sound thread
#include "minorGems/system/MutexLock.h"

extern MutexLock globalLock;



MusicPlayer::MusicPlayer( unsigned long inSamplesPerSecond,
                          MusicNoteWaveTable *inWaveTable,
                          double inPartLength )
    : mWaveTable( inWaveTable ),
      mActiveNotes( new SimpleVector<SoundSamples*>() ),
      mNotePositions( new SimpleVector<unsigned long>() ),
      mPartLengthInSeconds( inPartLength ),
      mCurrentPlayTime( 0 ),
      mSampleRate( inSamplesPerSecond ) {
    }



MusicPlayer::~MusicPlayer() {
    int numActiveNotes = mActiveNotes->size();

    for( int i=0; i<numActiveNotes; i++ ) {
        delete *( mActiveNotes->getElement( i ) );
        }

    delete mActiveNotes;
    delete mNotePositions;
    }



SoundSamples *MusicPlayer::getMoreMusic( unsigned long inNumSamples ) {

    double bufferLengthInSeconds = (double)inNumSamples / (double)mSampleRate;

    if( mCurrentPlayTime + bufferLengthInSeconds > mPartLengthInSeconds ) {
        // this buffer request falls off end of music

        // split into two separate parts

        unsigned long samplesInFirstPart =
            (unsigned long)(
                ( mPartLengthInSeconds - mCurrentPlayTime ) * mSampleRate );

        SoundSamples *firstPart = getMoreMusic( samplesInFirstPart );

        // reset play position        
        mCurrentPlayTime = 0;
        
        int samplesInSecondPart = inNumSamples - samplesInFirstPart;

        SoundSamples *secondPart = getMoreMusic( samplesInSecondPart );

        // combine the two
        SoundSamples *bothParts = new SoundSamples( inNumSamples );

        memcpy( bothParts->mLeftChannel,
                firstPart->mLeftChannel,
                samplesInFirstPart * sizeof( float ) );
        memcpy( bothParts->mRightChannel,
                firstPart->mRightChannel,
                samplesInFirstPart * sizeof( float ) );
        
        // skip in bothParts to location where second part should go
        memcpy( &( bothParts->mLeftChannel[ samplesInFirstPart ] ),
                secondPart->mLeftChannel,
                samplesInSecondPart * sizeof( float ) );
        memcpy( &( bothParts->mRightChannel[ samplesInFirstPart ] ),
                secondPart->mRightChannel,
                samplesInSecondPart * sizeof( float ) );
        
        delete firstPart;
        delete secondPart;

        return bothParts;
        }
    
    globalLock.lock();
    

    // get parts and positions of gardeners
    int numParts = 0;
    Vector3D **positions =
        globalWorld->getAllGardenerPositions( &numParts );
    MusicPart **musicParts =
        globalWorld->getAllGardenerMusicParts( &numParts );
    double *volumeModifiers =
        globalWorld->getAllGardenerMusicVolumeModifiers( &numParts );

    
    // get center position from current player position
    Vector3D *centerPosition = new Vector3D( &globalPlayerCurrentPosition );



    if( numParts == 0 ) {
        // no pieces in sculpture... nothing to play

        delete [] positions;
        delete [] musicParts;
        delete [] volumeModifiers;
        
        delete centerPosition;

        globalLock.unlock();
        
        // return silent samples
        return new SoundSamples( inNumSamples );        
        }

    int i;

    for( i=0; i<numParts; i++ ) {
          
        MusicPart *part = musicParts[i];

        Vector3D *partPosition = positions[i];
        
        MusicNote **notes;
        double *noteStartOffsets;
                    
            
        int numNotes = 
            part->getNotesStartingInInterval( mCurrentPlayTime, 
                                              bufferLengthInSeconds,
                                              &notes,
                                              &noteStartOffsets );
                    
            
        // render each note and add it to our list of active notes
        for( int j=0; j<numNotes; j++ ) {

            double totalNoteOffset = noteStartOffsets[j];

            unsigned long numSilentSamples =
              (unsigned long)( totalNoteOffset * mSampleRate );
                
            unsigned long numNoteSamples;
            float *noteSamples =
              mWaveTable->mapNoteToSamples( notes[j],
                                            &numNoteSamples );

                
            unsigned long totalSamples = numSilentSamples + numNoteSamples;

                
            SoundSamples *samplesObject = new SoundSamples( totalSamples );

            float *leftChannel = samplesObject->mLeftChannel;
            float *rightChannel = samplesObject->mRightChannel;



            // compute stereo panning position

            /*
             * Notes by Phil Burk (creator of portaudio):
             *
             * If you want to keep the power constant, then (L^2 + R^2)
             * should be constant.  One way to do that is to use sine and
             * cosine curves for left and right because
             * (sin^2 + cos^2) = 1.
             *
             * pan = 0.0 to  PI/2
             * LeftGain(pan) = cos(pan)
             * RightGain(pan) = sin(pan)
             */


            // idea:
            // First, grab distance for overall gain
            double distanceFromCenter =
                centerPosition->getDistance( partPosition );

            // limit how much distance affects volume
            distanceFromCenter *= 0.1;
            
            // lower r limit of 1 --- distance can only decrease volume, not
            // increase it as source gets closer than 1 unit away
            if( distanceFromCenter < 1 ) {
                distanceFromCenter = 1;
                }

            // compute a vector pointing to part 
            Vector3D vectorToPart( partPosition );
            vectorToPart.subtract( centerPosition );

            // flip stuff behind us so that it is in front of us
            // stereo mapping for stuff beind and in front is the same
            if( vectorToPart.mY < 0 ) {
                vectorToPart.mY *= -1;
                }

            // watch out for near-zero vector
            if( vectorToPart.getLength() < 0.01 ) {
                // default to dead center
                vectorToPart.setCoordinates( 0, 1, 0 );
                }

            
            // compute angle between hard-right vector and the vector to part
            // this will always be in the range [0,PI] (because we've reversed
            // negative y values in vector to part)

            // want to take cos of angle as right gain and sin as left gain, so
            // map [0, Pi] space in front of us to [0, PI/2]

            Vector3D rightVector( 1, 0, 0 );
            Angle3D *angle = rightVector.getZAngleTo( &vectorToPart );

            double halfZAngle = angle->mZ / 2;
            delete angle;

            
            //double rightGain = cos( halfZAngle );
            //double leftGain = sin( halfZAngle );

            // actually, for some reason, left and right are reversed
            // (x coordinates increase as we move left on the screen)
            // don't waste time figuring out why

            double leftGain = cos( halfZAngle );
            double rightGain = sin( halfZAngle );


            // modify gain with distance,
            // 1 / r^2
            double distanceFactor =
                1.0 / ( distanceFromCenter * distanceFromCenter );

            leftGain *= distanceFactor;
            rightGain *= distanceFactor;

            leftGain *= volumeModifiers[i];
            rightGain *= volumeModifiers[i];
            

            
            // samplesObject starts out with all 0 samples
            // just fill in the note samples beyond the starting silent
            // region
            char reversed = notes[j]->mReversed;
                
            for( unsigned long k=0; k<numNoteSamples; k++ ) {
                unsigned long sampleIndex;
                if( reversed ) {
                    sampleIndex =
                      ( numNoteSamples - k - 1 ) + numSilentSamples;
                    }
                else {
                    sampleIndex = k + numSilentSamples;
                    }
                    
                leftChannel[ sampleIndex ] =
                    leftGain * noteSamples[k];
                rightChannel[ sampleIndex ] =
                    rightGain * noteSamples[k];
                }

            mActiveNotes->push_back( samplesObject );
            mNotePositions->push_back( 0 );
                
            delete notes[j];
            }

        delete [] notes;
        delete [] noteStartOffsets;

        
        delete partPosition;
        
        // don't delete the music parts, since they are managed by world
        }

    delete [] positions;
    delete [] musicParts;
    delete [] volumeModifiers;

    delete centerPosition;

    // unlock here, since we're done touching shared data
    globalLock.unlock();
    
    
    // now we have added all notes that start playing sometime during
    // this buffer


    // next mix the samples from active notes that play during this buffer

    SoundSamples *returnSamples = new SoundSamples( inNumSamples );
    float *returnLeftChannel = returnSamples->mLeftChannel;
    float *returnRightChannel = returnSamples->mRightChannel;
    
    for( i=0; i<mActiveNotes->size(); i++ ) {

        SoundSamples *noteSamples = *( mActiveNotes->getElement( i ) );
        unsigned long notePosition = *( mNotePositions->getElement( i ) );

        unsigned long numNoteSamples = noteSamples->mSampleCount;
        
        unsigned long numSamplesToPlay = inNumSamples;

        char noteFinished = false;
        
        if( numSamplesToPlay + notePosition > numNoteSamples ) {
            // buffer goes beyond the end of this note

            numSamplesToPlay = numNoteSamples - notePosition;
            
            noteFinished = true;
            }
        
        float *noteLeftChannel = noteSamples->mLeftChannel;
        float *noteRightChannel = noteSamples->mRightChannel;
                
        for( unsigned long j=0; j<numSamplesToPlay; j++ ) {
            unsigned long noteIndex = j + notePosition;
            
            returnLeftChannel[j] += noteLeftChannel[ noteIndex ];
            returnRightChannel[j] += noteRightChannel[ noteIndex ];

            }

        notePosition += numSamplesToPlay;

        *( mNotePositions->getElement( i ) ) = notePosition;

        
        if( noteFinished ) {
            delete noteSamples;
            mActiveNotes->deleteElement( i );
            mNotePositions->deleteElement( i );

            // back up in for loop to reflect this note dropping out
            i--;
            }
        }
    
    

    // advance the grid position
    mCurrentPlayTime += bufferLengthInSeconds;

    // wrap
    while( mCurrentPlayTime > mPartLengthInSeconds ) {
        mCurrentPlayTime -= mPartLengthInSeconds;
        }

    
    return returnSamples;
    }




