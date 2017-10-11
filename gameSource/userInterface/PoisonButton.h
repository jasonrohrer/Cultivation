/*
 * Modification History
 *
 * 2006-September-19   Jason Rohrer
 * Created.
 */



#include "ButtonBase.h"



/**
 * Button that initiates the planting action.
 */
class PoisonButton : public ButtonBase {

    public:

        /**
         * See parameter description from BuuttonGL.
         */
        PoisonButton( double inAnchorX, double inAnchorY,
                      double inWidth, double inHeight ); 

        
        
        // implements the ButtonBase interface
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha );

    };


