/*
 * Modification History
 *
 * 2006-November-8   Jason Rohrer
 * Created.
 */


#include "PortalLayerGenetics.h"
#include <math.h>



PortalLayerGenetics::PortalLayerGenetics()
    : GardenerGenetics() {

    }



PortalLayerGenetics::PortalLayerGenetics( GardenerGenetics *inOpener )
    : GardenerGenetics( inOpener ) {

    }



double PortalLayerGenetics::getParameter( PortalLayerGeneLocator inLocator ) {
    double geneValue = mGenes[ inLocator ][0];
    
    switch( inLocator ) {
	    case glyphStartPositionX:
	    case glyphStartPositionY:
		    return mapValueToRange( geneValue, 0.1, 0.9 );
        case glyphStepsBeforeDirectionChange:
            return (int)mapValueToRange( geneValue, 5, 50 );
        case glyphStartAngle:
            return mapValueToRange( geneValue, 0, 2 * M_PI );
        case glyphDeltaAngle:
            return mapValueToRange( geneValue, 0, 0.5 * M_PI );
        case quadRotationSpeed:
            return mapValueToRange( geneValue, 0.5, 1 );
        case quadRotationDelta:
            return mapValueToRange( geneValue, 0, 0.5 );
        case offsetRadius:
		    return mapValueToRange( geneValue, .5, 1 );
        case offsetSineWeight:
            return mapValueToRange( geneValue, 0, 0.25 );
        case offsetSineSpeed:
            return mapValueToRange( geneValue, 0, 10 );
        case offsetSineSineSpeed:
            return mapValueToRange( geneValue, 0, 1 );
        case quadDensity:
          // looks better with no density variation
          // all high density
		  // return mapValueToRange( geneValue, 0.2, 0.5 );
		  return mapValueToRange( geneValue, 0.5, 0.5 );
        case quadScale:
            return mapValueToRange( geneValue, 0.3, 0.7 );
        case quadScaleSineWeight:
            return mapValueToRange( geneValue, 0, 0.2 );
        case quadScaleSineSpeed:
            return mapValueToRange( geneValue, 0, 4 );
        case quadScaleSineSineSpeed:
            return mapValueToRange( geneValue, 0, 1 );
            
        // default to returning the gene itself
        default:
            return geneValue;
        }    

    }
