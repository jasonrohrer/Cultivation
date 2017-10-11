/*
 * Modification History
 *
 * 2006-June-27  Jason Rohrer
 * Created.
 *
 * 2006-September-12   Jason Rohrer
 * Fixed the machine-gun mating bug.
 *
 * 2006-October-4   Jason Rohrer
 * Fixed music loudness bug.
 *
 * 2006-October-9   Jason Rohrer
 * Fixed crash on exit.
 * Reduced music sample rate.
 *
 * 2006-October-10   Jason Rohrer
 * Added a limit on maximum plot size.
 *
 * 2006-October-13   Jason Rohrer
 * Fixed a bug that can result in a crash at startup.
 *
 * 2006-October-30   Jason Rohrer
 * Started work on follow button.
 *
 * 2006-November-2   Jason Rohrer
 * Fixed a button selected object bug when user-controlled gardener dies.
 *
 * 2006-November-26   Jason Rohrer
 * Fixed bug reading language and font files on mac.
 *
 * 2006-December-14   Jason Rohrer
 * Fixed missing tooltips (and crash) after restart.
 * Added a pause button.
 * Switched to dragging for plot selection.
 *
 * 2007-November-17   Jason Rohrer
 * Fixed behavior when second corner of plot dropped on a GUI panel.
 * Disabled button clicks while paused (except to unpause or quit).
 *
 * 2007-December-11   Jason Rohrer
 * Locked max frame rate down to 80fps.
 */


#include <GL/glut.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


#include "minorGems/graphics/openGL/ScreenGL.h"
#include "minorGems/graphics/openGL/SceneHandlerGL.h"
#include "minorGems/graphics/Color.h"

// GL GUI includes
#include "minorGems/graphics/openGL/gui/GUIPanelGL.h"
#include "minorGems/graphics/openGL/gui/GUIContainerGL.h"
#include "minorGems/graphics/openGL/gui/GUITranslatorGL.h"
#include "minorGems/graphics/openGL/gui/ButtonGL.h"
#include "minorGems/graphics/openGL/gui/LabelGL.h"

// for tool tips
#include "minorGems/graphics/openGL/gui/TextGL.h"
#include "minorGems/io/file/FileOutputStream.h"
#include "minorGems/graphics/converters/TGAImageConverter.h"

// for video grabbing
// #include "minorGems/graphics/converters/PNGImageConverter.h"
// #include "minorGems/graphics/converters/JPEGImageConverter.h"

#include "minorGems/graphics/filters/BoxBlurFilter.h"
#include "minorGems/util/TranslationManager.h"


#include "minorGems/ui/event/ActionListener.h"




#include "minorGems/system/Time.h"
#include "minorGems/system/Thread.h"

// needed to synchronize between GL thread and sound thread
#include "minorGems/system/MutexLock.h"


#include "minorGems/util/stringUtils.h"

#include "minorGems/util/random/StdRandomSource.h"


#include "minorGems/math/geometry/Vector3D.h"


#include "features.h"
#include "gameFunctions.h"

#include "World.h"
#include "Plant.h"
#include "Gardener.h"
#include "GardenerAI2.h"

#include "glCommon.h"

#include "userInterface/TexturedPanel.h"
#include "userInterface/PlantButton.h"
#include "userInterface/PlotButton.h"
#include "userInterface/WaterButton.h"
#include "userInterface/HarvestButton.h"
#include "userInterface/EmotionButton.h"
#include "userInterface/MateButton.h"
#include "userInterface/FollowButton.h"
#include "userInterface/PoisonButton.h"
#include "userInterface/PauseButton.h"
#include "userInterface/RestartButton.h"
#include "userInterface/QuitButton.h"
#include "userInterface/NextTutorialButton.h"

#include "userInterface/TextBlockGL.h"
#include "userInterface/BorderPanel.h"

#include "userInterface/ObjectSelector.h"
#include "userInterface/EatButton.h"
#include "userInterface/GiftButton.h"
#include "userInterface/DiscardButton.h"


#include "sound/SoundPlayer.h"
#include "sound/SoundEffectsBank.h"
#include "sound/MusicPlayer.h"
#include "sound/MusicNoteWaveTable.h"



// seed with time for same behavior each time
int timeSeed = time( NULL );
//int timeSeed = 1156711006;
//int timeSeed = 1156767128;
//int timeSeed = 1156774629;
//int timeSeed = 1156775513;
StdRandomSource globalRandomSource( timeSeed );
//StdRandomSource globalRandomSource( 1156711006 );

// global pointer to current world
World *globalWorld;

Vector3D globalPlayerCurrentPosition;


// AI gardeners start out with 20x20 plots
// but player's screen limits player's plot to about 40x40
// to be fair, don't allow AI gardeners to make larger plots. 
double globalMaxPlotDimension = 40; 


int globalSoundSampleRate = 11025;

double globalMusicSongLength = 12;
int globalNumNotesPerMelody = 4;
double globalShortestNoteLength = 0.25;
double globalLongestNoteLength = 0.5;

int globalMaxSimultaneousSounds = 2;

// maximum number of gardeners we've seen so far
int globalMaxNumGardeners = 0;



SoundPlayer *globalSoundPlayer;
SoundEffectsBank *globalSoundEffectsBank;
MusicNoteWaveTable globalWaveTable( globalSoundSampleRate );
MusicPlayer globalMusicPlayer( globalSoundSampleRate, &globalWaveTable,
                               globalMusicSongLength );

// synchronize between GL thread and sound thread
MutexLock globalLock;


// set to true to gradually shift to mUserControlledGardener position
char smoothPositionTransition = false;
double smoothTransitionVelocity = 20;
        


// user action states
enum UserActionState{
    none = 1,
    plotFirstCorner,
    plotSecondCorner,
    // used to ignore button presses that may result when player
    // drops second corner of plot on top of a button
    doneDrawingPlot };




class GameSceneHandler :
    public SceneHandlerGL, public MouseHandlerGL, public KeyboardHandlerGL,
    public RedrawListenerGL, public ActionListener { 

        friend void addGardenerToGame( Gardener *inGardener,
                                       Vector3D *inPosition,
                                       Angle3D *inRotation );
        
	public:

        /**
         * Constructs a sceen handler.
         *
         * @param inScreen the screen to interact with.
         *   Must be destroyed by caller after this class is destroyed.
         */
        GameSceneHandler( ScreenGL *inScreen );

        virtual ~GameSceneHandler();


        
        /**
         * Executes necessary init code that reads from files.
         *
         * Must be called before using a newly-constructed GameSceneHandler.
         *
         * This call assumes that the needed files are in the current working
         * directory.
         */
        void initFromFiles();

        

        ScreenGL *mScreen;


        
        /**
         * Projects a point from the screen out onto the scene plane.
         *
         * @param inX, in Y, the x and y screen coordinates
         * @param outSceneX, outSceneY, pointers to where the sceen
         *   coordinates should be returned.
         */
        void projectScreenPointIntoScene( int inX, int inY,
                                          double *outSceneX,
                                          double *outSceneY );



        


        
        
        
		// implements the SceneHandlerGL interface
		virtual void drawScene();

        // implements the MouseHandlerGL interface
        virtual void mouseMoved( int inX, int inY );
        virtual void mouseDragged( int inX, int inY );
        virtual void mousePressed( int inX, int inY );
        virtual void mouseReleased( int inX, int inY );

        // implements the KeyboardHandlerGL interface
		virtual void keyPressed( unsigned char inKey, int inX, int inY );
		virtual void specialKeyPressed( int inKey, int inX, int inY );
		virtual void keyReleased( unsigned char inKey, int inX, int inY );
		virtual void specialKeyReleased( int inKey, int inX, int inY );
        
        // implements the RedrawListener interface
		virtual void fireRedraw();


        // implements the ActionListener interface
		virtual void actionPerformed( GUIComponent *inTarget );

        
        
    protected:
        
        // the time that the last frame was drawn
        unsigned long mLastFrameSeconds;
        unsigned long mLastFrameMilliseconds;

        // our current frame rate
        unsigned long mFrameMillisecondDelta;

        int mStartTimeSeconds;
        
        char mPaused;

        double mMaxFrameRate;

        char mPrintFrameRate;
        unsigned long mNumFrames;
        unsigned long mFrameBatchSize;
        unsigned long mFrameBatchStartTimeSeconds;
        unsigned long mFrameBatchStartTimeMilliseconds;


        Color mBackgroundColor;


        World mWorld;

        SoundPlayer mSoundPlayer;
        SoundEffectsBank mSoundEffectsBank;
        
        Gardener *mUserControlledGardener;
        
        SimpleVector<Gardener *> mOtherGardeners;
        SimpleVector<GardenerAI *> mOtherGardenerAI;
        

        // GUI
        GUITranslatorGL *mMainPanelGuiTranslator;
		GUIPanelGL *mMainPanel;
        GUIPanelGL *mSidePanel;

        BorderPanel *mTutorialPanel;
        TextBlockGL *mTutorialTextBlock;

        // the key of the current tutorial on display
        // must point to NULL or a const string
        char *mCurrentTutorialKey;

        char mTutorialEnabled;

        // track which tutorials have already been show
        SimpleVector<char *> mSpentTutorialKeys;

        
        
		// these do not need to be destroyed (destroyed by main panel)
		// they are only saved for use by the action listener function
		ButtonBase *mPlantButton;
        ButtonBase *mPlotButton;
        WaterButton *mWaterButton;
        ButtonBase *mHarvestButton;        
        MateButton *mMateButton;
        FollowButton *mFollowButton;
        PoisonButton *mPoisonButton;
        ButtonBase *mPauseButton;
        ButtonBase *mRestartButton;        
        ButtonBase *mQuitButton;
        ButtonBase *mEatButton;
        ButtonBase *mGiftButton;
        DiscardButton *mDiscardButton;

        ObjectSelector *mSelector;

        // tutorial buttons
        ButtonBase *mStopTutorialButton;
        ButtonBase *mNextTutorialButton;        
        
        UserActionState mActionState;

        // for tool tips
        TextGL *mTextGL;
        LabelGL *mToolTipLabel;
        GUIComponentGL *mCurrentTipGUIComponent;
        int mCurrentTipStoredItem;
        
        SimpleVector<ButtonGL*> mToolTipButtons;
        SimpleVector<const char*> mToolTipKeys;
        SimpleVector<double> mToolTipTimers;

        double mSelectorToolTipTimer;
        
        double mToolTipDisplayTime;


        // true if we're in video capture mode
        char mVideoCaptureMode;
        int mNumFramesToCapture;
        int mFramesCaptured;
        char *mFrameCaptureFileNamePrefix;
        int mVideoNumber;
        int mVideoFramesPerSecond;

        int mVideoFrameWidth;
        int mVideoFrameHeight;

        
        /**
         * Sets the second corner of user's plot.
         *
         * @param inPosition the world position of the second corner.
         *   Destroyed by caller.
         */
        void setSecondPlotCorner( Vector3D *inPosition );


        void updateStorageUI();


        /**
         * Shows a tutorial message.
         *
         * Each message is shown at most once (a second call with
         * the same key displays nothing).
         *
         * @param inTutorialKey the key of the tutorial to show.
         */
        void showTutorial( const char *inTutorialKey );

        
        /**
         * Called by event handlers when a given tutorial message is dismissed.
         *
         * @param inTutorialKey the key of the tutorial.
         */
        void tutorialDone( const char *inTutorialKey );

        
        
	};





