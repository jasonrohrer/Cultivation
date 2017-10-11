/*
 * Modification History
 *
 * 2006-July-2   Jason Rohrer
 * Created.
 *
 * 2006-October-4   Jason Rohrer
 * Fixed a crash when a baby's parent dies and is removed.
 *
 * 2006-October-27   Jason Rohrer
 * Added forceDead function.  Added poisonFruit function.
 *
 * 2006-October-30   Jason Rohrer
 * Added a following threshold gene.
 *
 * 2006-November-13   Jason Rohrer
 * Added a frozen state and a manual fade factor for drawing.
 *
 * 2006-November-24   Jason Rohrer
 * Made most/least liked gardener consistent across multiple function calls.
 *
 * 2006-November-25   Jason Rohrer
 * Added feeding of followers.
 *
 * 2006-December-25   Jason Rohrer
 * Added function to check if any stored fruit poisoned.
 */



#ifndef GARDENER_INCLUDED
#define GARDENER_INCLUDED



#include "minorGems/math/geometry/Vector3D.h"
#include "minorGems/graphics/openGL/SingleTextureGL.h"
#include "minorGems/graphics/Color.h"


#include "minorGems/util/SimpleVector.h"


#include "userInterface/ObjectStorage.h"

#include "Fruit.h"
#include "Seeds.h"

#include "Storable.h"
#include "DrawableObject.h"

#include "GardenerGenetics.h"

#include "sound/MusicPart.h"




// used to track color of each gardener scale
class ScaleCornerColors {
    public:
        // one color per corner
        Color mColors[4];

        // makes a copy of this color set
        ScaleCornerColors *copy();
    };



/**
 * A gardener.
 *
 * @author Jason Rohrer
 */
class Gardener : public ObjectStorage, public DrawableObject {

    public:

        
        
        /**
         * Constructs a gardener with random genetics.
         *
         * @param inStartPosition the position to start at.
         *   Destroyed by caller.
         */
        Gardener( Vector3D *inStartPosition );


        
        /**
         * Constructs a gardener that is a cross of two parents.
         *
         * @param inParentA, inParentB the two parents.  Destroyed by caller.
         */
        Gardener( Gardener *inParentA, Gardener *inParentB );

        

        virtual ~Gardener();


        
        // the genetics for this gardener
        GardenerGenetics mGenetics;

        
        // true if this gardener is a direct offspring
        // of the player's gardener
        char mUserCanControl;

        // true if the player is currently controlling this gardener
        char mUserControlling;
        
        
        /**
         * Gets the gardener's color.
         *
         * @return the color.  Destroyed by caller.
         */
        Color *getColor();


        
        /**
         * Gets the music part for this gardener.
         *
         * @return the music part.  Managed by this gardener.
         */
        MusicPart *getMusicPart();


        
        /**
         * Sets the position that this gardener wants to move toward.
         *
         * @param inPosition the desired position.
         *   Destroyed by caller.
         */
        void setDesiredPosition( Vector3D *inPosition );


        
        /**
         * Get the position that this gardener wants to move toward.
         *
         * @return the position.
         *   Destroyed by caller.
         */
        Vector3D *getDesiredPosition();


        
        /**
         * Sets whether this gardener is moving.
         *
         * @param inMoving true if moving.
         */
        void setMoving( char inMoving );


        
        /**
         * Gets whether gardener is moving.
         *
         * @return true if moving.
         */
        char isMoving();


            
        /**
         * Sets whether this gardener is carrying water.
         *
         * @param inCarryingWater true if carrying water.
         */
        void setCarryingWater( char inCarryingWater );


        
        /**
         * gets whether this gardener is carrying water.
         *
         * @return true if carrying water.
         */
        char getCarryingWater();


        
        /**
         * Gets the level of a nutrient.
         *
         * @param inNutrientIndex the index of the nutrient:
         *   0 for red,
         *   1 for green, and
         *   2 for blue.
         *
         * @return the nutrient level in [0,1].
         */
        double getNutrientLevel( int inNutrientIndex);



