/*
 * Modification History
 *
 * 2006-November-8   Jason Rohrer
 * Created.
 */



#ifndef PORTAL_LAYER_GENETICS_INCLUDED
#define PORTAL_LAYER_GENETICS_INCLUDED


#include "GardenerGenetics.h"



enum PortalLayerGeneLocator{
    // behavior modifiers
    glyphStartPositionX = 0,
    glyphStartPositionY,
    glyphStepsBeforeDirectionChange,
    glyphStartAngle,
    glyphDeltaAngle,
    quadRotationSpeed,
    quadRotationDelta,
    offsetRadius,
    offsetSineWeight,
    offsetSineSpeed,
    offsetSineSineSpeed,
    quadDensity,
    quadScale,
    quadScaleSineWeight,
    quadScaleSineSpeed,
    quadScaleSineSineSpeed,
    cornerAAlpha,
    cornerBAlpha,
    cornerCAlpha,
    cornerDAlpha };
    


/**
 * Genetics for a portal layer.  Based on the genes of the gardener that
 * opened the portal layer.
 *
 * @author Jason Rohrer
 */
class PortalLayerGenetics : public GardenerGenetics {

    public:


        
        /**
         * Constructs random genetics
         */
        PortalLayerGenetics();


        
        /**
         * Constructs genetics based on gardener genetics.
         *
         * @param inOpener the genetics of the gardener that opened this layer.
         *   Destroyed by caller.
         */         
        PortalLayerGenetics( GardenerGenetics *inOpener );



        /**
         * Maps a given gene to a parameter value.
         *
         * @param inLocator a gene locator.
         *
         * @return a parameter value for the specified gene.
         */
        double getParameter( PortalLayerGeneLocator inLocator );
        
        
    };



#endif
