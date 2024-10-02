/*
 * Ticker_HAL.h
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 *	M�dulo para definir la forma en la que la clase Ticker se conectar� con el hardware. En este caso la plataforma
 *	ESP-IDF ESP32
 *
 */

#ifndef TICKER_HAL_H
#define TICKER_HAL_H

#include "mbed_api.h"
#include "List.h"
#include "Thread.h"
#include "Queue.h"
#include "Mutex.h"


/** Base abstraction for timer interrupts
 *
 * @note Synchronization level: Interrupt safe
 * @ingroup drivers
 */
class Ticker_HAL {
public:

	/** Clave para activar la depuraci�n en tiempo de compilaci�n */
	static const bool DEBUG = true;

	/** Divisor del reloj asociado al timer hardware */
    static const int TimerDivider = 16;

    /** Escala del timer acorde al divisor establecido */
	static const int TimerScale = (APB_CLK_FREQ / TimerDivider);

	/** Grupo del timer utilizado para implementar objetos tipo Ticker */
    static const timer_group_t TimerGroup = TIMER_GROUP_0;

    /** Timer dentro del grupo selecci�nado */
    static const timer_idx_t TimerIdx = TIMER_0;

	/** Estructura de control de tickers
    */
	struct TickerData_t {
    	int32_t uuid;				/// Identificador del objeto
		Callback<void()> func;		/// Callback a invocar en los siguientes eventos
		uint64_t next_event;		/// Timestamp del siguiente evento en el que se ejecuta
		uint64_t timeout;			/// Temporizaci�n en us
	};

    /** Inicia la ejecuci�n del TimerManager
     */
    static void start();


    /** Obtiene el valor del contador actual
     * 	@return Contador actual
     */
	static uint64_t getRawCounter();


    /** Obtiene el valor del timestamp en milisegundos
     * 	@return Contador actual
     */
	static uint64_t getTimestamp();


    /** A�ade un offset al timestamp para sincronizar con relojes externos
     * 	@return Contador actual
     */
	static void setTimestampOffset(uint64_t offset) {_offset = offset; }


    /** Obtiene el valor del offset aplicado al timestamp
     * 	@return Offset
     */
	static uint64_t getTimestampOffset() { return _offset; }


    /** Instala un Ticker en la lista de objetos en ejecuci�n
     * 	@param tickData Objeto a instalar
     * 	@return Objeto instalado o NULL en caso de error
     */
    static TickerData_t* attach(TickerData_t* tickdata);


    /** Desinstala un objeto de la lista de ejecuci�n
     * 	@param tickData Objeto a desinstalar
     */
    static void detach(TickerData_t* tickdata);


    /** Rutina de atenci�n a la interrupci�n de alarmas
     */
    static void tickerISR();

protected:

    static List<TickerData_t> *_ticker_list;				/// Lista de objetos instalados
    static TickerData_t *_curr_ticker;						/// Puntero al objeto actual
    static uint64_t _offset;								/// Offset aplicado al timestamp

    /** Rutina de ejecuci�n (Thread) del TimerManager
     */
    static void task();


    /** Obtiene el siguiente ticker a cargar en el timer hardware inici�ndolo
     */
    static void executeNext();
};


#endif
