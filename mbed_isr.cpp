/*
 * mbed_isr.cpp
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 *	Implementa un sencillo control de ISR nesting
 *
 */

#include "mbed_api.h"
#include "Callback.h"


//------------------------------------------------------------------------------------
//--- PRIVATE TYPES ------------------------------------------------------------------
//------------------------------------------------------------------------------------

static volatile int isr_nesting = 0;			/// Variable global de control del isr nesting


//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


bool IS_ISR(){
	return ((isr_nesting > 0)? true : false);
}

int GET_ISR_NESTING(){
	return (isr_nesting);
}

void ENTER_ISR(){
	++isr_nesting;
}

void EXIT_ISR(){
	--isr_nesting;
	MBED_ASSERT(isr_nesting >= 0);
}


