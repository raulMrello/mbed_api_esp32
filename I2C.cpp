/*
 * I2C.cpp
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 */

#include "I2C.h"



//------------------------------------------------------------------------------------
//-- STATIC IMPLEMENTATION -----------------------------------------------------------
//------------------------------------------------------------------------------------
static const char* _MODULE_ = "[I2C]...........";
#define _EXPR_	(_debug && !IS_ISR())



//------------------------------------------------------------------------------------
static i2c_port_t pv_obj_count = I2C_NUM_0;			//!< Contador de instancias realizadas



//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
I2C::I2C(PinName sda, PinName scl, bool debug) : _debug(debug){
    // No lock needed in the constructor
	if(pv_obj_count >= I2C_NUM_MAX){
		return;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Creando instancia: ");
	_i2c_num = pv_obj_count;
	// Precargo próximo canal
	pv_obj_count = (pv_obj_count == I2C_NUM_0)? I2C_NUM_1 : I2C_NUM_MAX;

	// Inicializo parámetros por defecto
	_i2c.mode = I2C_MODE_MASTER;
	_i2c.sda_io_num = sda;
	_i2c.sda_pullup_en = GPIO_PULLUP_ENABLE;
	_i2c.scl_io_num = scl;
	_i2c.scl_pullup_en = GPIO_PULLUP_ENABLE;
	_i2c.master.clk_speed = DefaultSpeed;
	_timeout_ns = 1000000000/DefaultSpeed;
	if(i2c_param_config(_i2c_num, &_i2c) != ESP_OK){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "Error en i2c_param_config");
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "OK!");
}


//------------------------------------------------------------------------------------
void I2C::frequency(int hz) {
    lock();
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Cambiando velocidad a %dHz: ", hz);
    _i2c.master.clk_speed = hz;
    _timeout_ns = 1000000000/hz;
    if(i2c_param_config(_i2c_num, &_i2c) != ESP_OK){
    	DEBUG_TRACE_E(_EXPR_, _MODULE_, "Error en i2c_param_config");
    }
    if(i2c_driver_install(_i2c_num, _i2c.mode, 0, 0, 0) != ESP_OK){
    	DEBUG_TRACE_E(_EXPR_, _MODULE_, "Error en i2c_driver_install");
    }
    DEBUG_TRACE_D(_EXPR_, _MODULE_, "OK!");
    unlock();
}



//------------------------------------------------------------------------------------
// write - Master Transmitter Mode
int I2C::write(int address, const char* data, int length, bool repeated) {
    lock();
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Escribiendo %d bytes: ", length);
    DEBUG_TRACE_D(_EXPR_, _MODULE_, "creando comando, ");
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    DEBUG_TRACE_D(_EXPR_, _MODULE_, "start|");
    if(i2c_master_start(cmd) != ESP_OK){
    	return -1;
    }
    DEBUG_TRACE_D(_EXPR_, _MODULE_, "addr|");
	if(i2c_master_write_byte(cmd, (address | I2C_MASTER_WRITE), true) != ESP_OK){
		return -1;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "data|");
	if(i2c_master_write(cmd, (uint8_t*)data, length, true) != ESP_OK){
		return -1;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "stop");
	if(i2c_master_stop(cmd) != ESP_OK){
		return -1;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "(starting...)");
	esp_err_t ret = i2c_master_cmd_begin(_i2c_num, cmd, MBED_MILLIS_TO_TICK((length*_timeout_ns)/100));
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "result %d, ", ret);
	i2c_cmd_link_delete(cmd);
    DEBUG_TRACE_D(_EXPR_, _MODULE_, "OK!");
    unlock();
    return (ret == ESP_OK)? 0 : -1;
}


//------------------------------------------------------------------------------------
int I2C::write(int data) {
    lock();
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_write_byte(cmd, data, true);
	esp_err_t ret = i2c_master_cmd_begin(_i2c_num, cmd, MBED_MILLIS_TO_TICK(_timeout_ns/100));
	i2c_cmd_link_delete(cmd);
    unlock();
    return (ret == ESP_OK)? 0 : -1;
}


//------------------------------------------------------------------------------------
// read - Master Reciever Mode
int I2C::read(int address, char* data, int length, bool repeated) {
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Leyendo %d bytes: ", length);
    lock();
    DEBUG_TRACE_D(_EXPR_, _MODULE_, "creando comando, ");
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    DEBUG_TRACE_D(_EXPR_, _MODULE_, "start|");
	if(i2c_master_start(cmd) != ESP_OK){
		return -1;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "addr|");
	if(i2c_master_write_byte(cmd, (address | I2C_MASTER_READ), true) != ESP_OK){
		return -1;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "data_rd|");
	if (length > 1) {
		if(i2c_master_read(cmd, (uint8_t*)data, length - 1, (i2c_ack_type_t)0) != ESP_OK){
			return -1;
		}
	}
	if(i2c_master_read_byte(cmd, (uint8_t*)(data + length - 1), (i2c_ack_type_t)1) != ESP_OK){
		return -1;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "stop");
	if(i2c_master_stop(cmd) != ESP_OK){
		return -1;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "(starting...)");
	esp_err_t ret = i2c_master_cmd_begin(_i2c_num, cmd, MBED_MILLIS_TO_TICK((length*_timeout_ns)/100));
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "result %d, ", ret);
	i2c_cmd_link_delete(cmd);
    DEBUG_TRACE_D(_EXPR_, _MODULE_, "OK!");
    unlock();
    return (ret == ESP_OK)? 0 : -1;
}


//------------------------------------------------------------------------------------
int I2C::read(int ack) {
    lock();
    uint8_t data = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (ack) {
    	i2c_master_read_byte(cmd, &data, (i2c_ack_type_t)0);
    } else {
    	i2c_master_read_byte(cmd, &data, (i2c_ack_type_t)1);
    }
    i2c_master_cmd_begin(_i2c_num, cmd, MBED_MILLIS_TO_TICK(_timeout_ns/100));
    i2c_cmd_link_delete(cmd);
    unlock();
    return data;
}


//------------------------------------------------------------------------------------
void I2C::start(void) {
    lock();
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_cmd_begin(_i2c_num, cmd, MBED_MILLIS_TO_TICK(_timeout_ns/100));
	i2c_cmd_link_delete(cmd);
    unlock();
}


//------------------------------------------------------------------------------------
void I2C::stop(void) {
    lock();
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(_i2c_num, cmd, MBED_MILLIS_TO_TICK(_timeout_ns/100));
	i2c_cmd_link_delete(cmd);
    unlock();
}


//------------------------------------------------------------------------------------
void I2C::lock() {
    _mutex.lock();
}


//------------------------------------------------------------------------------------
void I2C::unlock() {
    _mutex.unlock();
}


//------------------------------------------------------------------------------------
//-- PRIVATE METHODS IMPLEMENTATION --------------------------------------------------
//------------------------------------------------------------------------------------