// a restart is triggered by a button press, but since it involves destroying
// and recreating the scene handler (which contains the buttons), we can
// only handle the reset in our fireRedraw callback (called by
// screen, which we don't destroy)
char restartFlag = false;

GameSceneHandler *sceneHandler;
ScreenGL *screen;

double baseViewZ = -23;



// function that destroys object when exit is called.
// exit is the only way to stop the GLUT-based ScreenGL
void cleanUpAtExit() {
    printf( "exiting\n" );

    destroyCommonTextures();
    
    delete sceneHandler;
    delete screen;
    }




int main( int inNumArgs, char **inArgs ) {

    printf( "Game seed = %d\n", timeSeed );    
    
    
    // must pass args to GLUT before constructing the screen
    glutInit( &inNumArgs, inArgs );

    screen =
        new ScreenGL( 300, 300, false, 80, false, 
                      "Cultivation", NULL, NULL, NULL );

    // lock while sceneHandler constructed so that
    // sound player does not try to access world before it is constructed
    globalLock.lock();
    
    sceneHandler = new GameSceneHandler( screen );

    // safe for sound player to touch world now
    globalLock.unlock();
    
    
    Vector3D move( 0, 0, baseViewZ );
    screen->moveView( &move );
    

    // do this mac check after constructing scene handler and screen,
    // since these cause various Mac frameworks to be loaded (which can
    // change the current working directory out from under us)
    #ifdef __mac__
        // make sure working directory is the same as the directory
        // that the app resides in
        // this is especially important on the mac platform, which
        // doesn't set a proper working directory for double-clicked
        // app bundles

        // arg 0 is the path to the app executable
        char *appDirectoryPath = stringDuplicate( inArgs[0] );
    
        char *appNamePointer = strstr( appDirectoryPath,
                                       "Cultivation.app" );

        if( appNamePointer != NULL ) {
            // terminate full app path to get parent directory
            appNamePointer[0] = '\0';
            
            chdir( appDirectoryPath );
            }
        
        delete [] appDirectoryPath;
    #endif

        
    // read features from file here, after initializing mac working directory
    initializeFeatures();

    // also do file-dependent part of init for GameSceneHandler here
    sceneHandler->initFromFiles();
    
    if( Features::largeWindow ) {
        screen->changeWindowSize( 600, 600 );
        }
        
    // register cleanup function, since screen->start() will never return
    atexit( cleanUpAtExit );

    

    screen->start();

    
    return 0;
    }



int getMaxDistanceForTransactions() {
    return 20;
    }



double getGardenerZPosition() {
    return -2.5;
    }



void addGardenerToGame( Gardener *inGardener,
                        Vector3D *inPosition,
                        Angle3D *inRotation ) {

    inGardener->setPlotHidden( false );
    
    World *world = &( sceneHandler->mWorld );

    world->addGardener( inGardener, inPosition, inRotation );
        
    GardenerAI *ai = new GardenerAI( inGardener, world );

    sceneHandler->mOtherGardeners.push_back( inGardener );
    sceneHandler->mOtherGardenerAI.push_back( ai );
    }



void setNumGardeners( int inCount ) {
    int numGardeners = inCount;

    // avoid divide by zero
    if( numGardeners < 1 ) {
        numGardeners = 1;
        }

    if( globalMaxNumGardeners < numGardeners ) {
        // an increase
        
        globalMaxNumGardeners = numGardeners;
        }

    // set volume based on maximum number of gardeners seen so far
    // Thus, as population grows, per-song volume declines
    // but as population shrinks, volume stays constant.

    // Otherwise, if we increase volume as gardeners die off, music
    // gets louder and louder (expecially when player's gardener is
    // last one left---it can be annoyingly loud).
    
    
    // avoid clipping
    double musicLoudness =
        ( 1 - 0.1 * globalMaxSimultaneousSounds ) / globalMaxNumGardeners;

    // make sure we don't access player after it has been destroyed
    if( globalSoundPlayer != NULL ) {
        globalSoundPlayer->setMusicLoudness( musicLoudness );
        }
    }




double mousePositionX=0, mousePositionY=0; 

Vector3D worldCornerA( -60, -60, 0 );
Vector3D worldCornerB( 60, 60, 0 );

GameSceneHandler::GameSceneHandler( ScreenGL *inScreen )
    : mScreen( inScreen ),
      mFrameMillisecondDelta( 0 ),
      mStartTimeSeconds( time( NULL ) ),
      mPaused( false ),
      mMaxFrameRate( 80 ),  // limit frame rate
      mPrintFrameRate( true ),
      mNumFrames( 0 ), mFrameBatchSize( 100 ),
      mFrameBatchStartTimeSeconds( time( NULL ) ),
      mFrameBatchStartTimeMilliseconds( 0 ),
      mBackgroundColor( 0, 0, 1, 1 ),
      mWorld( &worldCornerA, &worldCornerB ),
      mSoundPlayer( globalSoundSampleRate,
                    globalMaxSimultaneousSounds,
                    &globalMusicPlayer,
                    // default loudness
                    // set later when gardeners added
                    1,
                    // start global loudness at zero, fade in later
                    0 ),
      mSoundEffectsBank( &mSoundPlayer ),
      mActionState( none ) {
    
    

    glClearColor( mBackgroundColor.r,
                  mBackgroundColor.g,
                  mBackgroundColor.b,
                  mBackgroundColor.a );
    

    // set external pointer so it can be used in calls below
    sceneHandler = this;

    globalWorld = &mWorld;
    globalSoundPlayer = &mSoundPlayer;
    globalSoundEffectsBank = &mSoundEffectsBank;
    
    mScreen->addMouseHandler( this );
    mScreen->addKeyboardHandler( this );
    mScreen->addSceneHandler( this );
    mScreen->addRedrawListener( this );

    
    Time::getCurrentTime( &mLastFrameSeconds, &mLastFrameMilliseconds );

    // start user's gardner in random land spot 
    Vector3D startPosition( -100, -100, 0 );

    while( mWorld.isInWater( &startPosition ) ) {
        double x = globalRandomSource.getRandomBoundedDouble(
            worldCornerA.mX, worldCornerB.mX );
        double y = globalRandomSource.getRandomBoundedDouble(
            worldCornerA.mY, worldCornerB.mY );
    
        startPosition.setCoordinates( x, y, 0 );
        }
    
    mUserControlledGardener = new Gardener( &startPosition );

    mUserControlledGardener->setPlotHidden( false );

    // start off with 3random seeds
    mUserControlledGardener->storeItem( new Seeds() );
    mUserControlledGardener->storeItem( new Seeds() );
    mUserControlledGardener->storeItem( new Seeds() );
        
    
    mWorld.addGardener( mUserControlledGardener, &startPosition );

    mUserControlledGardener->mUserCanControl = true;
    mUserControlledGardener->mUserControlling = true;
    

    int numOtherGardeners = 3;
    

    for( int i=0; i<numOtherGardeners; i++ ) {

        // start gardner in random land spot 
        Vector3D gardenerPosition( -100, -100, 0 );

        while( mWorld.isInWater( &gardenerPosition ) ) {
            double x = globalRandomSource.getRandomBoundedDouble(
                worldCornerA.mX, worldCornerB.mX );
            double y = globalRandomSource.getRandomBoundedDouble(
                worldCornerA.mY, worldCornerB.mY );
            
            gardenerPosition.setCoordinates( x, y, 0 );
            }

        
        Gardener *gardener = new Gardener( &gardenerPosition );

        // start off with 3 random seeds
        gardener->storeItem( new Seeds() );
        gardener->storeItem( new Seeds() );
        gardener->storeItem( new Seeds() );

        addGardenerToGame( gardener, &gardenerPosition );

        // FIXME: watch this gardener instead
        // mUserControlledGardener = gardener;
        }
        
    
    mMainPanel = new TexturedPanel( 0, 0, 2, 0.1,
                                    new Color( 1,
                                               1,
                                               1, 0.5 ),
                                    false );

	mMainPanelGuiTranslator = new GUITranslatorGL( mMainPanel, screen );

	mScreen->addSceneHandler( mMainPanelGuiTranslator );
	mScreen->addMouseHandler( mMainPanelGuiTranslator );
	mScreen->addKeyboardHandler( mMainPanelGuiTranslator );


    mPlantButton = new PlantButton( 0.1, 0.01, 0.08, 0.08 );

    mMainPanel->add( mPlantButton );
    mPlantButton->addActionListener( this );


    mPlotButton = new PlotButton( 0.2, 0.01, 0.08, 0.08 );

    mMainPanel->add( mPlotButton );
    mPlotButton->addActionListener( this );


    mWaterButton = new WaterButton( 0.3, 0.01, 0.08, 0.08 );

    mMainPanel->add( mWaterButton );
    mWaterButton->addActionListener( this );


    mHarvestButton = new HarvestButton( 0.4, 0.01, 0.08, 0.08 );

    mMainPanel->add( mHarvestButton );
    mHarvestButton->addActionListener( this );    


    mMateButton = new MateButton( 0.5, 0.01, 0.08, 0.08 );

    mMainPanel->add( mMateButton );
    mMateButton->addActionListener( this );    


    
    mFollowButton = new FollowButton( 0.6, 0.01, 0.08, 0.08 );

    mMainPanel->add( mFollowButton );
    mFollowButton->addActionListener( this );    


    
    mPoisonButton = new PoisonButton( 0.7, 0.01, 0.08, 0.08 );

    mMainPanel->add( mPoisonButton );
    mPoisonButton->addActionListener( this );    

    

    // only show one (restart or pause)
    mPauseButton = new PauseButton( 0.8, 0.01, 0.08, 0.08 );
    mRestartButton = new RestartButton( 0.8, 0.01, 0.08, 0.08 );

    mMainPanel->add( mPauseButton );
    //mMainPanel->add( mRestartButton );
    
    mPauseButton->addActionListener( this );
    mRestartButton->addActionListener( this );    

    mQuitButton = new QuitButton( 0.9, 0.01, 0.08, 0.08 );

    mMainPanel->add( mQuitButton );
    mQuitButton->addActionListener( this );


    // layout side panel
    mSidePanel = new TexturedPanel( 0, 0.1, 0.1, 2,
                                    new Color( 1,
                                               1,
                                               1, 0.5 ),
                                    true );

    mMainPanel->add( mSidePanel );


    mDiscardButton = new DiscardButton( 0.01, 0.1, 0.08, 0.08 );

    mSidePanel->add( mDiscardButton );
    mDiscardButton->addActionListener( this );


    mSelector = new ObjectSelector( 0.01, 0.3, 0.08, 0.38,
                                    mUserControlledGardener );

    mSidePanel->add( mSelector );

    mEatButton = new EatButton( 0.01, 0.8, 0.08, 0.08 );

    mSidePanel->add( mEatButton );
    mEatButton->addActionListener( this );

    mGiftButton = new GiftButton( 0.01, 0.9, 0.08, 0.08 );

    mSidePanel->add( mGiftButton );
    mGiftButton->addActionListener( this );


    // all action buttons start out disabled (until we enable appropriate
    // ones during our first fireRedraw)
    // otherwise, we see a one-frame flash with all buttons enabled
    mPlantButton->setEnabled( false );
    
    mWaterButton->setEnabled( false );
    mHarvestButton->setEnabled( false );
    mEatButton->setEnabled( false );
    mGiftButton->setEnabled( false );
    mMateButton->setEnabled( false );
    mFollowButton->setEnabled( false );
    mPoisonButton->setEnabled( false );
    mDiscardButton->setEnabled( false );
    mRestartButton->setEnabled( false );

    // special exceptions:  plot, quit, and pause enabled
    mPlotButton->setEnabled( true );
    mQuitButton->setEnabled( true );
    mPauseButton->setEnabled( true );


    mTutorialPanel = new BorderPanel( 0.15, 0.7, 0.8, 0.3,
                                      new Color( 0,
                                                 0,
                                                 0, 0.75 ) );

    //mMainPanel->add( mTutorialPanel );

    mStopTutorialButton = new QuitButton( 0.19, 0.7, 0.04, 0.04 );
    mTutorialPanel->add( mStopTutorialButton );

    mNextTutorialButton = new NextTutorialButton( 0.87, 0.7, 0.04, 0.04 );
    mTutorialPanel->add( mNextTutorialButton );

    mStopTutorialButton->addActionListener( this );
    mNextTutorialButton->addActionListener( this );
    
    mCurrentTutorialKey = NULL;
    mTutorialEnabled = true;
    

    mVideoCaptureMode = false;
    // video in 5-second batches
    mNumFramesToCapture = 150;
    mFramesCaptured = 0;
    mVideoFramesPerSecond = 30;
    mFrameCaptureFileNamePrefix = (char *)"video/video_";
    mVideoNumber = 8;
    mVideoFrameWidth = 640;
    mVideoFrameHeight = 480;

    //mVideoFrameWidth = 320;
    //mVideoFrameHeight = 240;

    
    // all gardeners added, ready to go

    // start the sound fade in
    // (this avoids "garbage" sound being generated as gardeners are created).
    mSoundPlayer.fadeIn( 4 );
    }



