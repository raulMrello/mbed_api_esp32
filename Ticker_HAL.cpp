/*
 * Ticker_HAL.cpp
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 */

#include "Ticker_HAL.h"

//------------------------------------------------------------------------------------
//---- TYPES -------------------------------------------------------------------------
//------------------------------------------------------------------------------------

List<Ticker_HAL::TickerData_t> *Ticker_HAL::_ticker_list = NULL;
Ticker_HAL::TickerData_t* Ticker_HAL::_curr_ticker = NULL;
uint64_t Ticker_HAL::_offset = 0;



//------------------------------------------------------------------------------------
//---- STATIC ------------------------------------------------------------------------
//------------------------------------------------------------------------------------


static const Ticker_HAL::TickerData_t no_more_tickdata={0, NULL, 0, 0};


//------------------------------------------------------------------------------------
void tickerAlarmISR(void *para){
	ENTER_ISR();
	timer_idx_t timerIdx = (timer_idx_t)(int)para;
	if(timerIdx == Ticker_HAL::TimerIdx){
		Ticker_HAL::tickerISR();
	}
	EXIT_ISR();
}


//------------------------------------------------------------------------------------
//---- PUBLIC ------------------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void Ticker_HAL::start(){

	esp_err_t err = ESP_OK;

	// si el timer a�n no se ha iniciado...
	if(!_ticker_list){

		MBED_ASSERT(!IS_ISR());

		// crea lista de Tickers registrados
		_ticker_list = new List<Ticker_HAL::TickerData_t>();
		// de momento no hay ninguno cargado
		_curr_ticker = NULL;

		// Inicia el contador
		timer_config_t config;
		config.divider = TimerDivider;
		config.counter_dir = TIMER_COUNT_UP;
		config.counter_en = TIMER_PAUSE;
		config.alarm_en = TIMER_ALARM_EN;
		config.intr_type = TIMER_INTR_LEVEL;
		config.auto_reload = TIMER_AUTORELOAD_DIS;
		err = timer_init(TimerGroup, TimerIdx, &config);
		MBED_ASSERT(err == ESP_OK);
		// Carga el valor inicial
		err = timer_set_counter_value(TimerGroup, TimerIdx, 0x00000000ULL);
		MBED_ASSERT(err == ESP_OK);
		// Carga el valor de comparaci�n
		err = timer_set_alarm_value(TimerGroup, TimerIdx, UINT64_MAX);
		timer_set_alarm(TimerGroup, TimerIdx, TIMER_ALARM_EN);
		MBED_ASSERT(err == ESP_OK);
		// Habilita interrupci�n
		err = timer_enable_intr(TimerGroup, TimerIdx);
		// Registra manejador de interrupci�n
		MBED_ASSERT(err == ESP_OK);
		err = timer_isr_register(TimerGroup, TimerIdx, tickerAlarmISR, (void*)TimerIdx, 0, NULL);
		// Arranca el timer
		err = timer_start(TimerGroup, TimerIdx);
		MBED_ASSERT(err == ESP_OK);

		// Arranca el hilo de ejecuci�n propio
		_offset = 0;
	}
}

