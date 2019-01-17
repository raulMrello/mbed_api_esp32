/*
 * EventFlags.cpp
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 */

#include "EventFlags.h"


//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
EventFlags::EventFlags(const char *name){
	_id = xEventGroupCreate();
	MBED_ASSERT(_id);
}


//------------------------------------------------------------------------------------
EventFlags::~EventFlags(){
	vEventGroupDelete(_id);
}


//------------------------------------------------------------------------------------
uint32_t EventFlags::set(uint32_t flags){
	if(IS_ISR()){
		return xEventGroupSetBitsFromISR(_id, flags, NULL);
	}
	return xEventGroupSetBits(_id, flags);
}


//------------------------------------------------------------------------------------
uint32_t EventFlags::clear(uint32_t flags){
	if(IS_ISR()){
		return xEventGroupClearBitsFromISR(_id, flags);
	}
	return xEventGroupClearBits(_id, flags);
}


//------------------------------------------------------------------------------------
uint32_t EventFlags::get() const {
    return xEventGroupGetBits(_id);
}


//------------------------------------------------------------------------------------
uint32_t EventFlags::wait_all(uint32_t flags, uint32_t timeout, bool clear) {
	return xEventGroupWaitBits(_id, flags, (clear)? pdTRUE : pdFALSE, pdTRUE, MBED_MILLIS_TO_TICK(timeout));
}


//------------------------------------------------------------------------------------
uint32_t EventFlags::wait_any(uint32_t flags, uint32_t timeout, bool clear){
	return xEventGroupWaitBits(_id, flags, (clear)? pdTRUE : pdFALSE, pdFALSE, MBED_MILLIS_TO_TICK(timeout));
}