GameSceneHandler::~GameSceneHandler() {
    mScreen->removeMouseHandler( this );
    mScreen->removeSceneHandler( this );
    mScreen->removeRedrawListener( this );

    // delete whichever button is not in the panel
    if( ! mMainPanel->contains( mPauseButton ) ) {
        delete mPauseButton;
        }
    if( ! mMainPanel->contains( mRestartButton ) ) {
        delete mRestartButton;
        }

    // delete tutorial panel if it is not showing
    if( ! mMainPanel->contains( mTutorialPanel ) ) {
        delete mTutorialPanel;
        }
    
    // this will recursively delete all of our GUI components
	delete mMainPanelGuiTranslator;

    int numOtherGardeners = mOtherGardeners.size();
    for( int i=0; i<numOtherGardeners; i++ ) {
        GardenerAI *ai = *( mOtherGardenerAI.getElement( i ) );
        delete ai;
        }

    // make sure we don't try to access these anymore
    globalSoundPlayer = NULL;
    globalWorld = NULL;

    delete mTextGL;
    }



void GameSceneHandler::initFromFiles() {
    // tool tips
    // load font image
    TGAImageConverter tga;

    File fontFile( NULL, "font.tga" );
    FileInputStream fontInput( &fontFile );

    Image *fontImage = tga.deformatImage( &fontInput );

    //BoxBlurFilter blur( 1 );
    //fontImage->filter( &blur );

    if( fontImage == NULL ) {
        // default
        // blank font
        fontImage = new Image( 256, 256, 4, true );
        }
    
    mTextGL = new TextGL( fontImage,
                          // use alpha
                          true,
                          // variable character width
                          false,
                          // extra space around each character
                          0.1,
                          // space is half a character width
                          0.75 );
    delete fontImage;


    const char *startString = "";
    
    mTutorialTextBlock =
        new TextBlockGL( 0.19, 0.74, 0.72, 0.22,
                         (char *)startString, mTextGL, 37 );
    mTutorialPanel->add( mTutorialTextBlock );


    
    mToolTipButtons.push_back( mPlantButton );
    mToolTipButtons.push_back( mPlotButton );
    mToolTipButtons.push_back( mWaterButton );
    mToolTipButtons.push_back( mHarvestButton );        
    mToolTipButtons.push_back( mMateButton );
    mToolTipButtons.push_back( mFollowButton );
    mToolTipButtons.push_back( mPoisonButton );
    mToolTipButtons.push_back( mPauseButton );
    mToolTipButtons.push_back( mRestartButton );
    mToolTipButtons.push_back( mQuitButton );
    mToolTipButtons.push_back( mEatButton );
    mToolTipButtons.push_back( mGiftButton );
    mToolTipButtons.push_back( mDiscardButton );
    mToolTipButtons.push_back( mStopTutorialButton );
    mToolTipButtons.push_back( mNextTutorialButton );
    

    mToolTipKeys.push_back( "tip_PlantButton" );
    mToolTipKeys.push_back( "tip_PlotButton" );
    mToolTipKeys.push_back( "tip_WaterButton" );
    mToolTipKeys.push_back( "tip_HarvestButton" );        
    mToolTipKeys.push_back( "tip_MateButton" );
    mToolTipKeys.push_back( "tip_LeadButton" );
    mToolTipKeys.push_back( "tip_PoisonButton" );
    mToolTipKeys.push_back( "tip_PauseButton" );
    mToolTipKeys.push_back( "tip_RestartButton" );
    mToolTipKeys.push_back( "tip_QuitButton" );
    mToolTipKeys.push_back( "tip_EatButton" );
    mToolTipKeys.push_back( "tip_GiftButton" );
    mToolTipKeys.push_back( "tip_DiscardButton" );
    mToolTipKeys.push_back( "tip_StopTutorial" );
    mToolTipKeys.push_back( "tip_NextTutorial" );
    

    int numTips = mToolTipButtons.size();
    for( int t=0; t<numTips; t++ ) {
        mToolTipTimers.push_back( 0 );
        }
    mSelectorToolTipTimer = 0;
    
    mToolTipDisplayTime = 1;

    // label starts out as NULL
    mToolTipLabel = NULL;
    mCurrentTipGUIComponent = NULL;
    mCurrentTipStoredItem = -1;
    
    // translation language for tool tips
    File languageNameFile( NULL, "language.txt" );

    if( languageNameFile.exists() ) {
        char *languageNameText = languageNameFile.readFileContents();

        SimpleVector<char *> *tokens = tokenizeString( languageNameText );

        int numTokens = tokens->size();
        
        // first token is name
        if( numTokens > 0 ) {
            char *languageName = *( tokens->getElement( 0 ) );
        
            TranslationManager::setLanguage( languageName );
            }
        else {
            // default

            // TranslationManager already defaults to English, but
            // it looks for the language files at runtime before we have set
            // the current working.
            
            // Thus, we specify the default again here so that it looks
            // for its language files again.
            TranslationManager::setLanguage( "English" );
            }
        
        delete [] languageNameText;

        for( int t=0; t<numTokens; t++ ) {
            delete [] *( tokens->getElement( t ) );
            }
        delete tokens;
        }

    // ready for first tutorial

    showTutorial( "tutorial_question" );
    }



void GameSceneHandler::drawScene() {
    /*
    glClearColor( mBackgroundColor->r,
                  mBackgroundColor->g,
                  mBackgroundColor->b,
                  mBackgroundColor->a );
    */
	
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );
    




    




    // draw world objects here
    

    // draw the world
    mWorld.draw();


    // if in middle of specifying a new plot
    if( mActionState == plotFirstCorner ||
        mActionState == plotSecondCorner ) {

        // draw cross hairs from edge of screen to mouse position

        int w = screen->getWidth();
        int h = screen->getHeight();
            
        double xLeft, xRight, yBottom, yTop;

        // NOTE:
        // Cannot call this inside glBegin / glEnd for some reson
        // (results in NAN values if we do).
        projectScreenPointIntoScene( 0, 0, &xLeft, &yBottom );
        projectScreenPointIntoScene( w-1, h-1, &xRight, &yTop );

        
        glLineWidth( 1 );
        glBegin( GL_LINES ); {
            
            glColor4f( 1, 1, 1, 0.5 );            

            // horizontal
            glVertex2d( xLeft, mousePositionY );
            glVertex2d( xRight, mousePositionY );

            // vertical
            glVertex2d( mousePositionX, yBottom );
            glVertex2d( mousePositionX, yTop );
            }
        glEnd();
        
        }
        

    
    // draw a small crosshair to show user's desired position (where moving to)
    if( mUserControlledGardener->isMoving() ) {

        Vector3D *p = mUserControlledGardener->getDesiredPosition();

        glColor4f( 1, 1, 1, 0.5 );
        Angle3D crossAngle( 0, 0, M_PI / 4 );
        drawBlurPlus( p, 1, &crossAngle );
        
        delete p;
        }


    if( mVideoCaptureMode ) {
        int w = mScreen->getWidth();
        int h = mScreen->getHeight();

        // make sure window size change has kicked in
        // (size change happens after next redraw)
        // Otherwise, size of first frame may differ from subsequent frames.
        if( w == mVideoFrameWidth && h == mVideoFrameHeight ) {
            unsigned char *rgbBytes = new unsigned char[ w * h * 3 ];

            glReadPixels( 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, rgbBytes );

            Image frameImage( w, h, 3, false );

            double *channels[3];
            int c;
            for( c=0; c<3; c++ ) {
                channels[c] = frameImage.getChannel( c );
                }

            // image of screen is upside down
            int outputRow = 0;
            for( int y=h-1; y>=0; y-- ) {
                for( int x=0; x<w; x++ ) {

                    int outputPixelIndex = outputRow * w + x;

                    int screenPixelIndex = y * w + x;
                    int byteIndex = screenPixelIndex * 3;
            
                    for( c=0; c<3; c++ ) {
                        channels[c][outputPixelIndex] =
                            rgbBytes[ byteIndex + c ] / 255.0;
                        }
                    }
                outputRow++;
                }

        
            delete [] rgbBytes;


            char *outFileName = autoSprintf( "%s%d_%d.jpg",
                                             mFrameCaptureFileNamePrefix,
                                             mVideoNumber,
                                             mFramesCaptured );

            printf( "Output %s\n", outFileName );
            
            File frameFile( NULL, outFileName );
            FileOutputStream frameOutput( &frameFile );


            
            //PNGImageConverter png;
            //png.formatImage( &frameImage, &frameOutput );

            // comment out here so we don't need to link against libjpeg
            //JPEGImageConverter jpg( 95 );
            //jpg.formatImage( &frameImage, &frameOutput );
        
            mFramesCaptured++;
        

            if( mFramesCaptured >= mNumFramesToCapture ) {
                mVideoCaptureMode = false;

                // auto-pause after capturing a batch
                mPaused = true;
                
                // next capture uses new video number
                mVideoNumber ++;
                
                // show UI
                //mScreen->addSceneHandler( mMainPanelGuiTranslator );

                // shrink window
                mScreen->changeWindowSize( 300, 300 );
                }
            }
        }
    }



