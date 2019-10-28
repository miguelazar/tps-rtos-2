/*
 * oa.h
 *
 *  Created on: Jun 14, 2019
 *      Author: nacho
 *
 *       ESTE OBJETO ACTIVO UTILIZA EL FRAMEWORK DE FREERTOS. EL MISMO UTILIZA MEMORIA DINAMICA DE FREERTOS.
 *       SE SUGIERE LA UTILIZACION DE HEAP4.
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
oa_t* OAInitialize(Function_t function, driver_t * my_driver);

// Function que destruye el oa
bool_t OADestructor(oa_t* oa_actual);

// Function que procesa un packete recibido del OA
bool_t OAProcessPacket(oa_t* oa_actual, dataStruct_t data);

/*==================[internal data definition]===============================*/



/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

//verifica que todas sean letras minusculas
bool_t check_data_minus( dataStruct_t data )
{
	uint32_t i;
	for(i=1; i < data.size; i++)
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
	for(i=1; i < data->size; i++)
	{
		data->data[i] = data->data[i] -32; // "a" - "A" en ascii es 32
	}
	return TRUE;
}

//verifica que todas sean letras mayusculas
bool_t check_data_mayus( dataStruct_t data )
{
	uint32_t i;
	for(i=1; i < data.size; i++)
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
	for(i=1; i < data->size; i++)
	{
		data->data[i] = data->data[i] + 32; // "a" - "A" en ascii es 32
	}
	return TRUE;
}

// tarea del OA
void oa_event_dispatcher( void* taskParmPtr );


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
oa_t* OAInitialize(Function_t function, driver_t * my_driver)
{
	oa_t* oa_actual = NULL;

	if (oa_actual != NULL)
	{
		return FALSE; // OA en uso. Llamar a destructor previamente
	}

	if ( (function!=MAYUSCULIZAR) && (function!=MINUSCULIZAR))
	{
		return FALSE;// el parametro function es invalido
	}


	if (my_driver == NULL)
	{
		return FALSE; // Indicar driver de salida de mensajes
	}

	oa_actual = pvPortMalloc(sizeof (oa_t)); // Pido memoria para el OA
	if (oa_actual == NULL)
	{
		return FALSE; // No se pudo asignar memoria parra crear el nuevo OA
	}

	//una vez que se asigno memoria al OA, comienzo a guardar unformacion

	//guardo la configuaracion de la funcion en el OA
	oa_actual->oa_function = function;

	//guardo la configuaracion de la funcion en el OA
	oa_actual->out_driver = my_driver;

	// inicializo la cola de recepcion de eventos del OA
	// Los eventos se envian por copia. A futuro se puede mejorar el sistema enviando los eventos por referencia y usando pool de memoria.
	oa_actual->queue_events = xQueueCreate( EVENT_QUEUE_MAX_SIZE, sizeof( dataStruct_t ) );
	if( oa_actual->queue_events == NULL )
	{
		return FALSE; // La cola fallo en inicializarse
	}

	// Crear tarea en freeRTOS
	xTaskCreate(
			oa_event_dispatcher,                     // Funcion de la tarea a ejecutar
			(const char *)"oa_event_dispatcher",     // Nombre de la tarea como String amigable para el usuario
			configMINIMAL_STACK_SIZE*2,     // Cantidad de stack de la tarea
			(void*) oa_actual,           // Parametros de tarea
			tskIDLE_PRIORITY+1,          // Prioridad de la tarea
			&(oa_actual->my_handler)   // Puntero al handler de la tarea creada en el OA
	);

	// creo la tarea del OA de acuerdo a la funcion que deba hacer

	return oa_actual;
}



/*------------------------------------------------------------------*
OAKill()
// Function que destruye el OA actual
-*------------------------------------------------------------------*/
bool_t OADestructor(oa_t* oa_actual)
{

	TaskHandle_t temp_handler;


	//elimino la cola
	vQueueDelete( oa_actual->queue_events );

	//borro la configuaracion de la funcion en el OA (solo por si las dudas)
	oa_actual->oa_function = INDEFINIDO;

	temp_handler = oa_actual->my_handler;// guardo el handler para poder eliminar la tarea

	//libero memoria del OA
	vPortFree(oa_actual);

	//limpio el puntero
	oa_actual = NULL;

	//elimino la tarea del OA
	vTaskDelete( temp_handler );

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


// tarea del OA
void oa_event_dispatcher( void* taskParmPtr )
{
	oa_t*  oa_actual = taskParmPtr;

	dataStruct_t data;

	while(1)
	{
		if( uxQueueMessagesWaiting(oa_actual->queue_events) != 0 )
		{
			if( xQueueReceive( oa_actual->queue_events,  &data , portMAX_DELAY ))
			{
				if (oa_actual->oa_function == MAYUSCULIZAR)
				{
					if (check_data_minus(data))
					{
						mayusculizar(&data);
						DriverSend(oa_actual->out_driver,data);
					}
					else
					{
						strcpy(&(data.data),"ERROR");
						data.size = (sizeof("ERROR")/sizeof(char))-1;
						DriverSend(oa_actual->out_driver,data);
					}
				}

				else if (oa_actual->oa_function == MINUSCULIZAR)
				{
					if (check_data_mayus(data))
					{
						minusculizar(&data);
						DriverSend(oa_actual->out_driver,data);
					}
					else
					{
						strcpy(&(data.data),"ERROR");
						data.size = (sizeof("ERROR")/sizeof(char))-1;
						DriverSend(oa_actual->out_driver,data);
					}

				}
				else
				{
					// Si no es ninguno de los anteriores ignora los mensajes
				}

			}

		}
		else
		{

			OADestructor(oa_actual);
		}
	}

}


/*==================[end of file]============================================*/
