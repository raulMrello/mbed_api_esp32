/* mbed Microcontroller Library
 * Copyright (c) 2006-2012 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "Mutex.h"



//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
Mutex::Mutex() : _name("noname"){
	_id = xSemaphoreCreateMutex();
}


//------------------------------------------------------------------------------------
Mutex::Mutex(const char *name) : _name(name) {
	_id = xSemaphoreCreateMutex();
}


//------------------------------------------------------------------------------------
osStatus Mutex::lock(uint32_t millisec) {
	if(IS_ISR()){
		if(xSemaphoreTakeFromISR(_id, NULL) == pdTRUE){
			return osOK;
		}
		return osErrorOS;
	}
	if(xSemaphoreTake(_id, MBED_MILLIS_TO_TICK(millisec)) == pdTRUE){
		return osOK;
	}
	return osErrorTimeoutResource;
}


//------------------------------------------------------------------------------------
osStatus Mutex::unlock() {
	if(IS_ISR()){
		if(xSemaphoreGiveFromISR(_id, NULL) == pdTRUE){
			return osOK;
		}
		return osErrorOS;
	}
	if(xSemaphoreGive(_id) == pdTRUE){
		return osOK;
	}
	return osErrorOS;
}


//------------------------------------------------------------------------------------
Mutex::~Mutex() {
	vSemaphoreDelete(_id);
}


