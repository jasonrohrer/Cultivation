/*
 * Modification History
 *
 * 2006-July-5   Jason Rohrer
 * Created.
 */



#ifndef BUTTON_BASE_INCLUDED
#define BUTTON_BASE_INCLUDED


#include "../DrawableObject.h"


#include "minorGems/graphics/openGL/gui/ButtonGL.h"

#include "minorGems/graphics/openGL/SingleTextureGL.h"

#include "minorGems/math/geometry/Vector3D.h"



/**
 * Base class for all game buttons.
 *
 * Handles drawing button background and switching between pressed
 * and unpressed, as well as enabled and disabled, states.
 */
class ButtonBase : public ButtonGL {

    public:

        virtual ~ButtonBase();


        
        /**
         * Sets the selected object that this button should apply to.
         *
         * @param inObject the object that is selected, or NULL for none.
         *   Destroyed by caller after a different object is set with
         *   a call to setSelectedObject.
         */
        virtual void setSelectedObject( DrawableObject *inObject );
        
         
        
        // implements the ButtonGL interface
        virtual void drawPressed();
        virtual void drawUnpressed();

        
        /**
         * Draw the icon on this button.
         *
         * @param inCenter the center of the icon.
         *   Destroyed by caller.
         * @param inRadius the radius of the icon.
         * @param inAlpha the alpha factor for the icon.
         */
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha ) = 0;


        
    protected:

        /**
         * See parameter description from ButtonGL.
         *
         * Only should be called by subclass constructor.
         *
         * Additional parameter:
         *
         * @param inFadeLeft true to fade on left side, or false
         *   to fade on bottom.  Defaults to false.
         */
        ButtonBase( double inAnchorX, double inAnchorY,
                    double inWidth, double inHeight,
                    char inFadeLeft = false );

        

        /**
         * Draws this button with a given background color.
         *
         * @param inRed, inGreen, inBlue the background color.
         */
        void drawWithColor( float inRed, float inGreen, float inBlue );

        

        char mFadeLeft;
        
        Vector3D mIconCenter;
        double mIconRadius;

        DrawableObject *mSelectedObject;
        
    };


#endif
