/*
 * I2C.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	Portabilidad del driver I2C
 *	NOTAS:
 *		Puesto que el ESP32 tiene una matriz gpio en la que casi cualquier pin se puede utilizar como SDA y SCL y que
 *		únicamente cuenta con dos puertos I2C0 e I2C1, la forma de asignar el puerto será por orden. Así la primera llamada a
 *		I2C::I2C() utilizará el canal I2C0 si está libre, y si no lo está, usará I12C1. En caso de que ambos estén ocupados,
 *		no inicializará nada.
 */

#ifndef I2C_H
#define I2C_H

#include "mbed_api.h"
#include "Mutex.h"


class I2C {

public:
    enum RxStatus {
        NoData,
        MasterGeneralCall,
        MasterWrite,
        MasterRead
    };

    enum Acknowledge {
        NoACK = 0,
        ACK   = 1
    };

    /** Create an I2C Master interface, connected to the specified pins
     *
     *  @param sda I2C data line pin
     *  @param scl I2C clock line pin
     */
    I2C(PinName sda, PinName scl, bool enable_debug = false);

    /** Set the frequency of the I2C interface
     *
     *  @param hz The bus frequency in hertz
     */
    void frequency(int hz);

    /** Read from an I2C slave
     *
     * Performs a complete read transaction. The bottom bit of
     * the address is forced to 1 to indicate a read.
     *
     *  @param address 8-bit I2C slave address [ addr | 1 ]
     *  @param data Pointer to the byte-array to read data in to
     *  @param length Number of bytes to read
     *  @param repeated Repeated start, true - don't send stop at end
     *
     *  @returns
     *       0 on success (ack),
     *   non-0 on failure (nack)
     */
    int read(int address, char *data, int length, bool repeated = false);

    /** Read a single byte from the I2C bus
     *
     *  @param ack indicates if the byte is to be acknowledged (1 = acknowledge)
     *
     *  @returns
     *    the byte read
     */
    int read(int ack);

    /** Write to an I2C slave
     *
     * Performs a complete write transaction. The bottom bit of
     * the address is forced to 0 to indicate a write.
     *
     *  @param address 8-bit I2C slave address [ addr | 0 ]
     *  @param data Pointer to the byte-array data to send
     *  @param length Number of bytes to send
     *  @param repeated Repeated start, true - do not send stop at end
     *
     *  @returns
     *       0 on success (ack),
     *   non-0 on failure (nack)
     */
    int write(int address, const char *data, int length, bool repeated = false);

    /** Write single byte out on the I2C bus
     *
     *  @param data data to write out on bus
     *
     *  @returns
     *    '0' - NAK was received
     *    '1' - ACK was received,
     *    '2' - timeout
     */
    int write(int data);

    /** Creates a start condition on the I2C bus
     */

    void start(void);

    /** Creates a stop condition on the I2C bus
     */
    void stop(void);

    /** Acquire exclusive access to this I2C bus
     */
    virtual void lock(void);

    /** Release exclusive access to this I2C bus
     */
    virtual void unlock(void);

    virtual ~I2C() {
        // Do nothing
    }


protected:
    static const int DefaultSpeed = 1000000;	///!< Velocidad por defecto del bus 1MHz

    i2c_config_t _i2c;			///!< Estructura de configuración del periférico
    i2c_port_t _i2c_num;		///!< Canal I2C utilizado
    uint32_t _timeout_ns;		///!< Timeout para operaciones cmd_link en espera (en ns)
    Mutex _mutex;				///!< Mutex de acceso al driver
    bool _debug;				///!< Flag para controlar el nivel de depuración

};



#endif
