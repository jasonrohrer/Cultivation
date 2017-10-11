/*
 * Modification History
 *
 * 2006-July-24   Jason Rohrer
 * Created.
 */


#include "FlyingObject.h"



FlyingObject::FlyingObject( DrawableObject *inObject,
                            char inDestroyObject,
                            Gardener *inDestinationGardener,
                            Vector3D *inStartPosition,
                            char inLarge )
    : mObject( inObject ),
      mDestinationGardener( inDestinationGardener ),
      mCurrentPosition( new Vector3D( inStartPosition ) ),
      mDestroyObject( inDestroyObject ),
      mLarge( inLarge ) {

    }



FlyingObject::~FlyingObject() {

    if( mDestroyObject ) {
        delete mObject;
        }
    delete mCurrentPosition;
    }



void FlyingObject::reachedDestination() {
    // default:  do nothing
    }






