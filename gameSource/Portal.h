/*
 * Modification History
 *
 * 2006-October-26   Jason Rohrer
 * Created.
 *
 * 2006-December-25   Jason Rohrer
 * Added function for checking closed status of portal.
 */



#ifndef PORTAL_INCLUDED
#define PORTAL_INCLUDED



#include "minorGems/math/geometry/Vector3D.h"
#include "minorGems/math/geometry/Angle3D.h"

#include "minorGems/graphics/openGL/SingleTextureGL.h"

#include "minorGems/util/SimpleVector.h"


#include "DrawableObject.h"
#include "Gardener.h"
#include "PortalLayerGenetics.h"
#include "ImmortalGenetics.h"



/**
 * A portal for the end of the game.
 *
 * @author Jason Rohrer
 */
class Portal {

    public:

        /**
         * Constructs a portal.
         *
         */
        Portal();


        virtual ~Portal();


        
        /**
         * Increasese the level of the portal.
         *
         * @param inLevelOpener the gardener that opened this level, or
         *   NULL.  Defaults to NULL.
         */
        void upgrade( Gardener *inLevelOpener = NULL );


        
        /**
         * Gets open status of portal.
         *
         * @return true if portal is open.
         */
        char isOpen();


        
        /**
         * Gets closed status of portal.
         *
         * @return true if portal is closed and gardeners have been sent.
         */
        char isClosed();


        
        /**
         * Sends a gardener through the open portal.
         *
         * @param inGardener the gardener to send.  Destroyed by caller
         *   after inGardener marked as dead by this class.
         * @param inStartingPosition the starting position.  Destroyed by
         *   caller.
         * @param inStartingRotation the starting rotation.  Destroyed by
         *   caller.
         */         
        void  sendGardener( Gardener *inGardener,
                            Vector3D *inStartingPosition,
                            Angle3D *inStartingRotation );

        
        
        /**
         * Passes time for this portal.
         *
         * @param inTimeDeltaInSeconds the amount of time that has passed.
         */
        void passTime( double inTimeDeltaInSeconds );


        
        /**
         * Draws this portal.
         *
         * @param inPosition the center position of this portal.
         * @param inScale the scale of this portal, defaults to 1.
         * @param inMaxZ the maximum z location of portal parts to draw.
         * @param inMinZ the minimum z location of portal parts to draw.
         *  Parts outside the inMaxZ/inMinZ window are skipped.
         */
        void draw( Vector3D *inPosition, double inScale = 1,
                   double inMaxZ = 0, double inMinZ = -100 );
        
        

    private:

        int mMaxNumLevels;
        
        int mNumLevels;

        double mTopLevelZ;
        

        SingleTextureGL **mLayerTextures;
        
        PortalLayerGenetics **mLayerGenetics;

        double *mLayerSpawnProgress;

        double mOpeningProgress;

        char mClosing;
        double mClosingProgress;
        
        double mTimePassed;
        
        SimpleVector<Gardener *> mPassengers;
        SimpleVector<Vector3D *> mPassengerPositions;
        SimpleVector<Angle3D *> mPassengerRotations;
        SimpleVector<double> mPassengerFades;


        // track genetics of all gardeners sent
        SimpleVector<ImmortalGenetics *> mImmortals;

        
        
        /**
         * Closes the portal, sending the immortal gardeners to a file.
         */
        void close();

        
    };



#endif







