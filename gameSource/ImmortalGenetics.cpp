/*
 * Modification History
 *
 * 2006-November-17   Jason Rohrer
 * Created.
 */


#include "ImmortalGenetics.h"


#include <math.h>



ImmortalGenetics::ImmortalGenetics()
    : GardenerGenetics() {

    }



ImmortalGenetics::ImmortalGenetics( GardenerGenetics *inOpener )
    : GardenerGenetics( inOpener ) {

    }



double ImmortalGenetics::getParameter( ImmortalGeneLocator inLocator ) {
    double geneValue = mGenes[ inLocator ][0];
    
    switch( inLocator ) {
        case soulStartAngle:
            return mapValueToRange( geneValue, 0, 2 * M_PI );
        case soulDeltaAngle:
            return mapValueToRange( geneValue, -0.01, 0.01 );
        case soulDeltaDeltaAngle:
            //return mapValueToRange( geneValue, -0.005, 0.005 );
            return mapValueToRange( geneValue, -0.00001, 0.00001 );
        case soulDeltaDeltaDeltaAngle:
            return mapValueToRange( geneValue, -0.00005, 0.00005 );
        case soulDeltaAngleSineWeight:
            return mapValueToRange( geneValue, 0, 1 );
        case soulDeltaAngleSineSpeed:
            return mapValueToRange( geneValue, 0, 1 );
        case cornerASpread:
            return mapValueToRange( geneValue, 1, 3 );
        case cornerBSpread:
            return mapValueToRange( geneValue, 1, 3 );
        case cornerCSpread:
            return mapValueToRange( geneValue, 1, 3 );
        case cornerDSpread:
            return mapValueToRange( geneValue, 1, 3 );
        // default to returning the gene itself
        default:
            return geneValue;
        }    

    }
