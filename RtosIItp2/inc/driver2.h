/*
 * driver2.h
 *
 *  Created on: Jun 14, 2019
 *      Author: nacho
 *
 *      ESTE DRIVER INCLUYE EL USO DE UN POOL DE MOMORIA. POR LO TANTO NOS E USA EL MALLOC DE FREERTOS
 *      POR ESO SE PUEDE USAR EL HEAP1. EN CASO DE REQUERIRSE USAR MALLOC EN LA PLICACION SUPERIOR SE PUEDE USAR HEAP4
 */

#ifndef _DRIVER2_H_
#define _DRIVER2_H_

/*==================[inclusions]=============================================*/

// General Includes
#include "stdio.h"
#include "sapi.h"
#include <string.h>
//#include "stringManipulation.h"
#include "board.h"


// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"

// QM Pool
#include "qmpool.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/
#define dataMaxSize		124 // 128-4 para evitar desalineacion de memoria.

#define queTransmitirQueueSize		10
#define sizeOfPool		sizeof(dataStruct_t)*15
/*==================[typedef]================================================*/

// --- En esta estructura se guarda la informacion de cada paquete que llega
typedef struct{
	uint32_t size;
	char data[dataMaxSize];
}dataStruct_t;


// --- Esta estructura guarda las variables de cada instancia del driver
typedef struct {
	uartMap_t uart_type;
	uint32_t uart_speed;
	QueueHandle_t queTransmitir; //Cola de salida
	QueueHandle_t queLlego; //Cola de entrada

	TimerHandle_t rxTimer; // timer con timeout de recepcion
	TickType_t rx_callback_ms_delay; // tiempo de timeout para recepcion en ms

	TimerHandle_t txTimer; // timer con timeout de transmision
	TickType_t tx_callback_ms_delay; // tiempo de timeout para transmision en ms

	QMPool mem_pool_1; // Qmpool control struct
	uint8_t memoria_para_pool_1[sizeOfPool]; // memoria guardad para el pool
} driver_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

// Function que configura el driver de la UART
bool_t DriverConfigUART(driver_t* driver_actual,uartMap_t type,uint32_t speed);

// Function que inicializa el driver
bool_t DriverInitialize(driver_t* driver_actual);


// Function que recibe data a enviar por la UART
bool_t DriverSend(driver_t* driver_actual,dataStruct_t data);

// Function que envia data ya recivida por la UART
bool_t DriverReceive(driver_t* driver_actual,dataStruct_t * data);


/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _DRIVER2_H_ */
