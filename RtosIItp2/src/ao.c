/*
 * driver.c
 *
 *  Created on: Jun 14, 2019
 *      Author: nacho
 *
 *      ESTE DRIVER INCLUYE EL USO DE UN POOL DE MOMORIA. POR LO TANTO NOS E USA EL MALLOC DE FREERTOS
 *      POR ESO SE PUEDE USAR EL HEAP1. EN CASO DE REQUERIRSE USAR MALLOC EN LA PLICACION SUPERIOR SE PUEDE USAR HEAP4
 */


/*==================[inclusions]=============================================*/

#include "../inc/oa.h"       // <= own header
#include "sapi.h"         // <= sAPI header


/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

//verifica que todas sean letras minusculas
bool_t check_data_minus( dataStruct_t data );
//convierte las letras minusculas en mayusculas
bool_t mayusculizar( dataStruct_t* data );

//verifica que todas sean letras mayusculas
bool_t check_data_mayus( dataStruct_t data );
//convierte las letras mayusculas en minusculas
bool_t minusculizar( dataStruct_t* data );


/*==================[external functions declaration]=========================*/

// Function que configura el OA
bool_t OAConfigFunction(oa_t* oa_actual,Function_t function);

// Function que inicializa el oa
bool_t OAInitialize(oa_t* oa_actual,Function_t function);

// Function que destruye el oa
bool_t OAKill(oa_t* oa_actual);

// Function que procesa un packete recibido del OA
bool_t OAProcessPacket(oa_t* oa_actual, dataStruct_t data);

/*==================[internal data definition]===============================*/



/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

//verifica que todas sean letras minusculas
bool_t check_data_minus( dataStruct_t data )
{
	uint32_t i;
	for(i=0; i < data.size; i++)
	{
		if(data.data[i] > 122) // mayor a "z" en ASCII
		{
			return FALSE;
		}
		if(data.data[i] < 97) // menor a "a" en ASCII
		{
			return FALSE;
		}

	}
	return TRUE;
}

bool_t mayusculizar( dataStruct_t* data )
{
	uint32_t i;
	for(i=0; i < data->size; i++)
	{
		data->data[i] = data->data[i] -32; // "a" - "A" en ascii es 32
	}
	return TRUE;
}

//verifica que todas sean letras mayusculas
bool_t check_data_mayus( dataStruct_t data )
{
	uint32_t i;
	for(i=0; i < data.size; i++)
	{
		if(data.data[i] > 90) // mayor a "Z" en ASCII
		{
			return FALSE;
		}
		if(data.data[i] < 65) // menor a "A" en ASCII
		{
			return FALSE;
		}

	}
	return TRUE;
}

bool_t minusculizar( dataStruct_t* data )
{
	uint32_t i;
	for(i=0; i < data->size; i++)
	{
		data->data[i] = data->data[i] + 32; // "a" - "A" en ascii es 32
	}
	return TRUE;
}

/*==================[external functions definition]==========================*/



/*------------------------------------------------------------------*
OAConfigFunction()
// Function que configura la funcion de OA
-*------------------------------------------------------------------*/
bool_t OAConfigFunction(oa_t* oa_actual,Function_t function)
{
	oa_actual->oa_function = function;

	return TRUE;
}

/*------------------------------------------------------------------*
OAInitialize()
// Function que inicializa el OA actual
-*------------------------------------------------------------------*/
bool_t OAInitialize(oa_t* oa_actual,Function_t function)
{
	if (oa_actual != NULL)
	{
		return FALSE; // OA en uso. Llamar a destructor previamente
	}

	oa_actual = pvPortMalloc(sizeof (oa_t)); // Pido memoria para el OA
	if (oa_actual == NULL)
	{
		return FALSE; // No se pudo asignar memoria parra crear el nuevo OA
	}

	//guardo la configuaracion de la funcion en el OA
	oa_actual->oa_function = function;

	// inicializo la cola de recepcion de eventos del OA
	oa_actual->queue_events = xQueueCreate( EVENT_QUEUE_MAX_SIZE, sizeof( dataStruct_t* ) );
	if( oa_actual->queue_events == NULL )
	{
		return FALSE; // La cola fallo en inicializarse
	}

	// creo la tarea del OA de acuerdo a la funcion que deba hacer
	if (oa_actual->oa_function == MAYUSCULIZAR)
	{
		// Creo tarea mayusculizar
	}

	else if (oa_actual->oa_function == MINUSCULIZAR)
	{
		// Creo tarea mayusculizar

	}
	else
	{
		// Si no es ninguno de los anteriores devuelve FALSE
		return FALSE;
	}

	return TRUE;
}



/*------------------------------------------------------------------*
OAKill()
// Function que destruye el OA actual
-*------------------------------------------------------------------*/
bool_t OADestructor(oa_t* oa_actual)
{

	//elimino la tarea
	vTaskDelete( oa_actual->my_handler );

	//elimino la cola
	vQueueDelete( oa_actual->queue_events );

	//borro la configuaracion de la funcion en el OA (solo por si las dudas)
	oa_actual->oa_function = INDEFINIDO;

	//libero memoria del OA
	vPortFree(oa_actual);

	//limpio el puntero
	oa_actual = NULL;

	return TRUE;
}

/*------------------------------------------------------------------*
OAProcessPacket()
// Function que envia una packete a la cola del OA para ser procesado
-*------------------------------------------------------------------*/
bool_t OAProcessPacket(oa_t* oa_actual, dataStruct_t data)
{

	if (oa_actual == NULL)
	{
		return FALSE; // OA no inicializado
	}

	if( xQueueSend( oa_actual->queue_events, &data , ( TickType_t ) 0) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}




/*==================[end of file]============================================*/
