/*
 * Modification History
 *
 * 2006-July-6   Jason Rohrer
 * Created.
 */



#include "ButtonBase.h"

#include "minorGems/graphics/openGL/SingleTextureGL.h"



/**
 * Button that initiates the planting action.
 */
class EatButton : public ButtonBase {

    public:

        /**
         * See parameter description from BuuttonGL.
         */
        EatButton( double inAnchorX, double inAnchorY,
                   double inWidth, double inHeight ); 


        virtual ~EatButton();

        
        
        // implements the ButtonBase interface
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha );

    protected:

        SingleTextureGL *mMouthTexture;
        
    };


