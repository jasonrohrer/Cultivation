/*
 * Modification History
 *
 * 2006-September-27  Jason Rohrer
 * Created.
 */



#include "features.h"

#include "minorGems/io/file/File.h"

#include <stdio.h>


// set defaults
// all default to true unless set to 0 in features.txt
char Features::largeWindow = true;
char Features::drawClouds = true;
char Features::drawSurfaceNoise = true;
char Features::drawNiceCircles = true;
char Features::drawNicePlantLeaves = true;
char Features::drawComplexGardeners = true;
char Features::drawSoil = true;
char Features::drawWater = true;
char Features::drawShadows = true;
char Features::drawComplexPortal = true;



// map from string names to feature variable pointers
// used in the loop below to read from features.txt
int numFeatures = 10;
const char *nameMap[10] = { "largeWindow",
                           "drawClouds",
                           "drawSurfaceNoise",
                           "drawNiceCircles",
                           "drawNicePlantLeaves",
                           "drawComplexGardeners",
                           "drawSoil",
                           "drawWater",
                           "drawShadows",
                           "drawComplexPortal" };

char *variableMap[10] = { &( Features::largeWindow ),
                          &( Features::drawClouds ),
                         &( Features::drawSurfaceNoise ),
                         &( Features::drawNiceCircles ),
                         &( Features::drawNicePlantLeaves ),
                         &( Features::drawComplexGardeners ),
                         &( Features::drawSoil ),
                         &( Features::drawWater ),
                         &( Features::drawShadows ),
                         &( Features::drawComplexPortal ) };



void initializeFeatures() {
    FILE *featuresFile = fopen( "features.txt", "r" );

    char stringBuffer[100];
    int switchValue;
    
    if( featuresFile != NULL ) {

        
        int numRead = 2;

        while( numRead == 2 ) {
            // read more
            // read a string and a number
            numRead = fscanf( featuresFile,
                              " %99s %d ", stringBuffer, &switchValue );

            if( numRead == 2 ) {
                // process these values

                // look for a match in our feature map
                char found = false;
                for( int i=0; i<numFeatures && !found; i++ ) {

                    if( strcmp( nameMap[i], stringBuffer ) == 0 ) {
                        // hit

                        // set our variable
                        *( variableMap[i] ) = switchValue;
                        
                        found = true;
                        }
                    }
                
                }
            }
        
        
        fclose( featuresFile );
        }
    
    }