        /**
         * Gets whether this gardener is pregnant.
         *
         * @return true if pregnant.
         */
        char isPregnant();



        /**
         * Gets the amount of life left in this gardener.
         *
         * 1 at start of life, Near 0 when near death.
         *
         * @return the life value.
         */
        double getLife();
        
        
        
        /**
         * Returns true if this gardener has died.
         */
        char isDead();

        

        /**
         * Force this gardener to die.
         */
        void forceDead();


        
        /**
         * Sets this gardeners frozen flag.
         * Frozen gardeners do not move and are not drawn by the standard
         * World drawing routine.
         *
         * @param inFrozen true to mark as frozen.
         */
        void setFrozen( char inFrozen );



        /**
         * Gets whether this gardener is frozen.
         *
         * @return true if frozen.
         */
        char isFrozen();
        
        
        
        /**
         * Sets ghost mode.
         *
         * @param inMode true for ghost mode (still moves, but invisible).
         */
        void setGhostMode( char inMode );

        

        /**
         * Gets ghost mode.
         *
         * @param true for ghost mode (still moves, but invisible).
         */
        char isGhost();


        
        /**
         * Poisons fruit that the gardener is holding.
         */
        void poisonFruit();


        
        /**
         * Eats the currently selected fruit.
         */
        void eat();


        
        /**
         * Tell this gardener about another gardener.
         *
         * @param inGardener the gardener to be aware of.
         *   Must be destroyed by caller after this gardener is
         *   destroyed.
         */
        void trackOtherGardener( Gardener *inGardener );


        
        /**
         * Tell this gardener to stop tracking another gardener.
         *
         * @param inGardener the gardener to stop tracking.
         *   Must be destroyed by caller after this gardener is
         *   destroyed.
         */
        void untrackOtherGardener( Gardener *inGardener );


        
        /**
         * Tell this gardener to get angry at another gardener.
         *
         * @param inGardener the gardener to get angry at.
         *   Must be destroyed by caller after this gardener is
         *   destroyed.
         */
        void getAngry( Gardener *inGardener );

        

        
        /**
         * Tell this gardener to get fully angry at another gardener.
         *
         * @param inGardener the gardener to get angry at.
         *   Must be destroyed by caller after this gardener is
         *   destroyed.
         */
        void getMaxAngry( Gardener *inGardener );


        
        /**
         * Tell this gardener to get friendly at another gardener.
         *
         * @param inGardener the gardener to get friendly toward.
         *   Must be destroyed by caller after this gardener is
         *   destroyed.
         */           
        void getFriendly( Gardener *inGardener );

        

        /**
         * Gets how much this gardener likes another.
         *
         * @param inGardener the gardener to get the like metric for.
         *   Must be destroyed by caller after this gardener is
         *   destroyed.
         *
         * @return a value in [0,1] if gardener tracked, or -1 if untracked.
         */
        double getLikeMetric( Gardener *inGardener );



        /**
         * Gets whether we like another gardener enough to mate with it.
         *
         * @param inGardener the other gardener.  Must be a gardener we
         *   are already tracking.
         *
         * @return true if we like inGardener enough to mate with it.
         */
        char likeEnoughToMate( Gardener *inGardener );



        /**
         * Gets whether we like another gardener enough to follow it.
         *
         * @param inGardener the other gardener.  Must be a gardener we
         *   are already tracking.
         *
         * @return true if we like inGardener enough to follow it.
         */
        char likeEnoughToFollow( Gardener *inGardener );

        
        
        /**
         * Gets the gardener that we like the most.
         *
         * @return the gardener we like the most, or NULL if our like
         *   metric is not higher than 0.5 for any gardener.
         */
        Gardener *getMostLikedGardener();


        
        /**
         * Gets the gardener that we like the least.
         *
         * @return the gardener we like the least, or NULL if our like
         *   metric is not lower than 0.5 for any gardener.
         */
        Gardener *getLeastLikedGardener();


        
        /**
         * Sets the current emotion displayed by this gardener.
         *
         * @param inEmotion in range [-1, 1], where -1 is most angry,
         *   and 1 is most happy.
         */
        void setEmotionDisplay( double inEmotion );
        
        
        
