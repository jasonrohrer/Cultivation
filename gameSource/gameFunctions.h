/*
 * Modification History
 *
 * 2006-August-13   Jason Rohrer
 * Created.
 */



#include "Gardener.h"

#include "minorGems/math/geometry/Vector3D.h"
#include "minorGems/math/geometry/Angle3D.h"



int getMaxDistanceForTransactions();



double getGardenerZPosition();



/**
 * Adds an AI-controlled gardener to the game.
 *
 * @param inGardener the gardener to add.  Destroyed by game.
 * @param inPosition the starting position for inGardener.
 *   Destroyed by caller.
 * @param inRotation the starting rotation, or NULL for default.
 *   Destroyed by caller.
 */
void addGardenerToGame( Gardener *inGardener,
                        Vector3D *inPosition,
                        Angle3D *inRotation = NULL );



/**
 * Adjusts music volume to avoid clipping.
 *
 * Should be called every time gardener count changes.
 *
 * @param inCount the number of gardeners.
 */
void setNumGardeners( int inCount );
