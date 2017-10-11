/*
 * Modification History
 *
 * 2006-August-14   Jason Rohrer
 * Created.
 */



#include "ButtonBase.h"



/**
 * Mate button.
 */
class MateButton : public ButtonBase {

    public:

        /**
         * See parameter description from BuuttonGL.
         */
        MateButton( double inAnchorX, double inAnchorY,
                    double inWidth, double inHeight ); 


        
        /**
         * Sets a second selected object that this button should apply to.
         *
         * @param inObject the object that is selected, or NULL for none.
         *   Destroyed by caller after a different object is set with
         *   a call to setSelectedObject.
         */
        virtual void setSecondSelectedObject( DrawableObject *inObject );

        
        
        // implements the ButtonBase interface
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha );


        
    protected:

        DrawableObject *mSecondSelectedObject;
    };


