/*
 * Serial.cpp
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 */

#include "Serial.h"


//------------------------------------------------------------------------------------
//-- STATIC IMPLEMENTATION -----------------------------------------------------------
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
static const char* _MODULE_ = "[Serial]........";
#define _EXPR_	(_debug && !IS_ISR())


//------------------------------------------------------------------------------------
#define DEBUG_CHECK(x)																\
do {                                         										\
	esp_err_t __err_rc = (x);                                       			 	\
	if (__err_rc != ESP_OK) {                                 				      	\
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "Err = %d, line = %d, func = %s, expr = %s", __err_rc, __LINE__, __ASSERT_FUNC, #x);   \
	}                                                               				\
} while(0);


//------------------------------------------------------------------------------------
static void defaultCb(){
}


//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
Serial::Serial(	PinName tx, PinName rx, int bufsize, int baud, char eof,
				uart_port_t uart_num, bool debugmode, uart_hw_flowcontrol_t type, PinName rts, PinName cts,
				uart_word_length_t bits, uart_parity_t parity, uart_stop_bits_t stop_bits) :
					_debug(debugmode), _eof(eof), _en_rx(false), _tx_bufsize(bufsize), _rx_bufsize(bufsize)	{

	_ready = false;
	if(uart_num >= UART_NUM_MAX){
		return;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Creando instancia: ");
	_uart_num = uart_num;

	_cb_rx = callback(defaultCb);
	_cb_tx = callback(defaultCb);
	_cb_rx_tmr = callback(defaultCb);
	_cb_rx_ovf = callback(defaultCb);

	_rx_timeout = (10 * 1000000)/baud;
	
	_uart_config.baud_rate = baud;		//!< baud_rate
	_uart_config.data_bits = bits;		//!< data_bits
	_uart_config.parity = parity;		//!< parity
	_uart_config.stop_bits = stop_bits;	//!< stop_bits
	_uart_config.flow_ctrl = type; 		//!< flow_ctrl
	_uart_config.rx_flow_ctrl_thresh = 0;			//!< rx_flow_ctrl_thresh
	_uart_config.use_ref_tick = false;	//!< use_ref_tick

	/*_uart_config = {
	        baud,		//!< baud_rate
			bits,		//!< data_bits
			parity,		//!< parity
			stop_bits,	//!< stop_bits
			type, 		//!< flow_ctrl
	        0,			//!< rx_flow_ctrl_thresh
	        false		//!< use_ref_tick
	    };*/

	DEBUG_TRACE_D(_EXPR_, _MODULE_, "ajustando params, ");
	DEBUG_CHECK(uart_param_config(_uart_num, &_uart_config) == ESP_OK);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "pines, ");
	DEBUG_CHECK(uart_set_pin(_uart_num, tx, rx, (rts==NC)? UART_PIN_NO_CHANGE : rts, (cts==NC)? UART_PIN_NO_CHANGE : cts) == ESP_OK);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "instalando!");

	if(uart_driver_install(_uart_num, _rx_bufsize, _tx_bufsize, DefaultQueueDepth, &_queue, 0) == ESP_OK){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Iniciando tarea de gesti�n");
		_th.setName("Serial");
		_th.start(callback(this, &Serial::task));
	}
}


//------------------------------------------------------------------------------------
Serial::~Serial() {
	// desactivo interrupciones tx,rx
    uart_disable_rx_intr(_uart_num);
    uart_disable_tx_intr(_uart_num);

    // Desinstalo callbacks
    _cb_rx = NULL;
    _cb_tx = NULL;
    _cb_rx_tmr = NULL;
    _cb_rx_ovf = NULL;

    // libero isr y driver
    uart_isr_free(_uart_num);
    uart_driver_delete(_uart_num);
}


//------------------------------------------------------------------------------------
void Serial::baud(int baudrate) {
    _mutex.lock();
    _rx_timeout = (10 * 1000000)/baudrate;
    uart_set_baudrate(_uart_num, baudrate);
    _mutex.unlock();
}


//---------------------------------------------------------------------------------
void Serial::config(Callback<void()> rx_done, Callback <void()> rx_timeout, Callback <void()> rx_ovf, uint32_t us_timeout, char eof){
	if(rx_done != (Callback<void()>)NULL){
		_cb_rx = rx_done;
	}
	if(rx_timeout != (Callback<void()>)NULL){
		_cb_rx_tmr = rx_timeout;
	}
	if(rx_ovf != (Callback<void()>)NULL){
		_cb_rx_ovf = rx_ovf;
	}
	_eof = eof;
    _rx_timeout = us_timeout;
}


//------------------------------------------------------------------------------------
bool Serial::send(void* data, uint16_t size, Callback<void()> tx_done){
	if(tx_done == (Callback<void()>)NULL){
		_cb_tx = callback(defaultCb);
	}
	else{
		_cb_tx = tx_done;
	}
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Sending %d bytes from buffer... ", size);
	int sent = 0;
	if((sent = uart_write_bytes(_uart_num, (const char*)data, size)) != -1){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "OK!, pushed into fifo %d bytes", sent);
		return true;
	}
	DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR!");
	return false;
}


