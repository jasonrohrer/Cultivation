/*
 * Modification History
 *
 * 2006-July-23   Jason Rohrer
 * Created.
 *
 * 2006-December-17   Jason Rohrer
 * Added support for mouse hover.
 *
 * 2006-December-26   Jason Rohrer
 * Fixed a bug in border drawing.
 */



#include "ObjectSelector.h"

#include <GL/gl.h>



ObjectSelector::ObjectSelector( double inAnchorX, double inAnchorY,
                                double inWidth, double inHeight,
                                ObjectStorage *inStorage )
    : GUIComponentGL( inAnchorX, inAnchorY, inWidth, inHeight ),
      mStorage( inStorage ),
      mHoverObjectIndex( -1 ) {


    }



void ObjectSelector::setStorage( ObjectStorage *inStorage ) {
    mStorage = inStorage;
    }



int ObjectSelector::getHoverObject() {

    // make sure it's still in-bounds
    int objectCount = mStorage->getStoredObjects( NULL );
    if( mHoverObjectIndex >= objectCount ) {
        mHoverObjectIndex = -1;
        }
    return mHoverObjectIndex;
    }



void ObjectSelector::mouseMoved( double inX, double inY ) {

    // reset hover object
    mHoverObjectIndex = -1;
    
    if( isInside( inX, inY ) ) {

        // map inY to one of our objects

        int objectCount = mStorage->getStoredObjects( NULL );

        double relativeY = inY - mAnchorY;

        double drawUnitsPerObject = getDrawUnitsPerObject();
        
        int index = (int)( relativeY /  drawUnitsPerObject );

        if( index < objectCount && index >= 0 ) {
            mHoverObjectIndex = index;
            }
        }
    }



void ObjectSelector::mouseDragged( double inX, double inY ) {
    }



void ObjectSelector::mousePressed( double inX, double inY ) {
    }



void ObjectSelector::mouseReleased( double inX, double inY ) {

    if( isInside( inX, inY ) ) {

        // map inY to one of our objects

        int objectCount = mStorage->getStoredObjects( NULL );

        double relativeY = inY - mAnchorY;

        double drawUnitsPerObject = getDrawUnitsPerObject();
        
        int index = (int)( relativeY /  drawUnitsPerObject );

        if( index < objectCount && index >= 0 ) {
            mStorage->setSelectedObjectIndex( index );
            }
        else {
            // leave existing selection
            // mStorage->setSelectedObjectIndex( -1 );
            }        
        }
    }



void ObjectSelector::fireRedraw() {

    // first, draw background
    
    double alpha = 1.0;

    if( ! isEnabled() ) {
        alpha = 0.25;
        }

    double alphaBottomRight, alphaTopRight, alphaBottomLeft, alphaTopLeft;

    alphaBottomRight = alpha;
    alphaTopRight = alpha;

    alphaBottomLeft = 0;
    alphaTopLeft = 0;

    float r = 0.3f;
    float g = 0.3f;
    float b = 0.3f;
    
    
	glBegin( GL_QUADS ); {

        glColor4f( r, g, b, alphaBottomLeft );
		glVertex2d( mAnchorX, mAnchorY );

        glColor4f( r, g, b, alphaTopLeft );
        glVertex2d( mAnchorX, mAnchorY + mHeight );

        glColor4f( r, g, b, alphaTopRight );
        glVertex2d( mAnchorX + mWidth, mAnchorY + mHeight );

        glColor4f( r, g, b, alphaBottomRight );
        glVertex2d( mAnchorX + mWidth, mAnchorY );

        }
    glEnd(); 


    if( ! isEnabled() ) {
        // don't draw any objects
        return;
        }

    
    DrawableObject **objects;
    
    int objectCount = mStorage->getStoredObjects( &objects );

    if( objectCount > 0 ) {
        // draw the objects
        
        int selectedIndex = mStorage->getSelectedObjectIndex();
    
    
        double drawUnitsPerObject = getDrawUnitsPerObject();

    
        double drawX = mAnchorX + mWidth / 2;

        double drawY = mAnchorY + drawUnitsPerObject / 2;

        for( int i=0; i<objectCount; i++ ) {


            if( i == selectedIndex ) {
                // draw dark box behind selected

                double xStart = mAnchorX;
                double xEnd = mAnchorX + mWidth;

                double yStart = mAnchorY + drawUnitsPerObject * i;
                double yEnd = yStart + drawUnitsPerObject;

            
                // fade to transparent at left edge
            
                glBegin( GL_QUADS ); {

                    glColor4f( 0, 0, 0, 0 );
                    glVertex2d( xStart, yStart );
                    glVertex2d( xStart, yEnd );

                    glColor4f( 0, 0, 0, 0.45 );
                    glVertex2d( xEnd, yEnd );
                    glVertex2d( xEnd, yStart );
                    }
                glEnd();
                }


            Vector3D drawPosition( drawX, drawY, 0 );
        
            objects[i]->draw( &drawPosition,
                              // let object fill half the space
                              drawUnitsPerObject / 2 );

            drawY += drawUnitsPerObject;

        
            }
        }
    
    delete [] objects;


    // do this on top of objects so that it's not covered by selected
    // object's highlight

    // add a border 
    glBegin( GL_LINE_LOOP ); {

        glColor4f( 0.7f, 0.7f, 0.7f, alphaBottomLeft );
        glVertex2d( mAnchorX, mAnchorY );

        glColor4f( 0.7f, 0.7f, 0.7f, alphaTopLeft );
        glVertex2d( mAnchorX, mAnchorY + mHeight );

        glColor4f( 0.7f, 0.7f, 0.7f, alphaTopRight );
        glVertex2d( mAnchorX + mWidth, mAnchorY + mHeight );

        glColor4f( 0.7f, 0.7f, 0.7f, alphaBottomRight );
        glVertex2d( mAnchorX + mWidth, mAnchorY );
        }
    glEnd();
    }



double ObjectSelector::getDrawUnitsPerObject() {


    int objectCount = mStorage->getStoredObjects( NULL );
    
    double drawUnitsPerObject = mHeight /  objectCount;

    if( drawUnitsPerObject > mWidth ) {
        drawUnitsPerObject = mWidth;
        }

    return drawUnitsPerObject;
    }
