/*
 * Modification History
 *
 * 2006-December-14   Jason Rohrer
 * Created.
 */



#include "ButtonBase.h"


#include "minorGems/graphics/openGL/SingleTextureGL.h"



/**
 * Button that initiates the planting action.
 */
class PauseButton : public ButtonBase {

    public:

        /**
         * See parameter description from BuuttonGL.
         */
        PauseButton( double inAnchorX, double inAnchorY,
                     double inWidth, double inHeight ); 

        virtual ~PauseButton();
        
        
        // implements the ButtonBase interface
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha );

    protected:
        
        SingleTextureGL *mPauseTexture;


    };


