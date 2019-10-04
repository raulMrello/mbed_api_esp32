/*
 * RawSerial.h
 *
 *  Created on: Oct 2019
 *      Author: raulMrello
 *
 *	Portabilidad del driver RawSerial compatible con ESP-IDF
 */

#ifndef RAWSERIAL_H
#define RAWSERIAL_H

#include "mbed_api.h"
#include "Thread.h"
#include "Semaphore.h"
#include "Callback.h"



class RawSerial {

public:

	/** Configuración por defecto del componente */
    enum Parity {
        None = 0,
        Odd,
        Even
    };

    enum IrqType {
        RxIrq = 0,
        TxIrq,
		ErrIrq,
        IrqCnt
    };

    enum Flow {
        Disabled = 0,
        RTS,
        CTS,
        RTSCTS
    };

    /** Create a RawSerial port, connected to the specified transmit and receive pins, with the specified baud.
     *
     *  @param tx Transmit pin
     *  @param rx Receive pin
     *  @param baud The baud rate of the serial port (optional, defaults to MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE)
     *  @param uart_num Incluyo este parámetro para seleccionar la UART
     *  @param priority Prioridad del hilo de contol
     *  @param stack_size Tamaño del stack del hilo de control
     *
     *  @note
     *    Either tx or rx may be specified as NC if unused
     */
    RawSerial(PinName tx, PinName rx, int baud, uart_port_t uart_num = UART_NUM_1, osPriority priority=osPriorityNormal, uint32_t stack_size = OS_STACK_SIZE);

    virtual ~RawSerial();


    /** Set the baud rate of the serial port
     *
     *  @param baudrate The baudrate of the serial port (default = 9600).
     */
    void baud(int baudrate);


    /** Set the transmission format used by the serial port
     *
     *  @param bits The number of bits in a word (5-8; default = 8)
     *  @param parity The parity used (RawSerial::None, RawSerial::Odd, RawSerial::Even, RawSerial::Forced1, RawSerial::Forced0; default = RawSerial::None)
     *  @param stop_bits The number of stop bits (1=1, 2=2, 3=1.5)
     */
    void format(int bits = 8, Parity parity = None, int stop_bits = 1);


    /**
     * Establece el control de flujo
     * @param rts Pin rts
     * @param cts Pin cts
     * @param flow Tipo de control de flujo
     */
    void setFlowCtrl(PinName rts=NC, PinName cts=NC, Flow flow = Disabled);


    /** Determine if there is a character available to read
     *
     *  @returns
     *    1 if there is a character available to read,
     *    0 otherwise
     */
    int readable();


    /** Determine if there is space available to write a character
     *
     *  @returns
     *    1 if there is space to write a character,
     *    0 otherwise
     */
    int writeable();


    /** Attach a function to call whenever a serial interrupt is generated
     *
     *  @param func A pointer to a void function, or 0 to set as none
     *  @param type Which serial interrupt to attach the member function to (Serial::RxIrq for receive, TxIrq for transmit buffer empty)
     */
    void attach(Callback<void()> func, IrqType type = RxIrq){
    	_irq[type] = func;
    }


    /** Write a char to the serial port
     *
     * @param c The char to write
     *
     * @returns The written char or -1 if an error occurred
     */
    int putChar(int c);

	#ifdef putc
	#undef putc
	#endif
    int putc(int c){ return putChar(c); }


    /** Read a char from the serial port
     *
     * @returns The char read from the serial port
     */
    int getChar();

	#ifdef getc
	#undef getc
	#endif
	int getc(){ return getChar(); }


    /** Write a string to the serial port
     *
     * @param str The string to write
     *
     * @returns 0 if the write succeeds, EOF for error
     */
    int puts(const char *str);
    int printf(const char *format, ...);

protected:

    /* Acquire exclusive access to this serial port
     */
    virtual void lock(void){
        // No lock used - external synchronization required
    }

    /* Release exclusive access to this serial port
     */
    virtual void unlock(void){
        // No lock used - external synchronization required
    }


    /** Control de la tarea asociada a la UART ESP32 */
    Thread* _th;
    QueueHandle_t _queue;
    Semaphore _sem{0,1};
    osPriority _priority;
    uint32_t _stack_size;

    /** Máximo número acumulado de eventos en la tarea asociada a la UART */
    static const uint32_t DefaultQueueDepth = 16;

    /** Callbacks */
    Callback<void()> _irq[IrqCnt];

    /** Controlador UART ESP32 */
    uart_config_t _uart_config;
    uart_port_t _uart_num;
    uart_event_type_t _curr_event;
    PinName _tx, _rx, _rts, _cts;
    bool _installed;
    bool _started;

    /**
     * Instalación del driver
     */
    void _install();

    /**
     * Tarea de control
     */
    void _task();
};


#endif