void GameSceneHandler::mouseMoved( int inX, int inY ) {   
    projectScreenPointIntoScene( inX, inY, &mousePositionX, &mousePositionY );
    
    Vector3D position( mousePositionX, mousePositionY, 0 );
    }



void GameSceneHandler::mouseDragged( int inX, int inY ) {
    double guiX, guiY;
    mMainPanelGuiTranslator->translate( inX, inY, &guiX, &guiY );
    
        
    projectScreenPointIntoScene( inX, inY,
                                 &mousePositionX, &mousePositionY );
    

    Vector3D position( mousePositionX, mousePositionY, 0 );

    switch( mActionState ) {
        case none:
            // not in the middle of an action

            //ignore it
                
            break;
        case plotFirstCorner:

            // start drawing a new plot
            mWorld.setGardenerPlot( mUserControlledGardener,
                                    &position, &position );

            mActionState = plotSecondCorner;
                
            break;
        case plotSecondCorner:
            // in progress of dragging to select
                
            setSecondPlotCorner( &position );
            break;
        case doneDrawingPlot:
            // should never hit this case
            printf( "Warning, hit an unexpected case in "
                    "GameSceneHandler::mouseDragged\n" );                
            break;
        }

    }




void GameSceneHandler::mousePressed( int inX, int inY ) {

    double guiX, guiY;
    mMainPanelGuiTranslator->translate( inX, inY, &guiX, &guiY );
    
        
    projectScreenPointIntoScene( inX, inY,
                                 &mousePositionX, &mousePositionY );
    

    Vector3D position( mousePositionX, mousePositionY, 0 );

    switch( mActionState ) {
        case none:
            // not in the middle of an action

            //ignore it
                
            break;
        case plotFirstCorner:

            // start drawing a new plot
            mWorld.setGardenerPlot( mUserControlledGardener,
                                    &position, &position );

            mActionState = plotSecondCorner;
                
            break;
        case plotSecondCorner:

            // should never hit this case
            printf( "Warning, hit an unexpected case in "
                    "GameSceneHandler::mousePressed\n" );
            break;
        case doneDrawingPlot:
            // should never hit this case
            printf( "Warning, hit an unexpected case in "
                    "GameSceneHandler::mousePressed\n" );                
            break;
        } 

    }



void GameSceneHandler::mouseReleased( int inX, int inY ) {

    double guiX, guiY;
    mMainPanelGuiTranslator->translate( inX, inY, &guiX, &guiY );
    
    char inGuiRegion = true;
    
    if( ! mMainPanel->isInside( guiX, guiY )
        &&
        ! mSidePanel->isInside( guiX, guiY )
        &&
        ( ! mMainPanel->contains( mTutorialPanel ) ||
          ! mTutorialPanel->isInside( guiX, guiY ) ) ) {
        
        inGuiRegion = false;
        }
    
    
    projectScreenPointIntoScene( inX, inY,
                                 &mousePositionX, &mousePositionY );
    
    
    Vector3D position( mousePositionX, mousePositionY, 0 );

    switch( mActionState ) {
        case none:
            // not in the middle of an action

            // trying to move?
                
            // ignore clicks that are inside the main GUI panel
            if( !inGuiRegion ) {
                // move the user's gardener        
                mUserControlledGardener->setDesiredPosition( &position );
                }                
            break;
        case plotFirstCorner:

            // should never hit this case
            printf( "Warning, hit an unexpected case in "
                    "GameSceneHandler::mouseReleased\n" );                
            break;
        case plotSecondCorner:
            setSecondPlotCorner( &position );
                
            // done drawing plot
            if( inGuiRegion ) {
                // second corner landed in gui region

                // tell GUI to ignore click
                mActionState = doneDrawingPlot;
                }
            else {
                mActionState = none;
                }
                
            mPlotButton->setEnabled( true );

            if( mWorld.getPlotArea( mUserControlledGardener ) > 0 ) {
                showTutorial( "tutorial_plant" );
                }
            break;
        case doneDrawingPlot:
            // should never hit this case
            printf( "Warning, hit an unexpected case in "
                    "GameSceneHandler::mouseReleased\n" );                
            break;
        } 
    }