        /**
         * Sets whether this gardener's plot should be hidden.
         *
         * @param inHidden true to hid plot.
         */
        void setPlotHidden( char inHidden );


        
        /**
         * Gets whether plot is hidden.
         *
         * @return true if hidden.
         */
        char isPlotHidden();



        /**
         * Adds item to storage.
         *
         * @param inItem the item to store.  Will be destroyed by this class.
         */         
        void storeItem( Storable *inItem );



        /**
         * Gets the number of fruits stored by this gardener.
         *
         * @return the fruit count.
         */
        int getStoredFruitCount();

        

        /**
         * Removes the selected fruit from storage.
         *
         * @param inSaveSeeds true to save the seeds in storage when
         *   removing this fruit from storage.  Defaults to false.
         * @param inGetAnyFruit true to get any stored fruit even
         *   if no fruit is selected.  Defaults to false.
         *
         * @return the selected fruit, or NULL if there is no fruit.
         *   Must be destroyed by caller.
         */
        Fruit *getSelectedFruit( char inSaveSeeds = false,
                                 char inGetAnyFruit = false );


        
        /**
         * Checks whether any stored fruit is poisoned.
         *
         * @return true if at least one stored fruit is poisoned.
         */
        char isStoredFruitPoisoned();

        

        /**
         * Gets the storage index of a fruit high in a given nutrient.
         *
         * @param inNutrientIndex a index of 0, 1, or 2.
         *
         * @return the index of a fruit, or -1 if there are no fruits stored.
         */
        int getIndexOfFruitHighInNutrient( int inNutrientIndex );


        
        
        /**
         * Gets pointer to selected seeds from storage.
         *
         * @return the selected seeds, or NULL if none selected.
         *   Destroyed by this class, and may not be valid across
         *   calls to passTime.
         */
        Seeds *getSelectedSeeds();


        
        /**
         * Removes seeds from storage and destroys them.
         *
         * @param inSeeds the seeds to destroy.  Managed by this class.
         */
        void removeSeeds( Seeds *inSeeds );
        


        /**
         * Gets all the seeds stored by this gardener.
         *
         * @return a vector of pointers to all seeds in storage.
         *   Vector destroyed by caller.  Seeds destroyed by this class, and
         *   may not be valid across calls to passTime.
         */
        SimpleVector<Seeds *> *getAllSeeds();


        
        /**
         * Gets pointer to selected item from storage.
         *
         * @return the selected item, or NULL if none selected.
         *   Destroyed by this class, and may not be valid across
         *   calls to passTime.
         */
        Storable *getSelectedStorable();


        
        /**
         * Gets a specific storable.
         *
         * @param inIndex the index to get.
         *
         * @return the item at inIndex, or NULL.
         *   Destroyed by this class, and may not be valid across
         *   calls to passTime.
         */
        Storable *getStorable( int inIndex );


        
        /**
         * Deletes the selected item.
         */
        void deleteSelectedStorable();

        

        /**
         * Make this gardener pregnant.
         *
         * Should only be called on a gardener that is not already pregnant.
         *
         * @param inOffspring the offspring that this gardener will carry.
         *   Destroyed by this class.
         */
        void setPregnant( Gardener *inOffspring );



        /**
         * Gets the parent that we are following.
         *
         * @return our parent, or NULL if we're full grown.
         */
        Gardener *getParentToFollow();


        
        /**
         * Tells this gardener to forget its parent.
         */
        void forgetParent();
        

        
        /**
         * Tells this gardener that a given offspring is no longer
         * following it.
         */
        void dropOutsideOffspring( Gardener *inOffspring );

        
        
