/*
 * Modification History
 *
 * 2006-July-24   Jason Rohrer
 * Created.
 */



#ifndef GIFT_BUTTON_INCLUDED
#define GIFT_BUTTON_INCLUDED



#include "ButtonBase.h"


#include "minorGems/graphics/openGL/SingleTextureGL.h"



/**
 * Gift button.
 */
class GiftButton : public ButtonBase {

    public:

        /**
         * See parameter description from BuuttonGL.
         */
        GiftButton( double inAnchorX, double inAnchorY,
                    double inWidth, double inHeight ); 

        
        
        virtual ~GiftButton();

        
        
        // implements the ButtonBase interface
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha );


    protected:
        
        SingleTextureGL *mArrowTexture;

    };



#endif

