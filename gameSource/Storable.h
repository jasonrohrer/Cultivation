/*
 * Modification History
 *
 * 2006-July-27   Jason Rohrer
 * Created.
 */



#ifndef STORABLE_INCLUDED
#define STORABLE_INCLUDED


#include "DrawableObject.h"



enum StorableType{ fruitType = 1, seedsType };



/**
 * Interface for items that can be stored by gardeners.
 *
 * @author Jason Rohrer
 */
class Storable : public DrawableObject {

    public:


        
        virtual ~Storable() {
            }

        
        
        /**
         * Get the type of this storable object.
         *
         * @return the type.
         */
        virtual StorableType getType() = 0;

    };



#endif
