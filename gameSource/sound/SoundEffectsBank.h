/*
 * Modification History
 *
 * 2007-August-7   Jason Rohrer
 * Created.
 */



#ifndef SOUND_EFFECTS_BANK_INCLUDED
#define SOUND_EFFECTS_BANK_INCLUDED


#include "SoundSamples.h"
#include "SoundPlayer.h"


#define numSoundEffects 8


enum SoundEffectKey {    
    effect_plantSeed = 0,
    effect_pickupWater,
    effect_dumpWater,
    effect_poison,
    effect_pickFruit,
    effect_eatFruit,
    effect_giveFruit,
    effect_mate
    };



/**
 * Class that encapsulates a collection of playable sound effects.
 *
 * @author Jason Rohrer
 */
class SoundEffectsBank {


        
    public:

        /**
         * Constructs a bank.
         *
         * @param inPlayer where the sound effects should be played.
         *   Destroyed by caller after this class is destroyed.
         */
        SoundEffectsBank( SoundPlayer *inPlayer );
        
        ~SoundEffectsBank();
        


        /**
         * Plays a specified sound effect.
         *
         * @param inKey the key for the effect to play.
         */
        void playEffect( SoundEffectKey inKey );
                
        
    protected:
        SoundPlayer *mSoundPlayer;
        
        SoundSamples *mEffectSamples[ numSoundEffects ];
        
        double mEffectLoudness[ numSoundEffects ];
        
        
    };



#endif