void GameSceneHandler::fireRedraw() {

    if( restartFlag ) {
        // destroy self

        mScreen->removeSceneHandler( sceneHandler );
        mScreen->removeKeyboardHandler( sceneHandler );
        
        mScreen->removeSceneHandler( mMainPanelGuiTranslator );
        mScreen->removeMouseHandler( mMainPanelGuiTranslator );
        mScreen->removeKeyboardHandler( mMainPanelGuiTranslator );
        

        // prevent bad access from audio thread during deletion and
        // new construction
        globalLock.lock();

        delete sceneHandler;

        // construt new
        sceneHandler = new GameSceneHandler( screen );

        globalLock.unlock();


        
        sceneHandler->initFromFiles();
        
        restartFlag = false;

        smoothPositionTransition = true;
        return;
        }

    
    if( mPaused ) {
        // ignore redraw event

        // sleep to avoid wasting CPU cycles
        Thread::staticSleep( 1000 );
        
        // also ignore time that passes while paused
        Time::getCurrentTime( &mLastFrameSeconds, &mLastFrameMilliseconds );
        
        return;
        }

    
    globalLock.lock();


    
    // deal with frame timing issues
    
    unsigned long lastMillisecondDelta = mFrameMillisecondDelta;
    
    // how many milliseconds have passed since the last frame
    mFrameMillisecondDelta =
        Time::getMillisecondsSince( mLastFrameSeconds,
                                    mLastFrameMilliseconds );

    
    // lock down to mMaxFrameRate frames per second
    unsigned long minFrameTime = (unsigned long)( 1000 / mMaxFrameRate );
    if( mFrameMillisecondDelta < minFrameTime ) {
        unsigned long timeToSleep = minFrameTime - mFrameMillisecondDelta;
        Thread::staticSleep( timeToSleep );

        // get new frame second delta, including sleep time
        mFrameMillisecondDelta =
            Time::getMillisecondsSince( mLastFrameSeconds,
                                        mLastFrameMilliseconds );
        }

    // avoid huge position "jumps" if we have a very large delay during a frame
    // (possibly caused by something going on in the background)
    // This will favor a slight visual slow down, but this is better than
    // a disorienting jump

    // skip this check if we are just starting up
    if( lastMillisecondDelta != 0 ) {
        if( mFrameMillisecondDelta > 6 * lastMillisecondDelta ) {
            // limit:  this frame represents at most twice the jump of the last
            // frame
            // printf( "Limiting time jump (requested=%lu ms, last=%lu ms)\n",
            //        mFrameMillisecondDelta, lastMillisecondDelta );

            if( mFrameMillisecondDelta > 10000 ) {
                printf( "Time between frames more than 10 seconds:\n" );
                // way too big... investigate
                printf( "Last time = %lu s, %lu ms\n",
                        mLastFrameSeconds, mLastFrameMilliseconds );

                Time::getCurrentTime( &mLastFrameSeconds,
                                      &mLastFrameMilliseconds );
                printf( "current time = %lu s, %lu ms\n",
                        mLastFrameSeconds, mLastFrameMilliseconds );

                }
            
            mFrameMillisecondDelta = 2 * lastMillisecondDelta;
            
            }
        }
    
    double frameSecondsDelta = (double)mFrameMillisecondDelta / 1000.0;

    
    // record the time that this frame was drawn
    Time::getCurrentTime( &mLastFrameSeconds, &mLastFrameMilliseconds );


    if( mVideoCaptureMode ) {
        // ignore true frame time
        frameSecondsDelta = 1.0 / (double)mVideoFramesPerSecond;
        }

    
    double worldTimePassedInSeconds = frameSecondsDelta;

    // no time passes while tutorial up
    // (however, we still used frameSecondsDelta below for tool tip timing)
    if( mMainPanel->contains( mTutorialPanel ) ) {
        worldTimePassedInSeconds = 0;
        }
    
    // tell managers about the time delta

    mWorld.passTime( worldTimePassedInSeconds );

    int numOtherGardeners = mOtherGardeners.size();

    int i;
    for( i=0; i<numOtherGardeners; i++ ) {
        
        GardenerAI *ai = *( mOtherGardenerAI.getElement( i ) );
        
        ai->passTime( worldTimePassedInSeconds );
        }



    mNumFrames ++;

    if( mPrintFrameRate ) {
        
        if( mNumFrames % mFrameBatchSize == 0 ) {
            // finished a batch
            
            unsigned long timeDelta =
                Time::getMillisecondsSince( mFrameBatchStartTimeSeconds,
                                            mFrameBatchStartTimeMilliseconds );

            double frameRate =
                1000 * (double)mFrameBatchSize / (double)timeDelta;
            
            printf( "Frame rate = %f frames/second\n", frameRate );
            
            mFrameBatchStartTimeSeconds = mLastFrameSeconds;
            mFrameBatchStartTimeMilliseconds = mLastFrameMilliseconds;
            }
        }




    
    // find and remove dead
    char foundDead = true;
    while( foundDead ) {
        
        foundDead = false;
        
        numOtherGardeners = mOtherGardeners.size();
        
        for( i=0; i<numOtherGardeners && !foundDead; i++ ) {
            
            Gardener *gardener = *( mOtherGardeners.getElement( i ) );
            if( gardener->isDead() ) {
                
                GardenerAI *ai = *( mOtherGardenerAI.getElement( i ) );
                
                mOtherGardeners.deleteElementEqualTo( gardener );
                mOtherGardenerAI.deleteElementEqualTo( ai );
                
                delete ai;
                
                // this will delete gardener
                mWorld.removeGardener( gardener );
                
                foundDead = true;
                }
            }
        }



    // timing for tool tips
    double toolTipShrinkFactor = 0.7;
    
    mToolTipButtons.size();

    int numTips = mToolTipButtons.size();
    for( int t=0; t<numTips; t++ ) {
        ButtonGL *button = *( mToolTipButtons.getElement( t ) );

        double *timerPointer = mToolTipTimers.getElement( t );

        // special case
        // buttons that are enabled but not visible
        char ignoreMouseOver = false;
        if( ( button == mNextTutorialButton
              ||
              button == mStopTutorialButton )
            &&
            ! mMainPanel->contains( mTutorialPanel ) ) {

            ignoreMouseOver = true;
            }
        
        if( ! ignoreMouseOver && button->isMouseOver() ) {
            *timerPointer += frameSecondsDelta;
            }
        else {
            *timerPointer = 0;

            if( mCurrentTipGUIComponent == button ) {
                // mouse moved out of current button
                
                // remove old
                mMainPanel->remove( mToolTipLabel );
                delete mToolTipLabel;
                mToolTipLabel = NULL;
                mCurrentTipGUIComponent = NULL;
                }
            }

        
        if( ! mVideoCaptureMode &&
            *timerPointer >= mToolTipDisplayTime ) {

            if( mCurrentTipGUIComponent != button ) {
                // switch tip labels

                if( mToolTipLabel != NULL ) {
                    // remove old
                    mMainPanel->remove( mToolTipLabel );
                    delete mToolTipLabel;
                    mToolTipLabel = NULL;
                    mCurrentTipGUIComponent = NULL;
                    }
                
                const char *tipKey = *( mToolTipKeys.getElement( t ) );

                char *tipString =
                    (char *)TranslationManager::translate( tipKey );

                // special case extra prefix for water button
                // and lead button
                const char *tipStringPrefix = "";
                if( button == mWaterButton ) {
                    if( mUserControlledGardener->getCarryingWater() ) {

                        tipStringPrefix =
                            TranslationManager::translate(
                                "tip_water_dump" );
                        }
                    else {
                        tipStringPrefix =
                            TranslationManager::translate(
                                "tip_water_pickup" );
                        }
                    }
                else if( button == mFollowButton ) {
                    Gardener *closestGardener =
                        mWorld.getClosestGardener( mUserControlledGardener );

                    if( closestGardener != NULL &&
                        closestGardener->getLeader() ==
                           mUserControlledGardener ) {

                        // already leading closest
                        // button would drop them
                        tipStringPrefix =
                            (char *)TranslationManager::translate(
                                                        "tip_stop_leading" );
                        }
                    else {
                        tipStringPrefix =
                            (char *)TranslationManager::translate(
                                                        "tip_lead" );
                        }
                    }

                char *totalTipString = autoSprintf( "%s%s", tipStringPrefix,
                                                    tipString );
                
                // center tooltip label at top of screen

                // 1:1 aspect ratio
                double height = 0.04;
                double width = height * strlen( totalTipString );

                height *= toolTipShrinkFactor;
                width *= toolTipShrinkFactor;

                double actualDrawWidth =
                    height * mTextGL->measureTextWidth( totalTipString );
                
                // because of side panel, center of main panel looks like
                // it is at 0.55 instead of 0.5
                mToolTipLabel = new LabelGL( 0.55 - 0.5 * actualDrawWidth,
                                             0.11,
                                             width,
                                             height,
                                             totalTipString,
                                             mTextGL );

                delete [] totalTipString;
                
                mMainPanel->add( mToolTipLabel );

                mCurrentTipGUIComponent = button;
                }
            }
        }

    // special case:
    // tool tips for object selector
    if( mCurrentTipGUIComponent == NULL ||
        mCurrentTipGUIComponent == mSelector ) {

        // check tip timing for object selector

        int objectIndex = mSelector->getHoverObject();
        
        if( objectIndex != -1 ) {
            mSelectorToolTipTimer += frameSecondsDelta;
            }
        else {
            mSelectorToolTipTimer = 0;

            if( mCurrentTipGUIComponent == mSelector ) {
                // mouse moved out of selector
                
                // remove old
                mMainPanel->remove( mToolTipLabel );
                delete mToolTipLabel;
                mToolTipLabel = NULL;
                mCurrentTipGUIComponent = NULL;
                mCurrentTipStoredItem = -1;
                }
            }

        
        if( ! mVideoCaptureMode &&
            mSelectorToolTipTimer >= mToolTipDisplayTime ) {

            if( mCurrentTipGUIComponent != mSelector ||
                objectIndex != mCurrentTipStoredItem ) {
                // switch tip labels to the selector's lable

                if( mToolTipLabel != NULL ) {
                    // remove old
                    mMainPanel->remove( mToolTipLabel );
                    delete mToolTipLabel;
                    mToolTipLabel = NULL;
                    mCurrentTipGUIComponent = NULL;
                    mCurrentTipStoredItem = -1;
                    }
                
                // show a tool tip for the selector

                Storable *storedObject =
                    mUserControlledGardener->getStorable( objectIndex );
                
                
                const char *tipStringPrefix = "";
                const char *tipStringSuffix = "";
                
                if( storedObject->getType() == fruitType ) {

                    tipStringPrefix =
                        TranslationManager::translate(
                            "tip_fruit" );

                    Fruit *fruit = (Fruit *)storedObject;

                    if( fruit->isPoisoned() ) {
                        tipStringSuffix =
                            TranslationManager::translate(
                                "tip_poisoned" );
                        }
                    }
                else {
                    tipStringPrefix =
                        (char *)TranslationManager::translate(
                            "tip_seed" );

                    Seeds *seed = (Seeds *)storedObject;

                    if( seed->mIdealSoilType == 0 ) {
                        tipStringSuffix =
                            (char *)TranslationManager::translate(
                                "tip_darkSoil" );
                        }
                    else {
                        tipStringSuffix =
                            (char *)TranslationManager::translate(
                                "tip_lightSoil" );
                        }
                    }

                char *totalTipString = autoSprintf( "%s%s", tipStringPrefix,
                                                    tipStringSuffix );
                
                // center tooltip label at top of screen

                // 1:1 aspect ratio
                double height = 0.04;
                double width = height * strlen( totalTipString );
                                
                height *= toolTipShrinkFactor;
                width *= toolTipShrinkFactor;

                double actualDrawWidth =
                    height * mTextGL->measureTextWidth( totalTipString );
                
                // because of side panel, center of main panel looks like
                // it is at 0.55 instead of 0.5
                mToolTipLabel = new LabelGL( 0.55 - 0.5 * actualDrawWidth,
                                             0.11,
                                             width,
                                             height,
                                             totalTipString,
                                             mTextGL );

                delete [] totalTipString;
                
                mMainPanel->add( mToolTipLabel );

                mCurrentTipGUIComponent = mSelector;
                mCurrentTipStoredItem = objectIndex;
                }
            
            }

        }

    
    mRestartButton->setEnabled( false );    
    
    // check if user dead
    if( mUserControlledGardener->isDead() ) {
        showTutorial( "tutorial_death1" );
        
        if( !mUserControlledGardener->isGhost() ) {

            Vector3D *gardenerPosition =
                mWorld.getGardenerPosition( mUserControlledGardener );
            
            mWorld.augmentPortal( gardenerPosition, mUserControlledGardener );

            delete gardenerPosition;
            }
        
        Gardener *nextOffspring = mWorld.getNextUserControllableGardener();

        if( nextOffspring != NULL ) {
            // remove and destroy
            mWorld.removeGardener( mUserControlledGardener );
            
            mUserControlledGardener = nextOffspring;
            mUserControlledGardener->mUserControlling = true;
            
            // turn off AI for this gardener
            int index = mOtherGardeners.getElementIndex( nextOffspring );


            GardenerAI *ai = *( mOtherGardenerAI.getElement( index ) );
                
            mOtherGardeners.deleteElementEqualTo( nextOffspring );
            mOtherGardenerAI.deleteElementEqualTo( ai );
                
            delete ai;

            // switch storage that is displayed
            mSelector->setStorage( mUserControlledGardener );

            // smooth switch to new
            smoothPositionTransition = true;
            }
        else {
            // no offspring left
            
            // become ghost
            mUserControlledGardener->setGhostMode( true );
            
            // tell other gardeners to ignore
            mWorld.ignoreGardener( mUserControlledGardener );
            
            // show restart
            mRestartButton->setEnabled( true );

            
            if( mMainPanel->contains( mPauseButton ) ) {
                // replace pause with restart

                mMainPanel->remove( mPauseButton );
                mMainPanel->add( mRestartButton );

                // if this is not our first death, the death1 tutorial
                // will have already been shown

                // however, we should still let the player know that the
                // game is over by showing death1a

                // check if death1 tutorial already spent
                char death1Spent = false;
                
                int numSpentKeys = mSpentTutorialKeys.size();
                for( int k=0; k<numSpentKeys && !death1Spent; k++ ) {
                    char *key = *( mSpentTutorialKeys.getElement( k ) );

                    if( strcmp( key, "tutorial_death1" ) == 0 ) {
                        death1Spent = true;
                        }
                    }

                if( death1Spent ) {
                    // try showing death6a instead
                    showTutorial( "tutorial_death1a" );
                    }
                }

            
            // disable others
            mPlantButton->setEnabled( false );
            mPlotButton->setEnabled( false );
            mWaterButton->setEnabled( false );
            mHarvestButton->setEnabled( false );
            mEatButton->setEnabled( false );
            mGiftButton->setEnabled( false );
            mMateButton->setEnabled( false );
            mFollowButton->setEnabled( false );
            mPoisonButton->setEnabled( false );
            mDiscardButton->setEnabled( false );

            mSelector->setEnabled( false );
            
            // remove selected objects
            mPlantButton->setSelectedObject( NULL );
            mHarvestButton->setSelectedObject( NULL );
            mEatButton->setSelectedObject( NULL );
            mGiftButton->setSelectedObject( NULL );
            mDiscardButton->setSelectedObject( NULL );

            mMateButton->setSelectedObject( NULL );
            mMateButton->setSecondSelectedObject( NULL );
            mFollowButton->setSelectedObject( NULL );
            mFollowButton->setSecondSelectedObject( NULL );
            }
        }
    

    
    // set the screen position based on our new gardener position
    // do this even if dead (ghost mode)
    
    Vector3D *position =
        mWorld.getGardenerPosition( mUserControlledGardener );

    char canPlant = mWorld.canPlant( position );

    
    position->mZ = baseViewZ;

    if( smoothPositionTransition ) {
        Vector3D *currentScreenPosition = screen->getViewPosition();


        Vector3D motionDirection( position );
        motionDirection.subtract( currentScreenPosition );
        double neededMoveLength = motionDirection.getLength();
        motionDirection.normalize();

        double ourMoveLength =
            frameSecondsDelta * smoothTransitionVelocity;
        if( ourMoveLength > neededMoveLength ) {
            ourMoveLength = neededMoveLength;

            // surpassed full move

            // no longer move smoothly
            smoothPositionTransition = false;
            }
        
        motionDirection.scale( ourMoveLength );

        currentScreenPosition->add( &motionDirection );

        screen->setViewPosition( currentScreenPosition );

        globalPlayerCurrentPosition.setCoordinates( currentScreenPosition );
        
        
        delete currentScreenPosition;        
        }
    else {
        // instant jump
        screen->setViewPosition( position );

        globalPlayerCurrentPosition.setCoordinates( position );
        }

    delete position;
    
    // set z back to zero, because we got position from view position, which
    // is pulled back in z space
    globalPlayerCurrentPosition.mZ = 0;


    // rest of stuff below is for UI component enabling/disabling
    // state of world accessed by sound thread doesn't change
    // safe to unlock
    globalLock.unlock();

    
    if( mUserControlledGardener->isDead() ) {
        // still dead
        // don't do anything but update position (ghost mode)

        // check for win condition, though
        // ( portal closed )


        if( mWorld.isPortalClosed() ) {
            showTutorial( "tutorial_immortal1" );
            }
        
        return;
        }
    

    // only allow planting if in plot
    char inPlot = mWorld.isInPlot( mUserControlledGardener );
    char inWater = mWorld.isInWater( mUserControlledGardener );
    
    Seeds *selectedSeeds = mUserControlledGardener->getSelectedSeeds();
    
    mPlantButton->setEnabled( canPlant &&
                              inPlot &&
                              !inWater &&
                              selectedSeeds != NULL );

    if( selectedSeeds != NULL ) {
        mPlantButton->setSelectedObject( selectedSeeds->getSampleLeaf() );
        }
    else {
        mPlantButton->setSelectedObject( NULL );
        }

    Plant *closePlant = mWorld.getClosestPlotPlant( mUserControlledGardener );


    // default to no highlight
    mWorld.setHighlightPosition( NULL );
    
    // only allow water grabbing if in water
    if( ! mUserControlledGardener->getCarryingWater() ) {

        mWaterButton->setFull( false );
        
        char inWater = mWorld.isInWater( mUserControlledGardener );

        mWaterButton->setEnabled( inWater );
        }
    else {
        // already carrying water
        mWaterButton->setFull( true );
        
        // only allow dumping if close to a plant in plot that is not
        // done growing yet
        
        if( closePlant != NULL &&
            ! closePlant->isFullGrown() ) {

            mWaterButton->setEnabled( true );

            if( mWaterButton->isMouseOver() ) {
                // highlight closest plant
                Vector3D *plantPosition =
                    mWorld.getPlantPosition( closePlant );

                mWorld.setHighlightPosition( plantPosition );

                delete plantPosition;
                }
            }
        else if( mWorld.isInWater( mUserControlledGardener ) ) {
            // can also dump it back in water
            mWaterButton->setEnabled( true );
            }
        else {
            mWaterButton->setEnabled( false );
            }
        }

    // enable poison button near any plant
    
    // consider only "close" plants
    closePlant = mWorld.getClosestPlant( &globalPlayerCurrentPosition,
                                         NULL, false );

    if( closePlant != NULL &&
        // only can poison plants not already poisoned
        closePlant->getPoisonStatus() == 0 ) {
        
        mPoisonButton->setEnabled( true );

        if( mPoisonButton->isMouseOver() ) {
            // highlight closest plant
            Vector3D *plantPosition =
                mWorld.getPlantPosition( closePlant );
            
            mWorld.setHighlightPosition( plantPosition );
            
            delete plantPosition;
            }
        }
    else {
        mPoisonButton->setEnabled( false );
        }

    
    
    Plant *closeRipePlant =
        mWorld.getClosestPlotPlant( mUserControlledGardener, false,
                                    // true to only consider ripe
                                    true );

    
    if( closeRipePlant != NULL ) {

        // enable harvest button
        mHarvestButton->setEnabled( true );

        mHarvestButton->setSelectedObject( closeRipePlant->peekAtRipeFruit() );
        
        if( mHarvestButton->isMouseOver() ) {
            // highlight ripe fruit
            closeRipePlant->highlightFirstRipeFruit( true );
            }
        else {
            closeRipePlant->highlightFirstRipeFruit( false );
            }
        }
    else {
        mHarvestButton->setEnabled( false );
        mHarvestButton->setSelectedObject( NULL );
        }


    updateStorageUI();
    
    

    mMateButton->setEnabled( false );
    mMateButton->setSelectedObject( NULL );
    mMateButton->setSecondSelectedObject( NULL );

    if( !mUserControlledGardener->isPregnant() &&
        ! mWorld.isTargetOfPregnancy( mUserControlledGardener ) ) {
        // not already pregnant
        // and not already target of pregnancy
        
        Gardener *closestGardener =
            mWorld.getClosestGardener( mUserControlledGardener );
        
        if( closestGardener != NULL
            &&
            ! closestGardener->isPregnant()
            &&
            ! mWorld.isTargetOfPregnancy( closestGardener ) ) {

            // a gardener is close and not already pregnant
            // and not already target of a pregnancy
            
            double distance =
                mWorld.getGardenerDistance( mUserControlledGardener,
                                            closestGardener );
            // is it close enough
            if( distance < getMaxDistanceForTransactions() ) {

                // does it like us enough?
                if( closestGardener->likeEnoughToMate(
                    mUserControlledGardener ) ) {

                    mMateButton->setEnabled( true );

                    mMateButton->setSelectedObject( mUserControlledGardener );
                    mMateButton->setSecondSelectedObject( closestGardener );

                    if( mMateButton->isMouseOver() ) {
                        Vector3D *matePosition =
                            mWorld.getGardenerPosition( closestGardener );
                    
                        mWorld.setHighlightPosition( matePosition );
                    
                        delete matePosition;
                        }
                    }
                }
            }
        }




    mFollowButton->setEnabled( false );
    mFollowButton->setStopFollowingMode( false );
    mFollowButton->setSelectedObject( NULL );
    mFollowButton->setSecondSelectedObject( NULL );    
        
    Gardener *closestGardener =
        mWorld.getClosestGardener( mUserControlledGardener );
        
    if( closestGardener != NULL
        &&
        closestGardener->getLeader() == NULL ) {

        // a gardener is close and not already following
            
        double distance =
            mWorld.getGardenerDistance( mUserControlledGardener,
                                        closestGardener );
        // is it close enough
        if( distance < getMaxDistanceForTransactions() ) {

            // does it like us enough?
            if( closestGardener->likeEnoughToFollow(
                mUserControlledGardener ) ) {

                mFollowButton->setEnabled( true );
                
                mFollowButton->setSelectedObject(
                    mUserControlledGardener );
                mFollowButton->setSecondSelectedObject( closestGardener );
                
                if( mFollowButton->isMouseOver() ) {
                    Vector3D *followerPosition =
                        mWorld.getGardenerPosition( closestGardener );
                    
                    mWorld.setHighlightPosition( followerPosition );
                    
                    delete followerPosition;
                    }
                }
            }
        }
    else if( closestGardener != NULL
        &&
        closestGardener->getLeader() == mUserControlledGardener ) {

        // already following us

        // enable the "stop following" button

        mFollowButton->setEnabled( true );
        mFollowButton->setStopFollowingMode( true );
        mFollowButton->setSelectedObject( mUserControlledGardener );
        mFollowButton->setSecondSelectedObject( closestGardener );

        if( mFollowButton->isMouseOver() ) {
            Vector3D *followerPosition =
                mWorld.getGardenerPosition( closestGardener );
            
            mWorld.setHighlightPosition( followerPosition );
            
            delete followerPosition;
            }
        }

    
    // handle special tutorial states

    if( mTutorialEnabled ) {

        if( mUserControlledGardener->getNutrientLevel( 0 ) < 0.85
            ||
            mUserControlledGardener->getNutrientLevel( 1 ) < 0.85
            ||
            mUserControlledGardener->getNutrientLevel( 2 ) < 0.85 ) {

            showTutorial( "tutorial_survival1" );
            }
            

        SimpleVector<Plant*> *plotPlants =
            mWorld.getPlotPlants( mUserControlledGardener );
        char plantRipe = false;
        
        int numPlotPlants = plotPlants->size();
        for( int p=0; p<numPlotPlants && !plantRipe; p++ ) {
            Plant *plant = *( plotPlants->getElement( p ) );
            
            if( plant->isRipe() ) {
                plantRipe = true;
                }
            }
    
        delete plotPlants;
    
        if( plantRipe ) {
            showTutorial( "tutorial_harvest1" );
            }

        if( mUserControlledGardener->getLife() < 0.5 ) {
            showTutorial( "tutorial_lifespan1" );
            }

        Gardener *nextOffspring =
            mWorld.getNextUserControllableGardener( mUserControlledGardener );

        if( nextOffspring != NULL ) {
            showTutorial( "tutorial_birth1" );
            }

        if( mUserControlledGardener->isStoredFruitPoisoned() ) {
            showTutorial( "tutorial_poisoned1" );
            }

        if( ! mUserControlledGardener->isDead() &&
            mWorld.isPortalOpen() ) {

            showTutorial( "tutorial_gate1" );
            }
        

        int storedFruitCount = mUserControlledGardener->getStoredFruitCount(); 
        if( storedFruitCount > 0 ) {
            showTutorial( "tutorial_eat1" );
            }
        if( storedFruitCount > 1 ) {
            showTutorial( "tutorial_gift1" );
            }
        

        if( mUserControlledGardener->isPregnant() ) {
            showTutorial( "tutorial_pregnant1" );
            }
        }

    // handle case where user dropped second plot corner on one of the GUI
    // panels but NOT on a button (so no false button click was generated)
    if( mActionState == doneDrawingPlot ) {
        mActionState = none;
        }
    }


