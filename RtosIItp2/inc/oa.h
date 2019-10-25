/*
 * oa.h
 *
 *  Created on: Jun 14, 2019
 *      Author: nacho
 *
 *      ESTE DRIVER INCLUYE EL USO DE UN POOL DE MOMORIA. POR LO TANTO NOS E USA EL MALLOC DE FREERTOS
 *      POR ESO SE PUEDE USAR EL HEAP1. EN CASO DE REQUERIRSE USAR MALLOC EN LA PLICACION SUPERIOR SE PUEDE USAR HEAP4
 */

#ifndef _OA_H_
#define _OA_H_

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

#include "driver.h"


/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/
#define EVENT_QUEUE_MAX_SIZE		5 // queuee MAX size

/*==================[typedef]================================================*/

typedef enum {MAYUSCULIZAR, MINUSCULIZAR, INDEFINIDO} Function_t;

// --- Esta estructura guarda las variables de cada instancia del objeto activo
typedef struct
{
	Function_t oa_function;
	QueueHandle_t queue_events; //Cola de eventos de entrada
	TaskHandle_t my_handler; //handler de la tarea
} oa_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

// Function que configura el OA
bool_t OAConfigFunction(oa_t* oa_actual,Function_t function);

// Function que inicializa el OA
bool_t OAInitialize(oa_t* oa_actual,Function_t function);

// Function que destruye el OA
bool_t OADestructor(oa_t* oa_actual);

// Function que procesa un packete recibido del OA
bool_t OAProcessPacket(oa_t* oa_actual, dataStruct_t data);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _OA_H_ */
