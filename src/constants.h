
#define DEV

//#define DEMO

//#define TITLE_LOGO_ONLY

#define ALWAYS_FPS true

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

/// ///

#define TICK_FREQUENCY 50

#define TILE_PIX 16

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


#define TRAUMA_DECAY 0.05f;
#define TRAUMA_AMPLIFICATION 4

#define SQRT_HALF 0.70710678118f

/////////////

#define N_OBST 64

#define MAX_PEGS 256

#define MAX_PEG_PATHS 32

#define MAX_PEG_SIZE 5

/////////////

#define N_WATERFALLS 8

// Trying without this
#define UI_OFFSET_TOP 0

#define SCROLL_OFFSET_MAX (WFALL_PIX_Y + UI_OFFSET_TOP - DEVICE_PIX_Y)

// How many pixels down the side do we have to prevent the ball from bouncing out off
#define PHYSWALL_PIX_Y (WFALL_PIX_Y + UI_OFFSET_TOP)

#define FLAG_BALL 1
#define FLAG_PEG 2
#define FLAG_WALL 4

#define BUTTON_PRESSED_FRAMES 8

#define VERSION "v0.1"

#define N_MUSIC_TRACKS 5

// Physics 

#define BALL_RADIUS 8.0f
#define BALL_MASS 1.0f
#define MOTION_TRAIL_LEN 4

#define BOX_WIDTH 22.5f
#define BOX_HEIGHT 12.5f
#define BOX_MAX (MAX(BOX_WIDTH, BOX_HEIGHT) * SQRT_HALF)

#define M_PIf  3.14159265358f
#define M_2PIf 6.28318530718f

#define TIMESTEP (1.0f / TICK_FREQUENCY)
#define ELASTICITY 0.8f 
#define FRICTION 0.0f 
#define G cpv(0.0f, 256.0f)

#define POOT_STRENGTH 300.0f

// SCREEN EASING
// Larger value to make moving quicker
#define SCREEN_ACC 0.9f
// Smaller value to limit max velocity
#define SCREEN_FRIC 0.9f
// Larger number for stronger bounce-back
#define SCREEN_BBACK 0.2f
// Reduction factor to make the crank scrolling not be too fast
#define CRANK_SCROLL_MODIFIER 0.1f
// Per frame updates towards set-position
#define SCREEN_EASING 0.25f
// Time multiplier for a slow, full all-screen wipe 
#define END_SWEEP_SCALE 0.25f

// PARALAX
#define PARALAX_FAR 0.1f;
#define PARALAX_NEAR -0.1f