/*
 * Author: Tyler Cavill and Jia Hao Guo
 * File: game.c
 * Purpose: This is a space invader alike game programme.
 * 
 */

#include "system.h"
#include "pacer.h"
#include "pio.h"
#include "tinygl.h"
#include "navswitch.h"
#include "../../fonts/font5x7_1.h"
#include "util/delay.h"
#include <avr/io.h>
#include "ledmat.h"
#include <stdbool.h>
#include <stdlib.h>
#include "ir_uart.h"
#include "game.h"


void displayText(void)
{
    pacer_wait();
    tinygl_update();
}

//check if a bullet is inactive or if it is in an invalid row or column.
//if either these are the case delete the bullet.
Bullet *clearBullets(Bullet * bullet)
{
    Bullet *firstBullet = bullet;
    Bullet *previousBullet = bullet;
    while (bullet != NULL) {
        if (bullet->isActive == 0) {
            if (bullet == firstBullet) {
                firstBullet = bullet->nextBullet;
                free(bullet);
                bullet = firstBullet;
            } else {
                previousBullet->nextBullet = bullet->nextBullet;
                free(bullet);
                bullet = previousBullet->nextBullet;
            }
        } else {
            if (bullet->row > MAX_ROW || bullet->row < MIN_ROW) {
                bullet->isActive = 0;
            } else if (bullet->col > MAX_COL || bullet->col < MIN_COL) {
                bullet->isActive = 0;
            } else {
                bullet = bullet->nextBullet;
            }
        }
    }
    return firstBullet;
}

Alien *clearAlien(Alien * alien)
{
    Alien *firstAlien = alien;
    Alien *previousAlien = alien;
    while (alien != NULL) {
        if (alien->row > MAX_ROW) {
            if (alien == firstAlien) {
                firstAlien = alien->nextAlien;
                free(alien);
                alien = firstAlien;
            } else {
                previousAlien->nextAlien = alien->nextAlien;
                free(alien);
                alien = previousAlien->nextAlien;
            }
            livesLeft -= 1;
		}
        else{
			alien = alien->nextAlien;
		}
    }
    return firstAlien;
}

void addAlien(Alien **currentAlien, Alien **firstAlien)
{
	Alien *newAlien = malloc(sizeof(Alien));
	newAlien->row = -1;
	newAlien->col = random() % (MAX_COL + 1);
	newAlien->nextAlien = NULL;
	if (*firstAlien == NULL) {
        *firstAlien = newAlien;
        *currentAlien = newAlien;
    } else {
        (*currentAlien)->nextAlien = newAlien;
         *currentAlien = newAlien;
    }
}

//move bullets every so many game ticks this will run.
//runs over all the bullets and moves them down a row
void moveBullets(Bullet * bullet)
{
    ledmat_init();
    while (bullet != NULL) {
        bullet->row -= 1;
        bullet = bullet->nextBullet;
    }
}

void moveAlien(Alien * alien)
{
    ledmat_init();
    while (alien != NULL) {
        alien->row += 1;
        alien = alien->nextAlien;
    }
}

//get input from the nav switch
void getInput(int myPosition[], Bullet ** firstBullet,
              Bullet ** currentBullet)
{
	
    if (navswitch_push_event_p(NAVSWITCH_WEST)) {
        //turn off previous column
        pio_config_set(ledmat_cols[myPosition[1]], PIO_OUTPUT_HIGH);
        myPosition[1] -= 1;
        //flow over to the otherside
        if (myPosition[1] < 0) {
            myPosition[1] = 4;
        }
        pio_config_set(ledmat_cols[myPosition[1]], PIO_OUTPUT_LOW);
    } else if (navswitch_push_event_p(NAVSWITCH_EAST)) {
        //turn off previous column
        pio_config_set(ledmat_cols[myPosition[1]], PIO_OUTPUT_HIGH);
        myPosition[1] += 1;
        //flow over to the otherside
        if (myPosition[1] > 4) {
            myPosition[1] = 0;
        }
        pio_config_set(ledmat_cols[myPosition[1]], PIO_OUTPUT_LOW);
    }
    //create a new bullet
    else if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
        Bullet *newBullet = malloc(sizeof(Bullet));
        newBullet->row = myPosition[0] - 1;
        newBullet->col = myPosition[1];
        newBullet->nextBullet = NULL;
        newBullet->isActive = 1;
        if (*firstBullet == NULL) {
            *firstBullet = newBullet;
            *currentBullet = newBullet;
        } else {
            (*currentBullet)->nextBullet = newBullet;
            *currentBullet = newBullet;
        }
        
    }
}


Alien* checkCollision(Bullet** firstBullet, Alien* firstAlien){
	Alien *currentAlien = firstAlien;
	Alien *previousAlien = NULL;
	
	Bullet *currentBullet = *firstBullet;
	Bullet *previousBullet = NULL;
	
	while (currentAlien != NULL) {
		currentBullet = *firstBullet;
		
		while (currentBullet != NULL) {
			if ((currentBullet->row == currentAlien->row || 
			currentBullet->row + 1 == currentAlien->row) &&
			currentBullet->col == currentAlien->col) {
				if (previousAlien != NULL) {
					previousAlien->nextAlien = currentAlien->nextAlien;
					free(currentAlien);
					currentAlien = previousAlien->nextAlien;
				} else {
					firstAlien = currentAlien->nextAlien;
					free(currentAlien);
					currentAlien = firstAlien;
				}
				/*if(previousBullet != NULL){
					previousBullet->nextBullet = currentBullet->nextBullet;
					free(currentBullet);
					currentBullet = previousBullet->nextBullet;
				} else{
					*firstBullet = currentBullet->nextBullet;
					free(currentBullet);
					currentBullet = *firstBullet;
				}*/
				ir_uart_putc('A');
			}
			currentBullet = currentBullet->nextBullet;
		}
		previousAlien = currentAlien;
		currentAlien = currentAlien->nextAlien;
		
		
	}
	
	return firstAlien;
}

