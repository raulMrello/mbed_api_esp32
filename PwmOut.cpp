/*
 * PwmOut.cpp
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 */


#include "PwmOut.h"



//------------------------------------------------------------------------------------
//-- STATIC IMPLEMENTATION -----------------------------------------------------------
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
PwmOut::PwmOut(PinName pin, mcpwm_unit_t mcpwm_num, mcpwm_timer_t tmr_num, mcpwm_io_signals_t io_num) :
	_mcpwm_num(mcpwm_num), _tmr_num(tmr_num), _io_num(io_num), _duty(0.0f), _is_init(false){

	switch(_io_num){
		case MCPWM0A:
		case MCPWM1A:
		case MCPWM2A:{
			_op_num = MCPWM_OPR_A;
			break;
		}
		case MCPWM0B:
		case MCPWM1B:
		case MCPWM2B:{
			_op_num = MCPWM_OPR_B;
			break;
		}
		default:
			// ERROR
			_mcpwm_num = MCPWM_UNIT_MAX;
			return;
	}
	mcpwm_gpio_init(_mcpwm_num, _io_num, pin);
	_pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
	_pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
	_pwm_config.counter_mode = MCPWM_UP_COUNTER;
	_pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
	_duty_us = 0;
	_period_us = 0;
}


//------------------------------------------------------------------------------------
void PwmOut::period_us(int us){
	if(_mcpwm_num >= MCPWM_UNIT_MAX){
		return;
	}
	_period_us = us;
	int hz = 1000000/us;

	if(!_is_init){
		_pwm_config.frequency = hz;
		mcpwm_init(_mcpwm_num, _tmr_num, &_pwm_config);    //Configure PWM0A & PWM0B with above settings
	}
	else{
		mcpwm_set_frequency(_mcpwm_num, _tmr_num, hz);
	}
	// recalcula el duty cycle
	write(_duty);
}


//------------------------------------------------------------------------------------
void PwmOut::pulsewidth_us(int us){
	if(_mcpwm_num >= MCPWM_UNIT_MAX){
		return;
	}
	write((float)(us/_period_us));
}


//------------------------------------------------------------------------------------
void PwmOut::write(float value){
	if(_mcpwm_num >= MCPWM_UNIT_MAX){
		return;
	}
	_duty = (value < 0)? 0 : value;
	_duty = (_duty > 1.0f)? 1.0f : _duty;
	_duty_us = (int)(_duty * _period_us);
	mcpwm_set_duty(_mcpwm_num, _tmr_num, _op_num, 100 * _duty);
}

