/*
 * Timer.h
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 *	Portabilidad de la clase Timer de mbed-os v5x a ESP-IDF|FreeRtos
 *
 */

#ifndef MBED_TIMER_H
#define MBED_TIMER_H

#include "mbed_api.h"
#include "Ticker_HAL.h"


class Timer {

public:
    Timer();
    ~Timer();

    enum Status{
    	Stopped,
		Started,
    };

    /** Start the timer
     */
    void start();

    /** Stop the timer
     */
    void stop();

    /** Reset the timer to 0.
     *
     * If it was already counting, it will continue
     */
    void reset();

    /** Get the time passed in seconds
     *
     *  @returns    Time passed in seconds
     */
    float read();

    /** Get the time passed in milli-seconds
     *
     *  @returns    Time passed in milli seconds
     */
    int read_ms();

    /** Get the time passed in micro-seconds
     *
     *  @returns    Time passed in micro seconds
     */
    int read_us();

protected:

    uint64_t _last;   	/// Timestamp de última lectura
    uint64_t _start;   	/// Timestamp de arranque o reset
    Status _stat;		/// Estado de ejecución
};

#endif
