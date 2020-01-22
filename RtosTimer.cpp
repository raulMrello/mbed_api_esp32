/*
 * RtosTimer.cpp
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 */

#include "RtosTimer.h"


//------------------------------------------------------------------------------------
//--- PRIVATE TYPES ------------------------------------------------------------------
//------------------------------------------------------------------------------------

static const char* _MODULE_ = "[RtosTimer].....";
#define _EXPR_	(_defdbg && !IS_ISR())



static void defaultCallback(){
}

static void vCallbackFunction(TimerHandle_t arg){
	Callback<void()>* cback = (Callback<void()> *)pvTimerGetTimerID(arg);
	if(cback){
		cback->call();
	}
}


//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
RtosTimer::RtosTimer(Callback<void()> func, os_timer_type type, const char* name, bool defdbg) : _name(name), _defdbg(defdbg) {
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Creando RtosTimer <%s>...", _name);
	_id = 0;
    _function = func;
    _type = type;
}


//------------------------------------------------------------------------------------
osStatus RtosTimer::start(uint32_t millisec) {
	if(_id==0){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Creando xTimerCreate para RtosTimer <%s>", _name);
		if((_id = xTimerCreate(_name, MBED_MILLIS_TO_TICK(millisec), _type, (void*)&_function, vCallbackFunction)) == 0){
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR xTimerCreate en RtosTimer <%s>", _name);
			return osError;
		}
		if(IS_ISR()){
			if(xTimerStartFromISR(_id, NULL) == pdPASS){
				return osOK;
			}
		}
		else{
			if(xTimerStart(_id, 0) == pdPASS){
				return osOK;
			}
		}
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR al iniciar RtosTimer <%s>", _name);
		return osError;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Reiniciando RtosTimer <%s>", _name);
	// lo reinicia cambiando el periodo
	if(IS_ISR()){
		if(xTimerChangePeriodFromISR(_id, MBED_MILLIS_TO_TICK(millisec), NULL) == pdPASS){
			return osOK;
		}
	}
	else{
		if(xTimerChangePeriod(_id, MBED_MILLIS_TO_TICK(millisec), 0) == pdPASS){
			return osOK;
		}
	}
	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR al iniciar RtosTimer <%s>", _name);
	return osError;
}


//------------------------------------------------------------------------------------
osStatus RtosTimer::stop(void) {
	if(xTimerIsTimerActive(_id) == pdFALSE){
		return osOK;
	}
	if(IS_ISR()){
		if(xTimerStopFromISR(_id, NULL) == pdPASS){
			return osOK;
		}
		return osError;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Deteniendo RtosTimer <%s>", _name);
	if(xTimerStop(_id, 0) == pdPASS){
		return osOK;
	}

	return osError;
}


//------------------------------------------------------------------------------------
RtosTimer::~RtosTimer() {
	xTimerDelete(_id, 0);
	_id = 0;
	_function = callback(defaultCallback);
}

