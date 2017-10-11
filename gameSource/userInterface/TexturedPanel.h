/*
 * Modification History
 *
 * 2006-December-18   Jason Rohrer
 * Created.
 */



#ifndef TEXTURED_PANEL_INCLUDED
#define TEXTURED_PANEL_INCLUDED




#include "minorGems/graphics/openGL/gui/GUIPanelGL.h"

#include "minorGems/graphics/openGL/SingleTextureGL.h"

#include "minorGems/math/geometry/Vector3D.h"



/**
 * Extension of GUIPanelGL that has a texture.
 */
class TexturedPanel : public GUIPanelGL {

    public:

        /**
         * Same as constructor for GUIPanelGL, except:
         *
         * @param inVertical true if this is a vertical panel, or false
         *   if horizontal.
         */
        TexturedPanel(
			double inAnchorX, double inAnchorY, double inWidth,
			double inHeight, Color *inColor, char inVertical );


		
		virtual ~TexturedPanel();


		
		// override fireRedraw() in GUIPanelGL
		virtual void fireRedraw();

        
    protected:
        char mVertical;

        
        SingleTextureGL *mBackgroundTexture;
        
    };


#endif
