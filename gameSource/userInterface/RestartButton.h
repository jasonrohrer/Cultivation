/*
 * Modification History
 *
 * 2006-July-6   Jason Rohrer
 * Created.
 */



#ifndef RESTART_BUTTON_INCLUDED
#define RESTART_BUTTON_INCLUDED



#include "ButtonBase.h"


#include "minorGems/graphics/openGL/SingleTextureGL.h"



/**
 * Button that initiates the planting action.
 */
class RestartButton : public ButtonBase {

    public:

        /**
         * See parameter description from BuuttonGL.
         */
        RestartButton( double inAnchorX, double inAnchorY,
                       double inWidth, double inHeight ); 

        virtual ~RestartButton();
        
        
        // implements the ButtonBase interface
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha );

    protected:
        
        SingleTextureGL *mArrowTexture;


    };


#endif

