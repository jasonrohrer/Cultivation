/*
 * Modification History
 *
 * 2006-July-13   Jason Rohrer
 * Created.
 */



#ifndef TEXT_DISPLAY_INCLUDED
#define TEXT_DISPLAY_INCLUDED



#include "minorGems/graphics/Color.h"

#include "minorGems/graphics/openGL/gui/GUIComponentGL.h"



/**
 * A simple text display that uses glutStrokeCharacter to render text.
 */
class TextDisplay : public GUIComponentGL {

    public:


        /**
		 * Constructs a component.
         *
         * @param inLongestPossibleText the longest possible text
         *   that this display might contain.  Used to compute a constant
         *   scale factor for all texts.
         * @param inColor the color of the text.
         *   Destroyed by caller.
		 * @param inAnchorX the x position of the upper left corner
		 *   of this component.
		 * @param inAnchorY the y position of the upper left corner
		 *   of this component.
		 * @param inWidth the width of this component.
		 * @param inHeight the height of this component.
		 */
		TextDisplay( const char *inLongestPossibleText,
                     Color *inColor,
                     double inAnchorX, double inAnchorY, double inWidth,
                     double inHeight );


        virtual ~TextDisplay();
        

        /**
         * Sets the text to display.
         *
         * @param inText the text.
         */
        void setText( const char *inText );

        

        // overrides function from GUIComponentGL
        virtual void fireRedraw();

        
    protected:

        Color mColor;

        char *mText;

        double mScaleFactor;

        
    };


#endif
