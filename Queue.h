/*
 * QUEUE.h
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 *	Portabilidad de la clase Thread de mbed-os v5x a ESP-IDF|FreeRtos
 *
 */

#ifndef MBED_QUEUE_H
#define MBED_QUEUE_H



#include "mbed_api.h"


/** The Queue class allow to control, send, receive, or wait for messages.
 A message can be a integer or pointer value  to a certain type T that is send
 to a thread or interrupt service routine.
  @tparam  T         data type of a single message element.
  @tparam  queue_sz  maximum number of messages in queue.

 @note
 Memory considerations: The queue control structures will be created on current thread's stack, both for the mbed OS
 and underlying RTOS objects (static or dynamic RTOS memory pools are not being used).
*/
template<typename T, uint32_t queue_sz>
class Queue {
public:
    /** Create and initialize a message Queue. */
    Queue(bool create_queue = true) : _curr_data(NULL) {
    	if(create_queue){
    		_qid = xQueueCreate(queue_sz, sizeof(T*));
    		MBED_ASSERT(_qid);
    		return;
    	}
    	_qid = NULL;
    }


    ~Queue() {
    	if(_qid){
    		vQueueDelete(_qid);
    	}
    }

    /** Put a message in a Queue.
      @param   data      message pointer.
      @param   millisec  timeout value or 0 in case of no time-out. (default: 0)
      @param   prio      priority value or 0 in case of default. (default: 0)
      @return  status code that indicates the execution status of the function:
               @a osOK the message has been put into the queue.
               @a osErrorTimeout the message could not be put into the queue in the given time.
               @a osErrorResource not enough space in the queue.
               @a osErrorParameter internal error or non-zero timeout specified in an ISR.
    */
    osStatus put(T* data, uint32_t millisec=0, uint8_t prio=0) {
    	if(!_qid || !data){
    		return osErrorValue;
    	}
    	//uint32_t pdata = (uint32_t)data;
    	if(IS_ISR()){
    		return ((xQueueSendFromISR(_qid, &data, NULL) == pdTRUE)? osOK : osErrorOS);
    	}
    	return ((xQueueSend(_qid, &data, MBED_MILLIS_TO_TICK(millisec)) == pdTRUE)? osOK : osErrorOS);
    }

    /** Get a message or Wait for a message from a Queue. Messages are retrieved in a descending priority order or
        first in first out when the priorities are the same.
      @param   millisec  timeout value or 0 in case of no time-out. (default: osWaitForever).
      @return  event information that includes the message in event.value and the status code in event.status:
               @a osEventMessage message received.
               @a osOK no message is available in the queue and no timeout was specified.
               @a osEventTimeout no message has arrived during the given timeout period.
               @a osErrorParameter a parameter is invalid or outside of a permitted range.
    */
    osEvent get(uint32_t millisec=osWaitForever) {
    	osEvent event;
    	if(!_qid){
    		event.status = (osStatus)osErrorValue;
    		return event;
    	}

        if(xQueueReceive(_qid, &_curr_data, MBED_MILLIS_TO_TICK(millisec)) != pdTRUE){
        	event.status = (osStatus)osEventTimeout;
        	event.def.message_id = _qid;
        	return event;
        }
        event.status = (osStatus)osEventMessage;
		event.value.p = (void*)_curr_data;
		event.def.message_id = _qid;
        return event;
    }

    /** Obtiene una referencia al handle
     * 	@return  handle
     */
    QueueHandle_t* getHandle() { return &_qid; }

protected:
    QueueHandle_t _qid;
    T* _curr_data;
};


#endif

/** @}*/
