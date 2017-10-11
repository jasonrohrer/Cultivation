/*
 * Modification History
 *
 * 2006-December-19   Jason Rohrer
 * Created.
 */


#ifndef NEXT_TUTORIAL_BUTTON_INCLUDED
#define NEXT_TUTORIAL_BUTTON_INCLUDED



#include "RestartButton.h"


/**
 * Button that moves on to the next tutorial page.
 *
 * Inherits from RestartButton to reuse the green arrow.
 */
class NextTutorialButton : public RestartButton {

    public:

        /**
         * See parameter description from BuuttonGL.
         */
        NextTutorialButton( double inAnchorX, double inAnchorY,
                            double inWidth, double inHeight ); 

        
        // implements the ButtonBase interface
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha );

    };


#endif