//------------------------------------------------------------------------------------
void Ticker_HAL::tickerISR(){

	/* Obtiene el estado de la alarma y el valor del contador */
	uint32_t intr_status;
	switch(TimerGroup){
		case TIMER_GROUP_0:{
			intr_status = TIMERG0.int_st_timers.val;
			TIMERG0.hw_timer[TimerIdx].update = 1;
			uint64_t timer_counter_value = ((uint64_t) TIMERG0.hw_timer[TimerIdx].cnt_high) << 32 | TIMERG0.hw_timer[TimerIdx].cnt_low;

			/* Clear the interrupt and update the alarm time */
			if ((intr_status & BIT(TimerIdx)) && TimerIdx == TIMER_0) {
				TIMERG0.int_clr_timers.t0 = 1;
			}
			else if ((intr_status & BIT(TimerIdx)) && TimerIdx == TIMER_1) {
				TIMERG0.int_clr_timers.t1 = 1;
			}

			// en caso de que no haya asignado ning�n ticker, lo pausa y lo pone a 0 y marca una alarma en el infinito
			// por �ltimo lo inicia
			if(!_curr_ticker){
				_curr_ticker = (TickerData_t*)&no_more_tickdata;
				// lo pausa
				TIMERG0.hw_timer[TimerIdx].config.enable = 0;
				// establece cuenta a 0
				TIMERG0.hw_timer[TimerIdx].load_high = (uint32_t)0;
				TIMERG0.hw_timer[TimerIdx].load_low = (uint32_t)0;
				TIMERG0.hw_timer[TimerIdx].reload = 1;
				// set alarm_value
				TIMERG0.hw_timer[TimerIdx].alarm_high = (uint32_t) 0xFFFFFFF;
				TIMERG0.hw_timer[TimerIdx].alarm_low = (uint32_t) 0xFFFFFFF;
			}
			// si no hay m�s tickers, marca alarma en infinito
			else if(_curr_ticker == (TickerData_t*)&no_more_tickdata){
				_curr_ticker = (TickerData_t*)&no_more_tickdata;
				// set alarm_value
				TIMERG0.hw_timer[TimerIdx].alarm_high = (uint32_t) 0xFFFFFFF;
				TIMERG0.hw_timer[TimerIdx].alarm_low = (uint32_t) 0xFFFFFFF;
			}
			// en otro caso, procesa evento de ticker a nivel de ISR
			else{
				_curr_ticker->func.call();
				executeNext();
			}
			// set alarm
			TIMERG0.hw_timer[TimerIdx].config.alarm_en = TIMER_ALARM_EN;
			// arranca de nuevo
			TIMERG0.hw_timer[TimerIdx].config.enable = 1;

			return;
		}

		case TIMER_GROUP_1:{
			intr_status = TIMERG1.int_st_timers.val;
			TIMERG1.hw_timer[TimerIdx].update = 1;
			uint64_t timer_counter_value = ((uint64_t) TIMERG1.hw_timer[TimerIdx].cnt_high) << 32 | TIMERG1.hw_timer[TimerIdx].cnt_low;

			/* Clear the interrupt and update the alarm time */
			if ((intr_status & BIT(TimerIdx)) && TimerIdx == TIMER_0) {
				TIMERG1.int_clr_timers.t0 = 1;
			}
			else if ((intr_status & BIT(TimerIdx)) && TimerIdx == TIMER_1) {
				TIMERG1.int_clr_timers.t1 = 1;
			}

			// en caso de que no haya asignado ning�n ticker, lo pausa y lo pone a 0 dejando las interrupciones desactivadas
			if(!_curr_ticker){
				_curr_ticker = (TickerData_t*)&no_more_tickdata;
				// lo pausa
				TIMERG1.hw_timer[TimerIdx].config.enable = 0;
				// establece cuenta a 0
				TIMERG1.hw_timer[TimerIdx].load_high = (uint32_t)0;
				TIMERG1.hw_timer[TimerIdx].load_low = (uint32_t)0;
				TIMERG1.hw_timer[TimerIdx].reload = 1;
				// set alarm_value
				TIMERG1.hw_timer[TimerIdx].alarm_high = (uint32_t) 0xFFFFFFF;
				TIMERG1.hw_timer[TimerIdx].alarm_low = (uint32_t) 0xFFFFFFF;
			}
			// si no hay m�s tickers, marca alarma en infinito
			else if(_curr_ticker == (TickerData_t*)&no_more_tickdata){
				_curr_ticker = (TickerData_t*)&no_more_tickdata;
				// set alarm_value
				TIMERG1.hw_timer[TimerIdx].alarm_high = (uint32_t) 0xFFFFFFF;
				TIMERG1.hw_timer[TimerIdx].alarm_low = (uint32_t) 0xFFFFFFF;
			}
			// en otro caso, procesa evento de ticker a nivel de ISR
			else{
				_curr_ticker->func.call();
				executeNext();
			}
			// set alarm
			TIMERG1.hw_timer[TimerIdx].config.alarm_en = TIMER_ALARM_EN;
			// arranca de nuevo
			TIMERG1.hw_timer[TimerIdx].config.enable = 1;
			return;
		}
	}
}


