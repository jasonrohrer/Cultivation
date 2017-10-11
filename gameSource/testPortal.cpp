#include "Portal.h"
#include "Gardener.h"

#include "minorGems/util/random/StdRandomSource.h"

extern StdRandomSource globalRandomSource;

#include <stdio.h>


void testPortal() {

    printf( "Test int = %ld\n", globalRandomSource.getRandomInt() );
    // call again to put in different state
    printf( "Test int = %ld\n", globalRandomSource.getRandomInt() );

    Portal portal;
    
    while( ! portal.isOpen() ) {
        
        portal.upgrade();
        portal.passTime( 10000 );
        
        }
    

    int numGardeners = 16;
    
    for( int i=0; i<numGardeners; i++ ) {

        Vector3D startPosition( 0, 0, 0 );
        Angle3D startRotation( 0, 0, 0 );
        Gardener *gardener = new Gardener( &startPosition );
        
        
        if( i == numGardeners - 1 ) {
            // last one, close portal
            gardener->mUserControlling = true;
            }
        

        portal.sendGardener( gardener, &startPosition, &startRotation );
        portal.passTime( 10000 );

        delete gardener;
        }
    }
