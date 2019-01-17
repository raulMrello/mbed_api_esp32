/*
 * InterruptIn.cpp
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 */

#include "InterruptIn.h"



//------------------------------------------------------------------------------------
//-- STATIC IMPLEMENTATION -----------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
/** Macro para imprimir trazas de depuración, siempre que se haya configurado un objeto
 *	Logger válido (ej: _debug)
 */
static const char* _MODULE_ = "[InterruptIn]...";
#define _EXPR_	(_defdbg && !IS_ISR())



#define DEBUG_CHECK(x)																\
do {                                         										\
	esp_err_t __err_rc = (x);                                       			 	\
	if (__err_rc != ESP_OK) {                                 				      	\
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "Err = %d, line = %d, func = %s, expr = %s", __err_rc, __LINE__, __ASSERT_FUNC, #x);   \
	}                                                               				\
} while(0);



//------------------------------------------------------------------------------------
static bool g_isr_svc_installed = false;


//------------------------------------------------------------------------------------
static void IRAM_ATTR gpio_isr_handler(void* arg){
	InterruptIn* me = (InterruptIn*)arg;
	ENTER_ISR();
	if(me->read()){
		InterruptIn::_irq_handler((uint32_t)me, IRQ_RISE);
		EXIT_ISR();
		return;
	}
	InterruptIn::_irq_handler((uint32_t)me, IRQ_FALL);
	EXIT_ISR();
}


//------------------------------------------------------------------------------------
static void defaultIsrHandler(){
}



//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
InterruptIn::InterruptIn(PinName pin, bool defdbg) : _gpio() {
    // No lock needed in the constructor
	_defdbg = defdbg;

	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Creando objeto en pin %d", pin);

	_rise = callback(&defaultIsrHandler);
	_fall = callback(&defaultIsrHandler);

	_pin = pin;
	//disable interrupt
	_gpio.intr_type = GPIO_INTR_DISABLE;
	//set as output mode
	_gpio.mode = GPIO_MODE_INPUT;
	//bit mask of the pins that you want to set,e.g.GPIO18/19
	if(pin < GPIO_NUM_32){
		_gpio.pin_bit_mask = (1<<pin);
	}
	else{
		_gpio.pin_bit_mask = ((uint64_t)(((uint64_t)1)<<pin));
	}
	//disable pull-down mode
	_gpio.pull_down_en = GPIO_PULLDOWN_DISABLE;
	//disable pull-up mode
	_gpio.pull_up_en = GPIO_PULLUP_DISABLE;
	//configure GPIO with the given settings
	DEBUG_CHECK(gpio_config(&_gpio));
	//install _gpio isr service (default flags =0 )
	if(!g_isr_svc_installed){
		DEBUG_CHECK(gpio_install_isr_service(0));
		g_isr_svc_installed = true;
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Instalado isr-service");
	}
}


//------------------------------------------------------------------------------------
InterruptIn::~InterruptIn() {
	//disable interrupt
	_gpio.intr_type = GPIO_INTR_DISABLE;
	//set as output mode
	_gpio.mode = GPIO_MODE_INPUT;
	//bit mask of the pins that you want to set,e.g.GPIO18/19
	if(_pin < GPIO_NUM_32){
		_gpio.pin_bit_mask = (1<<_pin);
	}
	else{
		_gpio.pin_bit_mask = ((uint64_t)(((uint64_t)1)<<_pin));
	}
	//disable pull-down mode
	_gpio.pull_down_en = GPIO_PULLDOWN_DISABLE;
	//disable pull-up mode
	_gpio.pull_up_en = GPIO_PULLUP_DISABLE;
	//configure GPIO with the given settings
	DEBUG_CHECK(gpio_config(&_gpio));
}


//------------------------------------------------------------------------------------
int InterruptIn::read() {
    // Read only
	return gpio_get_level(_pin);
}


//------------------------------------------------------------------------------------
void InterruptIn::mode(PinMode pull) {
	switch(pull){
		case PullUp:{
			DEBUG_CHECK(gpio_set_pull_mode(_pin, GPIO_PULLUP_ONLY));
			break;
		}
		case PullDown:{
			DEBUG_CHECK(gpio_set_pull_mode(_pin, GPIO_PULLDOWN_ONLY));
			break;
		}
		default:{
			DEBUG_CHECK(gpio_set_pull_mode(_pin, GPIO_FLOATING));
			break;
		}
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Ajustado pull-mode = %d", pull);
}


//------------------------------------------------------------------------------------
void InterruptIn::rise(Callback<void()> func) {
	if(func != (Callback<void()>)NULL){
		_rise = func;
		_gpio.intr_type = (gpio_int_type_t)(_gpio.intr_type | GPIO_INTR_POSEDGE);
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Activando isr-rise intr_type = %d", _gpio.intr_type);
	}
	else{
		_rise = callback(&defaultIsrHandler);
		_gpio.intr_type = (gpio_int_type_t)(_gpio.intr_type & (~GPIO_INTR_POSEDGE));
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Desactivando isr-rise intr_type = %d", _gpio.intr_type);
	}
	DEBUG_CHECK(gpio_set_intr_type(_pin, _gpio.intr_type));
	if(_gpio.intr_type != GPIO_INTR_DISABLE){
		DEBUG_CHECK(gpio_isr_handler_add(_pin, gpio_isr_handler, (void*)this));
	}
	else{
		DEBUG_CHECK(gpio_isr_handler_remove(_pin));
	}
}


//------------------------------------------------------------------------------------
void InterruptIn::fall(Callback<void()> func) {
	if(func != (Callback<void()>)NULL){
		_fall = func;
		_gpio.intr_type = (gpio_int_type_t)(_gpio.intr_type | GPIO_INTR_NEGEDGE);
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Activando isr-fall intr_type = %d", _gpio.intr_type);
	}
	else{
		_fall = callback(&defaultIsrHandler);
		_gpio.intr_type = (gpio_int_type_t)(_gpio.intr_type & (~GPIO_INTR_NEGEDGE));
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Desactivando isr-fall intr_type = %d", _gpio.intr_type);
	}
	DEBUG_CHECK(gpio_set_intr_type(_pin, _gpio.intr_type));
	if(_gpio.intr_type != GPIO_INTR_DISABLE){
		DEBUG_CHECK(gpio_isr_handler_add(_pin, gpio_isr_handler, (void*)this));
	}
	else{
		DEBUG_CHECK(gpio_isr_handler_remove(_pin));
	}
}


//------------------------------------------------------------------------------------
void InterruptIn::enable_irq() {
	if(_gpio.intr_type != GPIO_INTR_DISABLE){
		DEBUG_CHECK(gpio_isr_handler_add(_pin, gpio_isr_handler, (void*)this));
	}
}


//------------------------------------------------------------------------------------
void InterruptIn::disable_irq() {
	DEBUG_CHECK(gpio_isr_handler_remove(_pin));
}


//------------------------------------------------------------------------------------
InterruptIn::operator int() {
    // Underlying call is atomic
    return read();
}


//------------------------------------------------------------------------------------
void InterruptIn::_irq_handler(uint32_t id, gpio_irq_event event) {
    InterruptIn *handler = (InterruptIn*)id;
    switch (event) {
        case IRQ_RISE:
            handler->_rise.call();
            break;
        case IRQ_FALL:
            handler->_fall.call();
            break;
        case IRQ_NONE: break;
    }
}

//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------



