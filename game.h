/*
 * Author: Tyler Cavill and Jia Hao Guo
 * File: game.h
 * Purpose: Header file for a space invader alike game programme.
 * 
 */

#define LOOP_RATE 500
#define MESSAGE_RATE 10
#define MAX_ENEMIES 3
#define MAX_BULLETS 5
#define SPAWN_RATE 5
#define MIN_ROW 0
#define MIN_COL 0
#define MAX_ROW 6
#define MAX_COL 4
#define PLAYER_UPDATE_TICKS 7
#define BULLET_UPDATE_TICKS 8
#define ALIEN_UPDATE_TICKS 9
#define MOVE_BULLET_TICKS 500
#define MOVE_ALIEN_TICKS 2000
#define BULLET_UPDATE_WAIT_TICKS 3
#define ALIEN_UPDATE_WAIT_TICKS 3
#define CREATE_ALIEN_TICK 10000

/* Define PIO pins driving LED matrix rows and columns.  */
static pio_t ledmat_rows[] = {
    LEDMAT_ROW1_PIO, LEDMAT_ROW2_PIO, LEDMAT_ROW3_PIO, LEDMAT_ROW4_PIO,
    LEDMAT_ROW5_PIO, LEDMAT_ROW6_PIO, LEDMAT_ROW7_PIO
};

static pio_t ledmat_cols[] = {
    LEDMAT_COL1_PIO, LEDMAT_COL2_PIO, LEDMAT_COL3_PIO,
    LEDMAT_COL4_PIO, LEDMAT_COL5_PIO
};

typedef struct bullet_s Bullet;
typedef struct alien_s Alien;

struct bullet_s {
    bool isActive;
    int row;
    int col;
    Bullet *nextBullet;
};

struct alien_s {
    int row;
    int col;
    Alien *nextAlien;
};

static uint8_t livesLeft = 5;
static uint16_t gameSpeed = 0;


int playGame(void);
void flashScreen(Bullet * firstBullet, Alien * firstAlien, int myPosition[], uint16_t counter);
char irInput(void);
void moveBullets(Bullet * bullet);
void getInput(int myPosition[], Bullet ** firstBullet,
              Bullet ** currentBullet);