//----------------------------------------------------------------------
//read any input from the IR device
char irInput(void)
{
    char character = 0;
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
		ir_uart_putc(character);
	}
    if (ir_uart_read_ready_p()) {
        character = ir_uart_getc();
    }
    return character;
}
//----------------------------------------------------------------------

/* this gets looped over every game tick.
 * this function every few ticks flashes the player position and the
 * positions of all of the bullets
 */
void flashScreen(Bullet * firstBullet, Alien * firstAlien, int myPosition[], uint16_t counter)
{

    //flash the player position
    if (counter % PLAYER_UPDATE_TICKS == 0) {
        ledmat_init();
        pio_config_set(ledmat_cols[myPosition[1]], PIO_OUTPUT_LOW);
        pio_config_set(ledmat_rows[myPosition[0]], PIO_OUTPUT_LOW);
    }
    //flash the bullets
    else if (counter % BULLET_UPDATE_TICKS == 0) {
        ledmat_init();
        Bullet *bullet = firstBullet;
        uint8_t waitCounter = 0;

        while (bullet != NULL) {
            if (waitCounter % BULLET_UPDATE_WAIT_TICKS == 0) {
                ledmat_init();
                if (bullet->isActive == 1) {
                    pio_config_set(ledmat_rows[bullet->row],
                                   PIO_OUTPUT_LOW);
                    pio_config_set(ledmat_cols[bullet->col],
                                   PIO_OUTPUT_LOW);
                }
                bullet = bullet->nextBullet;
            }
            waitCounter++;
        }

    }
    //flash the bullets
    else if (counter % ALIEN_UPDATE_TICKS == 0) {
        ledmat_init();
        Alien *alien = firstAlien;
        uint8_t waitCounter = 0;

        while (alien != NULL) {
            if (waitCounter % ALIEN_UPDATE_WAIT_TICKS == 0) {
                ledmat_init();
                
                pio_config_set(ledmat_rows[alien->row],
                                   PIO_OUTPUT_LOW);
                pio_config_set(ledmat_cols[alien->col],
                                   PIO_OUTPUT_LOW);
              
                alien = alien->nextAlien;
            }
            waitCounter++;
        }

    }
}

//the actual game function
int playGame(void)
{
    //start position of bottom middle
    int myPosition[] = { MAX_ROW, MAX_COL / 2 };

    ledmat_init();
    pio_config_set(ledmat_rows[myPosition[0]], PIO_OUTPUT_LOW);
    pio_config_set(ledmat_cols[myPosition[1]], PIO_OUTPUT_LOW);

    uint16_t counter = 0;
    Bullet *firstBullet = NULL;
    Bullet *currentBullet = NULL;
    
    Alien *firstAlien = NULL;
    Alien *currentAlien = NULL;
    
    char character = 0;

    while (1) {

        flashScreen(firstBullet, firstAlien, myPosition, counter);

        if (counter % MOVE_BULLET_TICKS == 0) {
            firstBullet = clearBullets(firstBullet);
            moveBullets(firstBullet);
            firstAlien = checkCollision(&firstBullet, firstAlien);
        }
        
        if (counter % (gameSpeed) == 0) {
			addAlien(&currentAlien, &firstAlien);	
		}
		
		if (counter % MOVE_ALIEN_TICKS == 0) {
			firstAlien = clearAlien(firstAlien);
			moveAlien(firstAlien);
		}
		
		if(livesLeft == 0){
			ir_uart_putc('Z');
			return 0;
		}

        ledmat_init();
        navswitch_update();
        getInput(myPosition, &firstBullet, &currentBullet);

		//----------------------------------------------------------------
        //get any IR input, 'A' signifises that the other player killed an alien so our game speeds up
        //'Z' signifises that the other player lost
        character = irInput();
        if (character == 'A') {
            gameSpeed = gameSpeed - 250;
            if(gameSpeed < 1){
				gameSpeed = 1;
			}
        } else if (character == 'Z') {
            return 1;
        }
        //----------------------------------------------------------------

        counter++;
    }

    return 0;
}

void gameInit(){
	livesLeft = 5;
	gameSpeed = CREATE_ALIEN_TICK;
}

int main(void)
{
    system_init();

    tinygl_init(LOOP_RATE);
    tinygl_font_set(&font5x7_1);
    tinygl_text_speed_set(MESSAGE_RATE);
    tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
    tinygl_text("To play press the navswitch down");
    tinygl_update();

    TCCR1A = 0x00;
    TCCR1B = 0x05;
    TCCR1C = 0x00;

    navswitch_init();
    pacer_init(LOOP_RATE);
    ir_uart_init();


    while (1) {
        displayText();
        navswitch_update();
        if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
            tinygl_clear();
            break;
        }
    }
    
	while(1){
		int wonGame = 0;
		gameInit();
		wonGame = playGame();
		if (wonGame == 1) {
			tinygl_text("Game winner. Press navswitch down to play again");
		} else {
			tinygl_text("Game loser. Press navswitch down to play again");
		}
		while (1) {
			displayText();
			navswitch_update();
			if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
				tinygl_clear();
				break;
			}
		}
	}

    return 0;
}
