/*
 * mbed-os.h
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 *	mbed-os incluye diversas definiciones generales para realizar un porting de la API de MBED-OS v5.x a otras plataformas,
 *	en este caso a FREERTOS + ESP32.
 *
 */
#ifndef MBED_API_H
#define MBED_API_H


//------------------------------------------------------------------------------------
//--- LIBRER�AS EST�NDAR -------------------------------------------------------------
//------------------------------------------------------------------------------------

#include <math.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>



//------------------------------------------------------------------------------------
//--- LIBRER�AS ESP32 ESP-IDF SDK ----------------------------------------------------
//------------------------------------------------------------------------------------

/// ESP32 dependencies
#include <sys/cdefs.h>
#include <sdkconfig.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <tcpip_adapter.h>
#include <esp_event_loop.h>
#include "esp_spi_flash.h"
#include <esp_types.h>
#include <esp_log.h>
#include <soc/timer_group_struct.h>
#include <driver/periph_ctrl.h>
#include <driver/timer.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/dac.h>
#include <driver/i2c.h>
#include <driver/spi_master.h>
#include <driver/uart.h>
#include <driver/mcpwm.h>
#include <driver/rmt.h>
#include <soc/mcpwm_reg.h>
#include <soc/mcpwm_struct.h>
#include <nvs_flash.h>
#include <nvs.h>
#include "esp_task_wdt.h"
#include "esp_ota_ops.h"


//------------------------------------------------------------------------------------
//--- LIBRER�AS FREERTOS -------------------------------------------------------------
//------------------------------------------------------------------------------------


#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/ringbuf.h"


//------------------------------------------------------------------------------------
//--- MISC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------

/** Tipo definido para utilizar PinNames como uint32_t y evitar errores en la serializaci�n cuando
 *  sizeof(PinName) es diferente seg�n el compilador y/o la plataforma */
typedef uint32_t PinName32;


//------------------------------------------------------------------------------------
//--- DEPENDENCIAS MBED-OS -----------------------------------------------------------
//------------------------------------------------------------------------------------

/// Status code values returned by CMSIS-RTOS functions.
typedef enum {
  osOK                      =  0,         ///< Operation completed successfully.
  osError                   = -1,         ///< Unspecified RTOS error: run-time error but no other error message fits.
  osErrorTimeout            = -2,         ///< Operation not completed within the timeout period.
  osErrorResource           = -3,         ///< Resource not available.
  osErrorParameter          = -4,         ///< Parameter error.
  osErrorNoMemory           = -5,         ///< System is out of memory: it was impossible to allocate or reserve memory for the operation.
  osErrorISR                = -6,         ///< Not allowed in ISR context: the function cannot be called from interrupt service routines.
  osStatusReserved          = 0x7FFFFFFF  ///< Prevents enum down-size compiler optimization.
} osStatus_t;

/// Tipos de configuraci�n de los pines gpio
typedef enum {
    PullNone          = 0,
    PullUp            = 1,
    PullDown          = 2,
    OpenDrainPullUp   = 3,
    OpenDrainNoPull   = 4,
    OpenDrainPullDown = 5,
    PushPullNoPull    = PullNone,
    PushPullPullUp    = PullUp,
    PushPullPullDown  = PullDown,
    OpenDrain         = OpenDrainPullUp,
    PullDefault       = PullNone
} PinMode;

/** GPIO IRQ events
 */
typedef enum {
    IRQ_NONE,
    IRQ_RISE,
    IRQ_FALL
} gpio_irq_event;

/// mbed typedefs
typedef gpio_num_t				PinName;
#define NC						(PinName)0xFFFFFFFF
typedef int32_t                 osStatus;
#define osEventSignal           (0x08)
#define osEventMessage          (0x10)
#define osEventMail             (0x20)
#define osEventTimeout          (0x40)
#define osOK					(0)
#define osErrorOS               (-1)
#define osErrorTimeoutResource  (-2)
#define osErrorISRRecursive     (-126)
#define osErrorValue            (-127)
#define osErrorPriority         (-128)

#define osDelay					Thread::wait

