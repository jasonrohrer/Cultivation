/*
 * Modification History
 *
 * 2006-July-4   Jason Rohrer
 * Created.
 *
 * 2006-July-5   Jason Rohrer
 * Changed to subclass ButtonBase.
 */



#include "ButtonBase.h"



/**
 * Button that initiates the planting action.
 */
class PlantButton : public ButtonBase {

    public:

        /**
         * See parameter description from BuuttonGL.
         */
        PlantButton( double inAnchorX, double inAnchorY,
                     double inWidth, double inHeight ); 

        
        // implements the ButtonBase interface
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha );
    };