void GameSceneHandler::updateStorageUI() {
    
    Storable *item = mUserControlledGardener->getSelectedStorable();
    

    // can only eat fruit
    // for now, can only give fruit
    if( item != NULL && item->getType() == fruitType ) {
        // fruit selected
        mEatButton->setEnabled( true );

        mEatButton->setSelectedObject( item );
        
        
        mGiftButton->setEnabled( false );
        mGiftButton->setSelectedObject( NULL );
        
        
        Gardener *closestGardener =
            mWorld.getClosestGardener( mUserControlledGardener );

        if( closestGardener != NULL ) {

            double distance =
                mWorld.getGardenerDistance( mUserControlledGardener,
                                            closestGardener );
            if( distance < getMaxDistanceForTransactions() ) {
                Fruit *fruit = (Fruit*)item;

                // can't give a gift of poisoned fruit
                if( !fruit->isPoisoned() ) {
                    mGiftButton->setEnabled( true );

                    mGiftButton->setSelectedObject( item );

                    if( mGiftButton->isMouseOver() ) {
                        Vector3D *recipientPosition =
                            mWorld.getGardenerPosition( closestGardener );
                    
                        mWorld.setHighlightPosition( recipientPosition );
                    
                        delete recipientPosition;
                        }
				    }
                }

            }        
        }
    else {
        mEatButton->setEnabled( false );
        mEatButton->setSelectedObject( NULL );
        
        mGiftButton->setEnabled( false );
        mGiftButton->setSelectedObject( NULL );
        }


    if( item != NULL ) {
        mDiscardButton->setSelectedObject( item );
        mDiscardButton->setEnabled( true );
        }
    else {
        mDiscardButton->setSelectedObject( NULL );
        mDiscardButton->setEnabled( false );
        }


    // update plant button also

    Seeds *selectedSeeds = mUserControlledGardener->getSelectedSeeds();
        
    
    if( selectedSeeds != NULL ) {
        mPlantButton->setSelectedObject( selectedSeeds->getSampleLeaf() );
        }
    else {
        mPlantButton->setSelectedObject( NULL );
        }
    }


 
