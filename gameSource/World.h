/*
 * Modification History
 *
 * 2006-July-2   Jason Rohrer
 * Created.
 *
 * 2006-September-12   Jason Rohrer
 * Fixed the machine-gun mating bug.
 *
 * 2006-October-27   Jason Rohrer
 * Added portal.
 *
 * 2006-December-25   Jason Rohrer
 * Added function for checking if portal is open.
 * Added function for checking closed status of portal.
 * Changed so that gardeners get angry if you mate with their most liked.
 *
 * 2007-November-17   Jason Rohrer
 * Added ignoreGardener function to fix bug in ghost mode.
 */



#ifndef WORLD_INCLUDED
#define WORLD_INCLUDED



#include "Plant.h"
#include "Gardener.h"
#include "Portal.h"
#include "FlyingObject.h"
#include "SoilMap.h"
#include "sound/MusicPart.h"

#include "minorGems/util/SimpleVector.h"
#include "minorGems/math/geometry/Vector3D.h"

#include "minorGems/graphics/openGL/SingleTextureGL.h"




/**
 * Class that represents the game world and contains all objects.
 *
 * @author Jason Rohrer
 */
class World {

    public:


        /**
         * Constructs a world.
         *
         * @param inCornerA, inCornerB the corners defining the
         *   boundary of the world.
         *   Destroyed by caller.
         */
        World( Vector3D *inCornerA, Vector3D *inCornerB );
        
        ~World();
        

        
        /**
         * Adds a plant to the world.
         *
         * @param inGardener the gardener adding the plant.
         *   Must already be managed by this world.
         * @param inPlant the plant.
         *   Destroyed by this world.
         * @param inPosition the position to add the plant at.
         *   Destroyed by caller.
         */
        void addPlant( Gardener *inGardener,
                       Plant *inPlant, Vector3D *inPosition );

        

        /**
         * Adds a gardener to the world.
         *
         * @param inGardener the gardener.
         *   Destroyed by this world.
         * @param inPosition the position to add the gardener at.
         *   Destroyed by caller.
         * @param inRotation the starting rotation, or NULL for default.
         *   Destroyed by caller.
         */
        void addGardener( Gardener *inGardener, Vector3D *inPosition,
                          Angle3D *inRotation = NULL );


        
        /**
         * Removes a gardener from this world and destroys it.
         *
         * @param inGardener the gardener.  Must be managed by this world.
         *   Destroyed by this world.
         */
        void removeGardener( Gardener *inGardener );



        /**
         * Clears dependencies on gardener without fully removing it.
         * Other gardeners will henceforth act like this gardener does not
         * exist.
         *
         * @param inGardener the gardener.  Must be managed by this world.
         *   Destroyed by this world.
         */
        void ignoreGardener( Gardener *inGardener );

        

        /**
         * Gets the position of a gardener.
         *
         * @param inGardener the gardener in this world to get the position
         *   of.  Will be destroyed by this world if this gardener is
         *   already being managed by this world.
         *
         * @return the position, or NULL if inGardener is not managed
         *   by this world.  Destroyed by caller.
         */
        Vector3D *getGardenerPosition( Gardener *inGardener );

        

        /**
         * Get the positions of all gardeners in the world.
         *
         * @param outNumGardeners pointer to where the number of gardeners
         *   should be returned.
         *
         * @return an array of positions.  Array and contained positions
         *   must be destroyed by caller.
         */
        Vector3D **getAllGardenerPositions( int *outNumGardeners );



        /**
         * Gets all gardeners in the world.
         *
         * @param outNumGardeners pointer to where number of gardeners
         *   should be returned.
         *
         * @return an array of gardeners.  Array destroyed by caller, gardeners
         *   managed by world.
         */
        Gardener **getAllGardeners( int *outNumGardeners );

        

        /**
         * Get the music parts for all gardeners in the world.
         *
         * @param outNumGardeners pointer to where the number of gardeners
         *   should be returned.
         *
         * @return an array of music parts.  Array must be destroyed by caller.
         *   Music parts are managed by this world.
         */
        MusicPart **getAllGardenerMusicParts( int *outNumGardeners );


        
        /**
         * Gets a volume modifier for each music part.
         * Based on life of each gardener (grow quite near death).
         *
         * @param outNumGardeners pointer to where the number of gardeners
         *   should be returned.
         *
         * @return an array of values in the range [0,1].  Array destroyed
         *   by caller.
         */
        double *getAllGardenerMusicVolumeModifiers( int *outNumGardeners );

        

