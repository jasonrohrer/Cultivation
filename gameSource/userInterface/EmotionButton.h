/*
 * Modification History
 *
 * 2006-July-9   Jason Rohrer
 * Created.
 */



#include "ButtonBase.h"



/**
 * Button that initiates the planting action.
 */
class EmotionButton : public ButtonBase {

    public:

        /**
         * See parameter description from BuuttonGL.
         */
        EmotionButton( double inAnchorX, double inAnchorY,
                       double inWidth, double inHeight ); 

        
        // implements the ButtonBase interface
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha );
    };


