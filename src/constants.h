
//#define DEV 1

//#define TAKE_SCREENSHOTS 1

#define ALWAYS_FPS 1

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

// DIMENSIONS

#define TICK_FREQUENCY 50

#define TURRET_RADIUS 32
#define TURRET_ANGLE_MIN 15.0f
#define TURRET_ANGLE_MAX (360.0f - TURRET_ANGLE_MIN)
#define TURRET_LAUNCH_FRAMES 8

#define OVERSCROLL_HEIGHT 16

#define DEVICE_PIX_X 400
#define DEVICE_PIX_Y 240

#define HALF_DEVICE_PIX_X (DEVICE_PIX_X/2)
#define HALF_DEVICE_PIX_Y (DEVICE_PIX_Y/2)

#define WF_MAX_HEIGHT 4

#define WF_DIVISION_PIX_Y 60

#define WFSHEET_SIZE_X 1
#define WFSHEET_SIZE_Y 12

// Comment line to disable
//#define WF_FIXED_BG 1

#define NUMERAL_PIX_X 128
#define NUMERAL_PIX_Y 160
#define NUMERAL_BUF 40

#define TITLETEXT_HEIGHT 32

#define STENCIL_WIPE_N 30

#define SPECIAL_BLAST_RADIUS 128
#define SPECIAL_BLAST_FRAMES 9

#define FINAL_PEG_SLOWMO_RADIUS 64

#define POND_SPLASH_WIDTH 128
#define POND_SPLASH_HEIGHT 82
#define POND_SPLASH_ANIM_FRAMES 16

#define POND_WATER_FRAMES 128
#define POND_WATER_HEIGHT 60
#define POND_WATER_TILES 4

#define SPECIAL_TEXT_WIDTH 256

#define N_FOUNTAINS 3
#define FOUNTAIN_FRAMES 64

#define N_SPLASHES 4

#define N_END_BLASTS 4
#define END_BLAST_FRAMES 32
#define MAX_END_BLASTS 32
#define END_BLAST_HWIDTH (96/2)

#define N_BACKLINES 32
#define BACKLINE_WIDTH 6
#define BACKLINE_HEIGHT 64

// TIMINGS

#define BASE_TIME (TICK_FREQUENCY)

#define TIME_TITLE_TRANSITION (BASE_TIME)
#define TIME_TITLE_HOLE_TO_SPLASH (BASE_TIME*2)
#define TIME_DISPLAY_SPLASH (BASE_TIME/2)
#define TIME_SPLASH_TO_GAME (BASE_TIME)
#define TIME_AIM_SCROLL_TO_TOP (BASE_TIME)
#define TIME_FIRE_MARBLE ((9*TICK_FREQUENCY)/10)
#define TIME_STUCK_POP (BASE_TIME/4)
#define TIME_GUTTER_TO_SCORE (BASE_TIME*2)
#define TIME_BALL_DROP_DELAY (BASE_TIME/3)
#define TIME_SCORE_TO_TRY_AGAIN (BASE_TIME*5)
#define TIME_SCORE_TO_SPLASH (BASE_TIME)

// JUICE

#define TRAUMA_DECAY 0.05f;
#define TRAUMA_AMPLIFICATION 4

#define TRAUMA_SPECIAL_HIT 1.5f
#define FREEZE_SPECIAL_HIT (TICK_FREQUENCY / 10)

#define TRAUMA_REQUIRED_HIT 1.0f
#define FREEZE_REQUIRED_HIT (TICK_FREQUENCY / 10)

#define TRAUMA_PEG_HIT (TRAUMA_REQUIRED_HIT/2.0f)
#define FREEZE_PEG_HIT (FREEZE_REQUIRED_HIT/2)

#define TRAUMA_BLAST_HIT 3.0f

#define EASE_TITLE_DOWNWARDS kEaseInOutSine
#define EASE_TITLE_UPWARDS kEaseInOutSine
#define EASE_TITLE_HOLE_TO_SPLASH kEaseInOutSine
#define EASE_MARBLE_FIRE kEaseOutSine
#define EASE_SPLASH_TO_GAME kEaseInOutSine
// This next one is a B press while scrolled down, it should be snappy
#define EASE_AIM_SCROLL_TO_TOP kEaseInOutSine
#define EASE_GUTTER_TO_TOP kEaseInOutSine
#define EASE_GUTTER_TO_SCORE kEaseOutSine
#define EASE_SCORE_TO_TRY_AGAIN kEaseInOutSine
#define EASE_SCORE_TO_SPLASH kEaseOutSine

// PEGS

#define MAX_PEGS 256

#define MAX_LINEAR_PATH_SEGMENTS 64 

#define MAX_PEG_SIZE 3

#define MAX_PEGS_ON_PATH 32

#define MAX_POPS 4
#define POP_EARLY_Y 32.0f
#define POP_ANIM_FRAMES 16
#define POP_ANIM_HWIDTH 32

/// ///

#define N_WF 4

#define WF_SPECIAL_START 10
#define WF_SPECIAL_END 20

//

#define N_PLINGS 7

#define N_MUSIC_TRACKS 12

#define N_WF_TRACKS 6


// PHYSICS 

#define SQRT_HALF 0.70710678118f
#define M_PIf 3.14159265358f
#define M_2PIf 6.28318530718f

// How many pixels down the side do we have to prevent the ball from bouncing out off
#define PHYSWALL_PIX_Y (DEVICE_PIX_Y * WF_MAX_HEIGHT)

#define BALL_RADIUS 8.0f
#define BALL_MASS 1.0f
#define MOTION_TRAIL_LEN 4
#define MAX_BALLS 2

#define PREDICTION_TRACE_LEN 32

#define BOX_HWIDTH 22.5f
#define BOX_HALF_HEIGHT 12.5f
#define BOX_HALF_MAX (MAX(BOX_HWIDTH, BOX_HALF_HEIGHT) * SQRT_HALF)

#define HEX_WIDTH 22.5f
#define HEX_MAX (HEX_WIDTH * SQRT_HALF)

#define TRI_WIDTH 22.5f
#define TRI_MAX (TRI_WIDTH * SQRT_HALF)

#define TIMESTEP (1.0f / TICK_FREQUENCY)
#define ELASTICITY 0.8f 
#define ULTRA_BOUNCE 1.1f
#define FRICTION 0.0f 
// Was 256
#define G cpv(0.0f, 196.0f)

#define POOT_STRENGTH 300.0f

#define WF_VELOCITY 0.6f

// Velocity squared under which the ball is considered stuck
#define BALL_IS_STUCK 1000.0f
#define STUCK_TICKS (TICK_FREQUENCY*2)

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
#define PARALLAX_GENTLE_FAR 0.333333f
#define PARALLAX_GENTLE_NEAR (0.0f - PARALLAX_GENTLE_FAR)

#define PARALLAX_HARD_FAR 0.5f
#define PARALLAX_HARD_NEAR (0.0f - PARALLAX_HARD_FAR)

// MARGIN BUFFERING
#define BUF 16

// SAVE FORMAT
#define SAVE_FORMAT 1
#define SAVE_FORMAT_1_MAX_PLAYERS 4
#define SAVE_FORMAT_1_MAX_LEVELS 128
#define SAVE_FORMAT_1_MAX_HOLES 16
#define SAVE_FORMAT_1_NAME "cascade_savegame_v1.dat"

// Must be less than their SAVE_FORMAT counterparts
#define MAX_PLAYERS 3
#define MAX_LEVELS 99
#define MAX_HOLES 9

#define VERSION "v0.1"

#define PRELOADING_STEPS 31