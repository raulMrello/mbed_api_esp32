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
#ifndef TICKER_H
#define TICKER_H

#include "mbed_api.h"
#include "Ticker_HAL.h"


class Ticker  {

public:
	/** Constructor. Asigna un identificador único. En caso dejar el valor por defecto (-1) tomará
	 * 	un uuid igual a si mismo (uuid = this).
	 * @param uuid Identificador del Ticker
	 */
    Ticker(int32_t uuid = -1);


    virtual ~Ticker() {
        detach();
    }


    /** Attach a function to be called by the Ticker, specifying the interval in seconds
     *
     *  @param func pointer to the function to be called
     *  @param t the time between calls in seconds
     */
    void attach(Callback<void()> func, float t) {
        attach_us(func, (uint64_t)(t * 1000000.0f));
    }


    /** Attach a function to be called by the Ticker, specifying the interval in us
     *
     *  @param func pointer to the function to be called
     *  @param t the time between calls in us
     */
    void attach_us(Callback<void()> func, uint64_t microsec);


    /** Detach the function
     */
    void detach();


protected:
    Ticker_HAL::TickerData_t _tdata;	// Estructura que contiene callback y timestamp

};


#endif
