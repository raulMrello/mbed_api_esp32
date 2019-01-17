/*
 * DigitalOut.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	Portabilidad del driver DigitalOut
 *
 */

#ifndef DIGITALOUT_H
#define DIGITALOUT_H

#include "mbed_api.h"


class DigitalOut {

public:
    /** Create a DigitalOut connected to the specified pin
     *
     *  @param pin DigitalOut pin to connect to
     */
    DigitalOut(PinName pin) : gpio() {
        // No lock needed in the constructor
    	gpioInit(pin);

    }

    /** Create a DigitalOut connected to the specified pin
     *
     *  @param pin DigitalOut pin to connect to
     *  @param value the initial pin value
     */
    DigitalOut(PinName pin, int value) : gpio() {
        // No lock needed in the constructor
    	gpioInit(pin);
    	gpio_set_level(pin, (unsigned int)((value != 0)? 1 : 0));
    }

    /** Set the output, specified as 0 or 1 (int)
     *
     *  @param value An integer specifying the pin output value,
     *      0 for logical 0, 1 (or any other non-zero value) for logical 1
     */
    void write(int value) {
        // Thread safe / atomic HAL call
    	gpio_set_level(_pin, (unsigned int)((value != 0)? 1 : 0));
    }

    /** Return the output setting, represented as 0 or 1 (int)
     *
     *  @returns
     *    an integer representing the output setting of the pin,
     *    0 for logical 0, 1 for logical 1
     */
    int read() {
        // Thread safe / atomic HAL call
        return gpio_get_level(_pin);
    }

    /** Return the output setting, represented as 0 or 1 (int)
     *
     *  @returns
     *    Non zero value if pin is connected to uc GPIO
     *    0 if gpio object was initialized with NC
     */
    int is_connected() {
        // Thread safe / atomic HAL call
        return ((_pin == NC)? 0 : _pin);
    }

    /** A shorthand for write()
     * \sa DigitalOut::write()
     */
    DigitalOut& operator= (int value) {
        // Underlying write is thread safe
        write(value);
        return *this;
    }

    /** A shorthand for write()
     * \sa DigitalOut::write()
     */
    DigitalOut& operator= (DigitalOut& rhs) {
        write(rhs.read());
        return *this;
    }

    /** A shorthand for read()
     * \sa DigitalOut::read()
     */
    operator int() {
        // Underlying call is thread safe
        return read();
    }

protected:
    gpio_config_t gpio;
    PinName _pin;

    void gpioInit(PinName pin){
    	_pin = pin;
    	//disable interrupt
		gpio.intr_type = GPIO_INTR_DISABLE;
		//set as output mode
		gpio.mode = GPIO_MODE_OUTPUT;
		//bit mask of the pins that you want to set,e.g.GPIO18/19
		if(pin < GPIO_NUM_32){
			gpio.pin_bit_mask = (1<<pin);
		}
		else{
			gpio.pin_bit_mask = ((uint64_t)(((uint64_t)1)<<pin));
		}
		//disable pull-down mode
		gpio.pull_down_en = GPIO_PULLDOWN_DISABLE;
		//disable pull-up mode
		gpio.pull_up_en = GPIO_PULLUP_DISABLE;
		//configure GPIO with the given settings
		gpio_config(&gpio);
    }
};


#endif
