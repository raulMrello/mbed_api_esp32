/*
 * Thread.cpp
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 */

#include "Thread.h"


//------------------------------------------------------------------------------------
//--- PRIVATE TYPES ------------------------------------------------------------------
//------------------------------------------------------------------------------------

/** Rutina estática para iniciar la callback asociada al thread */
static void TaskMain(void* arg){
	Callback<void()>* cback = (Callback<void()> *)arg;
	cback->call();
}



//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
Thread::Thread(osPriority priority, uint32_t stack_size, unsigned char *stack_mem, const char *name) : _name(name) {
    _tid = 0;
    _priority = priority;
    _stack_size = stack_size;
}


//------------------------------------------------------------------------------------
osStatus Thread::start(Callback<void()> task) {
    _mutex.lock();

    if ((_tid != 0)) {
        _mutex.unlock();
        return osErrorParameter;
    }

    _task = task;
    xTaskCreate(TaskMain, _name, _stack_size, (void*)&_task, _priority, &_tid);
    if(!_tid){
    	_mutex.unlock();
        return osErrorResource;
    }
    _mutex.unlock();
    return osOK;
}


//------------------------------------------------------------------------------------
osStatus Thread::terminate() {
	if(!_tid){
		return osErrorResource;
	}
    _mutex.lock();
    vTaskDelete(_tid);
    Thread::wait(1);

    _mutex.unlock();
    return osOK;
}


//------------------------------------------------------------------------------------
int32_t Thread::signal_set(int32_t flags) {
	// ejecuta en contexto ISR
	if(IS_ISR()){
		BaseType_t pxHigherPriorityTaskWoken = pdTRUE;
		if(xTaskNotifyFromISR(_tid, flags, eSetBits, &pxHigherPriorityTaskWoken) != pdPASS){
			return 0;
		}
		return flags;
	}

	// ejecuta en contexto de tarea
	if(xTaskNotify(_tid, flags, eSetBits) != pdPASS){
		return 0;
	}
	return flags;
}


//------------------------------------------------------------------------------------
Thread::State Thread::get_state() {
	State user_state = Inactive;
	if(!_tid){
		return user_state;
	}

    switch(eTaskGetState(_tid)) {
        case eReady:
            user_state = Ready;
            break;
        case eRunning:
            user_state = Running;
            break;
        case eBlocked:
            user_state = Waiting;
            break;
        case eDeleted:
        default:
            user_state = Deleted;
            break;
    }

    return user_state;
}


//------------------------------------------------------------------------------------
osEvent Thread::signal_wait(int32_t signals, uint32_t millisec) {
	osEvent evt;
	uint32_t flags = 0;

	do{
		if(xTaskNotifyWait(flags, ULONG_MAX, &flags, MBED_MILLIS_TO_TICK(millisec)) != pdPASS){
			evt.status = (osStatus)osEventTimeout;
			return evt;
		}
	}while(signals != 0 && (signals & flags) == 0);
	evt.status = (osStatus)osEventSignal;
	evt.value.signals = flags;
	return evt;
}

osStatus Thread::wait(uint32_t millisec) {
	vTaskDelay(MBED_MILLIS_TO_TICK(millisec));
    return osOK;
}

osStatus Thread::yield() {
	taskYIELD();
    return osOK;
}


//------------------------------------------------------------------------------------
Thread::~Thread() {
    // terminate is thread safe
    terminate();
}