typedef void* osMailQId;
typedef void* osMessageQId;

/// Event structure contains detailed information about an event.
typedef struct {
	osStatus          status;   	///< status code: event or error information
	union {
		uint32_t      v;   			///< message as 32-bit value
		void          *p;   		///< message or mail as void pointer
		int32_t       signals;  	///< signal flags
	} value;                    	///< event value
	union {
		osMailQId     mail_id;  	///< mail id obtained by \ref osMailCreate
		osMessageQId  message_id;	///< message id obtained by \ref osMessageCreate
	} def;                          ///< event definition
} osEvent;



//------------------------------------------------------------------------------------
//--- ADAPTACI�N MBED-OS -> FREERTOS -------------------------------------------------
//------------------------------------------------------------------------------------


typedef TaskHandle_t osThreadId;			/// Conversi�n threadId de mbed-os a freertos
typedef UBaseType_t	osPriority;				/// Conversi�n de priority de mbed-os a freertos
typedef osPriority osPriority_t;
typedef TimerHandle_t osTimerId;			/// Conversi�n timerId de mbed-os a freertos
typedef UBaseType_t os_timer_type;			/// Conversi�n timer_type de mbed-os a freertos
typedef EventGroupHandle_t osEventFlagsId;	/// Conversi�n eventflagid de mbed-os a freertos
typedef SemaphoreHandle_t osSemaphoreId;	/// Conversi�n semaphoreid de mbed-os a freertos

#define osFlagsError				0x80000000U
#define osFlagsWaitAny				0x00000000U
#define OS_STACK_SIZE				2048
#define osWaitForever				portMAX_DELAY
#define osThreadGetId()				xTaskGetCurrentTaskHandle()
#define osSignalSet(tid,ev)			xTaskNotify(tid, ev, eSetBits)
#define osPriorityIdle				0
#define osPriorityCritical			(configMAX_PRIORITIES - 1)
#define osPriorityNormal			(configMAX_PRIORITIES / 2)
#define osPriorityAboveNormal1  	osPriorityNormal+1       ///< Priority: above normal + 1
#define osPriorityAboveNormal2  	osPriorityNormal+2       ///< Priority: above normal + 2
#define osPriorityAboveNormal3  	osPriorityNormal+3       ///< Priority: above normal + 3
#define osPriorityAboveNormal4  	osPriorityNormal+4       ///< Priority: above normal + 4
#define osPriorityAboveNormal5  	osPriorityNormal+5       ///< Priority: above normal + 5
#define osPriorityAboveNormal6  	osPriorityNormal+6       ///< Priority: above normal + 6
#define osPriorityAboveNormal7  	osPriorityNormal+7       ///< Priority: above normal + 7
#define osPriorityAboveNormal(x)	(osPriorityNormal + (x))
#define osPriorityBelowNormal(x)	(osPriorityNormal - (x))
#define osTimerPeriodic				pdTRUE
#define osTimerOnce					pdFALSE

#define MBED_MILLIS_TO_TICK(t)		(((t) == osWaitForever)? portMAX_DELAY : pdMS_TO_TICKS(t))


/// ISR nesting global control
bool IS_ISR();
void ENTER_ISR();
void EXIT_ISR();
int GET_ISR_NESTING();

/// wait common functions
void wait(float s);
void wait_ms(int ms);
void wait_us(int us);
#define HAL_GetTick()		Ticker_HAL::getTimestamp()


//------------------------------------------------------------------------------------
//--- LIBRER�AS MBED-OS  -------------------------------------------------------------
//------------------------------------------------------------------------------------

#include "Callback.h"		/// Cabecera para la implementaci�n de callbacks


//------------------------------------------------------------------------------------
//--- UTILIDADES DE DEPURACI�N/LOGGING -----------------------------------------------
//------------------------------------------------------------------------------------

#define DEBUG_SET_LEVEL(level)	esp_log_level_set("*", level);        // set all components to level

/** Macro para imprimir trazas de depuraci�n via Syslog. Requiere definir 'mbed_syslog' */
extern void (*syslog_print)(const char* level, const char* tag, const char* format, ...);
extern bool _busy;

