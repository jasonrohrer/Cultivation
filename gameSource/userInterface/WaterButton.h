/*
 * Modification History
 *
 * 2006-July-5   Jason Rohrer
 * Created.
 */



#include "ButtonBase.h"



/**
 * Button that initiates the planting action.
 */
class WaterButton : public ButtonBase {

    public:

        /**
         * See parameter description from BuuttonGL.
         */
        WaterButton( double inAnchorX, double inAnchorY,
                     double inWidth, double inHeight ); 

        

        /**
         * Sets the full status of this button.
         *
         * @param inFullStatus true to set to full.
         */
        void setFull( char inFullStatus );

        
        
        // implements the ButtonBase interface
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha );

    protected:

        char mFullStatus;
    };


