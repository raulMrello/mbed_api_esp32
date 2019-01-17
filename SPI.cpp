/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "SPI.h"


//------------------------------------------------------------------------------------
//-- STATIC IMPLEMENTATION -----------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
spi_host_device_t pv_obj_count = HSPI_HOST;

static void isrSpiTransferComplete(spi_transaction_t *trans){

}


//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
SPI::SPI(PinName mosi, PinName miso, PinName sclk, PinName ssel) : _spi() {
    // No lock needed in the constructor
	if(pv_obj_count < HSPI_HOST || pv_obj_count > VSPI_HOST){
		return;
	}
	_spi_num = pv_obj_count;
	if(pv_obj_count == SPI_HOST){
		pv_obj_count = HSPI_HOST;
	}
	else if(pv_obj_count == HSPI_HOST){
		pv_obj_count = VSPI_HOST;
	}
	else{
		pv_obj_count = SPI_HOST;
	}

	_nss = NULL;
	if(ssel != NC){
		_nss = new DigitalOut(ssel);
		_nss->write(1);
	}

	esp_err_t ret;
	spi_bus_config_t buscfg = {
			mosi, 			//!< mosi_io_num
			miso, 			//!< miso_io_num
			sclk, 			//!< sclk_io_num
			-1,  			//!< quadwp_io_num
			-1,  			//!< quadhd_io_num
			0 				//!< max_transfer_sz
	};

	spi_device_interface_config_t devcfg = {
			0,				//!< command_bits
			0,				//!< address_bits
			0,				//!< dummy_bits
			0, 				//!< mode
			0, 				//!< duty_cycle_pos
			0, 				//!< cs_ena_pretrans
			0, 				//!< cs_ena_posttrans
			DefaultSpeed, 	//!< clock_speed_hz
			-1/*ssel*/, 				//!< spics_io_num
			0, 				//!< flags
			1, 				//!< queue_size
			0, 				//!< pre_cb
			isrSpiTransferComplete //!< post_cb
		};
	if(ssel != NC){
		devcfg.spics_io_num = ssel;
	}

	_buscfg = buscfg;
	_devcfg = devcfg;

	//Initialize the SPI bus
	ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
	MBED_ASSERT(ret==ESP_OK);
	_ffflags = 0;

	_dma_tx_buf = (char*)Heap::memAlloc(DefaultDMABufferSize);
	MBED_ASSERT(_dma_tx_buf);
	_dma_rx_buf = (char*)Heap::memAlloc(DefaultDMABufferSize);
	MBED_ASSERT(_dma_rx_buf);
}

//------------------------------------------------------------------------------------
void SPI::format(int bits, int mode) {
    lock();
    if((_ffflags & FormatFlag) == 0){
		_devcfg.mode = mode;
		// activo flag
		_ffflags |= FormatFlag;
		// si la configuración está completa, finaliza el registro del driver
		if(_ffflags == (FormatFlag | FrequencyFlag)){
			//Attach the slave to the SPI bus
			esp_err_t ret=spi_bus_add_device(_spi_num, &_devcfg, &_spi);
			MBED_ASSERT(ret==ESP_OK);
		}
		unlock();
    }
}


//------------------------------------------------------------------------------------
void SPI::frequency(int hz) {
    lock();
    if((_ffflags & FrequencyFlag) == 0){
    	_devcfg.clock_speed_hz = hz;
		// activo flag
		_ffflags |= FrequencyFlag;
		// si la configuración está completa, finaliza el registro del driver
		if(_ffflags == (FormatFlag | FrequencyFlag)){
			//Attach the slave to the SPI bus
			esp_err_t ret=spi_bus_add_device(_spi_num, &_devcfg, &_spi);
			MBED_ASSERT(ret==ESP_OK);
		}
		unlock();
    }
}


//------------------------------------------------------------------------------------
int SPI::write(int value) {
	uint8_t read;
    lock();
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       		// Zero out the transaction
    t.length	 = 8;    					// Tamaño en bits
    t.tx_buffer = &value;
    t.rx_buffer = &read;

	//Transmite y espera confirmación de transacción completada
    int result = 0;
    if(spi_device_transmit(_spi, &t) == ESP_OK){
    	result = read;
    }
	unlock();
    return result;
}


//------------------------------------------------------------------------------------
int SPI::write(const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length) {
	lock();
	int max_len = (tx_length > rx_length)? tx_length : rx_length;
	MBED_ASSERT(max_len <= DefaultDMABufferSize);

	memset(_dma_tx_buf, 0, max_len);
	memset(_dma_rx_buf, 0, max_len);
	memcpy(_dma_tx_buf, tx_buffer, tx_length);

	spi_transaction_t t;
    memset(&t, 0, sizeof(t));       // Zero out the transaction
	t.length	= max_len * 8;    	// Tamaño en bits
	t.tx_buffer	= _dma_tx_buf;    	// buffer a enviar, debe ser reservado con: pvPortMallocCaps(size, MALLOC_CAP_DMA)
	t.rx_buffer = _dma_rx_buf;		// buffer a recibir, deber ser reservado con:pvPortMallocCaps(size, MALLOC_CAP_DMA)

	//Transmite usando DMA! y espera confirmación de transacción completada
	if(_nss){
		_nss->write(0);
	}

    int result = 0;
    if(spi_device_transmit(_spi, &t) == ESP_OK){
    	result = max_len;
    }
	if(_nss){
		_nss->write(1);
	}

	if(rx_buffer){
		memcpy(rx_buffer, _dma_rx_buf, rx_length);
	}

	unlock();
    return result;
}


//------------------------------------------------------------------------------------
void SPI::lock() {
    _mutex.lock();
}


//------------------------------------------------------------------------------------
void SPI::unlock() {
    _mutex.unlock();
}




