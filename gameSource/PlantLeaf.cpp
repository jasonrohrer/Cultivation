/*
 * Modification History
 *
 * 2006-August-20   Jason Rohrer
 * Created.
 *
 * 2006-September-13   Jason Rohrer
 * Blurred all channels to deal with pixelation in color bands.
 */



#include "PlantLeaf.h"
#include "features.h"

#include "minorGems/graphics/Image.h"
#include "minorGems/graphics/filters/BoxBlurFilter.h"

#include "minorGems/util/SimpleVector.h"

#include <string.h>
#include <math.h>




Image *getCircleImage( int inSize ) {

    int textureSize = inSize;
    
    
    Image *textureImage = new Image( textureSize, textureSize, 4 );

    double *channels[4];

    int pixelsPerChannel = textureSize * textureSize;
    
    int i;
    int p;
    for( i=0; i<4; i++ ) {
        channels[i] = textureImage->getChannel( i );
        }

    // start all pixels as white
    for( i=0; i<3; i++ ) {
        for( int p=0; p<pixelsPerChannel; p++ ) {
            channels[i][p] = 1;
            }
        }

    // entire image alpha is zero
    for( p=0; p<pixelsPerChannel; p++ ) {
        channels[3][p] = 0;
        }

    double radius = textureSize / 2;
    
    for( int y=0; y<textureSize; y++ ) {
        for( int x=0; x<textureSize; x++ ) {

            double value;

            double xRelative = x - (0.5 *textureSize );
            double yRelative = y - (0.5 *textureSize );
            
            if( sqrt( xRelative * xRelative + yRelative * yRelative )
                <= radius ) {
                value = 1;
                }
            else {
                value = 0;
                }

            channels[3][ y * textureSize + x ] = value;
            }
        }

    return textureImage;
    }



class CellWalker {
    public:

        // current position
        double mX, mY;
        
        double mDeltaX, mDeltaY;
        
        double mDeltaAngle, mDeltaDeltaAngle;

        int mStepCount;

        // factor that we divide mSpawnInterval by for our offspring
        double mSpawnIntervalFactor;
        
        int mSpawnInterval;

        double mSpawnAngle;

        // true to spawn two walkers at each branch point
        char mSpawnDouble;
        
        char mDead;
        
        CellWalker()
            : mStepCount( 0 ), mDead( false ) {

            }

        
        /**
         * Step to next location.
         *
         * @return true if should spawn new walker after this step.
         */
        char step() {
            mX += mDeltaX;
            mY += mDeltaY;

            // rotate delta
            Vector3D deltaVector( mDeltaX, mDeltaY, 0 );

            Angle3D deltaAngle( 0, 0, mDeltaAngle );

            deltaVector.rotate( &deltaAngle );

            mDeltaX = deltaVector.mX;
            mDeltaY = deltaVector.mY;


            // change delta angle
            mDeltaAngle += mDeltaDeltaAngle;
            
                
            mStepCount ++;

            if( mStepCount % mSpawnInterval == 0 ) {
                return true;
                }
            else {
                return false;
                }
            }


        
        /**
         * Spawns a new walker.
         *
         * @param inDirection -1 to spawn in negative direction, or +1
         *   to spawn in positive direction.
         *
         * @return a new walker, destroyed by caller.
         */
        CellWalker *spawn( int inDirection ) {

            CellWalker *newWalker = new CellWalker();

            newWalker->mX = mX;
            newWalker->mY = mY;

            Vector3D ourDirection( mDeltaX, mDeltaY, 0 );

            Angle3D spawnAngle( 0, 0, inDirection * mSpawnAngle );
            
            ourDirection.rotate( &spawnAngle );

            newWalker->mDeltaX = ourDirection.mX;
            newWalker->mDeltaY = ourDirection.mY;

            newWalker->mDeltaAngle = inDirection * mDeltaAngle;
            newWalker->mDeltaDeltaAngle = inDirection * mDeltaDeltaAngle;
            
            
            newWalker->mStepCount = 0;

            newWalker->mSpawnIntervalFactor = mSpawnIntervalFactor;
            
            newWalker->mSpawnInterval =
                (int)( mSpawnInterval / mSpawnIntervalFactor );

            // impose a minimum spawn interval
            if( newWalker->mSpawnInterval < 2 ) {
                newWalker->mSpawnInterval = 2;
                }
            
            newWalker->mSpawnDouble = mSpawnDouble;
            
            newWalker->mSpawnAngle = mSpawnAngle;

            return newWalker;
            }
        
    };