        /**
         * Gets the position of a plant.
         *
         * @param inPlant the plant in this world to get the position
         *   of.  Will be destroyed by this world if this plant is
         *   already being managed by this world.
         *
         * @return the position, or NULL if inPlant is not managed
         *   by this world.  Destroyed by caller.
         */
        Vector3D *getPlantPosition( Plant *inPlant );

        
        
        /**
         * Sets the plot for a gardener.
         *
         * @param inGardener the gardener to set the plot for.
         *   Must be a gardener already managed by this world.
         * @param inCornerA, inCornerB corners that define a
         *   rectangular plot, or NULL to set no plot.
         *   Destroyed by caller.
         */
        void setGardenerPlot( Gardener *inGardener,
                              Vector3D *inCornerA, Vector3D *inCornerB );

        

        /**
         * Gets the plot of a gardener.
         *
         * @param inGardener the gardener to get the plot for.
         * @param inCornerA, inCornerB pointers to where the corner
         *   vectors should be returned.  Will be set to NULL if
         *   inGardener doesn't have a plot.
         *   Resulting vectors destroyed by caller.
         */
        void getGardenerPlot( Gardener *inGardener,
                              Vector3D **inCornerA, Vector3D **inCornerB );


        
        /**
         * Gets the area of a gardener's plot.
         *
         * @param inGardener the gardener to get the plot area for.
         *
         * @return the area.
         */
        double getPlotArea( Gardener *inGardener );


        
        /**
         * Gets the area of the intersection between gardener plots.
         *
         * @param inGardenerA the first gardener.
         * @param inGardenerB the second gardener.
         *
         * @return the area of the plot intersection.
         */
        double getPlotIntersectionArea( Gardener *inGardenerA,
                                        Gardener *inGardenerB );
        

        
        /**
         * Gets the gardener closest to a given gardener.
         *
         * Both parameter and result are managed by world.
         *
         * @param inGardener the gardener to check.
         *
         * @return the closest gardener, or NULL if there
         *  are no other gardeners.
         */
        Gardener *getClosestGardener( Gardener *inGardener );


        

        /**
         * Gets the distance between gardeners.
         *
         * @param inGardenerA, inGardenerB the gardeners.  Managed by world.
         *
         * @return the distance between them.
         */
        double getGardenerDistance( Gardener *inGardenerA,
                                    Gardener *inGardenerB );

        

        /**
         * Gets the plant closest to a given plant.
         *
         * Both parameter and result are managed by world.
         *
         * @param inPlant the plant to check.
         *
         * @return the closest plant, or NULL if there
         *  are no other plants.
         */
        Plant *getClosestPlant( Plant *inPlant );



        /**
         * Gets the plant closest to a given position.
         *
         * @param inPosition the position to check.  Destroyed by caller.
         * @param inPlantToIgnore a plant to ignore, or NULL to consider all.
         *   Plant managed by this world.  Defaults to NULL.
         * @param inGetEvenIfTooFar true to get the closest plant
         *   to inPosition even if it is "too" far away from it.
         *   If set to false, this function will only return a plant
         *   if inPosition is "close enough" to one.
         *   Defaults to true.
         * @param inGetEvenIfPoisoned true to get the closest plant
         *   to inPosition even if it is poisoned.
         *   Defaults to false.
         *
         * @return the closest plant, or NULL if there
         *  are no other plants.  Managed by this world.
         */
        Plant *getClosestPlant( Vector3D *inPosition,
                                Plant *inPlantToIgnore = NULL,
                                char inGetEvenIfTooFar = true,
                                char inGetEvenIfPoisoned = false );

        

        /**
         * Gets whether there is room for a plant at a given position.
         *
         * @param inPosition the position.  Destroyed by caller.
         *
         * @return true if can plant.
         */         
        char canPlant( Vector3D *inPosition );

        

