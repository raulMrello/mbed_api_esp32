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
RawSerial::RawSerial(PinName tx, PinName rx, int baud, uart_port_t uart_num, osPriority priority, uint32_t stack_size){
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Iniciando RawSerial");
	_uart_config = {0};
	_tx = tx;
	_rx = rx;
	_rts = NC;
	_cts = NC;
	_uart_num = uart_num;
	_priority = priority;
	_stack_size = stack_size;
	_irq[RxIrq] 	= (Callback<void()>) NULL;
	_irq[TxIrq] 	= (Callback<void()>) NULL;
	_irq[ErrIrq] 	= (Callback<void()>) NULL;
	_uart_config.baud_rate = baud;
	_th = new Thread(_priority, _stack_size, NULL, "RawSerial");
	MBED_ASSERT(_th);
	_installed = false;
	_started = false;
}


//------------------------------------------------------------------------------------
RawSerial::~RawSerial() {
	if(_uart_num >= UART_NUM_MAX){
		return;
	}
	// desactivo interrupciones tx,rx
    uart_disable_rx_intr(_uart_num);
    uart_disable_tx_intr(_uart_num);

    // libero isr y driver
    uart_isr_free(_uart_num);
    uart_driver_delete(_uart_num);
}


//------------------------------------------------------------------------------------
void RawSerial::baud(int baudrate){
	if(_uart_num >= UART_NUM_MAX){
		return;
	}
	_uart_config.baud_rate = baudrate;
	if(_installed){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Actualizando baudrate=%d", baudrate);
		uart_set_baudrate(_uart_num, _uart_config.baud_rate);
	}
	else if(_uart_config.data_bits != 0){
		_install();
	}
}


//---------------------------------------------------------------------------------
void RawSerial::format(int bits, RawSerial::Parity parity, int stop_bits){
	if(_uart_num >= UART_NUM_MAX){
		return;
	}
	_uart_config.data_bits = bits-UART_DATA_BITS_MAX-1;
	_uart_config.parity = (parity == None)? UART_PARITY_DISABLE : ((parity==Odd)? UART_PARITY_ODD : UART_PARITY_EVEN);
	_uart_config.stop_bits = (stop_bits==3)? UART_STOP_BITS_1_5 : ((stop_bits==1)? UART_STOP_BITS_1 : UART_STOP_BITS_2);
	if(_installed){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Actualizando formato num_bits=%d, parity=%d, stop_bits=%d", bits, (int)parity, stop_bits);
		uart_set_word_length(_uart_num, _uart_config.data_bits);
		uart_set_parity(_uart_num, _uart_config.parity);
		uart_set_stop_bits(_uart_num, _uart_config.stop_bits);
	}
	else if(_uart_config.baud_rate != 0){
		_install();
	}
}


//------------------------------------------------------------------------------------
void RawSerial::setFlowCtrl(PinName rts, PinName cts, RawSerial::Flow flow){
	if(_uart_num >= UART_NUM_MAX){
		return;
	}
	_rts = (rts!=NC)? rts : UART_PIN_NO_CHANGE;
	_cts = (cts!=NC)? cts : UART_PIN_NO_CHANGE;
	_uart_config.flow_ctrl = (flow==Disabled)? UART_HW_FLOWCTRL_DISABLE : ((flow==RTS)? UART_HW_FLOWCTRL_RTS : ((flow==CTS)? UART_HW_FLOWCTRL_CTS : UART_HW_FLOWCTRL_CTS_RTS));
	if(_installed){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Actualizando control de flujo rts=%d, cts=%d, flow=%d", (int)rts, (int)cts, (int)flow);
		uart_set_pin(_uart_num, _tx, _rx, _rts, _cts);
		uart_set_hw_flow_ctrl(_uart_num, _uart_config.flow_ctrl, 0);
	}
}


//------------------------------------------------------------------------------------
int RawSerial::readable(){
	if(_uart_num >= UART_NUM_MAX){
		return 0;
	}
	size_t size = 0;
	uart_get_buffered_data_len(_uart_num, &size);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "readable = %d", size);
	return (size > 0)? 1 : 0;
}


