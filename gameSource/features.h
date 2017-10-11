/*
 * Modification History
 *
 * 2006-September-27  Jason Rohrer
 * Created.
 */



#ifndef FEATURES_INCLUDED
#define FEATURES_INCLUDED



/**
 * Initialized feature variables by reading from features.txt file.
 */
void initializeFeatures();



// each feature is represented by a boolean variable

// all default to true unless set to 0 in features.txt

class Features {
    public:
        static char largeWindow;
        static char drawClouds;
        static char drawSurfaceNoise;
        static char drawNiceCircles;
        static char drawNicePlantLeaves;
        static char drawComplexGardeners;
        static char drawSoil;
        static char drawWater;
        static char drawShadows;
        static char drawComplexPortal;
        
    };



#endif
