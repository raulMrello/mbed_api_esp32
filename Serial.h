/*
 * SerialBase.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	Portabilidad del driver SerialBase + RawSerial en un nuevo módulo denominado Serial compatible con ESP-IDF y la
 *	funcionalidad UART_EVENTS
 *	NOTAS:
 *		El chip ESP32 dispone de 3 canales UART (UART0, UART1, UART2)
 */

#ifndef SERIAL_H
#define SERIAL_H

#include "mbed_api.h"
#include "mbed.h"



class Serial  {

public:

    enum IrqType {
        RxIrq = 0,
        TxIrq,
		ErrIrq,
        IrqCnt
    };

    /** Constructor que crea una instancia
	 *
	 *  @param tx tx pin
	 *  @param rx rx pin
	 *  @param baud The baudrate of the serial port (default = 9600).
	 *  @param eof Caracter de fín de trama
	 *  @param uart_num UART
     *  @param debugmode Flag para activar o no el modo depuracion
     *  @param type the flow control type (default=Disabled, RTS, CTS, RTSCTS)
     *  @param flow1 the first flow control pin (default=NC, RTS for RTS or RTSCTS, CTS for CTS)
     *  @param flow2 the second flow control pin (default=NC, CTS for RTSCTS)
     *  @param bits The number of bits in a word (5-8; default = 8)
     *  @param parity The parity used (default Serial::None, Serial::Odd, Serial::Even)
     *  @param stop_bits The number of stop bits (1 or 2; default = 1)
	 */
	Serial( PinName tx, PinName rx, int bufsize=1024, int baud=9600, char eof=0,
			uart_port_t uart_num=UART_NUM_0, bool debugmode=false, uart_hw_flowcontrol_t type=UART_HW_FLOWCTRL_DISABLE, PinName rts=NC, PinName cts=NC,
			uart_word_length_t bits=UART_DATA_8_BITS, uart_parity_t parity=UART_PARITY_DISABLE, uart_stop_bits_t stop_bits=UART_STOP_BITS_1);

	/**
	 * Destructor
	 */
    virtual ~Serial();


    /**
     * Chequea si el componente está habilitado y listo para su ejecución
     * @return Estado del componente
     */
    bool ready(){ return _ready;  }


    /** Set the baud rate of the serial port
     *
     *  @param baudrate The baudrate of the serial port (default = 9600).
     */
    void baud(int baudrate);


    /** config()
     *  Configura las callbacks
     *  @param rx_done Callback a invocar tras la recepción completa
     *  @param rx_timeout Callback a invocar tras un fallo por timeout
     *  @param rx_ovf Callback a invocar tras un fallo por overflow en el buffer de recepción
     *  @param us_timeout Tiempo en us para recibir la trama antes de notificar un error por timeout o por fin de trama
     *  @param eof Caracter de fin de trama (end_of_file)
     */
    void config(Callback<void()> rx_done, Callback <void()> rx_timeout, Callback <void()> rx_ovf, uint32_t us_timeout, char eof = 0);


    /** send()
     *  Prepara para una nueva transimisión gestionada por interrupciones. El final de transmisión
     *  se notifica invocando la callback
     *  @param data Buffer de datos de origen
     *  @param size Tamaño del buffer a enviar
     *  @param cb_data_sent Callback a invocar al finalizar el envío
     *  @return Indica si la transferencia se ha iniciado (true) o no (false)
     */
    bool send(void* data, uint16_t size, Callback<void()> tx_done = (Callback<void()>)NULL);


    /** printf()
     * 	Impresión con formato
     * @param format Formato y lista de parámetros
     * @return
     */
    int printff(const char *format, ...);


    /** recv()/read()
     *  Lee el contenido del buffer de recepción hasta un máximo de maxsize bytes
     *  @param buf Buffer de destino en el que copiar la trama recibida
     *  @param maxsize Tamaño del buffer de destino
     *  @param timeout_us Tiempo a esperar en millis hasta que haya datos en el buffer de recepción
     *  @return Número de bytes copiados
     */
    uint16_t read(void* buf, uint16_t maxsize, int32_t timeout_ms = 0){
    	return recv(buf, maxsize, timeout_ms);
    }
    uint16_t recv(void* buf, uint16_t maxsize, int32_t timeout_ms = 0);


    /** startReceiver()
     *  Habilita el receptor en modo isr-managed y por lo tanto lo deja listo para recibir
     *  datos en modo interrupción
     */
    void startReceiver();


    /** stopReceiver()
     *  Deshabilita el receptor en modo isr-managed y por lo tanto deja de recibir
     *  datos en modo interrupción
     */
    void stopReceiver();


    /** busy()
     *  Informa si el transmisor está ocupado o no
     *  @return True: ocupado, False: listo para enviar
     */
    bool busy(){ return((uart_wait_tx_done(_uart_num, 0) == ESP_OK)? false : true);}

    /** Determine if there is a character available to read
     *
     *  @returns
     *    1 if there is a character available to read,
     *    0 otherwise
     */
    int readable(){
    	size_t size = 0;
    	uart_get_buffered_data_len(_uart_num, &size);
    	return (size > 0)? 1 : 0;
    }


    /** Determine if there is space available to write a character
     *
     *  @returns
     *    1 if there is space to write a character,
     *    0 otherwise
     */
    int writeable(){ return((uart_wait_tx_done(_uart_num, 0) == ESP_OK)? 1 : 0);}

    /** Write a char to the serial port
     *
     * @param c The char to write
     *
     * @returns The written char or -1 if an error occured
     */
    int putch(int c){
    	if(send(&c, 1, NULL)){
    		return 1;
    	}
    	return -1;
    }

    /** Read a char from the serial port
     *
     * @returns The char read from the serial port
     */
    int getch(){
    	int data;
    	if(recv(&data, 1)==0){
    		return -1;
    	}
    	return (data);
    }


    uart_event_type_t getCurrentEvent(){
    	return _curr_event;
    }

    /** setDebugMode()
     *  Habilita o Deshabilita el modo de depuración
     *  @param endis True Habilita el modo depuración, False cancela el modo depuración
     */
    void setDebugMode(bool endis) {_debug = endis; }


protected:

    /** Tarea de gestión en Freertos
     */
    void task();


    /** Máximo número acumulado de eventos en la tarea asociada a la UART */
    static const uint32_t DefaultQueueDepth = 16;

    bool _ready;
    bool _debug;
    char _eof;
    bool _en_rx;
    int _tx_bufsize;
    int _rx_bufsize;
    uint32_t _rx_timeout;
    uart_config_t _uart_config;
    uart_port_t _uart_num;
    Thread _th{osPriorityNormal, 3072, NULL, "Serial"};
    QueueHandle_t _queue;
    uart_event_type_t _curr_event;
//    Queue<uart_event_t, DefaultQueueDepth> _queue{false};
    Mutex _mutex;
    Callback<void()> _cb_rx;
    Callback<void()> _cb_tx;
    Callback<void()> _cb_rx_tmr;
    Callback<void()> _cb_rx_ovf;
};


#endif