//------------------------------------------------------------------------------------
int RawSerial::writeable(){
	if(_uart_num >= UART_NUM_MAX){
		return 0;
	}
	return((uart_wait_tx_done(_uart_num, 0) == ESP_OK)? 1 : 0);
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
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "%d bytes get from uart", size);
	if(size > 0){
		uint8_t buf;
		int bytes_read = uart_read_bytes(_uart_num, &buf, 1, MBED_MILLIS_TO_TICK(osWaitForever));
		return (int)buf;
	}
	return 0;
}



//------------------------------------------------------------------------------------
//-- PRIVATE METHODS IMPLEMENTATION --------------------------------------------------
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
void RawSerial::_install(){
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Configurando uart");
	if(uart_param_config(_uart_num, &_uart_config) != ESP_OK){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "Error al configurar uart");
		return;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Estableciendo pines");
	if(uart_set_pin(_uart_num, _tx, _rx, (_rts==NC)? UART_PIN_NO_CHANGE : _rts, (_cts==NC)? UART_PIN_NO_CHANGE : _cts) != ESP_OK){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "Error al configurar pines de la uart");
		return;

	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Instalando uart");
	if(uart_driver_install(_uart_num, 2*UART_FIFO_LEN, 2*UART_FIFO_LEN, DefaultQueueDepth, &_queue, 0) != ESP_OK){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "Error al instalar uart");
		return;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Driver instalado");
	_installed = true;
	_started = false;
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Iniciando thread");
	_th->start(callback(this, &RawSerial::_task));
	_sem.wait();
	_started = true;
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "RawSerial iniciado!");
}


//------------------------------------------------------------------------------------
void RawSerial::_task() {
	uart_event_t event;
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Iniciando tarea y cola de mensajes...");
	_sem.release();
	for(;;){
		if (xQueueReceive(_queue, (void * )&event, (portTickType)osWaitForever)){
			uart_event_t*  evt = &event;
			MBED_ASSERT(evt);
			_curr_event = evt->type;
			DEBUG_TRACE_D(_EXPR_, _MODULE_, "Se ha recibido el evento %x", (uint32_t)evt->type);
			switch (evt->type) {
				case UART_DATA_BREAK: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_data_break!");
					/* Evento al finalizar un envío */
					_irq[TxIrq].call();
					break;
				}

				case UART_DATA: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_data ");
					size_t bytes = 0;
					uart_get_buffered_data_len(_uart_num, &bytes);
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "%d bytes", bytes);
					_irq[RxIrq].call();
					break;
				}

				/// Error fifo overflow
				case UART_FIFO_OVF: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_fifo_ovf!");
					// If fifo overflow happened, you should consider adding flow control for your application.
					// We can read data out out the buffer, or directly flush the Rx buffer.
					uart_flush(_uart_num);
					_irq[ErrIrq].call();
					break;
				}

				/// Error buffer lleno
				case UART_BUFFER_FULL: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_buffer_full!");
					// If buffer full happened, you should consider increasing your buffer size
					// We can read data out out the buffer, or directly flush the Rx buffer.
					uart_flush(_uart_num);
					_irq[ErrIrq].call();
					break;
				}

				/// Detección de BREAK en recepción (es el fin de trama recibido)
				case UART_BREAK: {
					break;
				}

				/// Error de paridad
				case UART_PARITY_ERR: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_parity_err!");
					uart_flush(_uart_num);
					_irq[ErrIrq].call();
					break;
				}

				/// Error de frame
				case UART_FRAME_ERR: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_frame_err!");
					uart_flush(_uart_num);
					_irq[ErrIrq].call();
					break;
				}

				/// Detección de patrón recibido
				case UART_PATTERN_DET: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_pattern_det!");
					break;
				}

				default:
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: unhandled_evt=%d!", evt->type);
					break;
			}
		}
	}
}



