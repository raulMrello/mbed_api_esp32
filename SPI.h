/*
 * SPI.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	Portabilidad del driver SPI
 *	NOTAS:
 *		El chip ESP32 dispone de 4 canales SPI siendo SPI0 el utilizado para controla la Flash y por lo tanto no disponible,
 *		quedando libres los canales SPI1, HSPI (SPI2) y VSPI (SPI3).
 */

#ifndef SPI_H
#define SPI_H

#include "mbed_api.h"
#include "Heap.h"
#include "Mutex.h"
#include "DigitalOut.h"


class SPI {

public:

    /** Create a SPI master connected to the specified pins
     *
     *  mosi or miso can be specfied as NC if not used
     *
     *  @param mosi SPI Master Out, Slave In pin
     *  @param miso SPI Master In, Slave Out pin
     *  @param sclk SPI Clock pin
     *  @param ssel SPI chip select pin
     */
    SPI(PinName mosi, PinName miso, PinName sclk, PinName ssel=NC);

    /** Configure the data transmission format
     *
     *  @param bits Number of bits per SPI frame (4 - 16)
     *  @param mode Clock polarity and phase mode (0 - 3)
     *
     * @code
     * mode | POL PHA
     * -----+--------
     *   0  |  0   0
     *   1  |  0   1
     *   2  |  1   0
     *   3  |  1   1
     * @endcode
     */
    void format(int bits, int mode = 0);

    /** Set the spi bus clock frequency
     *
     *  @param hz SCLK frequency in hz (default = 1MHz)
     */
    void frequency(int hz = 1000000);

    /** Write to the SPI Slave and return the response
     *
     *  @param value Data to be sent to the SPI slave
     *
     *  @returns
     *    Response from the SPI slave
     */
    virtual int write(int value);

    /** Write to the SPI Slave and obtain the response
     *
     *  The total number of bytes sent and recieved will be the maximum of
     *  tx_length and rx_length. The bytes written will be padded with the
     *  value 0xff.
     *
     *  @param tx_buffer Pointer to the byte-array of data to write to the device
     *  @param tx_length Number of bytes to write, may be zero
     *  @param rx_buffer Pointer to the byte-array of data to read from the device
     *  @param rx_length Number of bytes to read, may be zero
     *  @returns
     *      The number of bytes written and read from the device. This is
     *      maximum of tx_length and rx_length.
     */
    virtual int write(const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length);

    /** Acquire exclusive access to this SPI bus
     */
    virtual void lock(void);

    /** Release exclusive access to this SPI bus
     */
    virtual void unlock(void);


    virtual ~SPI() {
    }

protected:
    static const int DefaultSpeed = 1000000;
    static const int DefaultDMABufferSize = 128;
    spi_device_handle_t _spi;
    DigitalOut* _nss;
    spi_bus_config_t _buscfg;
    spi_device_interface_config_t _devcfg;
    spi_host_device_t _spi_num;
    Mutex _mutex;
    enum ffflag{
    	FormatFlag = (1<<0),
		FrequencyFlag = (1<<1),
    };
    uint8_t _ffflags;
    char* _dma_rx_buf;
    char* _dma_tx_buf;

private:
};



#endif
