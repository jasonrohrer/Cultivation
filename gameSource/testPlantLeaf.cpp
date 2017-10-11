

#include "PlantLeaf.cpp"
#include "PlantGenetics.cpp"
#include "Genetics.cpp"

//StdRandomSource globalRandomSource( time( NULL ) );
StdRandomSource globalRandomSource( 5002 );


//#include "minorGems/io/file/File.h"
#include "minorGems/io/file/FileOutputStream.h"
#include "minorGems/graphics/converters/TGAImageConverter.h"
#include "minorGems/graphics/converters/JPEGImageConverter.h"
#include "minorGems/graphics/converters/unix/JPEGImageConverterUnix.cpp"
#include "minorGems/graphics/filters/BoxBlurFilter.h"


int fileCount = 0;

int main() {

    int textureSize = 64;

    int pixelsPerChannel = textureSize * textureSize;

    //PlantGenetics genetics;
    
    for( int i=0; i<50; i++ ) {
        PlantGenetics genetics;

        Vector3D terminus;
        Image *textureImage = getCellularImage( textureSize, &genetics,
                                                &terminus );

        BoxBlurFilter blur( 1 );

        textureImage->filter( &blur);
        
        char *fileName = autoSprintf( "leafTexture_%d.tga", fileCount );
        fileCount++;
        
        File outFileB( NULL, fileName );
        delete [] fileName;
        FileOutputStream *outStreamB = new FileOutputStream( &outFileB );

        /*
        JPEGImageConverter converter( 100 );

        // make a 3 channel image showing only alpha data
        Image *outImage = new Image( textureSize, textureSize, 3 );

        memcpy( outImage->getChannel( 0 ), textureImage->getChannel( 3 ),
                pixelsPerChannel * sizeof( double ) );
        memcpy( outImage->getChannel( 1 ), textureImage->getChannel( 1 ),
                pixelsPerChannel * sizeof( double )  );
        memcpy( outImage->getChannel( 2 ), textureImage->getChannel( 3 ),
                pixelsPerChannel * sizeof( double )  );
    
    
        converter.formatImage( outImage, outStreamB );
        delete outImage;

        */

        TGAImageConverter converter;

        converter.formatImage( textureImage, outStreamB );
        
        delete outStreamB;
        
        delete textureImage;
        }

    return 0;
    }
