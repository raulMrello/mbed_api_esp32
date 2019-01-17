/*
 * AnalogIn.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	Portabilidad del driver AnalogIn
 *	NOTAS:
 *		ESP32 dispone de dos ADC dependiendo de la fuente analógica:
 *		ADC1 utiliza entradas del sistema como la tensión Vdd, sensores cap, etc...
 *		ADC2 utiliza pines GPIO, por lo tanto en función de la entrada seleccionada se utilizará uno u otro ADC.
 *
 */

#ifndef MBED_ANALOGIN_H
#define MBED_ANALOGIN_H

#include "mbed_api.h"
#include "Mutex.h"



class AnalogIn {

public:

    /** Create an AnalogIn, connected to the specified pin
     *
     * @param pin AnalogIn pin to connect to
     */
    AnalogIn(PinName pin) {

    	switch(pin){
    		case GPIO_NUM_36:
    			_unit = ADC_UNIT_1;
    			_channel = ADC_CHANNEL_0;
    			break;
    		case GPIO_NUM_37:
    			_unit = ADC_UNIT_1;
    			_channel = ADC_CHANNEL_1;
				break;
    		case GPIO_NUM_38:
    			_unit = ADC_UNIT_1;
    			_channel = ADC_CHANNEL_2;
    			break;
    		case GPIO_NUM_39:
    			_unit = ADC_UNIT_1;
    			_channel = ADC_CHANNEL_3;
    			break;
    		case GPIO_NUM_33:
    			_unit = ADC_UNIT_1;
    			_channel = ADC_CHANNEL_4;
    			break;
    		case GPIO_NUM_32:
    			_channel = ADC_CHANNEL_5;
    			_unit = ADC_UNIT_1;
    			break;
    		case GPIO_NUM_34:
    			_unit = ADC_UNIT_1;
    			_channel = ADC_CHANNEL_6;
    			break;
    		case GPIO_NUM_35:
    			_unit = ADC_UNIT_1;
    			_channel = ADC_CHANNEL_7;
    			break;

    		case GPIO_NUM_4:
    			_unit = ADC_UNIT_2;
    			_channel = ADC_CHANNEL_0;
    			break;
    		case GPIO_NUM_0:
    			_unit = ADC_UNIT_2;
    			_channel = ADC_CHANNEL_1;
    			break;
    		case GPIO_NUM_2:
    			_unit = ADC_UNIT_2;
    			_channel = ADC_CHANNEL_2;
    			break;
    		case GPIO_NUM_15:
    			_unit = ADC_UNIT_2;
    			_channel = ADC_CHANNEL_3;
    			break;
    		case GPIO_NUM_13:
    			_unit = ADC_UNIT_2;
    			_channel = ADC_CHANNEL_4;
    			break;
    		case GPIO_NUM_12:
    			_unit = ADC_UNIT_2;
    			_channel = ADC_CHANNEL_5;
    			break;
    		case GPIO_NUM_14:
    			_unit = ADC_UNIT_2;
    			_channel = ADC_CHANNEL_6;
    			break;
    		case GPIO_NUM_27:
    			_unit = ADC_UNIT_2;
    			_channel = ADC_CHANNEL_7;
    			break;
    		case GPIO_NUM_25:
    			_unit = ADC_UNIT_2;
    			_channel = ADC_CHANNEL_8;
    			break;
    		case GPIO_NUM_26:
    			_unit = ADC_UNIT_2;
    			_channel = ADC_CHANNEL_9;
    			break;

    		default:
    			return;
    	}
    	lock();
    	if(_unit == ADC_UNIT_1){
    		adc1_config_channel_atten((adc1_channel_t)_channel, ADC_ATTEN_DB_11);
    		adc1_config_width(ADC_WIDTH_BIT_12);
    	}
    	else{
    		adc2_config_channel_atten((adc2_channel_t)_channel, ADC_ATTEN_DB_11);
    	}
    	unlock();
    }

    /** Read the input voltage, represented as a float in the range [0.0, 1.0]
     *
     * @returns A floating-point value representing the current input voltage, measured as a percentage
     */
    float read() {
    	return ((float)read_u16())/Scale;
    }

    /** Read the input voltage, represented as an unsigned short in the range [0x0, 0xFFFF]
     *
     * @returns
     *   16-bit unsigned short representing the current input voltage, normalised to a 16-bit value
     */
    unsigned short read_u16() {
    	if(_unit == ADC_UNIT_1){
    		lock();
    		int data = adc1_get_raw((adc1_channel_t)_channel);
    		unlock();
    		if(data != -1){
    			return (unsigned short)data;
    		}
    		return 0;
		}
		else{
			lock();
			int data = -1;
			if(adc2_get_raw((adc2_channel_t)_channel, ADC_WIDTH_12Bit, &data) == ESP_OK){
				unlock();
				return (unsigned short)data;
			}
			unlock();
			return 0;
		}
    }

    /** An operator shorthand for read()
     *
     * The float() operator can be used as a shorthand for read() to simplify common code sequences
     *
     * Example:
     * @code
     * float x = volume.read();
     * float x = volume;
     *
     * if(volume.read() > 0.25) { ... }
     * if(volume > 0.25) { ... }
     * @endcode
     */
    operator float() {
        // Underlying call is thread safe
        return read();
    }

    virtual ~AnalogIn() {
        // Do nothing
    }

protected:

    static const uint16_t Scale = 4095;		//!< Fondo de escala por defecto para 12-bits

    virtual void lock() {
        _mutex.lock();
    }

    virtual void unlock() {
        _mutex.unlock();
    }

    adc_unit_t _unit;
    adc_channel_t _channel;
    static Mutex _mutex;
};



#endif

