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

#include "SerialBase.h"



class RawSerial : public SerialBase {

public:
    /** Create a RawSerial port, connected to the specified transmit and receive pins, with the specified baud.
     *
     *  @param tx Transmit pin
     *  @param rx Receive pin
     *  @param baud The baud rate of the serial port (optional, defaults to MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE)
     *  @param uart_num Incluyo este parámetro para seleccionar la UART
     *
     *  @note
     *    Either tx or rx may be specified as NC if unused
     */
    RawSerial(PinName tx, PinName rx, int baud, uart_port_t uart_num = UART_NUM_1);

    virtual ~RawSerial();

    /** Write a char to the serial port
     *
     * @param c The char to write
     *
     * @returns The written char or -1 if an error occurred
     */
    int putChar(int c);

    /** Read a char from the serial port
     *
     * @returns The char read from the serial port
     */
    int getChar();

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

};


#endif
