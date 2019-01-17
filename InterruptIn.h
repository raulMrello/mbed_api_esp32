/*
 * InterruptIn.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	Portabilidad del driver InterruptIn
 *
 */

#ifndef INTERRUPTIN_H
#define INTERRUPTIN_H

#include "mbed_api.h"


class InterruptIn  {

public:

    /** Create an InterruptIn connected to the specified pin
     *
     *  @param pin InterruptIn pin to connect to
     *  @param defdbg Flags de depuración
     */
    InterruptIn(PinName pin, bool defdbg = false);
    virtual ~InterruptIn();

    /** Read the input, represented as 0 or 1 (int)
     *
     *  @returns
     *    An integer representing the state of the input pin,
     *    0 for logical 0, 1 for logical 1
     */
    int read();

    /** An operator shorthand for read()
     */
    operator int();


    /** Attach a function to call when a rising edge occurs on the input
     *
     *  @param func A pointer to a void function, or 0 to set as none
     */
    void rise(Callback<void()> func);


    /** Attach a function to call when a falling edge occurs on the input
     *
     *  @param func A pointer to a void function, or 0 to set as none
     */
    void fall(Callback<void()> func);


    /** Set the input pin mode
     *
     *  @param pull PullUp, PullDown, PullNone
     */
    void mode(PinMode pull);

    /** Enable IRQ. This method depends on hw implementation, might enable one
     *  port interrupts. For further information, check gpio_irq_enable().
     */
    void enable_irq();

    /** Disable IRQ. This method depends on hw implementation, might disable one
     *  port interrupts. For further information, check gpio_irq_disable().
     */
    void disable_irq();

    static void _irq_handler(uint32_t id, gpio_irq_event event);

private:
    gpio_config_t _gpio;
    PinName _pin;
    bool _defdbg;
    Callback<void()> _rise;
    Callback<void()> _fall;
};



#endif
