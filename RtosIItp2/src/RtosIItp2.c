/* Copyright 2017-2018, Eric Pernia
 * All rights reserved.
 *
 * This file is part of sAPI Library.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*==================[inclusiones]============================================*/
#include "RtosIItp2.h"

/*==================[definiciones y macros]==================================*/

/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de variables globales]====================*/

/*==================[declaraciones de funciones internas]====================*/
void APP_Task( void* taskParmPtr );

//devuelve que funcion viene en un paquete
Function_t get_function_from_packet( dataStruct_t data );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void)
{
	// ---------- CONFIGURACIONES ------------------------------
	// Inicializar y configurar la plataforma
	boardConfig();

	// Led para dar señal de vida
	gpioWrite( LED3, ON );

	//Instancio la UART junto con el driver
	static driver_t my_driver;
	DriverConfigUART(&my_driver,UART_USB,115200); // configuro el driver
	DriverInitialize( &my_driver); // inicializo el driver

	// Crear tarea en freeRTOS
	xTaskCreate(
			APP_Task,                     // Funcion de la tarea a ejecutar
			(const char *)"APP_Task",     // Nombre de la tarea como String amigable para el usuario
			configMINIMAL_STACK_SIZE*3,     // Cantidad de stack de la tarea
			(void*) &my_driver,           // Parametros de tarea
			tskIDLE_PRIORITY+1,          // Prioridad de la tarea
			0                            // Puntero a la tarea creada en el sistema
	);


	//FreeRTOS Scheduler Start
	vTaskStartScheduler();



	while( TRUE )
	{

	}
	return 0;
}


// Implementacion de funcion de la tarea
void APP_Task( void* taskParmPtr )
{
	// ---------- CONFIGURACIONES ------------------------------
	portTickType xLastWakeTime;
	portTickType xPeriodo =  1000 / portTICK_RATE_MS;

	bool_t global_flag = FALSE;

	dataStruct_t test_data;

	//envio mensaje de inicio
	DriverReceive(taskParmPtr,&test_data);
	strcpy(&(test_data.data),"Sistema inicializado");
	test_data.size = sizeof("Sistema inicializado")/sizeof(char);
	DriverSend(taskParmPtr,test_data);

	oa_t *instancia_generic = NULL;
	oa_t *instancia_mayus = NULL; //para mayusculizar
	oa_t *instancia_minus = NULL; //para minusculizar

	//Function_t funcion_recibida;


	// ---------- REPETIR POR SIEMPRE --------------------------
	while(TRUE)
	{
		// Intercambia el estado del LEDB
		static tick_t start_delay = 50;

		if (global_flag == TRUE)
		{
			global_flag= FALSE;
			gpioWrite( LEDB, ON );

		}
		else
		{
			global_flag = TRUE;
			gpioWrite( LEDB, OFF );
		}


		if (DriverReceive(taskParmPtr,&test_data))
		{
			if (get_function_from_packet( test_data ) == MAYUSCULIZAR)
			{
				if(instancia_mayus == NULL)
				{
					instancia_generic = OAInitialize( MAYUSCULIZAR, taskParmPtr);
				}
				//instancia_generic = instancia_mayus;

			}
			if (get_function_from_packet( test_data ) == MINUSCULIZAR)
			{
				if(instancia_minus == NULL)
				{
					instancia_generic = OAInitialize(MINUSCULIZAR, taskParmPtr);
				}
				//instancia_generic = instancia_minus;
			}
			//envio a procesar el paquete al OA
			OAProcessPacket( instancia_generic  , test_data);
		}

		vTaskDelay( start_delay / portTICK_RATE_MS );

	}
}


Function_t get_function_from_packet( dataStruct_t data )
{
	if(data.data[0] == MAYUSCULIZAR_FUNC_ID) // operacion mayusculizar
	{
		return MAYUSCULIZAR;
	}
	if(data.data[0] == MINUSCULIZAR_FUNC_ID) // operacion mayusculizar
	{
		return MINUSCULIZAR;
	}
	else
	{
		return MINUSCULIZAR;
	}
}


/*==================[fin del archivo]========================================*/
