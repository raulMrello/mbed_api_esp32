/*
 * Thread.h
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 *	Portabilidad de la clase Thread de mbed-os v5x a ESP-IDF|FreeRtos
 *
 */

#ifndef MBED_THREAD_H
#define MBED_THREAD_H

#include "mbed_api.h"
#include "Mutex.h"
#include "EventFlags.h"



class Thread {
public:


    /** Allocate a new thread without starting execution
      @param   priority       initial priority of the thread function. (default: osPriorityNormal).
      @param   stack_size     stack size (in bytes) requirements for the thread function. (default: OS_STACK_SIZE).
      @param   stack_mem      pointer to the stack area to be used by this thread (default: NULL).
      @param   name           name to be used for this thread. It has to stay allocated for the lifetime of the thread (default: NULL)
    */
    Thread(osPriority priority=osPriorityNormal, uint32_t stack_size=OS_STACK_SIZE, unsigned char *stack_mem=NULL, const char *name="no-name");

    /** Starts a thread executing the specified function.
      @param   task           function to be executed by this thread.
      @return  status code that indicates the execution status of the function.
      @note a thread can only be started once
    */
    osStatus start(Callback<void()> task);

    /** Terminate execution of a thread and remove it from Active Threads
      @return  status code that indicates the execution status of the function.
    */
    osStatus terminate();

    /** Get priority of an active thread
      @return  current priority value of the thread function.
    */
    osPriority get_priority() { return _priority; }

    /** Set the specified Thread Flags for the thread.
      @param   signals  specifies the signal flags of the thread that should be set.
      @return  signal flags after setting or osFlagsError in case of incorrect parameters.
    */
    int32_t signal_set(int32_t signals);

    /** State of the Thread */
    enum State {
        Inactive,           /**< NOT USED */
        Ready,              /**< Ready to run */
        Running,            /**< Running */
        WaitingDelay,       /**< Waiting for a delay to occur */
        WaitingJoin,        /**< Waiting for thread to join. Only happens when using RTX directly. */
        WaitingThreadFlag,  /**< Waiting for a thread flag to be set */
        WaitingEventFlag,   /**< Waiting for a event flag to be set */
        WaitingMutex,       /**< Waiting for a mutex event to occur */
        WaitingSemaphore,   /**< Waiting for a semaphore event to occur */
        WaitingMemoryPool,  /**< Waiting for a memory pool */
        WaitingMessageGet,  /**< Waiting for message to arrive */
        WaitingMessagePut,  /**< Waiting for message to be send */
        WaitingInterval,    /**< NOT USED */
        WaitingOr,          /**< NOT USED */
        WaitingAnd,         /**< NOT USED */
        WaitingMailbox,     /**< NOT USED (Mail is implemented as MemoryPool and Queue) */
		Waiting,			/// Añadido por compatibilidad con FreeRTOS

        /* Not in sync with RTX below here */
        Deleted,            /**< The task has been deleted or not started */
    };

    /** State of this Thread
      @return  the State of this Thread
    */
    State get_state();
    
    /** Get the total stack memory size for this Thread
      @return  the total stack memory size in bytes
    */
    uint32_t stack_size() { return _stack_size; }
    
    /** Get thread name
      @return  thread name or NULL if the name was not set.
     */
    const char *get_name() {return _name; }


    /** Wait for one or more Thread Flags to become signaled for the current RUNNING thread.
      @param   signals   wait until all specified signal flags are set or 0 for any single signal flag.
      @param   millisec  timeout value or 0 in case of no time-out. (default: osWaitForever).
      @return  event flag information or error code. @note if @a millisec is set to 0 and flag is no set the event carries osOK value.
      @note not callable from interrupt
    */
    static osEvent signal_wait(int32_t signals, uint32_t millisec=osWaitForever);

    /** Wait for a specified time period in millisec:
      @param   millisec  time delay value
      @return  status code that indicates the execution status of the function.
      @note not callable from interrupt
    */
    static osStatus wait(uint32_t millisec);

    /** Pass control to next thread that is in state READY.
      @return  status code that indicates the execution status of the function.
      @note not callable from interrupt
    */
    static osStatus yield();

    /** Get the thread id of the current running thread.
      @return  thread ID for reference by other functions or NULL in case of error.
    */
    static osThreadId gettid() {return xTaskGetCurrentTaskHandle(); }


    virtual ~Thread();

    /** Asigna un nombre al thread
      @param   name Nombre del thread
    */
    void setName(const char* name){ _name = name; }

    /** Wait for thread to terminate
      @return  status code that indicates the execution status of the function.

      @note You cannot call this function from ISR context.
    */
    osStatus join(){
    	return terminate();
    }


protected:

    Callback<void()>  	_task;			/// Función a ejecutar para iniciar la tarea
    const char* 		_name;			/// Nombre
    uint32_t 			_stack_size;	/// Tamaño del stack asignado
    osPriority 			_priority;		/// Prioridad
    osThreadId 			_tid;			/// Identificador
    Mutex       		_mutex;			/// Mutex de acceso exclusivo
};


#endif