/** Macro para trazas ERROR */
#define DEBUG_TRACE_E(expr, tag, format, ...)			\
if(expr && !_busy){										\
	_busy = true;										\
	ESP_LOGE(tag, format, ##__VA_ARGS__);				\
	if(syslog_print){									\
		syslog_print("E", tag, format, ##__VA_ARGS__);	\
	}													\
	_busy = false;										\
}

/** Macro para trazas WARNING */
#define DEBUG_TRACE_W(expr, tag, format, ...)			\
if(expr && !_busy){										\
	_busy = true;										\
	ESP_LOGW(tag, format, ##__VA_ARGS__);				\
	if(syslog_print){									\
		syslog_print("W", tag, format, ##__VA_ARGS__);	\
	}													\
	_busy = false;										\
}

/** Macro para trazas INFO */
#define DEBUG_TRACE_I(expr, tag, format, ...)			\
if(expr && !_busy){										\
	_busy = true;										\
	ESP_LOGI(tag, format, ##__VA_ARGS__);				\
	if(syslog_print){									\
		syslog_print("I", tag, format, ##__VA_ARGS__);	\
	}													\
	_busy = false;										\
}

/** Macro para trazas DEBUG */
#define DEBUG_TRACE_D(expr, tag, format, ...)			\
if(expr && !_busy){										\
	_busy = true;										\
	ESP_LOGD(tag, format, ##__VA_ARGS__);				\
	if(syslog_print){									\
		syslog_print("D", tag, format, ##__VA_ARGS__);	\
	}													\
	_busy = false;										\
}
/** Macro para trazas VERBOSE */
#define DEBUG_TRACE_V(expr, tag, format, ...)			\
if(expr && !_busy){										\
	_busy = true;										\
	ESP_LOGV(tag, format, ##__VA_ARGS__);				\
	if(syslog_print){									\
		syslog_print("V", tag, format, ##__VA_ARGS__);	\
	}													\
	_busy = false;										\
}

/** Macro para trazas locales ERROR */
#define DEBUG_LOCAL_TRACE_E(expr, tag, format, ...)		\
if(expr && !_busy){										\
	_busy = true;										\
	ESP_LOGE(tag, format, ##__VA_ARGS__);				\
	_busy = false;										\
}

/** Macro para trazas locales WARNING */
#define DEBUG_LOCAL_TRACE_W(expr, tag, format, ...)		\
if(expr && !_busy){										\
	_busy = true;										\
	ESP_LOGW(tag, format, ##__VA_ARGS__);				\
	_busy = false;										\
}


/** Macro para trazas locales INFO */
#define DEBUG_LOCAL_TRACE_I(expr, tag, format, ...)		\
if(expr && !_busy){										\
	_busy = true;										\
	ESP_LOGI(tag, format, ##__VA_ARGS__);				\
	_busy = false;										\
}


/** Macro para trazas locales DEBUG */
#define DEBUG_LOCAL_TRACE_D(expr, tag, format, ...)		\
if(expr && !_busy){										\
	_busy = true;										\
	ESP_LOGD(tag, format, ##__VA_ARGS__);				\
	_busy = false;										\
}

/** Macro para trazas locales VERBOSE */
#define DEBUG_LOCAL_TRACE_V(expr, tag, format, ...)		\
if(expr && !_busy){										\
	_busy = true;										\
	ESP_LOGV(tag, format, ##__VA_ARGS__);				\
	_busy = false;										\
}


//------------------------------------------------------------------------------------
//--- UTILIDADES DE PROP�SITO GENERAL ------------------------------------------------
//------------------------------------------------------------------------------------


/** Utilidad para conocer el tama�o en n�mero de elementos de cualquier tipo de array */
template <typename T, size_t N> inline size_t SizeOfArray(const T(&)[N]) { return N; }
template <typename T, size_t N> inline size_t MemSizeOfArray(const T(&)[N]) { return sizeof(T) * N; }
template <typename T, size_t N, size_t M> inline size_t MemSizeOfArray2(const T(&)[N][M]) { return sizeof(T) * N * M; }


using namespace std;

#endif
