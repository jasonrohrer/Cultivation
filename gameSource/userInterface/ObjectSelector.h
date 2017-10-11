/*
 * Modification History
 *
 * 2006-July-23   Jason Rohrer
 * Created.
 *
 * 2006-December-17   Jason Rohrer
 * Added support for mouse hover.
 */



#ifndef OBJECT_SELECTOR_INCLUDED
#define OBJECT_SELECTOR_INCLUDED


#include "ObjectStorage.h"


#include "minorGems/graphics/openGL/gui/GUIComponentGL.h"


/**
 * Base class for all game buttons.
 *
 * Handles drawing button background and switching between pressed
 * and unpressed, as well as enabled and disabled, states.
 */
class ObjectSelector : public GUIComponentGL {

    public:


        /**
         * Constructs a Selector that is backed by a storage object.
         *
         *
		 * @param inAnchorX the x position of the upper left corner
		 *   of this component.
		 * @param inAnchorY the y position of the upper left corner
		 *   of this component.
		 * @param inWidth the width of this component.
		 * @param inHeight the height of this component.
         * @paramm inStorage the storage that this Selector displays.
         *   Must be destroyed after this Selector is destroyed.
         */
        ObjectSelector( double inAnchorX, double inAnchorY,
                        double inWidth, double inHeight,
                        ObjectStorage *inStorage );

        

        /**
         * Sets the storage that this selector depicts.
         *
         * @param inStorage the underlying storage.
         *   Must be destroyed after this Selector is destroyed.
         *   Previous storage can be destroyed after it is replaced
         *   with a call to setStorage.
         */
        void setStorage( ObjectStorage *inStorage );

        

        /**
         * Gets the object that the mouse is hovering over.
         *
         * @return the object index in the displayed ObjectStorage, or
         *   -1 if no object is under the mouse.
         */
        int getHoverObject();

        

        // override functions in GUIComponentGL
        virtual void mouseMoved( double inX, double inY );
		virtual void mouseDragged( double inX, double inY );
		virtual void mousePressed( double inX, double inY );
		virtual void mouseReleased( double inX, double inY );
		virtual void fireRedraw();
        
        
    protected:

        ObjectStorage *mStorage;

        int mHoverObjectIndex;
        

        /**
         * Gets the vertical draw units allocated to each object.
         */
        double getDrawUnitsPerObject();

        
    };


#endif
