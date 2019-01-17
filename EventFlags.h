/*
 * EventFlags.h
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 *	Portabilidad de la clase EventFlags de mbed-os v5x a ESP-IDF|FreeRtos
 *
 */

#ifndef MBED_EVENTFLAGS_H
#define MBED_EVENTFLAGS_H

#include "mbed_api.h"

/** The EventFlags class is used to signal or wait for an arbitrary event or events.
 @note 
 EventFlags support 31 flags so the MSB flag is ignored, it is used to return an error code (@a osFlagsError)
 @note
 Memory considerations: The EventFlags control structures will be created on current thread's stack, both for the mbed OS
 and underlying RTOS objects (static or dynamic RTOS memory pools are not being used).
*/
class EventFlags  {
public:
    /** Create and Initialize a EventFlags object

     @param name name to be used for this EventFlags. It has to stay allocated for the lifetime of the thread.
    */
    EventFlags(const char *name = "no-name");

    /** Set the specified Event Flags.
      @param   flags  specifies the flags that shall be set.
      @return  event flags after setting or error code if highest bit set (@a osFlagsError).
     */
    uint32_t set(uint32_t flags);

    /** Clear the specified Event Flags.
      @param   flags  specifies the flags that shall be cleared. (default: 0x7fffffff - all flags)
      @return  event flags before clearing or error code if highest bit set (@a osFlagsError).
     */
    uint32_t clear(uint32_t flags = 0x7fffffff);

    /** Get the currently set Event Flags.
      @return  set event flags.
     */
    uint32_t get() const;

    /** Wait for all of the specified event flags to become signaled.
      @param   flags    specifies the flags to wait for.
      @param   timeout  timeout value or 0 in case of no time-out. (default: osWaitForever)
      @param   clear    specifies wether to clear the flags after waiting for them. (default: true)
      @return  event flags before clearing or error code if highest bit set (@a osFlagsError).
     */
    uint32_t wait_all(uint32_t flags = 0, uint32_t timeout = osWaitForever, bool clear = true);

    /** Wait for any of the specified event flags to become signaled.
      @param   flags    specifies the flags to wait for. (default: 0)
      @param   timeout  timeout value or 0 in case of no time-out. (default: osWaitForever)
      @param   clear    specifies wether to clear the flags after waiting for them. (default: true)
      @return  event flags before clearing or error code if highest bit set (@a osFlagsError).
     */
    uint32_t wait_any(uint32_t flags = 0, uint32_t timeout = osWaitForever, bool clear = true);

    ~EventFlags();

private:
   osEventFlagsId _id;
};


#endif

/** @}*/
