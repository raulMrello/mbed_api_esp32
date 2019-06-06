/*
 * Semaphore.cpp
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 */

#include "Semaphore.h"



//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
Semaphore::Semaphore(int32_t count, uint16_t max_count) {
	_id = xSemaphoreCreateCounting(max_count, count);
    MBED_ASSERT(_id);
}


//------------------------------------------------------------------------------------
int32_t Semaphore::wait(uint32_t millisec) {
	if(IS_ISR()){
		if(xSemaphoreTakeFromISR(_id, NULL) == pdTRUE){
			return uxSemaphoreGetCount(_id) + 1;
		}
		return 0;
	}
	if(xSemaphoreTake(_id, MBED_MILLIS_TO_TICK(millisec)) == pdTRUE){
		return uxSemaphoreGetCount(_id) + 1;
	}
	return 0;
}


//------------------------------------------------------------------------------------
osStatus Semaphore::release(void) {
	if(IS_ISR()){
		if(xSemaphoreGiveFromISR(_id, NULL) == pdTRUE){
			return osOK;
		}
		return osError;
	}
	if(xSemaphoreGive(_id) != pdTRUE){
		return osError;
	}
    return osOK;
}


//------------------------------------------------------------------------------------
Semaphore::~Semaphore() {
	vSemaphoreDelete(_id);
}

