/*
 * Modification History
 *
 * 2006-July-25   Jason Rohrer
 * Created.
 */


#include "emotionIcons.h"

#include "glCommon.h"


#include <GL/gl.h>



void DislikeIcon::draw( Vector3D *inPosition, double inScale ) {


    // draw a red x
    glColor4f( 1, 0, 0, 0.5 );

    Angle3D angle( 0, 0, M_PI / 4 );
    drawBlurPlus( inPosition, inScale, &angle ); 
    }



void LikeIcon::draw( Vector3D *inPosition, double inScale ) {

    

    // draw a green +

    glColor4f( 0, 1, 0, 0.5 );

    Angle3D angle( 0, 0, 0 );
    drawBlurPlus( inPosition, inScale, &angle ); 
    }
