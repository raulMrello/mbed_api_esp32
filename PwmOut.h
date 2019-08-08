/*
 * PwmOut.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	Portabilidad del driver PwmOut
 *	NOTAS:
 *		El chip ESP32 dispone dos unidades MCPWM cada una con 3 timers y 2 salidas PWM cada uno
 */

#ifndef PWMOUT_H
#define PWMOUT_H

#include "mbed_api.h"



class PwmOut {

public:

    /** Create a PwmOut connected to the specified pin
     *
     *  @param pin PwmOut pin to connect to
     *  @param mcpwm_num Canal MCPWM a utilizar
     *  @param io_num Salida a utilizar
     */
    PwmOut(PinName pin, mcpwm_unit_t mcpwm_num = MCPWM_UNIT_0, mcpwm_timer_t tmr_num = MCPWM_TIMER_0, mcpwm_io_signals_t io_num = MCPWM0A);

    ~PwmOut() {
    }

    /** Set the ouput duty-cycle, specified as a percentage (float)
     *
     *  @param value A floating-point value representing the output duty-cycle,
     *    specified as a percentage. The value should lie between
     *    0.0f (representing on 0%) and 1.0f (representing on 100%).
     *    Values outside this range will be saturated to 0.0f or 1.0f.
     */
    void write(float value);

    /** Return the current output duty-cycle setting, measured as a percentage (float)
     *
     *  @returns
     *    A floating-point value representing the current duty-cycle being output on the pin,
     *    measured as a percentage. The returned value will lie between
     *    0.0f (representing on 0%) and 1.0f (representing on 100%).
     *
     *  @note
     *  This value may not match exactly the value set by a previous write().
     */
    float read() {
        return _duty;
    }

    /** Set the PWM period, specified in seconds (float), keeping the duty cycle the same.
     *
     *  @param seconds Change the period of a PWM signal in seconds (float) without modifying the duty cycle
     *  @note
     *   The resolution is currently in microseconds; periods smaller than this
     *   will be set to zero.
     */
    void period(float seconds) {
    	period_us((int)(1000000 * seconds));
    }

    /** Set the PWM period, specified in milli-seconds (int), keeping the duty cycle the same.
     *  @param ms Change the period of a PWM signal in milli-seconds without modifying the duty cycle
     */
    void period_ms(int ms) {
    	period_us(1000 * ms);
    }

    /** Set the PWM period, specified in micro-seconds (int), keeping the duty cycle the same.
     *  @param us Change the period of a PWM signal in micro-seconds without modifying the duty cycle
     */
    void period_us(int us);

    /** Set the PWM pulsewidth, specified in seconds (float), keeping the period the same.
     *  @param seconds Change the pulse width of a PWM signal specified in seconds (float)
     */
    void pulsewidth(float seconds) {
    	pulsewidth_us((int)(1000000 * seconds));
    }

    /** Set the PWM pulsewidth, specified in milli-seconds (int), keeping the period the same.
     *  @param ms Change the pulse width of a PWM signal specified in milli-seconds
     */
    void pulsewidth_ms(int ms) {
    	pulsewidth_us(1000 * ms);
    }

    /** Set the PWM pulsewidth, specified in micro-seconds (int), keeping the period the same.
     *  @param us Change the pulse width of a PWM signal specified in micro-seconds  
     */
    void pulsewidth_us(int us);

    /** A operator shorthand for write()
     *  \sa PwmOut::write()
     */
    PwmOut& operator= (float value) {
        // Underlying call is thread safe
        write(value);
        return *this;
    }

    /** A operator shorthand for write()
     * \sa PwmOut::write()
     */    
    PwmOut& operator= (PwmOut& rhs) {
        // Underlying call is thread safe
        write(rhs.read());
        return *this;
    }

    /** An operator shorthand for read()
     * \sa PwmOut::read()
     */
    operator float() {
        // Underlying call is thread safe
        return read();
    }

protected:

    mcpwm_unit_t _mcpwm_num;
    mcpwm_timer_t _tmr_num;
    mcpwm_operator_t _op_num;
    mcpwm_io_signals_t _io_num;
    mcpwm_config_t _pwm_config;
    float _duty;
    int _period_us;
    int _duty_us;
    bool _is_init;
};



#endif
