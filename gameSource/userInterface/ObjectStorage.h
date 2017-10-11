/*
 * Modification History
 *
 * 2006-July-23   Jason Rohrer
 * Created.
 *
 * 2006-October-14   Jason Rohrer
 * Added virtual destructor.
 */



#ifndef OBJECT_STORAGE_INCLUDED
#define OBJECT_STORAGE_INCLUDED


#include "../DrawableObject.h"



/**
 * Interface for classes that can be the base for an ObjectSelector user
 * interface element.
 */
class ObjectStorage {

    public:


        
        /**
         * Gets the objects stored by this class.
         *
         * @param outObjects pointer to where an array of objects should
         *   be returned, or NULL to just return an object count.
         *   Array must be destroyed by caller, but objects are managed by
         *   this class.
         *
         * @return the number of objects stored.
         */
        virtual int getStoredObjects( DrawableObject ***outObjects ) = 0; 

        

        /**
         * Gets the index number of the selected object.
         *
         * @return the selected index, or -1 if no object selected.
         */
        virtual int getSelectedObjectIndex() = 0;


        
        /**
         * Sets the index number of the selected object.
         *
         * @param inIndex the index to select,
         *   or -1 to select no object selected.
         */
        virtual void setSelectedObjectIndex( int inIndex ) = 0;


        
        // ensure proper destruction of subclasses
        virtual ~ObjectStorage() {
            }

    };



#endif
