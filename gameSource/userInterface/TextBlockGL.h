/*
 * Modification History
 *
 * 2006-December-19    Jason Rohrer
 * Created.
 */
 
 
#ifndef TEXT_BLOCK_GL_INCLUDED
#define TEXT_BLOCK_GL_INCLUDED 

#include "minorGems/graphics/openGL/gui/LabelGL.h"



/**
 * A multi-line text label for OpenGL-based GUIs.
 *
 * @author Jason Rohrer
 */
class TextBlockGL : public LabelGL {


    public:



        /**
         * Constructs a TextBlock.
         *
         * Parameters are same as for LabelGL constructor, except:
         *
         * @param inMaxCharactersPerLine the maximum number of characters
         *   per line of the text block.
         */
        TextBlockGL(
            double inAnchorX, double inAnchorY, double inWidth,
            double inHeight, char *inString, TextGL *inText,
            int inMaxCharactersPerLine );

        
        // override fireRedraw in LabelGL
        virtual void fireRedraw();

        
    protected:

        unsigned long mMaxCharactersPerLine;

    };



#endif