//------------------------------------------------------------------------------------
int Serial::printff(const char *format, ...) {
	int sent = 0;
	static Callback<void()> txcb = callback(defaultCb);
    _mutex.lock();
    va_list arg;
    va_start(arg, format);
    // If stdlib does not properly handle a size of 0, supply a dummy buffer with a size of 1.
    char dummy_buf[1];
    int len = vsnprintf(dummy_buf, sizeof(dummy_buf), format, arg);
	char *temp = new char[len + 1];
	vsprintf(temp, format, arg);
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Printing:[%s]", temp);
	sent = strlen(temp);
	if(!send(temp, strlen(temp), txcb)){
		sent = 0;
	}
	delete[] temp;
    va_end(arg);
    _mutex.unlock();
    return sent;
}


//------------------------------------------------------------------------------------
uint16_t Serial::recv(void* buf, uint16_t maxsize, int32_t timeout_ms){
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "receiving: ");

	size_t size=0;
	do{
		if(uart_get_buffered_data_len(_uart_num, &size) != ESP_OK){
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR!");
			return 0;
		}
		timeout_ms--;
		if(size == 0 && timeout_ms > 0){
			wait_us(1000);
		}
	}while(size==0 && timeout_ms > 0);

	DEBUG_TRACE_D(_EXPR_, _MODULE_, "%d bytes, timeout restante %d ms. ", size, timeout_ms);
	if(size == 0){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "");
		return 0;
	}

	size = (maxsize < size)? maxsize : size;

	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Reading... ");
	int bytes_read = uart_read_bytes(_uart_num, (uint8_t*)buf, size, MBED_MILLIS_TO_TICK(_rx_timeout));

	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Read %d bytes", bytes_read);
	bytes_read = (bytes_read < 0)? 0 : bytes_read;
	return bytes_read;
}


//------------------------------------------------------------------------------------
void Serial::startReceiver(){
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "startReceiver");
	_mutex.lock();
	uart_flush(_uart_num);
	_en_rx = true;
	_mutex.unlock();
}


//------------------------------------------------------------------------------------
void Serial::stopReceiver(){
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "stopReceiver");
	_mutex.lock();
	_en_rx = false;
	uart_flush(_uart_num);
	_mutex.unlock();
}

//------------------------------------------------------------------------------------
//-- PRIVATE METHODS IMPLEMENTATION --------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void Serial::task() {
	uart_event_t event;
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "Iniciando tarea y cola de mensajes...");
	_ready = true;
	for(;;){
		if (xQueueReceive(_queue, (void * )&event, (portTickType)osWaitForever)){
			uart_event_t*  evt = &event;
			MBED_ASSERT(evt);
			_curr_event = evt->type;
			switch (evt->type) {
				case UART_DATA_BREAK: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_data_break!");
					/* Evento al finalizar un env�o */
					_cb_tx.call();
					break;
				}

				case UART_DATA: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_data ");
					if(_en_rx){
						size_t bytes = 0;
						uart_get_buffered_data_len(_uart_num, &bytes);
						DEBUG_TRACE_D(_EXPR_, _MODULE_, "%d bytes", bytes);
						_cb_rx.call();
					}
					else{
						uart_flush(_uart_num);
					}
					/* Event of UART receiving data
					 * We'd better handler data event fast, there would be much more data events
					 * than other types of events.
					 * If we take too much time on data event, the queue might be full.
					 * In this example, we don't process data in event, but read data outside.
					 */
					break;
				}

				/// Error fifo overflow
				case UART_FIFO_OVF: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_fifo_ovf!");
					// If fifo overflow happened, you should consider adding flow control for your application.
					// We can read data out out the buffer, or directly flush the Rx buffer.
					uart_flush(_uart_num);
					_cb_rx_ovf.call();
					break;
				}

				/// Error buffer lleno
				case UART_BUFFER_FULL: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_buffer_full!");
					// If buffer full happened, you should consider increasing your buffer size
					// We can read data out out the buffer, or directly flush the Rx buffer.
					uart_flush(_uart_num);
					_cb_rx_ovf.call();
					break;
				}

				/// Detecci�n de BREAK en recepci�n (es el fin de trama recibido)
				case UART_BREAK: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_break!");
					if(_en_rx){
						_cb_rx.call();
					}
					else{
						uart_flush(_uart_num);
					}
					break;
				}

				/// Error de paridad
				case UART_PARITY_ERR: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_parity_err!");
					uart_flush(_uart_num);
					_cb_rx_tmr.call();
					break;
				}

				/// Error de frame
				case UART_FRAME_ERR: {
					DEBUG_TRACE_D(_EXPR_, _MODULE_, "EVT: uart_frame_err!");
					uart_flush(_uart_num);
					_cb_rx_tmr.call();
					break;
				}

				/// Detecci�n de patr�n recibido
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

