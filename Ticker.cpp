/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "Ticker.h"



//------------------------------------------------------------------------------------
//---- STATIC ------------------------------------------------------------------------
//------------------------------------------------------------------------------------

static void defaultCallback(){
}



//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
Ticker::Ticker(int32_t uuid) {
	// inicia el subsistema de timming
	Ticker_HAL::start();

	// Inicia parámetros a valores por defecto
	if(uuid == -1){
		uuid = (int32_t)this;
	}
	_tdata.uuid = uuid;
	_tdata.func = callback(defaultCallback);
	_tdata.timeout = 0;
	_tdata.next_event = 0;
}


//------------------------------------------------------------------------------------
void Ticker::attach_us(Callback<void()> func, uint64_t microsec) {
	// desconecta una posible referencia anterior
	detach();

	// reajusta los parámetros
    _tdata.func = func;
    // calcula el timeout en ticks en relación al Scale
    _tdata.timeout = (microsec * Ticker_HAL::TimerScale)/1000000;
    _tdata.next_event = Ticker_HAL::getRawCounter() + _tdata.timeout;
	// conecta la nueva referencia
	Ticker_HAL::attach(&_tdata);
}


//------------------------------------------------------------------------------------
void Ticker::detach() {
	// desinstala objeto
	Ticker_HAL::detach(&_tdata);
	// borra parámetros a valores por defecto
    _tdata.func = callback(defaultCallback);
}

