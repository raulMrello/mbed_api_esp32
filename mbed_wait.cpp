/*
 * mbed_wait.cpp
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	Implementa la portabilidad de temporizaciones basadas en wait
 *
 */

#include "mbed.h"


//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void wait(float s) {
    wait_us(s * 1000000.0f);
}


//------------------------------------------------------------------------------------
void wait_ms(int ms) {
    wait_us(ms * 1000);
}


//------------------------------------------------------------------------------------
void wait_us(int us) {
	int ms = us / 1000;
	if (MBED_MILLIS_TO_TICK(ms) > 1){
		Thread::wait(ms);
	}
	else{
		Timer tmr;
		tmr.start();
		while(tmr.read_us() < us);
	}
}

