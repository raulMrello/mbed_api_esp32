/*
 * RawSerial.cpp
 *
 *  Created on: Oct 2019
 *      Author: raulMrello
 *
 */

#include "RawSerial.h"


//------------------------------------------------------------------------------------
//-- STATIC IMPLEMENTATION -----------------------------------------------------------
//------------------------------------------------------------------------------------

#define STRING_STACK_LIMIT    120

//------------------------------------------------------------------------------------
static const char* _MODULE_ = "[RawSerial].....";
#define _EXPR_	(!IS_ISR())


//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
RawSerial::RawSerial(PinName tx, PinName rx, int baud, uart_port_t uart_num) : SerialBase(tx, rx, baud, uart_num)	{

}


//------------------------------------------------------------------------------------
RawSerial::~RawSerial() {
}


//------------------------------------------------------------------------------------
int RawSerial::putChar(int c) {
	if(!_started){
		return -1;
	}
	int sent = uart_write_bytes(_uart_num, (const char*)c, 1);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "%d bytes pushed into fifo", sent);
	return sent;
}


//------------------------------------------------------------------------------------
int RawSerial::puts(const char *str) {
	if(!_started){
		return -1;
	}
	int sent = uart_write_bytes(_uart_num, str, strlen(str));
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "%d bytes pushed into fifo", sent);
	return sent;
}

//---------------------------------------------------------------------------------
int RawSerial::printf(const char *format, ...) {
	if(!_started){
		return -1;
	}
    va_list arg;
    lock();
    va_start(arg, format);
    // ARMCC microlib does not properly handle a size of 0.
    // As a workaround supply a dummy buffer with a size of 1.
    char dummy_buf[1];
    int len = vsnprintf(dummy_buf, sizeof(dummy_buf), format, arg);
    if (len < STRING_STACK_LIMIT) {
        char temp[STRING_STACK_LIMIT];
        vsprintf(temp, format, arg);
        puts(temp);
    } else {
        char *temp = new char[len + 1];
        vsprintf(temp, format, arg);
        puts(temp);
        delete[] temp;
    }
    va_end(arg);
    unlock();
    return len;
}


//------------------------------------------------------------------------------------
int RawSerial::getChar(){
	if(!_started){
		return 0;
	}
	size_t size = 0;
	uart_get_buffered_data_len(_uart_num, &size);
	if(size > 0){
		uint8_t buf;
		int bytes_read = uart_read_bytes(_uart_num, &buf, 1, MBED_MILLIS_TO_TICK(osWaitForever));
		return (int)buf;
	}
	return 0;
}




