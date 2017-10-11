/*
 * Modification History
 *
 * 2006-November-17   Jason Rohrer
 * Created.
 */



#ifndef IMMORTAL_GENETICS_INCLUDED
#define IMMORTAL_GENETICS_INCLUDED


#include "GardenerGenetics.h"



enum ImmortalGeneLocator{
    soulStartX = 0,
    soulStartY,
    soulStartAngle,
    soulDeltaAngle,
    soulDeltaDeltaAngle,
    soulDeltaDeltaDeltaAngle,
    soulDeltaAngleSineWeight,
    soulDeltaAngleSineSpeed,
    cornerASpread,
    cornerBSpread,
    cornerCSpread,
    cornerDSpread };
    


/**
 * Genetics for an immortal representation.  Based on the genes of the
 * gardener that was immortalized.
 *
 * @author Jason Rohrer
 */
class ImmortalGenetics : public GardenerGenetics {

    public:


        
        /**
         * Constructs random genetics
         */
        ImmortalGenetics();


        
        /**
         * Constructs genetics based on gardener genetics.
         *
         * @param inGardener the genetics of the gardener that became immortal.
         *   Destroyed by caller.
         */         
        ImmortalGenetics( GardenerGenetics *inGardener );



        /**
         * Maps a given gene to a parameter value.
         *
         * @param inLocator a gene locator.
         *
         * @return a parameter value for the specified gene.
         */
        double getParameter( ImmortalGeneLocator inLocator );
        
        
    };



#endif