        /**
         * Gets the closest water to a gardener.
         *
         * @param inGardener the gardener to check.  Managed by this world.
         *
         * @return the position of the closest water point.
         *   Resulting vector destroyed by caller.
         */
        Vector3D *getClosestWater( Gardener *inGardener );

        

        /**
         * Gets the closest land point to a given position.
         *
         * @param inPosition the position to check.  Destroyed by caller.
         *
         * @return the closest land point.  Destroyed by caller.
         */
        Vector3D *getClosestLand( Vector3D *inPosition );


        
        /**
         * Gets a random point on the land of the world.
         *
         * @return a point.  Destroyed by caller.
         */
        Vector3D *getRandomLandPoint();
        


        /**
         * Gets the center of a gardener's plot.
         *
         * @param inGardener the gardener to get the plot for.
         * @return the position of the plot center, or NULL if gardener has
         *   no plot.
         *   Resulting vector destroyed by caller.
         */
        Vector3D *getPlotCenter( Gardener *inGardener );

        

        /**
         * Gets whether a garderner is in its plot.
         *
         * @param inGardener the gardener to check.
         *   Must be a gardener already managed by this world.
         *
         * @return true if inGardener is in its plot.
         */
        char isInPlot( Gardener *inGardener );


        
        /**
         * Gets whether a position is in a gardener's plot.
         *
         * @param inGardener the gardener to check.
         *   Must be a gardener already managed by this world.
         * @param inPosition the position to check.
         *   Destroyed by caller.
         *
         * @return true if inPosition is in inGardener's plot.
         */
        char isInPlot( Gardener *inGardener, Vector3D *inPosition );


        
        /**
         * Gets whether a garderner is in water.
         *
         * @param inGardener the gardener to check.
         *   Must be a gardener already managed by this world.
         *
         * @return true if inGardener is in water.
         */
        char isInWater( Gardener *inGardener );



        /**
         * Gets whether a point is in water.
         *
         * @param inPosition the position to check.
         *   destroyed by caller.
         *
         * @return true if inPosition is in water.
         */
        char isInWater( Vector3D *inPosition );

        

        /**
         * Gets the closest plant in this gardener's plot.
         *
         * @param inGardener the gardener to check.
         *   Must be a gardener already managed by this world.
         *
         * @param inGetEvenIfTooFar true to get the closest plant
         *   to inGardener even if inGardener is "too" far away from it.
         *   If set to false, this function will only return a plot plant
         *   if inGardener is "close enough" to one.
         * @param inGetOnlyRipe true to only consider ripe plants.
         *
         * @return the closest plant, or NULL if no plant is close
         *   enough.  Destroyed by this world.
         */
        Plant *getClosestPlotPlant( Gardener *inGardener,
                                    char inGetEvenIfTooFar = false,
                                    char inGetOnlyRipe = false );

        

        /**
         * Gets the plants in a gardener's plot.
         *
         * @param inGardener the gardener.
         *   Must be a gardener already managed by this world.
         *
         * @return a vector of plants.
         *   Vector must be destroyed by caller.  Plants will be
         *   destroyed by this world.
         */
        SimpleVector<Plant*> *getPlotPlants( Gardener *inGardener );


        
        /**
         * Gets a plant last tended by a gardener.
         *
         * @param inGardener the gardener to look for.
         *   Managed by this world.
         *
         * @return a plant last tended by inGardener, or NULL if all
         *   plants were last tended by other gardeners.
         *   Managed by this world.
         */
        Plant *getTendedPlant( Gardener *inGardener );

        

        /**
         * Causes a gardener to harvest a plant, if the gardener is close
         * enough, and if the plant is in the gardener's plot.
         *
         * @param inGardener the gardener.
         * @param inPlant the plant to harvest.
         *
         * Both parameters must already be managed by this world and
         * will be destroyed by this world.
         * If inPlant successfully harvested, it will be destroyed by this
         * call.
         */
        void harvestPlant( Gardener *inGardener, Plant *inPlant );


        
        /**
         * Dumps water on closest plant in gardener's plot.
         *
         * @param inGardener the gardener to dump water from.
         *   Must be a gardener already managed by this world.
         */
        void dumpWater( Gardener *inGardener );



