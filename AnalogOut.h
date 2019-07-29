/*
 * AnalogOut.h
 *
 *  Created on: Aug 2019
 *      Author: raulMrello
 *
 *	Portabilidad del driver AnalogOut
 *	NOTAS:
 *		ESP32 dispone de dos DAC GPO25 (CH1) y GPIO26 (CH2)
 *
 */

#ifndef ANALOGOUT_H
#define ANALOGOUT_H

#include "mbed_api.h"


class AnalogOut {

public:

    /** Create an AnalogOut connected to the specified pin
     *
     * @param pin AnalogOut pin to connect to
     */
    AnalogOut(PinName pin)  {
    	switch(pin){
    		case GPIO_NUM_25:
    			_channel = DAC_GPIO25_CHANNEL;
    			break;
    		case GPIO_NUM_26:
    			_channel = DAC_GPIO26_CHANNEL;
				break;

    		default:
    			return;
    	}
    }

    /** Set the output voltage, specified as a percentage (float)
     *
     *  @param value A floating-point value representing the output voltage,
     *    specified as a percentage. The value should lie between
     *    0.0f (representing 0v / 0%) and 1.0f (representing 3.3v / 100%).
     *    Values outside this range will be saturated to 0.0f or 1.0f.
     */
    void write(float value) {
    	_value = value;
    	dac_output_voltage(_channel, (uint8_t)((_value * 255)/255));
    	dac_output_enable(_channel);
    }

    /** Set the output voltage, represented as an unsigned short in the range [0x0, 0x00FF]
     *
     *  @param value 16-bit unsigned short representing the output voltage,
     *            normalized to a 16-bit value (0x0000 = 0v, 0x00FF = 3.3v)
     */
    void write_u16(unsigned short value) {
    	_value  = (float)value/255;
    	dac_output_voltage(_channel, (uint8_t)(value));
    	dac_output_enable(_channel);
    }

    /** Return the current output voltage setting, measured as a percentage (float)
     *
     *  @returns
     *    A floating-point value representing the current voltage being output on the pin,
     *    measured as a percentage. The returned value will lie between
     *    0.0f (representing 0v / 0%) and 1.0f (representing 3.3v / 100%).
     *
     *  @note
     *    This value may not match exactly the value set by a previous write().
     */
    float read()  {
        return _value;
    }

    /** An operator shorthand for write()
     * \sa AnalogOut::write()
     */
    AnalogOut &operator= (float percent)  {
        // Underlying write call is thread safe
        write(percent);
        return *this;
    }

    /** An operator shorthand for write()
     * \sa AnalogOut::write()
     */
    AnalogOut &operator= (AnalogOut &rhs)   {
        // Underlying write call is thread safe
        write(rhs.read());
        return *this;
    }

    /** An operator shorthand for read()
     * \sa AnalogOut::read()
     */
    operator float()  {
        // Underlying read call is thread safe
        return read();
    }

    virtual ~AnalogOut()  {
        // Do nothing
    }

protected:

    dac_channel_t _channel;
    float _value;
};


#endif