Image *getCellularImage( int inSize, PlantGenetics *inGenetics,
                         Vector3D *outLeafWalkerTerminus ) {
    
    int textureSize = inSize;
    
    
    Image *textureImage = new Image( textureSize, textureSize, 4, false );

    
    double *channels[4];

    int pixelsPerChannel = textureSize * textureSize;
    
    int i;
    int p;
    for( i=0; i<4; i++ ) {
        channels[i] = textureImage->getChannel( i );
        }



    // greenish hues in center, but allow red or blue too
    Color *innerColor = inGenetics->getColor( innerLeafColor );
    
    Color startColor;
    startColor.setValues( innerColor );
    delete innerColor;
    
    // only pure greens at leaf edge
    Color endColor( 0,
                    inGenetics->getParameter( outerLeafGreen ),
                    0 );

    
    // start all pixels as end color
    for( int p=0; p<pixelsPerChannel; p++ ) {
        channels[0][p] = endColor.r;
        channels[1][p] = endColor.g;
        channels[2][p] = endColor.b;
        }
    
        
    // entire image alpha is zero
    for( p=0; p<pixelsPerChannel; p++ ) {
        channels[3][p] = 0;
        }


    

    
                      
    
    
    // settings for cell expansion after walkers are done
    int neighborhoodRadius = 1;
    
    int numExpansionSteps = 4;


    
    // how close walkers can get to edge
    int walkerBoundary = neighborhoodRadius * ( numExpansionSteps + 1 );

    // track all walkers
    SimpleVector<CellWalker *> walkers;

    
    
    // start a walker
    
    CellWalker *firstWalker = new CellWalker();
    // start in center at bottom
    firstWalker->mX = textureSize / 2;
    firstWalker->mY = walkerBoundary;

    // aim up
    firstWalker->mDeltaX = 0;
    firstWalker->mDeltaY = 1;

    // walker parameters from genetics
    
    firstWalker->mDeltaAngle =
        inGenetics->getParameter( leafWalkerDeltaAngle );
    firstWalker->mDeltaDeltaAngle =
        inGenetics->getParameter( leafWalkerDeltaDeltaAngle );
    
    // thes spawn params allow spawn intervals to get both shorter and shorter
    // and longer and longer as we add additional branches
    // more variety
    firstWalker->mSpawnIntervalFactor =
        inGenetics->getParameter( leafWalkerSpawnIntervalFactor );
    
    firstWalker->mSpawnInterval =
        (int)( inGenetics->getParameter( leafWalkerStartingSpawnInterval ) );

    // wider possible spawn angles produce more interesting variety
    firstWalker->mSpawnAngle =
        inGenetics->getParameter( leafWalkerSpawnAngle );
    
    firstWalker->mSpawnDouble = true;

    if( inGenetics->getParameter( leafWalkerSpawnDouble ) > 0.5 ) {
        firstWalker->mSpawnDouble = true;
        }
    else {
        firstWalker->mSpawnDouble = false;
        }

    // add to walker set as our first walker
    walkers.push_back( firstWalker );
    

    // channel where we track paths of walkers
    double *walkerChannel = textureImage->copyChannel( 3 );
    

    char hitEdge = false;
    int maxNumWalkers = 1000;
    char allDead = false;
    
    while( !hitEdge && walkers.size() <= maxNumWalkers &&
           !allDead ) {
        
        allDead = true;

        
        // track new walkers
        SimpleVector<CellWalker *> newWalkers;
        
        // step each walker
        int numWalkers = walkers.size();
        int w;
        for( w=0; w<numWalkers; w++ ) {
            CellWalker *walker = *( walkers.getElement( w ) );

            if( ! walker->mDead ) {
                allDead = false;
                
                int oldX = (int)( walker->mX );
                int oldY = (int)( walker->mY );
                
                char spawn = walker->step();

                // draw pixel at new location
                // and check if hit edge
            
                int x = (int)( walker->mX );
                int y = (int)( walker->mY );

                if( x < walkerBoundary ||
                    x >= textureSize - walkerBoundary ||
                    y < walkerBoundary ||
                    y >= textureSize - walkerBoundary ) {
                    
                    hitEdge = true;
                    }
                else {
                    int pixelIndex = y * textureSize + x;
                    
                    if( ( oldX != x || oldY != y )
                        &&
                        walkerChannel[ pixelIndex ] == 1 ) {
                        // hit already filled area

                        // and not simply standing still and hitting our
                        // own last filled pixel
                        
                        // die
                        walker->mDead = true;
                        }
                    else {
                        // fill in image
                        channels[3][ pixelIndex ] = 1;

                        // walkers lay start color
                        channels[0][ pixelIndex ] = startColor.r;
                        channels[1][ pixelIndex ] = startColor.g;
                        channels[2][ pixelIndex ] = startColor.b;
                        
                        
                        // track history in walker channel
                        walkerChannel[ pixelIndex ] = 1;
                        }
                    }
                
                if( spawn ) {
                    newWalkers.push_back( walker->spawn( 1 ) );

                    if( walker->mSpawnDouble ) {
                        // spawn another in opposite direction
                        newWalkers.push_back( walker->spawn( -1 ) );
                        }
                    }
                }
            }

        // add new walkers
        int numNew = newWalkers.size();
        
        for( w=0; w<numNew; w++ ) {
            CellWalker *walker = *( newWalkers.getElement( w ) );

            walkers.push_back( walker );
            }

        }
            
    
    // hit edge
    int numWalkers = walkers.size();
    int w;
        
    // record final location of farthest walker
    double startX = textureSize / 2;
    double startY = 0;
    double maxDistance = 0;
    double maxX = startX;
    double maxY = startY;

    for( w=0; w<numWalkers; w++ ) {
        CellWalker *walker = *( walkers.getElement( w ) );

        double deltaX = startX - walker->mX;
        double deltaY = startY - walker->mY;
        
        double distance = sqrt( deltaX * deltaX + deltaY * deltaY );

        if( distance > maxDistance ) {
            maxX = walker->mX;
            maxY = walker->mY;

            maxDistance = distance;
            }
        }
        
    // (0,0) at bottom/center of texture
    outLeafWalkerTerminus->mX =
        ( maxX - (textureSize / 2) ) / textureSize;
    outLeafWalkerTerminus->mY = maxY / textureSize;
    

    // delete walkers    
    for( w=0; w<numWalkers; w++ ) {
        CellWalker *walker = *( walkers.getElement( w ) );
        delete walker;
        }

    delete [] walkerChannel;
    
    // short-circuit return

    if( false ) {
        return textureImage;
        }
    


    // now grow from seed pixels planted by walkers

    // Look at full pixels and fill every pixel in the neighborhood around
    // a full pixel

    // optimization found with profiler:
    // precompute "is in neighborhood" status for pixels in a box
    // to avoid doing a sqrt call in the inner loop

    int neighborhoodBoxSize = 2 * neighborhoodRadius + 1;
    
    char *isInNeighborhood = new char[ neighborhoodBoxSize *
                                       neighborhoodBoxSize ];

    for( int deltaY = -neighborhoodRadius;
         deltaY <= neighborhoodRadius; deltaY ++ ) {
        
        for( int deltaX = -neighborhoodRadius;
             deltaX <= neighborhoodRadius; deltaX ++ ) {

            int index =
                ( deltaY + neighborhoodRadius ) * neighborhoodBoxSize +
                ( deltaX + neighborhoodRadius );
            
            if( sqrt( deltaX * deltaX + deltaY * deltaY )
                <=
                neighborhoodRadius ) {

                isInNeighborhood[ index ] = true; 
                }
            else {
                isInNeighborhood[ index ] = false; 
                }
            }
        }


    // track which pixels are done (filled, and all neighbors filled)

    char *pixelDone = new char[ pixelsPerChannel ];

    memset( pixelDone, false, pixelsPerChannel );
    
    
    for( i=0; i<numExpansionSteps; i++ ) {
        double *newChannel = new double[ pixelsPerChannel ];

        // copy old channel
        memcpy( newChannel, channels[3], sizeof( double ) * pixelsPerChannel );


        // fill color depending on what step we are on 
        double colorValue =
            (double)(i + 1) /
            (double)numExpansionSteps;

        // blend between start and end
        Color *fillColor = Color::linearSum( &endColor, &startColor,
                                             colorValue );  

        
        
        
        // skip top and bottom rows and left/right columns
        // to avoid boundary cases
        for( int yIndex=neighborhoodRadius;
             yIndex < textureSize - neighborhoodRadius; yIndex++ ) {
            for( int xIndex=neighborhoodRadius;
                 xIndex < textureSize - neighborhoodRadius; xIndex++ ) {

                int x = xIndex;
                int y = yIndex;
                
                int pixelIndex = y * textureSize + x;
                
                if( !pixelDone[ pixelIndex ] &&
                    channels[3][ pixelIndex ] == 1 ) {
                    // this pixel not done yet
                    // this pixel is full

                    // mark as done
                    pixelDone[ pixelIndex ] = true;
                    
                    
                    // fill all neighbors inside neighborhoodRadius

                    // look at all pixels in box, then test if a given
                    // pixel is in the neighborhood circle
                    for( int deltaY = -neighborhoodRadius;
                         deltaY <= neighborhoodRadius;
                         deltaY ++ ) {

                        int yPixelIndex = ( y + deltaY ) * textureSize;

                        int yNeighborhoodTestIndex =
                            (deltaY + neighborhoodRadius) *
                            neighborhoodBoxSize;
                        
                        for( int deltaX = -neighborhoodRadius;
                             deltaX <= neighborhoodRadius;
                             deltaX ++ ) {

                            // ignore (0,0) case, since that is our current
                            // pixel
                            if( !( deltaX == 0 && deltaY == 0 ) ) {
                            
                                if( isInNeighborhood[
                                      yNeighborhoodTestIndex +
                                      (deltaX + neighborhoodRadius) ] ) {
                                    
                                    // in the circle

                                    int neighborPixelIndex =
                                        yPixelIndex + ( x + deltaX );
                                    
                                    if( newChannel[ neighborPixelIndex ]
                                        == 0 ) {

                                        // an unfilled neighbor

                                        // fill it with this step's color

                                        // fill it's alpha in new channel
                                        newChannel[ neighborPixelIndex ] = 1;

                                        // fill in color
                                        channels[0][ neighborPixelIndex ] =
                                            fillColor->r;
                                        channels[1][ neighborPixelIndex ] =
                                            fillColor->g;
                                        channels[2][ neighborPixelIndex ] =
                                            fillColor->b;
                                        }
                                    } 
                                }
                            } // end loop over deltaX
                        
                        } // end loop over deltaY

                    } // end check for full pixel

                } // end loop over all x in image

            } // end loop over all y in image

        delete fillColor;
        
        // copy new image into main channel
        memcpy( channels[3], newChannel, sizeof( double ) * pixelsPerChannel );

        delete [] newChannel;
        }  // end loop over expansion steps

    
    delete [] isInNeighborhood;

    delete [] pixelDone;
    
    
    return textureImage;
    }




