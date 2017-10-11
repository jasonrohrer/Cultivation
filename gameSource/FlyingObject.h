/*
 * Modification History
 *
 * 2006-July-24   Jason Rohrer
 * Created.
 */



#ifndef FLYING_OBJECT_INCLUDED
#define FLYING_OBJECT_INCLUDED



#include "minorGems/math/geometry/Vector3D.h"

#include "DrawableObject.h"
#include "Gardener.h"



/**
 * Wrapper class for objects that fly toward a gardener.
 *
 * @author Jason Rohrer
 */
class FlyingObject {

    public:

        /**
         * Constructs a wrapper.
         *
         * @param inObject the object to draw.
         * @param inDestroyObject true if this class should handle
         *   destruction of inObject.
         * @param inDestinationGardener where the object is flying to.
         *   Destroyed by caller after this class destroyed.
         * @param inStartPosition the start position of the object.
         *   Destroyed by caller.
         * @param inLarge set to true to make icon larger than normal.
         *  Defaults to false.
         */
        FlyingObject( DrawableObject *inObject,
                      char inDestroyObject,
                      Gardener *inDestinationGardener,
                      Vector3D *inStartPosition,
                      char inLarge = false );

        virtual ~FlyingObject();

        

        /**
         * Hook to call when this object reaches its destination.
         *
         * Should be overridden by sub classes to implement custom
         * destination behavior.
         */
        virtual void reachedDestination();

        
        
        DrawableObject *mObject;
        Gardener *mDestinationGardener;
        Vector3D *mCurrentPosition;

        

        /**
         * Gets whether icon should be drawn large.
         *
         * @return true if icon should be drawn large.
         */
        char isLarge() {
            return mLarge;
            }

        
        
    protected:
        char mDestroyObject;
        char mLarge;
    };



#endif
