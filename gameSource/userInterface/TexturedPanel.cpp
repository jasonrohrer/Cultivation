/*
 * Modification History
 *
 * 2006-December-18   Jason Rohrer
 * Created.
 */


#include "TexturedPanel.h"

#include "minorGems/graphics/filters/BoxBlurFilter.h"

#include "minorGems/util/random/StdRandomSource.h"

extern StdRandomSource globalRandomSource;



TexturedPanel::TexturedPanel(
    double inAnchorX, double inAnchorY, double inWidth,
    double inHeight, Color *inColor, char inVertical )
    :GUIPanelGL( inAnchorX, inAnchorY, inWidth, inHeight, inColor ),
     mVertical( inVertical ) {


    // generate texture 
    int textureSize = 64;
    
    Image *textureImage = new Image( textureSize, textureSize, 4, false );

    double *channels[4];

    int pixelsPerChannel = textureSize * textureSize;
    
    int i;
    int p;
    for( i=0; i<4; i++ ) {
        channels[i] = textureImage->getChannel( i );
        }
    

    // for now, fill randomly

    double lowColorValue = 0.2;
    double lowAlphaValue = 0.2;
    for( p=0; p<pixelsPerChannel; p++ ) {
        
        channels[3][p] =
            globalRandomSource.getRandomBoundedDouble( lowAlphaValue, 1 );
        channels[0][p] =
            globalRandomSource.getRandomBoundedDouble( lowColorValue, 1 );
        channels[1][p] =
            globalRandomSource.getRandomBoundedDouble( lowColorValue, 1 );
        channels[2][p] =
            globalRandomSource.getRandomBoundedDouble( lowColorValue, 1 );
        }
    
    
    // blur all channels
    int blurRadius = 1;
    BoxBlurFilter blur( blurRadius );
    textureImage->filter( &blur );
    
    mBackgroundTexture = new SingleTextureGL( textureImage,
                                              // wrap
                                              true );

    delete textureImage;

    // set filter for texture to nearest neighbor
    mBackgroundTexture->enable();
    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    mBackgroundTexture->disable();
    }


		
TexturedPanel::~TexturedPanel() {

    delete mBackgroundTexture;
    }


		
void TexturedPanel::fireRedraw() {
    double alphaBottomRight, alphaTopRight, alphaBottomLeft, alphaTopLeft;

    double xTextureAnchorMax, yTextureAnchorMax;
    if( mVertical ) {
        alphaBottomRight = 0;
        alphaTopRight = 0;

        alphaBottomLeft = 1;
        alphaTopLeft = 1;

        xTextureAnchorMax = mWidth / mHeight;
        yTextureAnchorMax = 1;
        }
    else {
        alphaBottomRight = 1;
        alphaTopRight = 0;

        alphaBottomLeft = 1;
        alphaTopLeft = 0;

        xTextureAnchorMax = 1;
        yTextureAnchorMax = mHeight / mWidth;
        }
    

    mBackgroundTexture->enable();
    
	glBegin( GL_QUADS ); {
        double widthToSkip = 0;

        if( !mVertical ) {
            // don't draw in left corner to leave room for special block
            widthToSkip = mHeight;
            }
        double widthToSkipFraction = widthToSkip / mWidth;

        
        glColor4f( 1, 1, 1, alphaBottomLeft ); 
        glTexCoord2f( widthToSkipFraction, 0 );
		glVertex2d( mAnchorX + widthToSkip, mAnchorY ); 

        glColor4f( 1, 1, 1, alphaBottomRight ); 
        glTexCoord2f( xTextureAnchorMax, 0 );
        glVertex2d( mAnchorX + mWidth, mAnchorY ); 

        glColor4f( 1, 1, 1, alphaTopRight ); 
        glTexCoord2f( xTextureAnchorMax, yTextureAnchorMax );
        glVertex2d( mAnchorX + mWidth, mAnchorY + mHeight );

        glColor4f( 1, 1, 1, alphaTopLeft ); 
        glTexCoord2f( widthToSkipFraction, yTextureAnchorMax );
        glVertex2d( mAnchorX + widthToSkip, mAnchorY + mHeight );
        

        if( !mVertical ) {
            // special block in lower-left corner

            // so that this panel blends with the left sidebar panel

            // a square block, mHeight by mHeight
            
            double widthFraction = mHeight / mWidth;
            

            glColor4f( 1, 1, 1, alphaBottomLeft ); 
            glTexCoord2f( 0, 0 );
            glVertex2d( mAnchorX, mAnchorY ); 

            glColor4f( 1, 1, 1, alphaBottomRight ); 
            glTexCoord2f( xTextureAnchorMax * widthFraction, 0 );
            glVertex2d( mAnchorX + mHeight, mAnchorY ); 

            glColor4f( 1, 1, 1, alphaTopRight ); 
            glTexCoord2f( xTextureAnchorMax * widthFraction,
                          yTextureAnchorMax );
            glVertex2d( mAnchorX + mHeight, mAnchorY + mHeight );

            glColor4f( 1, 1, 1, 1 ); 
            glTexCoord2f( 0, yTextureAnchorMax );
            glVertex2d( mAnchorX, mAnchorY + mHeight );
            }


        }
	glEnd();


    

    mBackgroundTexture->disable();
    
	// call the supercalss redraw
	GUIContainerGL::fireRedraw();
    }