        /**
         * Poisons the closest plant.
         *
         * @param inGardener the gardener to send poison from.
         *   Must be managed by this world.
         */
        void dumpPoison( Gardener *inGardener );


        
        /**
         * Removes a plant from this world.
         *
         * @param inPlant the plant to remove.  Must be managed by this world.
         *   Destroyed by this world.
         */
        void removePlant( Plant *inPlant );



        /**
         * Gives a fruit from one gardener to another.
         *
         * @param inGiver the gardener giving the fruit.  Must be managed
         *   by this world.
         * @param inReceiver the gardener receiving the fruit.  Must be managed
         *   by this world.
         * @param inFruit the fruit to give.
         *   Will be destroyed by this world.
         */
        void giveFruit( Gardener *inGiver, Gardener *inReceiver,
                        Fruit *inFruit );

        

        /**
         * Flies an emotion icon between two gardeners.
         *
         * @param inSource, inTarget the gardeners to pass icon between.
         * @param inIcon the icon to draw.  Will be destroyed when it
         *  reaches the inTarget.
         * @param inLarge set to true to make icon larger than normal.
         *  Defaults to false.
         */
        void flyEmotionIcon( Gardener *inSource, Gardener *inTarget,
                             DrawableObject *inIcon,
                             char inLarge = false );

        
        
        /**
         * Gets the soil condition at a given location in the world.
         *
         * @param inPosition the position to test.
         *   Destroyed by caller.
         *
         * @return a soil type value in the range [0,1].
         */
        double getSoilCondition( Vector3D *inPosition ) {
            return mSoilMap.getSoilCondition( inPosition );
            }

        

        /**
         * Mates two gardeners.
         *
         * @param inParentA, inParentB the gardeners to mate.  Managed by
         *   this world.
         */
        void mateGardeners( Gardener *inParentA, Gardener *inParentB );

        

        /**
         * Gets whether a gardener is about to get pregnant.
         *
         * @param inGardener the gardener to check.  Managed by this world.
         *
         * @return true if a pregnancy is flying toward this gardener.
         */
        char isTargetOfPregnancy( Gardener *inGardener );

        

        /**
         * Turns off the target status for a gardener.
         *
         * @param inGardener the gardener to turn off target status for.
         *   Managed by this world.
         */
        void cancelTargetOfPregnancy( Gardener *inGardener );

        

        /**
         * Gets the next gardener that the user can control
         *
         * @param inGardenerToSkip a gardener to ignore when looking, or
         *   NULL to consider all gardeners.  Defaults to NULL.
         *   
         * @return a gardener, or NULL if there are no controllable gardeners
         *   left.  Managed by this world.
         */
        Gardener *getNextUserControllableGardener(
            Gardener *inGardenerToSkip = NULL );


        
        /**
         * Sets a position to highlight.
         *
         * @param inPosition the position, or NULL to clear the highlight.
         *   Destroyed by caller.
         */
        void setHighlightPosition( Vector3D *inPosition );



        /**
         * Augments a nearby portal, or starts a new one.
         *
         * @param inPosition the position near which to augment a portal.
         * @param inAugmenter the gardener that augmented the portal.
         */
        void augmentPortal( Vector3D *inPosition, Gardener *inAugmenter  );



        /**
         * Checks if portal is open.
         *
         * @return true if open.
         */
        char isPortalOpen();



        /**
         * Checks if portal is closed.
         *
         * @return true if closed and gardeners have been sent.
         */
        char isPortalClosed();

        
        
        /**
         * Passes time in this world.
         *
         * @param inTimeDeltaInSeconds the amount of time that has passed.
         */
        void passTime( double inTimeDeltaInSeconds );


        
        /**
         * Draw this world and all of its objects into the current
         * OpenGL context.
         */
        void draw();


        
    private:

        Vector3D mCornerA, mCornerB;
        SoilMap mSoilMap;
        
        SingleTextureGL *mCloudTexture;

        double mCloudPosition;
        double mCloudMotionRate;
        