//------------------------------------------------------------------------------------
uint64_t Ticker_HAL::getRawCounter(){
	uint64_t now = 0;
	esp_err_t err = timer_get_counter_value(TimerGroup, TimerIdx, &now);
	MBED_ASSERT(err == ESP_OK);
	return now;
}


//------------------------------------------------------------------------------------
uint64_t Ticker_HAL::getTimestamp(){
	uint64_t now = 0;
	esp_err_t err = timer_get_counter_value(TimerGroup, TimerIdx, &now);
	MBED_ASSERT(err == ESP_OK);
	uint64_t mulfactor = 1000000;
	return (_offset + ((now * mulfactor)/Ticker_HAL::TimerScale));
}


//------------------------------------------------------------------------------------
Ticker_HAL::TickerData_t* Ticker_HAL::attach(Ticker_HAL::TickerData_t* tickdata){
	timg_dev_t* tim;
	switch(TimerGroup){
		case TIMER_GROUP_0:{
			tim = &TIMERG0;
			break;
		}
		case TIMER_GROUP_1:{
			tim = &TIMERG1;
			break;
		}
	}

	// desactiva la alarma
	if(!IS_ISR()){
		tim->hw_timer[TimerIdx].config.alarm_en = TIMER_ALARM_DIS;
	}
	_ticker_list->addItem(tickdata);
	executeNext();
	// set alarm
	if(!IS_ISR()){
		tim->hw_timer[TimerIdx].config.alarm_en = TIMER_ALARM_EN;
	}
	return tickdata;
}


//------------------------------------------------------------------------------------
void Ticker_HAL::detach(Ticker_HAL::TickerData_t* tickdata){

	// Si es la primera vez que se arranca el ticker, no hace falta desinstalarlo
	if(tickdata->timeout == 0){
		return;
	}

	timg_dev_t* tim;
	switch(TimerGroup){
		case TIMER_GROUP_0:{
			tim = &TIMERG0;
			break;
		}
		case TIMER_GROUP_1:{
			tim = &TIMERG1;
			break;
		}
	}
	// desactiva la alarma
	if(!IS_ISR()){
		tim->hw_timer[TimerIdx].config.alarm_en = TIMER_ALARM_DIS;
	}
	_ticker_list->removeItem(tickdata);
	executeNext();
	// set alarm
	if(!IS_ISR()){
		tim->hw_timer[TimerIdx].config.alarm_en = TIMER_ALARM_EN;
	}

}


//------------------------------------------------------------------------------------
//---- PROTECTED ---------------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void Ticker_HAL::executeNext(){
	// busca el m�s prioritario
	timg_dev_t* tim;
	switch(TimerGroup){
		case TIMER_GROUP_0:{
			tim = &TIMERG0;
			break;
		}
		case TIMER_GROUP_1:{
			tim = &TIMERG1;
			break;
		}
	}

	// busca el pr�ximo elemento
	TickerData_t *tnext = _ticker_list->getFirstItem();
	// si no hay m�s elementos
	if(!tnext){
		_curr_ticker = (TickerData_t *)&no_more_tickdata;
		// set alarm_value infinite
		tim->hw_timer[TimerIdx].alarm_high = (uint32_t) 0xFFFFFFFF;
		tim->hw_timer[TimerIdx].alarm_low = (uint32_t) 0xFFFFFFFF;
		return;
	}
	TickerData_t *tdata = _ticker_list->getNextItem();
	while(tdata){
		// chequea si es m�s prioritario
		if(tdata->next_event < tnext->next_event){
			tnext = tdata;
		}
		tdata = _ticker_list->getNextItem();
	}
	// set alarm_value
	_curr_ticker = tnext;
	tim->hw_timer[TimerIdx].alarm_high = (uint32_t) (_curr_ticker->next_event >> 32);
	tim->hw_timer[TimerIdx].alarm_low = (uint32_t) _curr_ticker->next_event;
}

