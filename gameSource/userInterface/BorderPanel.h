/*
 * Modification History
 *
 * 2006-December-27 Jason Rohrer
 * Created.
 */
 
 
#ifndef BORDER_PANEL_INCLUDED
#define BORDER_PANEL_INCLUDED 

#include "minorGems/graphics/openGL/gui/GUIPanelGL.h"

#include <GL/gl.h>


/**
 * A panel with a gray line border that fades near the top.
 *
 * @author Jason Rohrer
 */
class BorderPanel : public GUIPanelGL {


    public:



        /**
         * Constructs a panel.
         *
         * Parameters are same as for GUIPanelGL.
         */
        BorderPanel(
            double inAnchorX, double inAnchorY, double inWidth,
            double inHeight, Color *inColor );


        
        virtual ~BorderPanel();


        
        // override fireRedraw() in GUIPanelGL
        virtual void fireRedraw();

    };



inline BorderPanel::BorderPanel(
    double inAnchorX, double inAnchorY, double inWidth,
    double inHeight, Color *inColor )
    : GUIPanelGL( inAnchorX, inAnchorY, inWidth, inHeight, inColor ) {

    }



inline BorderPanel::~BorderPanel() {
    }

        
        
inline void BorderPanel::fireRedraw() {
    // superclass redraw first
    GUIPanelGL::fireRedraw();

    // draw our border on top
    glBegin( GL_LINE_LOOP ); {
        glColor4f( 0.7, 0.7, 0.7, 1 );
        glVertex2d( mAnchorX, mAnchorY ); 
        glVertex2d( mAnchorX + mWidth, mAnchorY ); 

        glColor4f( 0.7, 0.7, 0.7, 0 );
        glVertex2d( mAnchorX + mWidth, mAnchorY + mHeight );
        glVertex2d( mAnchorX, mAnchorY + mHeight );
        }
    glEnd();
    }



#endif



