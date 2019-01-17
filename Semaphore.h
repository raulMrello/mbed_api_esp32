/*
 * Semaphore.h
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 *	Portabilidad de la clase Semaphore de mbed-os v5x a ESP-IDF|FreeRtos
 *
 */

#ifndef MBED_SEMAPHORE_H
#define MBED_SEMAPHORE_H

#include "mbed_api.h"


/** The Semaphore class is used to manage and protect access to a set of shared resources.
 *
 * @note
 * Memory considerations: The semaphore control structures will be created on current thread's stack, both for the mbed OS
 * and underlying RTOS objects (static or dynamic RTOS memory pools are not being used).
 */
class Semaphore  {
public:
    /** Create and Initialize a Semaphore object used for managing resources.
      @param  count     number of available resources
      @param  max_count maximum number of available resources
    */
    Semaphore(int32_t count, uint16_t max_count);

    /** Wait until a Semaphore resource becomes available.
      @param   millisec  timeout value or 0 in case of no time-out. (default: osWaitForever).
      @return  number of available tokens, before taking one; or -1 in case of incorrect parameters
    */
    int32_t wait(uint32_t millisec=osWaitForever);

    /** Release a Semaphore resource that was obtain with Semaphore::wait.
      @return status code that indicates the execution status of the function:
              @a osOK the token has been correctly released.
              @a osErrorResource the maximum token count has been reached.
              @a osErrorParameter internal error.
    */
    osStatus release(void);

    ~Semaphore();

private:
    osSemaphoreId _id;
};


#endif

/** @}*/