PlantLeaf::PlantLeaf( PlantGenetics *inGenetics ) {
    int textureSize = 64;

    
    Image *textureImage = getCellularImage( textureSize, inGenetics,
                                            &mLeafWalkerTerminus );

    // count filled pixels to compute leaf area
    double *alphaChannel = textureImage->getChannel( 3 );

    int pixelsPerChannel = textureSize * textureSize;

    int numFilled = 0;

    for( int p=0; p<pixelsPerChannel; p++ ) {
        if( alphaChannel[p] == 1 ) {
            numFilled++;
            }
        }

    mLeafAreaFraction = (double)numFilled / (double)pixelsPerChannel;
    
    // blur all channels
    BoxBlurFilter blur( 1 );

    textureImage->filter( &blur);
    

    mTexture = new SingleTextureGL( textureImage,
                                    // no wrap
                                    false );

    delete textureImage; 
    }

        

PlantLeaf::~PlantLeaf() {
    delete mTexture;
    }



double PlantLeaf::getLeafAreaFraction() {
    return mLeafAreaFraction;
    }



void PlantLeaf::draw( Vector3D *inPosition, double inScale ) {
    Angle3D rotation( 0, 0, 0 );

    draw( inPosition, &rotation, inScale, NULL );
    }



void PlantLeaf::draw( Vector3D *inPosition, Angle3D *inRotation,
                      double inScale, Vector3D *outLeafWalkerTerminus ) {

    /*

    SimpleVector<Vector3D *> growthTips;
    SimpleVector<Angle3D *> tipAngles;

    // draw and insert one segment to start
    double currentLength = inScale / 2;
    
    Vector3D *start = new Vector3D( inPosition );
    Vector3D *end = new Vector3D( 0, 0, 0 );

    end->mY += currentLength;

    end->rotate( inRotation );

    end->add( start );


    glBegin( GL_LINES ); {
        
        glVertex3d( start->mX, start->mY, start->mZ );
        glVertex3d( end->mX, end->mY, end->mZ );
        }
    glEnd();

    delete start;
    
    // end is first tip

    growthTips.push_back( end );
    tipAngles.push_back( new Angle3D( inRotation ) );


    Angle3D incrementAngle( 0, 0, 0.4 );
    
    
    int numSteps = 5;

    while( numSteps > 0 ) {
        currentLength = currentLength /  2;
        
        SimpleVector<Vector3D *> newTips;
        SimpleVector<Angle3D *> newAngles;

        int numTips = growthTips.size();
        int i;
        
        for( i=0; i<numTips; i++ ) {
            Vector3D *oldTip = *( growthTips.getElement( i ) );
            Angle3D *tipAngle = *( tipAngles.getElement( i ) );
            
            // 2 branches at each tip

            Angle3D *angleA = new Angle3D( tipAngle );
            Angle3D *angleB = new Angle3D( tipAngle );

            angleA->add( &incrementAngle );
            angleB->subtract( &incrementAngle );

            Vector3D *tipA = new Vector3D( 0, currentLength, 0 );
            Vector3D *tipB = new Vector3D( 0, currentLength, 0 );

            tipA->rotate( angleA );
            tipB->rotate( angleB );

            tipA->add( oldTip );
            tipB->add( oldTip );

            // draw lines from old tip to new tips
            glBegin( GL_LINE_STRIP ); {

                glVertex3d( tipA->mX, tipA->mY, tipA->mZ );
                glVertex3d( oldTip->mX, oldTip->mY, oldTip->mZ );
                glVertex3d( tipB->mX, tipB->mY, tipB->mZ );
                }
            glEnd();

            newTips.push_back( tipA );
            newTips.push_back( tipB );

            newAngles.push_back( angleA );
            newAngles.push_back( angleB );
            
            
            delete oldTip;
            delete tipAngle;
            }

        // replace with new

        growthTips.deleteAll();
        tipAngles.deleteAll();


        numTips = newTips.size();

        for( i=0; i<numTips; i++ ) {
            growthTips.push_back( *( newTips.getElement( i ) ) );
            tipAngles.push_back( *( newAngles.getElement( i ) ) );
            }

        numSteps--;
        }

    

    // clean up

    int numTips = growthTips.size();
    int i;
    
    for( i=0; i<numTips; i++ ) {
        delete *( growthTips.getElement( i ) );
        delete *( tipAngles.getElement( i ) );
        }
    */    
    


    Vector3D corners[4];

    // first, set leaf base tip at zero
    corners[0].mX = - 0.5 * inScale;
    corners[0].mY = 0;
    corners[0].mZ = 0;

    corners[1].mX = 0.5 * inScale;
    corners[1].mY = 0;
    corners[1].mZ = 0;

    corners[2].mX = 0.5 * inScale;
    corners[2].mY = inScale;
    corners[2].mZ = 0;

    corners[3].mX = -0.5 * inScale;
    corners[3].mY = inScale;
    corners[3].mZ = 0;

    int i;
    
    // now rotate around base tip
    // then add inPosition so that tip is at inPosition
    for( i=0; i<4; i++ ) {
        corners[i].rotate( inRotation );
        corners[i].add( inPosition );
        }


    if( outLeafWalkerTerminus != NULL ) {
        outLeafWalkerTerminus->setCoordinates( &mLeafWalkerTerminus );

        outLeafWalkerTerminus->scale( inScale );

        outLeafWalkerTerminus->rotate( inRotation );
        outLeafWalkerTerminus->add( inPosition );        
        }
    
    
    

    if( Features::drawNicePlantLeaves ) {
        mTexture->enable();

        glBegin( GL_QUADS ); {
    
            glTexCoord2f( 0, 0 );
            glVertex3d( corners[0].mX, corners[0].mY, corners[0].mZ );
            
            glTexCoord2f( 1, 0 );
            glVertex3d( corners[1].mX, corners[1].mY, corners[1].mZ );
            
            glTexCoord2f( 1, 1 );
            glVertex3d( corners[2].mX, corners[2].mY, corners[2].mZ );
            
            glTexCoord2f( 0, 1 );
            glVertex3d( corners[3].mX, corners[3].mY, corners[3].mZ );
            }
        glEnd();
        mTexture->disable();
        }
    else {
        glBegin( GL_LINE_LOOP ); {
    
            glVertex3d( corners[0].mX, corners[0].mY, corners[0].mZ );
            
            glVertex3d( corners[1].mX, corners[1].mY, corners[1].mZ );
            
            glVertex3d( corners[2].mX, corners[2].mY, corners[2].mZ );
            
            glVertex3d( corners[3].mX, corners[3].mY, corners[3].mZ );
            }
        glEnd();
        }

    }