        double mGardenerVelocity;
        double mGardenerRotationVelocity;
        
        
        SimpleVector<Plant *> mPlants;
        SimpleVector<Vector3D *> mPlantPositions;
        // what gardener last tended each plant
        SimpleVector<Gardener *> mLastPlantTender;

        
        SimpleVector<Gardener *> mGardeners;
        SimpleVector<Vector3D *> mGardenerPositions;
        SimpleVector<Angle3D *> mGardenerRotations;
        SimpleVector<char> mGardenerRotationsComplete;
        SimpleVector<Vector3D *> mGardenerDestinationForRotation;
        
        SimpleVector<Vector3D *> mGardenerPlotsCornerA;
        SimpleVector<Vector3D *> mGardenerPlotsCornerB;

        
        Vector3D mPortalPosition;
        Portal *mPortal;
        
        
        // track if mating flying object is in motion that
        // is targetting a given gardener (not pregnant yet, but almost)
        SimpleVector<char> mGardenerTargetOfPregnancy;
        
        SimpleVector<FlyingObject *> mFlyingObjects;


        // how close can two plants be?
        double mMinPlantingDistance;

        // how far apart two objects can be and still count as close
        double mMaxDistanceThatCountsAsClose;

        // z position of gardeners
        double mGardenerZ;

        
        Vector3D *mHighlightPosition;

        
        
        /**
         * Tests whether a position is inside a rectangle.
         *
         * All params destroyed by caller.
         *
         * @param inPosition the position to test.
         * @param inCornerA, inCornerB the corners of the rectangle.
         */
        char isInRectangle( Vector3D *inPosition,
                            Vector3D *inCornerA, Vector3D *inCornerB );

        

        /**
         * Gets the intersection of two ranges.
         *
         * @param inStartA, inEndA the first range.
         * @param inStartB, inEndB the second range.
         * @param outStart, outEnd pointers to where resulting intersection
         *   range should be returned.
         *
         * @return true if they intersect.
         */
        char getRangeIntersection( double inStartA, double inEndA,
                                   double inStartB, double inEndB,
                                   double *outStart, double *outEnd );


        
        /**
         * Gets the intersection of two rectangles.
         *
         * All params and result destroyed by caller.
         *
         * @param inFirstCornerA, inFirstCornerB corners of first rectangle.
         * @param inSecondCornerA, inSecondCornerB corners of first rectangle.
         * @param outCornerA, outCornerB pointers to where corners of
         *   intersection should be returned.  Will be set to NULL if there
         *   is no intersection.
         */ 
        void getRectangleIntersection( Vector3D *inFirstCornerA,
                                       Vector3D *inFirstCornerB,
                                       Vector3D *inSecondCornerA,
                                       Vector3D *inSecondCornerB,
                                       Vector3D **outCornerA,
                                       Vector3D **outCornerB );

        

        /**
         * Gets area of a a rectangle.
         *
         * All params destroyed by caller.
         *
         * @param inCornerA, inCornerB the corners of the rectangle.
         *
         * @return the area.
         */
        double getRectangleArea( Vector3D *inCornerA,
                                 Vector3D *inCornerB );
        

        
        /**
         * Gets center of a a rectangle.
         *
         * All params and result destroyed by caller.
         *
         * @param inCornerA, inCornerB the corners of the rectangle.
         *
         * @return the center.
         */
        Vector3D *getRectangleCenter( Vector3D *inCornerA,
                                      Vector3D *inCornerB );

        
        
        /**
         * Removes a gardener from this world and destroys it.
         *
         * @param inGardenerIndex the index of the gardener in our vector.
         */
        void removeGardener( int inIndex );



        /**
         * Clears dependencies on gardener without fully removing it.
         * Other gardeners will henceforth act like this gardener does not
         * exist.
         *
         * @param inGardenerIndex the index of the gardener in our vector.
         */
        void ignoreGardener( int inIndex );



        /**
         * Anger other gardeners about an action that takes place in
         * a given position.  Other gardener only angered if position
         * is in that gardener's plot.
         *
         * @param inGardener the gardener doing the action.
         *   If managed by this world, destroyed by this world.
         * @param inPosition the position of the action.
         *   Destroyed by caller.
         */
        void angerOthers( Gardener *inGardener, Vector3D *inPosition );


        
        
        
        
    };



#endif
