
#define DEV 1

//#define DEMO

//#define TITLE_LOGO_ONLY

#define ALWAYS_FPS true

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

// DIMENSIONS

#define TICK_FREQUENCY 50

#define TURRET_RADIUS 32
#define TURRET_ANGLE_MIN 15.0f
#define TURRET_ANGLE_MAX (360.0f - TURRET_ANGLE_MIN)


#define DEVICE_PIX_X 400
#define DEVICE_PIX_Y 240

#define HALF_DEVICE_PIX_X (DEVICE_PIX_X/2)
#define HALF_DEVICE_PIX_Y (DEVICE_PIX_Y/2)

#define WFALL_PIX_X DEVICE_PIX_X
#define WFALL_PIX_Y (DEVICE_PIX_Y*4)

#define WF_DIVISION_PIX_Y 60

#define WFSHEET_SIZE_X 1
#define WFSHEET_SIZE_Y (WFALL_PIX_Y/WF_DIVISION_PIX_Y)

#define NUMERAL_PIX_X 128
#define NUMERAL_PIX_Y 160
#define NUMERAL_BUF 40

#define STENCIL_WIPE_N 30

// TIMINGS

#define TIME_TITLE_TRANSITION (TICK_FREQUENCY/2)
#define TIME_DISPLAY_SPLASH (TICK_FREQUENCY/2)
#define TIME_SPLASH_TO_GAME (TICK_FREQUENCY/2)
#define TIME_FIRE_BALL ((9*TICK_FREQUENCY)/10)
#define TIME_STUCK_POP (TICK_FREQUENCY/5)
#define TIME_GUTTER_TO_SCORE (TICK_FREQUENCY/2)
#define TIME_BALL_DROP_DELAY 8
#define TIME_SCORE_TO_TITLE (TICK_FREQUENCY/2)
#define TIME_SCORE_TO_SPLASH (TICK_FREQUENCY/2)

// JUICE

#define TRAUMA_DECAY 0.05f;
#define TRAUMA_AMPLIFICATION 4

#define TRAMA_REQUIRED_HIT 1.0f
#define FREEZE_REQUIRED_HIT (TICK_FREQUENCY / 10)

#define TRAMA_PEG_HIT (TRAMA_REQUIRED_HIT/2.0f)
#define FREEZE_PEG_HIT (FREEZE_REQUIRED_HIT/2)


// PEGS

#define N_OBST 64

#define MAX_PEGS 256

#define MAX_LINEAR_PATH_SEGMENTS 64 

#define MAX_PEG_SIZE 5

/////////////

#define N_WATERFALLS 2

// Trying without this
#define UI_OFFSET_TOP 0

#define SCROLL_OFFSET_MAX (WFALL_PIX_Y - ((3*DEVICE_PIX_Y)/4))



#define N_MUSIC_TRACKS 5

// PHYSICS 

#define SQRT_HALF 0.70710678118f
#define M_PIf 3.14159265358f
#define M_2PIf 6.28318530718f

// How many pixels down the side do we have to prevent the ball from bouncing out off
#define PHYSWALL_PIX_Y (WFALL_PIX_Y + UI_OFFSET_TOP)

#define BALL_RADIUS 8.0f
#define BALL_MASS 1.0f
#define MOTION_TRAIL_LEN 4

#define PREDICTION_TRACE_LEN 32

#define BOX_WIDTH 22.5f
#define BOX_HEIGHT 12.5f
#define BOX_MAX (MAX(BOX_WIDTH, BOX_HEIGHT) * SQRT_HALF)

#define TIMESTEP (1.0f / TICK_FREQUENCY)
#define ELASTICITY 0.8f 
#define FRICTION 0.0f 
#define G cpv(0.0f, 256.0f)

#define POOT_STRENGTH 300.0f

// Velocity squared under which the ball is considered stuck
#define BALL_IS_STUCK 100.0f
#define STUCK_TICKS (TICK_FREQUENCY*3)

#define HISTO_BALL_ACCELERATION 0.05f

#define FLAG_BALL 1
#define FLAG_PEG 2
#define FLAG_WALL 4

// SCREEN EASING
// Larger value to make moving quicker
#define SCREEN_ACC 0.9f
// Smaller value to limit max velocity
#define SCREEN_FRIC 0.9f
// Larger number for stronger bounce-back
#define SCREEN_BBACK 0.2f
// Reduction factor to make the crank scrolling not be too fast
#define CRANK_SCROLL_MODIFIER 0.1f
// Reduction factor to make the crank scrolling of numbers not be too fast
#define CRANK_NUMBERSCROLL_MODIFIER 0.005f
// Per frame updates towards set-position
#define SCREEN_EASING 0.25f
// Time multiplier for a slow, full all-screen wipe 
#define END_SWEEP_SCALE 0.25f

// PARALAX
#define PARALAX_FAR 0.1f;
#define PARALAX_NEAR -0.1f

// MARGIN BUFFERING
#define BUF 16

// SAVE FORMAT
#define SAVE_FORMAT 1
#define SAVE_FORMAT_1_MAX_PLAYERS 4
#define SAVE_FORMAT_1_MAX_LEVELS 128
#define SAVE_FORMAT_1_MAX_HOLES 16

// Must be less than their SAVE_FORMAT counterparts
#define MAX_PLAYERS 3
#define MAX_LEVELS 99
#define MAX_HOLES 9

#define VERSION "v0.1"