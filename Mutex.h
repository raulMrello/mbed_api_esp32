/*
 * Mutex.h
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 *	Portabilidad de la clase Mutex de mbed-os v5x a ESP-IDF|FreeRtos
 *
 */

#ifndef MBED_MUTEX_H
#define MBED_MUTEX_H

#include "mbed_api.h"

class Mutex {
public:
    /** Create and Initialize a Mutex object */
    Mutex();

    /** Create and Initialize a Mutex object

     @param name name to be used for this mutex. It has to stay allocated for the lifetime of the thread.
    */
    Mutex(const char *name);

    /** Wait until a Mutex becomes available.
      @param   millisec  timeout value or 0 in case of no time-out. (default: osWaitForever)
      @return  status code that indicates the execution status of the function:
               @a osOK the mutex has been obtained.
               @a osErrorTimeout the mutex could not be obtained in the given time.
               @a osErrorParameter internal error.
               @a osErrorResource the mutex could not be obtained when no timeout was specified.
               @a osErrorISR this function cannot be called from the interrupt service routine.
     */
    osStatus lock(uint32_t millisec=osWaitForever);

    /** Try to lock the mutex, and return immediately
      @return true if the mutex was acquired, false otherwise.
     */
    bool trylock();

    /** Unlock the mutex that has previously been locked by the same thread
      @return status code that indicates the execution status of the function:
              @a osOK the mutex has been released.
              @a osErrorParameter internal error.
              @a osErrorResource the mutex was not locked or the current thread wasn't the owner.
              @a osErrorISR this function cannot be called from the interrupt service routine.
     */
    osStatus unlock();

    ~Mutex();

private:
    SemaphoreHandle_t _id;
    const char* _name;
};


#endif

/** @}*/
