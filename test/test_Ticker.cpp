/* test_Ticker

   Unit test of MBED-API Ticker ESP32 porting
*/

//------------------------------------------------------------------------------------
//-- REQUIRED HEADERS & COMPONENTS FOR TESTING ---------------------------------------
//------------------------------------------------------------------------------------

#include "mbed.h"
#include "unity.h"
#include "AppConfig.h"
static const char* _MODULE_ = "[TEST_Ticker]....";
#define _EXPR_	(true)
Ticker* tick;
static bool _prerequisites_done=false;
static void executePrerequisites(){
	if(!_prerequisites_done){
		_prerequisites_done=true;
		esp_log_level_set(_MODULE_, ESP_LOG_DEBUG);
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "Iniciando Ticker_HAL");
		Ticker_HAL::start();
	}
}

static uint64_t curr_time = 0;
static uint64_t elapsed_time = 0;
static bool event_done=false;
static void ticker_callback(){
	elapsed_time = Ticker_HAL::getTimestamp();
	tick->detach();
	event_done=true;
}


//---------------------------------------------------------------------------
TEST_CASE("TEST_Ticker_0.01ms", "[mbed_api_esp32]") {
    executePrerequisites();

    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Creando Ticker");
    tick = new Ticker();
    TEST_ASSERT_NOT_NULL(tick);

    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Iniciando 10us Ticker");
    Thread::wait(100);
    event_done=false;
    curr_time = Ticker_HAL::getTimestamp();
    tick->attach_us(callback(&ticker_callback), 10);
    while(!event_done){
    	Thread::wait(10);
    }
    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Tiempo transcurrido %fus", (double)(elapsed_time - curr_time));
    delete(tick);
}


//---------------------------------------------------------------------------
TEST_CASE("TEST_Ticker_0.1ms", "[mbed_api_esp32]") {
    executePrerequisites();

    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Creando Ticker");
    tick = new Ticker();
    TEST_ASSERT_NOT_NULL(tick);

    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Iniciando 100us Ticker");
    Thread::wait(100);
    event_done=false;
    curr_time = Ticker_HAL::getTimestamp();
    tick->attach_us(callback(&ticker_callback), 100);
    while(!event_done){
    	Thread::wait(10);
    }
    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Tiempo transcurrido %fus", (double)(elapsed_time - curr_time));
    delete(tick);
}
//---------------------------------------------------------------------------
TEST_CASE("TEST_Ticker_1ms", "[mbed_api_esp32]") {
    executePrerequisites();

    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Creando Ticker");
    tick = new Ticker();
    TEST_ASSERT_NOT_NULL(tick);

    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Iniciando 1ms Ticker");
    Thread::wait(100);
    event_done=false;
    curr_time = Ticker_HAL::getTimestamp();
    tick->attach_us(callback(&ticker_callback), 1000);
    while(!event_done){
    	Thread::wait(10);
    }
    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Tiempo transcurrido %fus", (double)(elapsed_time - curr_time));
    delete(tick);
}


//---------------------------------------------------------------------------
TEST_CASE("TEST_Ticker_1sec", "[mbed_api_esp32]") {
    executePrerequisites();

    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Creando Ticker");
    tick = new Ticker();
    TEST_ASSERT_NOT_NULL(tick);

    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Iniciando 1sec Ticker");
    Thread::wait(100);
    event_done=false;
    curr_time = Ticker_HAL::getTimestamp();
    tick->attach_us(callback(&ticker_callback), 1000000);
    while(!event_done){
    	Thread::wait(10);
    }
    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Tiempo transcurrido %fus", (double)(elapsed_time - curr_time));
    delete(tick);
}


//---------------------------------------------------------------------------
TEST_CASE("TEST_Ticker_10sec", "[mbed_api_esp32]") {
    executePrerequisites();

    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Creando Ticker");
    tick = new Ticker();
    TEST_ASSERT_NOT_NULL(tick);

    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Iniciando 10sec Ticker");
    Thread::wait(100);
    event_done=false;
    curr_time = Ticker_HAL::getTimestamp();
    tick->attach_us(callback(&ticker_callback), 10000000);
    while(!event_done){
    	Thread::wait(10);
    }
    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Tiempo transcurrido %fus", (double)(elapsed_time - curr_time));
    delete(tick);
}

