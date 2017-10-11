/*
 * Modification History
 *
 * 2006-August-15   Jason Rohrer
 * Created.
 */



#include "ButtonBase.h"

#include "../DrawableObject.h"



/**
 * Button that initiates the planting action.
 */
class DiscardButton : public ButtonBase {

    public:

        /**
         * See parameter description from BuuttonGL.
         */
        DiscardButton( double inAnchorX, double inAnchorY,
                       double inWidth, double inHeight ); 

        
        
        // implements the ButtonBase interface
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha );
        
    };


