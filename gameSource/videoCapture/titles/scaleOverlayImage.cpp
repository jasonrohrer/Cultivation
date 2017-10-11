
#include "minorGems/graphics/Image.h"
#include "minorGems/graphics/Color.h"
#include "minorGems/graphics/converters/JPEGImageConverter.h"

#include "minorGems/io/file/File.h"
#include "minorGems/io/file/FileOutputStream.h"
#include "minorGems/io/file/FileInputStream.h"


#include <stdio.h>



inline double getColor( int inX, int inY, int inWidth, double *inChannel ) {
    
    int pixelIndex = inY * inWidth + inX;

    return inChannel[ pixelIndex ];
    }


# define getColor_fast( inX, inY, inWidth, inChannel ) \
    inChannel[ inY * inWidth + inX ]


inline double linearSum( double inA, double inB, double inWeightA ) {
    // double weightB = 1 - inWeightA;
    // return inA * inWeightA + inB * weightB;
    
    //   a * w + b (1-w)
    // = a * w + b - b * w
    // = (a - b) * w + b

    return (inA - inB) * inWeightA + inB;
    }


#define linearSum_fast( inA, inB, inWeightA ) \
   ( (inA - inB) * inWeightA + inB )



/**
 * Gets a sample from an image using bilinear interpolation.
 *
 * @param inX, inY floating point coordinates in pixels.
 * @param inImage the image to sample.
 * @param inBackground the color value outside inImage's boundaries.
 * @param outSample pointer to where sample should be returned.
 */
void getSample( double inX, double inY, Image *inImage,
                Color *inBackground, double *outSample ) {

    // use bilinear interpolation

    int w = inImage->getWidth();
    int h = inImage->getHeight();
    double *channels[3];
    channels[0] = inImage->getChannel( 0 );
    channels[1] = inImage->getChannel( 1 );
    channels[2] = inImage->getChannel( 2 );
    
	// find closest integer coords above and below the passed in coords
	int lowX = (int)inX;
	int lowY = (int)inY;

    // if inX or inY are negative, int conversion rounds them up!
    if( lowX > inX ) {
        lowX -= 1;
        }
    if( lowY > inY ) {
        lowY -= 1;
        }
    
	int highX = lowX + 1;
	int highY = lowY + 1;

    // weights for the high coords
    double highXWeight = inX - lowX;
    double highYWeight = inY - lowY;
    
    // interpolate each channel separately
    for( int c=0; c<3; c++ ) {
        double *channel = inImage->getChannel( c );
        
        // corner colors
        double lowXLowY;
        double lowXHighY;
        double highXLowY;
        double highXHighY;

        
        // for each corner, grab image color, or grab background color
        // if it's out of image bounds

        if( lowX < 0 || lowX >= w
            ||
            lowY < 0 || lowY >= h ) {
            
            lowXLowY = (*inBackground)[c];
            }
        else {
            lowXLowY = getColor_fast( lowX, lowY, w, channel );
            }

        if( lowX < 0 || lowX >= w
            ||
            highY < 0 || highY >= h ) {

            lowXHighY = (*inBackground)[c];
            }
        else {
            lowXHighY = getColor_fast( lowX, highY, w, channel );
            }


        if( highX < 0 || highX >= w
            ||
            lowY < 0 || lowY >= h ) {

            highXLowY = (*inBackground)[c];
            }
        else {
            highXLowY = getColor_fast( highX, lowY, w, channel );
            }

        if( highX < 0 || highX >= w
            ||
            highY < 0 || highY >= h ) {

            highXHighY = (*inBackground)[c];
            }
        else {
            highXHighY = getColor_fast( highX, highY, w, channel );
            }
        
    
    
        

        // average of two points with lowY coordinate
        double lowYAverage =
            linearSum_fast( highXLowY, lowXLowY, highXWeight );
    
        // average of two points with highY coordinate
        double highYAverage =
            linearSum_fast( highXHighY, lowXHighY, highXWeight );
	
	
        // average in y direction
        double finalValue =
            linearSum_fast( highYAverage, lowYAverage, highYWeight );
        
        outSample[c] = finalValue;
        }
	}



int main( int inNumArgs, char **inArgs ) {

    if( inNumArgs != 5 ) {
        printf( "Usage:  scaleOverlayImage "
                "background.jpg overlay.jpg scale out.jpg\n" );

        return 1;
        }
    
    // arg 1 background
    // arg 2 overlay
    // arg 3 scale of overlay
    // arg 4 output file
    
    double scale = 1.0;

    sscanf( inArgs[3], "%lf", &scale );
    

    JPEGImageConverter jpeg( 90 );


    File backFile( NULL, inArgs[1] );
    FileInputStream backIn( &backFile );
    
    Image *background = jpeg.deformatImage( &backIn );

    File overlayFile( NULL, inArgs[2] );
    FileInputStream overlayIn( &overlayFile );
    
    Image *overlay = jpeg.deformatImage( &overlayIn );


    // insert overlay into center of background using scale factor
    
    int backW = background->getWidth();
    int backH = background->getHeight();

    int overW = overlay->getWidth();
    int overH = overlay->getHeight();
    
    
    Color black( 0, 0, 0 );

    double *backChannels[3];

    int c;
    for( c=0; c<3; c++ ) {
        backChannels[c] = background->getChannel( c );
        }

    double invScale = 1.0 / scale;

    int startY = (int)( ( backH - scale * overH ) / 2.0 );
    int endY = (int)( startY + scale * overH );
    startY -= 1;
    endY += 1;
    if( startY < 0 ) {
        startY = 0;
        }
    if( endY > backH ) {
        endY = backH;
        }

    int startX = (int)( ( backW - scale * overW ) / 2.0 );
    int endX = (int)( startX + scale * overW );
    startX -= 1;
    endX += 1;
    if( startX < 0 ) {
        startX = 0;
        }
    if( endX > backW ) {
        endX = backW;
        }

    double *sample = new double[3];

    // precompute outside loop
    double xOffset = ( backW - scale * overW ) / 2.0;
    
    for( int y=startY; y<endY; y++ ) {
        // opt:  compute outside inner loop
        double overY = invScale * ( y - ( backH - scale * overH ) / 2.0 );
        
        for( int x=startX; x<endX; x++ ) {
            int backPixelIndex = y * backW + x;

            // transform coordinates into the overlay

            double overX = invScale * ( x - xOffset );

                
            getSample( overX, overY, overlay, &black,
                       sample );

            for( int c=0; c<3; c++ ) {

                backChannels[c][ backPixelIndex ] = sample[c];
                }
            }
        }

    delete [] sample;
    
    
    File outFile( NULL, inArgs[4] );
    FileOutputStream out( &outFile );
    
    jpeg.formatImage( background, &out );
    
    return 0;
    }



