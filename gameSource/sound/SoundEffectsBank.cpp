/*
 * Modification History
 *
 * 2007-August-7   Jason Rohrer
 * Created.
 */


#include "SoundEffectsBank.h"

#include "minorGems/util/random/StdRandomSource.h"


extern int globalSoundSampleRate;

extern StdRandomSource globalRandomSource;


SoundEffectsBank::SoundEffectsBank( SoundPlayer *inPlayer )
        : mSoundPlayer( inPlayer ) {

    double effectTime = 0.5;
    
    int numEffectSamples = (int)( effectTime * globalSoundSampleRate );
    

    float *left = new float[ numEffectSamples ];
    float *right = new float[ numEffectSamples ];
    
    int i;
    
    // swish swish swish for planting (digging sound)
    for( i=0; i<numEffectSamples; i++ ) {
        left[i] = right[i] =
            globalRandomSource.getRandomBoundedDouble( -1, 1 ) * 
            ( numEffectSamples - i ) / (float)numEffectSamples *
            ( ( i % 1500 ) / 1500.0 );
        }
    

    mEffectSamples[ effect_plantSeed ] = 
        new SoundSamples( numEffectSamples, left, right );
    

    left = new float[ numEffectSamples ];
    right = new float[ numEffectSamples ];

    // rising sine for pickup
    for( i=0; i<numEffectSamples; i++ ) {
        double iFraction = (double)i / (double)numEffectSamples;
        
        double frequency = 200 *( 1 - iFraction ) + 400 * iFraction;
        
        left[i] = right[i] =
            sin( frequency / globalSoundSampleRate * 2 * M_PI * i );
        }
    

    mEffectSamples[ effect_pickupWater ] = 
        new SoundSamples( numEffectSamples, left, right );



    left = new float[ numEffectSamples ];
    right = new float[ numEffectSamples ];

    // falling sine for dump
    for( i=0; i<numEffectSamples; i++ ) {
        double iFraction = (double)i / (double)numEffectSamples;
        
        double frequency = 400 *( 1 - iFraction ) + 200 * iFraction;
        
        left[i] = right[i] =
            sin( frequency / globalSoundSampleRate * 2 * M_PI * i );
        }
    

    mEffectSamples[ effect_dumpWater ] = 
        new SoundSamples( numEffectSamples, left, right );



    left = new float[ numEffectSamples ];
    right = new float[ numEffectSamples ];


    // deep falling sine for poison
    for( i=0; i<numEffectSamples; i++ ) {
        double iFraction = (double)i / (double)numEffectSamples;
        
        double frequency = 200 *( 1 - iFraction ) + 100 * iFraction;
        
        left[i] = right[i] =
            sin( frequency / globalSoundSampleRate * 2 * M_PI * i );
        }
    

    mEffectSamples[ effect_poison ] = 
        new SoundSamples( numEffectSamples, left, right );

    
    

    left = new float[ numEffectSamples ];
    right = new float[ numEffectSamples ];

    // high sine ping for gift
    for( i=0; i<numEffectSamples; i++ ) {
        double frequency = 800;
        
        left[i] = right[i] =
            ( numEffectSamples - i ) / (float)numEffectSamples *
            sin( frequency / globalSoundSampleRate * 2 * M_PI * i );
        }
    

    mEffectSamples[ effect_giveFruit ] = 
        new SoundSamples( numEffectSamples, left, right );


    



    // these ones are much shorter
    effectTime = 0.125;
    numEffectSamples = (int)( effectTime * globalSoundSampleRate );



    left = new float[ numEffectSamples ];
    right = new float[ numEffectSamples ];


    // quick falling sine for eat
    for( i=0; i<numEffectSamples; i++ ) {
        double iFraction = (double)i / (double)numEffectSamples;
        
        double frequency = 800 *( 1 - iFraction ) + 400 * iFraction;
        
        left[i] = right[i] =
            sin( frequency / globalSoundSampleRate * 2 * M_PI * i );
        }
    

    mEffectSamples[ effect_eatFruit ] = 
        new SoundSamples( numEffectSamples, left, right );



    left = new float[ numEffectSamples ];
    right = new float[ numEffectSamples ];

    // quick rising sine for pick
    for( i=0; i<numEffectSamples; i++ ) {
        double iFraction = (double)i / (double)numEffectSamples;
        
        double frequency = 400 *( 1 - iFraction ) + 800 * iFraction;
        
        left[i] = right[i] =
            ( numEffectSamples - i ) / (float)numEffectSamples *
            sin( frequency / globalSoundSampleRate * 2 * M_PI * i );
        }
    

    mEffectSamples[ effect_pickFruit ] = 
        new SoundSamples( numEffectSamples, left, right );



    left = new float[ numEffectSamples ];
    right = new float[ numEffectSamples ];

    // quick, low rising sine for mate
    for( i=0; i<numEffectSamples; i++ ) {
        double iFraction = (double)i / (double)numEffectSamples;
        
        double frequency = 200 *( 1 - iFraction ) + 400 * iFraction;
        
        left[i] = right[i] =
            ( numEffectSamples - i ) / (float)numEffectSamples *
            sin( frequency / globalSoundSampleRate * 2 * M_PI * i );
        }
    

    mEffectSamples[ effect_mate ] = 
        new SoundSamples( numEffectSamples, left, right );

    
    mEffectLoudness[ effect_plantSeed ] = 0.05;
    mEffectLoudness[ effect_pickupWater ] = 0.05;
    mEffectLoudness[ effect_dumpWater ] = 0.05;
    mEffectLoudness[ effect_poison ] = 0.1;
    mEffectLoudness[ effect_pickFruit ] = 0.05;
    mEffectLoudness[ effect_eatFruit ] = 0.05;
    mEffectLoudness[ effect_giveFruit ] = 0.05;
    mEffectLoudness[ effect_mate ] = 0.1; 
    }


        
SoundEffectsBank::~SoundEffectsBank() {
    for( int i=0; i<numSoundEffects; i++ ) {
        delete mEffectSamples[i];
        }
    }




void SoundEffectsBank::playEffect( SoundEffectKey inKey ) {
    mSoundPlayer->playSoundNow( mEffectSamples[ inKey ], false, 
                                mEffectLoudness[ inKey ] );
    }
