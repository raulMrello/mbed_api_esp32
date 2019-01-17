/*
 * Mail.h
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 *	Portabilidad de la clase Mail de mbed-os v5x a ESP-IDF|FreeRtos
 *
 */

#ifndef MBED_MAIL_H
#define MBED_

#include "mbed_api.h"
#include "Queue.h"
#include "MemoryPool.h"


/** The Mail class allow to control, send, receive, or wait for mail.
 A mail is a memory block that is send to a thread or interrupt service routine.
  @tparam  T         data type of a single message element.
  @tparam  queue_sz  maximum number of messages in queue.

 @note
 Memory considerations: The mail data store and control structures will be created on current thread's stack,
 both for the mbed OS and underlying RTOS objects (static or dynamic RTOS memory pools are not being used).
*/
template<typename T, uint32_t queue_sz>
class Mail  {
public:
    /** Create and Initialise Mail queue. */
    Mail() { };

    /** Allocate a memory block of type T
      @param   millisec  timeout value or 0 in case of no time-out. (default: 0).
      @return  pointer to memory block that can be filled with mail or NULL in case error.
    */
    T* alloc(uint32_t millisec=0) {
        return _pool.alloc();
    }

    /** Allocate a memory block of type T and set memory block to zero.
      @param   millisec  timeout value or 0 in case of no time-out.  (default: 0).
      @return  pointer to memory block that can be filled with mail or NULL in case error.
    */
    T* calloc(uint32_t millisec=0) {
        return _pool.calloc();
    }

    /** Put a mail in the queue.
      @param   mptr  memory block previously allocated with Mail::alloc or Mail::calloc.
      @return  status code that indicates the execution status of the function.
    */
    osStatus put(T *mptr) {
        return _queue.put(mptr);
    }

    /** Get a mail from a queue.
      @param   millisec  timeout value or 0 in case of no time-out. (default: osWaitForever).
      @return  event that contains mail information or error code.
    */
    osEvent get(uint32_t millisec=osWaitForever) {
        osEvent evt = _queue.get(millisec);
        if (evt.status == osEventMessage) {
            evt.status = osEventMail;
        }
        return evt;
    }

    /** Free a memory block from a mail.
      @param   mptr  pointer to the memory block that was obtained with Mail::get.
      @return  status code that indicates the execution status of the function.
    */
    osStatus free(T *mptr) {
        return _pool.free(mptr);
    }

private:
    Queue<T, queue_sz> _queue;
    MemoryPool<T, queue_sz> _pool;
};


#endif


/** @}*/
