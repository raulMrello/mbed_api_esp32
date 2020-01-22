/*
 * RtosTimer.h
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 *	Portabilidad de la clase RtosTimer de mbed-os v5x a ESP-IDF|FreeRtos
 *
 */

#ifndef MBED_RTOSTIMER_H
#define MBED_RTOSTIMER_H

#include "mbed_api.h"


/** The RtosTimer class allow creating and and controlling of timer functions in the system.
 A timer function is called when a time period expires whereby both on-shot and
 periodic timers are possible. A timer can be started, restarted, or stopped.

 Timers are handled in the thread osTimerThread.
 Callback functions run under control of this thread and may use CMSIS-RTOS API calls.

 @deprecated
 The RtosTimer has been superseded by the EventQueue. The RtosTimer and EventQueue duplicate
 the functionality of timing events outside of interrupt context, however the EventQueue
 has additional features to handle deferring other events to multiple contexts.

 For an example, the following code shows a simple use of the RtosTimer:
 @code
 DigitalOut led(LED1);
 void blink() {
     led = !led;
 }

 RtosTimer timer(&blink);
 int main() {
     timer.start(1000); // call blink every 1s
     wait_ms(5000);
     timer.stop(); // stop after 5s
 }
 @endcode

 This is the above example rewritten to use the EventQueue:
 @code
 DigitalOut led(LED1);
 void blink() {
     led = !led;
 }

 EventQueue queue(4*EVENTS_EVENT_SIZE);
 int main() {
    int blink_id = queue.call_every(1000, &blink); // call blink every 1s
    queue.dispatch(5000);
    queue.cancel(blink_id); // stop after 5s
 }
 @endcode

 @note
 Memory considerations: The timer control structures will be created on current thread's stack, both for the mbed OS
 and underlying RTOS objects (static or dynamic RTOS memory pools are not being used).
*/
class RtosTimer  {
public:

	/** Clave para activar la depuración en tiempo de compilación */
	static const bool DEBUG = true;

    
    /** Create timer.
      @param   func      function to be executed by this timer.
      @param   type      osTimerOnce for one-shot or osTimerPeriodic for periodic behaviour. (default: osTimerPeriodic)
      @deprecated
          The RtosTimer has been superseded by the EventQueue. See RtosTimer.h for more details
    */
    RtosTimer(Callback<void()> func, os_timer_type type=osTimerPeriodic, const char* name = "xtimer", bool defdbg = false);


    /** Stop the timer.
      @return  status code that indicates the execution status of the function:
          @a osOK the timer has been stopped.
          @a osErrorISR @a stop cannot be called from interrupt service routines.
          @a osErrorParameter internal error.
          @a osErrorResource the timer is not running.
    */
    osStatus stop(void);

    /** Start or restart the timer.
      @param   millisec  non-zero value of the timer.
      @return  status code that indicates the execution status of the function:
          @a osOK the timer has been started or restarted.
          @a osErrorISR @a start cannot be called from interrupt service routines.
          @a osErrorParameter internal error or incorrect parameter value.
          @a osErrorResource internal error (the timer is in an invalid timer state).
    */
    osStatus start(uint32_t millisec);

    ~RtosTimer();

    void doCallback(){
    	if(_function){
    		_function.call();
    	}
    }

protected:
    osTimerId 			_id;		/// Identificador del timer
    os_timer_type 		_type;		/// Tipo de timer (autoreload, oneshot)
    Callback<void()> 	_function;	/// Callback a invocar
    const char* 		_name;		/// nombre del timer
    bool 				_defdbg;	/// Flag para activar depuración por defecto <printf>
};



#endif

/** @}*/
