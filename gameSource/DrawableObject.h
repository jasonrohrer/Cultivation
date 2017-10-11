/*
 * Modification History
 *
 * 2006-July-23   Jason Rohrer
 * Created.
 *
 * 2006-September-17   Jason Rohrer
 * Added a virtual destructor to fix some memory leaks.
 */



#ifndef DRAWABLE_OBJECT_INCLUDED
#define DRAWABLE_OBJECT_INCLUDED



#include "minorGems/math/geometry/Vector3D.h"



/**
 * Interface for drawable objects.
 *
 * @author Jason Rohrer
 */
class DrawableObject {

    public:

        /**
         * Draw this object in the current OpenGL context.
         *
         * @param inPosition the position to draw this object at.
         * @param inScale the scale factor for this object.
         *   Defaults to 1.
         */
        virtual void draw( Vector3D *inPosition, double inScale = 1 ) = 0;


        
        // virtual destructor to ensure proper destruction of subclasses
        virtual ~DrawableObject() {
            };

        
    };



#endif