void GameSceneHandler::actionPerformed( GUIComponent *inTarget ) {
    
    if( mActionState == doneDrawingPlot ) {
        // just finished drawing plot, and user dropped second corner
        // on a button
        mActionState = none;
        
        // ignore button click
        return;
        }
    else if( mActionState != none ) {
        // in the middle of draggin plot
        // ignore button presses
        return;
        }

    if( mPaused && 
        !( inTarget == mPauseButton ||
           inTarget == mQuitButton ) ) {
        // while paused, ignore all clicks except pause and quit
        return;
        }
    
    

    if( inTarget == mRestartButton ) {
        restartFlag = true;
        }
    else if( inTarget == mPauseButton ) {
        mPaused = !mPaused;
        }
    else if( inTarget == mQuitButton ) {
        exit( 0 );
        }    
    else if( inTarget == mNextTutorialButton ) {
        tutorialDone( mCurrentTutorialKey );
        }
    else if( inTarget == mStopTutorialButton ) {
        // FIXME?  Crashes
        mTutorialEnabled = false;

        tutorialDone( mCurrentTutorialKey );
        }

    if( mMainPanel->contains( mTutorialPanel ) ) {
        // ignore other actions, since tutorial is up
        return;
        }
    
    
    if( inTarget == mPlantButton ) {
        
        Vector3D *position =
            mWorld.getGardenerPosition( mUserControlledGardener );

        double soilCondition = mWorld.getSoilCondition( position );

        Seeds *newPlantSeeds = mUserControlledGardener->getSelectedSeeds();
        
        globalSoundEffectsBank->playEffect( effect_plantSeed );

        mWorld.addPlant( mUserControlledGardener,
                         new Plant( soilCondition, newPlantSeeds ),
                         position );

        // delete seeds after planting
        mUserControlledGardener->removeSeeds( newPlantSeeds );
        
        delete position;

        showTutorial( "tutorial_water1" );
        }
    else if( inTarget == mPlotButton ) {
        mPlotButton->setEnabled( false );
        mActionState = plotFirstCorner;
        }

    else if( inTarget == mWaterButton ) {
        // don't worry about gardener position here, since button
        // enabling/disabling code handles it
        if( mUserControlledGardener->getCarryingWater() ) {
            // dump

            mUserControlledGardener->setCarryingWater( false );

            globalSoundEffectsBank->playEffect( effect_dumpWater );

            // add water to closest plant in plot
            mWorld.dumpWater( mUserControlledGardener );

            if( mTutorialEnabled ) {
                SimpleVector<Plant*> *plotPlants =
                    mWorld.getPlotPlants( mUserControlledGardener );
                
                int numPlotPlants = plotPlants->size();

                if( numPlotPlants > 0 ) {
                    showTutorial( "tutorial_water3" );
                    }
                delete plotPlants;
                }
            }
        else {
            // fetch
            
            globalSoundEffectsBank->playEffect( effect_pickupWater );

            mUserControlledGardener->setCarryingWater( true );

            if( mTutorialEnabled ) {
                SimpleVector<Plant*> *plotPlants =
                    mWorld.getPlotPlants( mUserControlledGardener );

                int numPlotPlants = plotPlants->size();

                if( numPlotPlants > 0 ) {
                    showTutorial( "tutorial_water2" );
                    }
                delete plotPlants;
                }
            }
        }
    else if( inTarget == mPoisonButton ) {
        // don't worry about gardener position here, since button
        // enabling/disabling code handles it

        globalSoundEffectsBank->playEffect( effect_poison );

        mWorld.dumpPoison( mUserControlledGardener );

        }
    else if( inTarget == mHarvestButton ) {
        Plant *closePlant =
            mWorld.getClosestPlotPlant( mUserControlledGardener, false,
                                        // only consider ripe
                                        true );
        
        globalSoundEffectsBank->playEffect( effect_pickFruit );

        mWorld.harvestPlant( mUserControlledGardener, closePlant );
        }
    else if( inTarget == mMateButton ) {
        Gardener *closestGardener =
            mWorld.getClosestGardener( mUserControlledGardener );

        if( closestGardener != NULL ) {
            globalSoundEffectsBank->playEffect( effect_mate );

            mWorld.mateGardeners( mUserControlledGardener,
                                  closestGardener );
            }
        }
    else if( inTarget == mFollowButton ) {
        Gardener *closestGardener =
            mWorld.getClosestGardener( mUserControlledGardener );

        if( closestGardener != NULL ) {

            if( closestGardener->getLeader() == NULL ) {
                closestGardener->setLeader( mUserControlledGardener );

                mUserControlledGardener->addFollower( closestGardener );
                }
            else {
                // tell them to stop following
                closestGardener->setLeader( NULL );

                mUserControlledGardener->dropFollower( closestGardener );
                }
            }
        }
    else if( false /*inTarget == mEmotionButton*/ ) {

        // print report
        printf( "Emotion Report:\n" );
        int numOthers = mOtherGardeners.size();

        for( int i=0; i<numOthers; i++ ) {
            Gardener *gardenerA = *( mOtherGardeners.getElement( i ) );

            printf( "%d:::  ", i );

            for( int j=0; j<numOthers; j++ ) {
                Gardener *gardenerB = *( mOtherGardeners.getElement( j ) );

                if( gardenerA != gardenerB ) {
                    printf( "[%d: %.1f] ",
                            j, gardenerA->getLikeMetric( gardenerB ) );
                    }
                else {
                    printf( "[ ---- ] ");
                    }
                }

            Gardener *gardenerB = mUserControlledGardener;

            if( gardenerA != gardenerB ) {
                printf( "[u: %.1f] ",
                        gardenerA->getLikeMetric( gardenerB ) );
                }
            else {
                printf( "[ ---- ] ");
                }

            printf( "\n" );
            }
        
        }
    
    else if( inTarget == mEatButton ) {
        globalSoundEffectsBank->playEffect( effect_eatFruit );

        mUserControlledGardener->eat();

        showTutorial( "tutorial_eat2" );
        }
    else if( inTarget == mGiftButton ) {

        Fruit *selectedFruit = mUserControlledGardener->getSelectedFruit();

        if( selectedFruit != NULL ) {
            Gardener *closestGardener =
                mWorld.getClosestGardener( mUserControlledGardener );

            if( closestGardener != NULL ) {
                globalSoundEffectsBank->playEffect( effect_giveFruit );

                mWorld.giveFruit( mUserControlledGardener,
                                  closestGardener,
                                  selectedFruit );

                showTutorial( "tutorial_emotion1" );
                }
            else {
                delete selectedFruit;
                }
            }
        }
    else if( inTarget == mDiscardButton ) {
        mUserControlledGardener->deleteSelectedStorable();
        }
    


    // some actions remove items from storage and destroy them

    updateStorageUI();    
    }



void GameSceneHandler::setSecondPlotCorner( Vector3D *inPosition ) {

    // get the existing first corner
    Vector3D *cornerA, *cornerB;
    mWorld.getGardenerPlot( mUserControlledGardener,
                            &cornerA, &cornerB );

    if( cornerA != NULL ) {
        // existing first corner, new second corner
        mWorld.setGardenerPlot( mUserControlledGardener,
                                cornerA, inPosition );
        delete cornerA;
        }
    
    if( cornerB != NULL ) {
        delete cornerB;
        }      
    }



