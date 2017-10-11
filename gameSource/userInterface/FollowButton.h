/*
 * Modification History
 *
 * 2006-October-30   Jason Rohrer
 * Created.
 */



#include "ButtonBase.h"

// inherit the gift button's Arrow icon
#include "GiftButton.h"


// for a red "x" graphic
#include "../emotionIcons.h"



/**
 * Follow button.
 */
class FollowButton : public GiftButton {

    public:

        /**
         * See parameter description from ButtonGL.
         */
        FollowButton( double inAnchorX, double inAnchorY,
                      double inWidth, double inHeight ); 

        

        /**
         * Toggles this button's display of the "stop following" icon.
         *
         * @param inStopFollowing true to switch "stop following" icon on.
         */
        void setStopFollowingMode( char inStopFollowing );

        
        
        /**
         * Sets a second selected object that this button should apply to.
         *
         * @param inObject the object that is selected, or NULL for none.
         *   Destroyed by caller after a different object is set with
         *   a call to setSelectedObject.
         */
        virtual void setSecondSelectedObject( DrawableObject *inObject );

        
        
        // implements the ButtonBase interface
        virtual void drawIcon( Vector3D *inCenter, double inRadius,
                               double inAlpha );


        
    protected:

        DrawableObject *mSecondSelectedObject;

        char mStopFollowingMode;

        DislikeIcon mRedX; 
        
    };