        /**
         * Feed this gardener.
         *
         * Meant to be called by parents on offspring.
         *
         * @param inRedNutrient, inGreenNutrient, inBlueNutrient the amount
         *   of each nutrient to feed.
         */
        void feed( double inRedNutrient, double inGreenNutrient,
                   double inBlueNutrient );



        /**
         * Sets the gardener that we should follow.
         *
         * @param inLeader the gardener to follow, or NULL to follow no one.
         *   Must be destroyed by caller after this gardener is destroyed.
         */
        void setLeader( Gardener *inLeader );


        
        /**
         * Gets the gardener that we are following.
         *
         * @return our leader, or NULL if we are following no one.
         *  Must be destroyed by caller after this gardener is destroyed.
         */
        Gardener *getLeader();        

        

        /**
         * Add or remove a follower to this gardener.
         *
         * @param inFollower the gardener to add or remove.
         *   Must be destroyed by caller after this gardener is destroyed.
         */
        void addFollower( Gardener *inFollower );
        void dropFollower( Gardener *inFollower );

        
        
        
        /**
         * Passes time for this gardener.
         *
         * @param inTimeDeltaInSeconds the amount of time that has passed.
         */
        void passTime( double inTimeDeltaInSeconds );


        // implements the DrawableObject interface
        void draw( Vector3D *inPosition, double inScale = 1 );

        
        void draw( Vector3D *inPosition, Angle3D *inRotation,
                   double inScale = 1,
                   double inFadeFactor = 1 );

        

        // implements the ObjectStorage interface
        virtual int getStoredObjects( DrawableObject ***outObjects ); 

        virtual int getSelectedObjectIndex();

        virtual void setSelectedObjectIndex( int inIndex );

        

    private:

        Vector3D mDesiredPosition;
        Angle3D mLastDrawAngle;
        
        SingleTextureGL *mBaseTexture;
        SingleTextureGL *mScaleTexture;
        SingleTextureGL *mBonesTexture;
        SingleTextureGL *mSkinTexture;

        SingleTextureGL *mEyeTexture;

        
        // scale parameters relative to gardener center
        SimpleVector<double> mScaleRotations;
        SimpleVector<double> mScaleSizes;
        SimpleVector<Vector3D *> mScalePositions;
        SimpleVector<ScaleCornerColors *> mScaleColors;

        Angle3D mGlobalScaleAngle;
        
        
        Color mColor;


        MusicPart *mMovingMusicPart;
        MusicPart *mMovingHoldingMusicPart;
        MusicPart *mHoldingMusicPart;
        MusicPart *mStillMusicPart;
        
        char mMoving;

        char mCarryingWater;
        double mWaterPickupFraction;
        
        double mRedNutrient;
        double mGreenNutrient;
        double mBlueNutrient;

        char mPoisoned;

        double mLife;
        double mBaseAgeRate;

        
        double mBaseEnergyConsumedPerSecond;
        
        
        char mDead;
        char mGhostMode;
        char mFrozen;
        

        SimpleVector<Gardener *> mOtherGardeners;
        SimpleVector<double> mLikeMetrics;

        Gardener *mMostLiked;
        Gardener *mLeastLiked;
        
        
        double mEmotionDisplay;
        

        char mPlotHidden;

        
        SimpleVector<Storable *> mStoredItems;
        int mSelectedStorageIndex;


        // NULL if not pregnant
        Gardener *mOffspring;
        double mPregnancyProgress;
        

        // for growing outside womb
        Gardener *mParent;
        // 0.5 at birth
        double mGrowthProgress;

        
        SimpleVector<Gardener *> mOutsideOffspring;


        Gardener *mLeader;
        SimpleVector<Gardener *> mFollowers;
        

        double mSecondsSinceLastFruitPoisoning;


        double mNearDeathPoint;
        
        
        void raiseEnergyWithStoredNutrients();


        
        // sets up starting state (common to all constructors)
        void setUpStartingState();

        
        
    };



#endif