void GameSceneHandler::projectScreenPointIntoScene( int inX, int inY,
                                                    double *outSceneX,
                                                    double *outSceneY ) {
    // first, get our current matrices
    GLdouble modelMatrix[16];
    GLdouble projMatrix[16];
    GLint viewport[4];

    glGetDoublev( GL_MODELVIEW_MATRIX, modelMatrix );
    glGetDoublev( GL_PROJECTION_MATRIX, projMatrix );
    glGetIntegerv( GL_VIEWPORT, viewport );



    /*
     From the OpenGL FAQ

     The concept of window space Z is often confusing. It's the depth buffer
     value expressed as a GLdouble in the range 0.0 to 1.0. Assuming a
     default glDepthRange(), a window coordinate with a Z value of 0.0
     corresponds to an eye coordinate located on the zNear clipping plane.
     Similarly, a window space Z value of 1.0 corresponds to an eye space
     coordinate located on the zFar plane. You can obtain any window space Z
     value by reading the depth buffer with glReadPixels().
    */

    // so, use screen z of 0 for near clipping plane, which ScreenGL has
    // set to 1 unit away from screen.

    // Thus, we need to find the z buffer value of our world plane (a z=0)
    // we can call this our "windowZ", since it is how z space is represented
    // in the window

    // if we project world (0,0,0) onto the screen, we will get the windowZ
    // of our world zero
    
    double windowXForWorldZero, windowYForWorldZero, windowZForWorldZero;
    gluProject( 0, 0, 0,
                modelMatrix, projMatrix, viewport,
                &windowXForWorldZero,
                &windowYForWorldZero,
                &windowZForWorldZero );


    // use unproject to map our window coordinates back into the world
    double xWorld, yWorld, zWorld;    
    gluUnProject( inX,
                  inY,
                  windowZForWorldZero,
                  modelMatrix, projMatrix, viewport,
                  &xWorld, &yWorld, &zWorld );


    
    // need to invert the resulting y and translate it for some reson

    Vector3D *viewPosition = screen->getViewPosition();

    yWorld = -yWorld + 2 * viewPosition->mY;

    delete viewPosition;


    
    *outSceneX = xWorld;
    *outSceneY = yWorld;
    }

void GameSceneHandler::keyPressed(
	unsigned char inKey, int inX, int inY ) {

    }



void GameSceneHandler::keyReleased(
	unsigned char inKey, int inX, int inY ) {
    
    // ignore key releases

    if( true ) {
        return;
        }

    // these keyboard options are useful for testing:
    
    if( inKey == 'f' || inKey == 'F' ) {
		mScreen->changeWindowSize( 640, 480 );
		}
    if( inKey == 's' || inKey == 'S' ) {
		mScreen->changeWindowSize( 100, 100 );
		}
    if( inKey == 'd' || inKey == 'D' ) {
		mScreen->changeWindowSize( 300, 300 );
		}
    // hack to force portal to open for quick testing
    if( inKey == 'p' || inKey == 'P' ) {

        Vector3D *gardenerPosition =
            mWorld.getGardenerPosition( mUserControlledGardener );

        // pass fresh gardener each time so each ring looks different
        Gardener temp( gardenerPosition );
        mWorld.augmentPortal( gardenerPosition, &temp );
        
        delete gardenerPosition;
        }
    // video capture
    if( false && inKey == 'v' || inKey == 'V' ) {
        if( !mVideoCaptureMode ) {
            mVideoCaptureMode = true;

            // hide UI
            //mScreen->removeSceneHandler( mMainPanelGuiTranslator );

            // video size 
            mScreen->changeWindowSize( mVideoFrameWidth, mVideoFrameHeight );

            mFramesCaptured = 0;
            }
        }
    
    }



void GameSceneHandler::specialKeyPressed(
	int inKey, int inX, int inY ) {
    
	}



void GameSceneHandler::specialKeyReleased(
	int inKey, int inX, int inY ) {

	} 



void GameSceneHandler::showTutorial( const char *inTutorialKey ) {
    if( ! mTutorialEnabled ) {
        // do nothing
        return;
        }

    if( mCurrentTutorialKey == inTutorialKey ) {
        // already showing this tutorial right now
        return;
        }

    int numSpent = mSpentTutorialKeys.size();

    for( int i=0; i<numSpent; i++ ) {
        char *spentKey = *( mSpentTutorialKeys.getElement( i ) );

        if( strcmp( spentKey, inTutorialKey ) == 0 ) {
            // do nothing,
            // tutorial already shown
            return;
            }
        }
    

    if( mCurrentTutorialKey != NULL ) {
        // an older tutorial message is still up

        // end it
        mSpentTutorialKeys.push_back( (char *)mCurrentTutorialKey );

        mCurrentTutorialKey = NULL;        
        }

    
    // else show it
    mCurrentTutorialKey = (char *)inTutorialKey;

    char *tutorialString =
        (char *)TranslationManager::translate( (char*)inTutorialKey );

    mTutorialTextBlock->setText( tutorialString );

    // we may be displaying one tutorial right after another
    // and leaving the panel up
    if( ! mMainPanel->contains( mTutorialPanel ) ) {

        mMainPanel->add( mTutorialPanel );        
        }
    }



void GameSceneHandler::tutorialDone( const char *inTutorialKey ) {
    mSpentTutorialKeys.push_back( (char *)inTutorialKey );
    
    mCurrentTutorialKey = NULL;

    

    
    // some tutorials trigger the following tutorial
    const char *nextTutorial = NULL;

    // only consider this case if tutorial is still enabled
    if( mTutorialEnabled ) {
    
        if( strcmp( inTutorialKey, "tutorial_question" ) == 0 ) {
            nextTutorial = "tutorial_welcome1";
            }
        else if( strcmp( inTutorialKey, "tutorial_welcome1" ) == 0 ) {
            nextTutorial = "tutorial_welcome2";
            }
        else if( strcmp( inTutorialKey, "tutorial_welcome2" ) == 0 ) {
            nextTutorial = "tutorial_moving";
            }
        else if( strcmp( inTutorialKey, "tutorial_survival1" ) == 0 ) {
            nextTutorial = "tutorial_survival2";
            }
        else if( strcmp( inTutorialKey, "tutorial_survival2" ) == 0 ) {
            nextTutorial = "tutorial_survival3";
            }
        else if( strcmp( inTutorialKey, "tutorial_survival3" ) == 0 ) {
            nextTutorial = "tutorial_survival4";
            }
        else if( strcmp( inTutorialKey, "tutorial_survival4" ) == 0 ) {
            nextTutorial = "tutorial_seeds1";
            }
        else if( strcmp( inTutorialKey, "tutorial_seeds1" ) == 0 ) {
            nextTutorial = "tutorial_plot";
            }
        else if( strcmp( inTutorialKey, "tutorial_water3" ) == 0 ) {
            nextTutorial = "tutorial_plot2";
            }
        else if( strcmp( inTutorialKey, "tutorial_plot2" ) == 0 ) {
            nextTutorial = "tutorial_plot3";
            }
        else if( strcmp( inTutorialKey, "tutorial_eat2" ) == 0 ) {
            nextTutorial = "tutorial_seeds2";
            }
        else if( strcmp( inTutorialKey, "tutorial_seeds2" ) == 0 ) {
            nextTutorial = "tutorial_seeds3";
            }
        else if( strcmp( inTutorialKey, "tutorial_seeds3" ) == 0 ) {
            nextTutorial = "tutorial_seeds4";
            }
        else if( strcmp( inTutorialKey, "tutorial_seeds4" ) == 0 ) {
            nextTutorial = "tutorial_seeds5";
            }
        else if( strcmp( inTutorialKey, "tutorial_seeds5" ) == 0 ) {
            nextTutorial = "tutorial_seeds6";
            }
        else if( strcmp( inTutorialKey, "tutorial_emotion1" ) == 0 ) {
            nextTutorial = "tutorial_emotion2";
            }
        else if( strcmp( inTutorialKey, "tutorial_emotion2" ) == 0 ) {
            nextTutorial = "tutorial_emotion3";
            }
        else if( strcmp( inTutorialKey, "tutorial_lifespan1" ) == 0 ) {
            Gardener *nextOffspring =
                mWorld.getNextUserControllableGardener(
                    mUserControlledGardener );

            if( nextOffspring != NULL ) {
                
                nextTutorial = "tutorial_lifespan2a";
                }
            else if( mUserControlledGardener->isPregnant() ) {
                nextTutorial = "tutorial_lifespan2b";
                }
            else {
                nextTutorial = "tutorial_lifespan2c";
                }            
            }
        else if( strcmp( inTutorialKey, "tutorial_lifespan2c" ) == 0 ) {
            nextTutorial = "tutorial_mating1";
            }
        else if( strcmp( inTutorialKey, "tutorial_mating1" ) == 0 ) {
            nextTutorial = "tutorial_mating2";
            }
        else if( strcmp( inTutorialKey, "tutorial_mating2" ) == 0 ) {
            nextTutorial = "tutorial_mating3";
            }
        else if( strcmp( inTutorialKey, "tutorial_birth1" ) == 0 ) {
            nextTutorial = "tutorial_birth2";
            }
        else if( strcmp( inTutorialKey, "tutorial_poisoned1" ) == 0 ) {
            nextTutorial = "tutorial_poisoned2";
            }
        else if( strcmp( inTutorialKey, "tutorial_death1" ) == 0 ) {
            nextTutorial = "tutorial_death2";
            }
        else if( strcmp( inTutorialKey, "tutorial_death2" ) == 0 ) {
            nextTutorial = "tutorial_death3";
            }
        else if( strcmp( inTutorialKey, "tutorial_death3" ) == 0 ) {
            nextTutorial = "tutorial_death4";
            }
        else if( strcmp( inTutorialKey, "tutorial_death4" ) == 0 ) {
            nextTutorial = "tutorial_death5";
            }
        else if( strcmp( inTutorialKey, "tutorial_death5" ) == 0 ) {

            if( mUserControlledGardener->isDead() ) {
                nextTutorial = "tutorial_death6a";
                }
            else {
                nextTutorial = "tutorial_death6b";
                }
            }
        else if( strcmp( inTutorialKey, "tutorial_gate1" ) == 0 ) {
            nextTutorial = "tutorial_gate2";
            }
        else if( strcmp( inTutorialKey, "tutorial_gate2" ) == 0 ) {
            nextTutorial = "tutorial_gate3";
            }
        else if( strcmp( inTutorialKey, "tutorial_gate3" ) == 0 ) {
            nextTutorial = "tutorial_gate4";
            }
        else if( strcmp( inTutorialKey, "tutorial_gate4" ) == 0 ) {
            nextTutorial = "tutorial_gate5";
            }
        else if( strcmp( inTutorialKey, "tutorial_immortal1" ) == 0 ) {
            nextTutorial = "tutorial_immortal2";
            }
        else if( strcmp( inTutorialKey, "tutorial_immortal2" ) == 0 ) {
            nextTutorial = "tutorial_immortal3";
            }
        else if( strcmp( inTutorialKey, "tutorial_immortal3" ) == 0 ) {
            nextTutorial = "tutorial_immortal4";
            }
        else if( strcmp( inTutorialKey, "tutorial_immortal4" ) == 0 ) {
            nextTutorial = "tutorial_immortal5";
            }
        }
    
    
    if( nextTutorial != NULL ) {
        showTutorial( nextTutorial );
        }
    else {
        mMainPanel->remove( mTutorialPanel );
        }
    }
