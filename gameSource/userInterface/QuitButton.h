/*
 * Modification History
 *
 * 2006-July-10   Jason Rohrer
 * Created.
 */



#include "ButtonBase.h"



/**
 * Button that initiates the planting action.
 */
class QuitButton : public ButtonBase {

    public:

        /**
         * See parameter description from BuuttonGL.
         */
        QuitButton( double inAnchorX, double inAnchorY,
                    double inWidth, double inHeight ); 

        
        // implements the ButtonBase interface
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha );
    };


