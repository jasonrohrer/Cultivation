

#include "Gardener.cpp"
#include "GardenerGenetics.cpp"
#include "Genetics.cpp"
#include "PlantGenetics.cpp"
#include "Seeds.cpp"
#include "Fruit.cpp"
#include "SoilMap.cpp"
#include "glCommon.cpp"
#include "landscape.cpp"

//StdRandomSource globalRandomSource( time( NULL ) );
StdRandomSource globalRandomSource( 5002 );


void addGardenerToGame(Gardener *, Vector3D *, Angle3D *) {
    // dummy
    }

int fileCount = 0;

int main() {
    
    for( int i=0; i<50; i++ ) {

        Vector3D startPosition( 0, 0, 0 );
        Gardener *gardener = new Gardener( &startPosition );

        delete gardener;
        }

    return 0;
    }
