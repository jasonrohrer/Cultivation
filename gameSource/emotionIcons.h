/*
 * Modification History
 *
 * 2006-July-25   Jason Rohrer
 * Created.
 */



#ifndef EMOTION_ICONS_INCLUDED
#define EMOTION_ICONS_INCLUDED


/**
 * A collection of DrawableObject classes for emotional transactions.
 */

#include "DrawableObject.h"



class DislikeIcon : public DrawableObject {

    public:
        // implements DrawableObject
        void draw( Vector3D *inPosition, double inScale = 1 );
    };



class LikeIcon : public DrawableObject {

    public:
        // implements DrawableObject
        void draw( Vector3D *inPosition, double inScale = 1 );
    };



#endif
