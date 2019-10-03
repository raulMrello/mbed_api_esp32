/*
 * SerialBase.h
 *
 *  Created on: Oct 2019
 *      Author: raulMrello
 *
 *	Portabilidad del driver SerialBase  compatible con ESP-IDF
 */

#ifndef SERIALBASE_H
#define SERIALBASE_H

#include "mbed_api.h"
#include "Thread.h"
#include "Semaphore.h"
#include "Callback.h"



class SerialBase  {

public:

	/** Configuración por defecto del componente */
	static const uint32_t DefaultTxBufferSize = 1024;
	static const uint32_t DefaultRxBufferSize = 1024;
	static const osPriority DefaultTaskPriority = osPriorityNormal;
	static const uint32_t DefaultTaskStackSize = 2048;
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

    /** Create a Serial port, connected to the specified transmit and receive pins, with the specified baud.
     *
     *  @param tx Transmit pin
     *  @param rx Receive pin
     *  @param baud The baud rate of the serial port
     *  @param uart_num Incluyo este parámetro para seleccionar la UART
     *
     *  @note
     *    Either tx or rx may be specified as NC if unused
     */
	SerialBase(PinName tx, PinName rx, int baud, uart_port_t uart_num = UART_NUM_0);


	/**
	 * Destructor
	 */
	virtual ~SerialBase();



    /** Set the baud rate of the serial port
     *
     *  @param baudrate The baudrate of the serial port (default = 9600).
     */
    void baud(int baudrate);


    /** Set the transmission format used by the serial port
     *
     *  @param bits The number of bits in a word (5-8; default = 8)
     *  @param parity The parity used (SerialBase::None, SerialBase::Odd, SerialBase::Even, SerialBase::Forced1, SerialBase::Forced0; default = SerialBase::None)
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

protected:


    /** Acquire exclusive access to this serial port
     */
    virtual void lock(void){

    }

    /** Release exclusive access to this serial port
     */
    virtual void unlock(void){

    }


    /** Control de la tarea asociada a la UART ESP32 */
    Thread _th{DefaultTaskPriority, DefaultTaskStackSize, NULL, "SerialBase"};
    QueueHandle_t _queue;
    Semaphore _sem{0,1};

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
