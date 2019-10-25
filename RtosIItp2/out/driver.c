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

#include "../inc/driver.h"       // <= own header
#include "sapi.h"         // <= sAPI header


/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/* Callbacks - Declaraciones */

void uartUsbReceiveCallback( void * driver_actual );
void uartUsbSendCallback( void * driver_actual );

void uart232ReceiveCallback( void * driver_actual );
void uart232SendCallback( void * driver_actual );

void RxUsbTimerCallback( TimerHandle_t xTimer );

void Rx232TimerCallback( TimerHandle_t xTimer );

/*==================[external functions declaration]=========================*/

// Function que envia data a enviar por la UART
bool_t DriverSend(driver_t* driver_actual,dataStruct_t data);

// Function que envia data a enviar por la UART
bool_t DriverReceive(driver_t* driver_actual,dataStruct_t * data);


/*==================[internal data definition]===============================*/

//Variables utilizadas para la transmisión por UART
uint8_t dataReceivedFromUartUsb = 0;
uint8_t dataToSendToUartUsb =0;
volatile bool_t  dataToSendToUartUsbPending= FALSE;
volatile bool_t  dataReceivedFromUartUsbPending= FALSE;
//char strData[100]="";
//char strDataFrameOut[120]="";

dataStruct_t* dataRecieveUsb;// frame para guardar lo que va llegando antes de ponerlo en la cola
dataStruct_t* dataSendUsb; // frame para guardar lo que voy enviando luego de sacarlo de la cola

dataStruct_t* dataRecieve232;// frame para guardar lo que va llegando antes de ponerlo en la cola
dataStruct_t* dataSend232; // frame para guardar lo que voy enviando luego de sacarlo de la cola


//-- Semaforos binarios
SemaphoreHandle_t SemBinPacketReceivedUsb = NULL;
SemaphoreHandle_t SemBinPacketReceived232 = NULL;

SemaphoreHandle_t SemBinChannelBusyUsb = NULL;
SemaphoreHandle_t SemBinChannelBusy232 = NULL;

SemaphoreHandle_t SemBinPacketTxDelayUsb = NULL;
SemaphoreHandle_t SemBinPacketTxDelay232 = NULL;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/


// Recibo de la PC en la UART_USB
void uartUsbReceiveCallback( void * driver_actual )
{
	BaseType_t xHigherPriorityTaskWoken;
	static uint32_t contador = 0;
	uint8_t lectura = 0;

	//Verifico que no haya terminado de recibir informacion previamente
	if( xSemaphoreTakeFromISR( SemBinPacketReceivedUsb, xHigherPriorityTaskWoken ) )
	{
		UBaseType_t uxSavedInterruptStatus;
		uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
		//esto significa que tengo la informacion en el puntero dataRecieveUsb termino de recibirse correctamente.
		xQueueSendFromISR(((driver_t*)driver_actual)->queLlego, & (dataRecieveUsb), &xHigherPriorityTaskWoken );// Envio el puntero a la cola
		// si la cola esta yena igualmente pierdo el paquete recibido.
		dataRecieveUsb= NULL; // limpio nuevamente el puntero
		taskEXIT_CRITICAL_FROM_ISR( uxSavedInterruptStatus );
	}


	if(dataRecieveUsb == NULL) // si el puntero esta vacio significa que esta comenzando la recepcion de un nuevo paquete.
	{
		lectura = uartRxRead(UART_USB);
		if (lectura == '[') // esta comenzando a leer un paquete nuevo
		{
			if( xSemaphoreTakeFromISR( SemBinChannelBusyUsb, xHigherPriorityTaskWoken ) ) // tomo el semaforo para ocupar el canal USB
			{

				//funcion agregada al QMPool para funcionar desdes ISR
				dataRecieveUsb = QMPool_get_from_ISR( &(((driver_t*)driver_actual)->mem_pool_1), 0U ); //Solicito un bloque de memoria
				if (dataRecieveUsb != NULL)
				{
					dataRecieveUsb->size = 0;// inicializo su tamaño en 1.
					// Deshabilito el transmitter Free IRQ para que no ocupe el canal y me genere un TO de recepcion
					//uartCallbackClr(((driver_t*)driver_actual)->uart_type,UART_TRANSMITER_FREE);
				}
			}
		}
	}

	else if ( dataRecieveUsb->size < dataMaxSize) // verifico que el tamaño sea dentro del tamaño aceptado. Sino no renuevo el timer.
	{
		lectura = uartRxRead(UART_USB);
		if (lectura == ']') // esta terminando de leer un paquete
		{
			// Libero el semaforo avisando que se termino la recepcion de un paquete entero
			xSemaphoreGiveFromISR( SemBinPacketReceivedUsb, xHigherPriorityTaskWoken );
			// Libero el semaforo avisando que se enceuntra el canal libre
			xSemaphoreGiveFromISR( SemBinChannelBusyUsb, xHigherPriorityTaskWoken);

		}
		else
		{
			dataRecieveUsb->data[dataRecieveUsb->size] = lectura;
			dataRecieveUsb->size++;

			if( xTimerStartFromISR(((driver_t*)driver_actual)->rxTimer,&xHigherPriorityTaskWoken ) != pdPASS )
			{
				// The start command was not executed successfully.  Take appropriate action here.

			}
		}
	}

	if( xHigherPriorityTaskWoken != pdFALSE )
	{
		/* Call the interrupt safe yield function here (actual function
        depends on the FreeRTOS port being used). */
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

// Envio a la PC desde la UART_232
void uart232SendCallback( void * driver_actual )
{
	/*
	 *
	 * for future development that includes 232 UART type
	 */
}


// Recibo de la PC en la UART_232
void uart232ReceiveCallback( void * driver_actual )
{
	/*
	 *
	 * for future development that includes 232 UART type
	 */
}

// Envio a la PC desde la UART_USB
void uartUsbSendCallback( void * driver_actual )
{
	BaseType_t xHigherPriorityTaskWoken;
	//UBaseType_t uxSavedInterruptStatus;
	static uint32_t contador = 0;

	xHigherPriorityTaskWoken = pdFALSE;/* We have not woken a task at the start of the ISR. */

	if(dataSendUsb == NULL) // si el puntero esta vacio reviso si hay algo nuevo en la cola
	{
		if( xSemaphoreTakeFromISR( SemBinPacketTxDelayUsb, xHigherPriorityTaskWoken ) ) // tomo el semaforo que indica que comence a transmitir
		{
			if( xQueueReceiveFromISR(((driver_t*)driver_actual)->queTransmitir, & (dataSendUsb), &xHigherPriorityTaskWoken ) )
			{
				// Reviso que no hay algo en la cola. Si no hay, nada no estpero.
				contador = 0; // inicializo el contador para comenzar a transmitir
			}
			else
			{
				xSemaphoreGiveFromISR( SemBinPacketTxDelayUsb, xHigherPriorityTaskWoken );// libero el semaforo para que vuelva a intentar
			}
		}
	}
	else // si no esta vacio es porque hay algo en transmision
	{

		if (contador ==0)
		{
			//Esta comenzando a transmitir
			if( xSemaphoreTakeFromISR( SemBinChannelBusyUsb, xHigherPriorityTaskWoken ) ) // tomo el semaforo para ocupar el canal USB
			{
				//si el canal esta libre envio el primer byte de inicio
				uartTxWrite(UART_USB, '[');
				//uartTxWrite(UART_USB, dataSendUsb->data[contador]);
				contador++;
			}
		}
		if (contador == dataSendUsb->size+1) // si el tamaño es igual es porque termino de enviar los bytes
		{
			//uartTxWrite(((driver_t*)driver_actual)->uart_type, dataSendUsb->data[contador]); // envio el ultiimo byte
			uartTxWrite(UART_USB, ']');// envio el ultiimo byte

			//Libero el semaforo indicando que el canal esta libre
			xSemaphoreGiveFromISR( SemBinChannelBusyUsb, xHigherPriorityTaskWoken );

			// Deshabilito el transmitter Free IRQ para que no ocupe el CPU completo
			uartCallbackClr(((driver_t*)driver_actual)->uart_type,UART_TRANSMITER_FREE);

			//Empiezo el timer para separa los frames de transmision
			if( xTimerStartFromISR(((driver_t*)driver_actual)->txTimer,&xHigherPriorityTaskWoken ) != pdPASS )
			{
				// The start command was not executed successfully.  Take appropriate action here.

			}

			//funcion agregada al QMPool para funcionar desdes ISR
			QMPool_put_from_ISR( &(((driver_t*)driver_actual)->mem_pool_1), dataSendUsb );//libero la memoria del pool


			//QMPool_put( &(((driver_t*)driver_actual)->mem_pool_1), dataSendUsb );//libero la memoria del pool
			dataSendUsb = NULL; // limpio el puntero
			contador = 0; // inicializo el contador para la proxima transmision
		}
		else // si transmision en curso
		{
			uartTxWrite(UART_USB, dataSendUsb->data[contador-1]);
			contador++;
		}
	}
	if( xHigherPriorityTaskWoken )
	{
		/* Actual macro used here is port specific. */
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}


void RxUsbTimerCallback( TimerHandle_t xTimer )
{

	static bool_t temp_flag = TRUE;

	if (temp_flag == TRUE)
	{
		temp_flag= FALSE;
		gpioWrite( LED2, ON );

	}
	else
	{
		temp_flag = TRUE;
		gpioWrite( LED2, OFF );
	}

	// Libero el semaforo avisando que se termino la recepcion de un paquete entero
	xSemaphoreGive( SemBinPacketReceivedUsb );
	// Libero el semaforo avisando que se enceuntra el canal libre
	xSemaphoreGive( SemBinChannelBusyUsb);

}

void Rx232TimerCallback( TimerHandle_t xTimer )
{
	// For future development
}

void TxUsbTimerCallback( TimerHandle_t xTimer )
{

	static bool_t temp_flag = TRUE;

	if (temp_flag == TRUE)
	{
		temp_flag= FALSE;
		gpioWrite( LED1, ON );

	}
	else
	{
		temp_flag = TRUE;
		gpioWrite( LED1, OFF );
	}

	// Libero el semaforo avisando que paso el tiempo necesario de separacion de frames
	xSemaphoreGive( SemBinPacketTxDelayUsb);

}

void Tx232TimerCallback( TimerHandle_t xTimer )
{
	// For future development
}


/*==================[external functions definition]==========================*/

bool_t DriverSend(driver_t* driver_actual,dataStruct_t data)
{

	if (data.size >= dataMaxSize) // valido que el paquete este dentro del tamaño permitido. Necesito guardar 1 byte para el CRC tambien
	{
		return FALSE;
	}

	dataStruct_t *mem_block = QMPool_get( &(driver_actual->mem_pool_1), 0U ); //Solicito un bloque de memoria

	if (mem_block == NULL)
	{
		return FALSE;
	}
	else
	{
		strcpy(&(mem_block->data), &(data.data)); // Guardo la informacion que viene en el nuevo puntero
		mem_block->size = data.size;

		//lo envio a la cola de transmision
		if( xQueueSend( driver_actual->queTransmitir, &mem_block , ( TickType_t ) 0))
		{

			uint8_t val; // variable para guardar el CRC
			val = crc8_init(); // inicializo el valor del CRC
			val = crc8_calc( val, &(data.data), data.size); //calculo el calor del CRC

			mem_block->data[mem_block->size] = val;// guardo el CRC al final del mensaje
			mem_block->size++; // incremento el tamaño del mensaje a enviar para que incluya el CRC


			switch(driver_actual->uart_type)
			{
			case UART_USB:
				// Seteo un callback al evento de transmisor libre y habilito su interrupcion
				uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, (void*) driver_actual);
				break;

			case UART_232:
				// Seteo un callback al evento de transmisor libre y habilito su interrupcion
				uartCallbackSet(UART_232, UART_TRANSMITER_FREE, uart232SendCallback, (void*) driver_actual);
				break;
			}
			uartSetPendingInterrupt(driver_actual->uart_type);// activo que hay pendiente un mensaje a enviar
			return TRUE; //Mensaje enviado a la cola
		}
		else
		{
			/* Failed to post the message, even after 10 ticks. */
			QMPool_put( &(driver_actual->mem_pool_1), mem_block );//libero la memoria del pool
			return FALSE;
		}
	}
	return TRUE;
}


bool_t DriverReceive(driver_t* driver_actual,dataStruct_t * data)
{
	dataStruct_t *temp_data;// aca guardo el puntero que viene de la cola de recepcion
	temp_data = NULL;

	//Verifico que no haya terminado de recibir un paquete y este pendiente de ser enviado a la cola
	if( xSemaphoreTake( SemBinPacketReceivedUsb, ( TickType_t ) 0  ) )
	{
		//esto significa que tengo la informacion en el puntero dataRecieveUsb termino de recibirse correctamente.
		xQueueSend(driver_actual->queLlego , & (dataRecieveUsb), ( TickType_t ) 0);// Envio el puntero a la cola
		// si la cola esta yena igualmente pierdo el paquete recibido.
		dataRecieveUsb= NULL; // limpio nuevamente el puntero
	}

	// si hay algo en la cola la envio a la capa superior
	if( xQueueReceive( driver_actual->queLlego, &temp_data , ( TickType_t ) 0))
	{
		if ( temp_data == NULL)
		{
			return FALSE;
		}
		if (crc8_check(&(temp_data->data), temp_data->size))
		{
			/* MEssage recieved sucesfully. */
			strcpy(&(data->data) , &(temp_data->data)); // Guardo la informacion que viene en el nuevo puntero
			data->size = temp_data->size-1; // resto 1 para quitar el CRC que se enceuntra en el ultimo byte

			QMPool_put( &(driver_actual->mem_pool_1), temp_data );//libero la memoria del pool
			return TRUE;
		}
		else
		{
			/* MEssage recieved with errors. */
			QMPool_put( &(driver_actual->mem_pool_1), temp_data );//libero la memoria del pool
			return FALSE;
		}

	}
	else
	{
		/* Failed to post the message, even after 10 ticks. */
		return FALSE;
	}
}




/*------------------------------------------------------------------*
DriverConfigUART()
// Function que configura el driver de la UART
-*------------------------------------------------------------------*/
bool_t DriverConfigUART(driver_t* driver_actual,uartMap_t type,uint32_t speed)
{
	driver_actual->uart_type = type;
	driver_actual->uart_speed = speed;

	return TRUE;
}


/*------------------------------------------------------------------*
DriverInitialize()
// Function que inicializa el driver
-*------------------------------------------------------------------*/
bool_t DriverInitialize(driver_t* driver_actual)
{

	//Inicialización del Pool
	QMPool_init( &driver_actual->mem_pool_1,
			&driver_actual->memoria_para_pool_1,
			sizeof( driver_actual->memoria_para_pool_1 ),
			sizeof( dataStruct_t ) ); /* Bloques del tamaño de dataStruct_t */

	/* Creo la cola donde voy a enviar mensajes por la UART */
	driver_actual->queTransmitir = xQueueCreate( queTransmitirQueueSize, sizeof( dataStruct_t* ) );
	if( driver_actual->queTransmitir == NULL )
	{
		return FALSE; // La cola fallo en inicializarse
	}

	/* Creo la cola donde pongo los mensaques que recibi por la UART */
	driver_actual->queLlego = xQueueCreate( queTransmitirQueueSize, sizeof( dataStruct_t* ) );
	if( driver_actual->queLlego == NULL )
	{
		return FALSE; // La cola fallo en inicializarse
	}

	// inicializo variable interna de envio
	dataRecieveUsb = NULL;
	dataSendUsb = NULL;
	dataRecieve232 = NULL;
	dataSend232 = NULL;

	// seteo el tiempo de TO del callback acuerdo a la velocidad de transmision definida en la UART
	driver_actual->rx_callback_ms_delay = 50/portTICK_RATE_MS; // queda el TO solo por seguridad

	// seteo el tiempo de TO del callback acuerdo a la velocidad de transmision definida en la UART
	driver_actual->tx_callback_ms_delay = (1000000/ driver_actual->uart_speed)/portTICK_RATE_MS;

	switch(driver_actual->uart_type)
	{
	case UART_USB:
		// I create the timer for timeout for UART USB packet reception
		driver_actual->rxTimer = xTimerCreate ("rx_timeout_timer",driver_actual->rx_callback_ms_delay,pdFALSE,( void * ) 0,RxUsbTimerCallback);
		if( driver_actual->rxTimer == NULL )
		{
			return FALSE; // El Timer fallo en inicializarse
		}

		// I create the timer for timeout for UART USB packet transmision
		driver_actual->txTimer = xTimerCreate ("tx_callback_ms_delay",driver_actual->tx_callback_ms_delay,pdFALSE,( void * ) 0,TxUsbTimerCallback);
		if( driver_actual->txTimer == NULL )
		{
			return FALSE; // El Timer fallo en inicializarse
		}

		// Inicializo semaforo de paquete recivido
		SemBinPacketReceivedUsb = xSemaphoreCreateBinary();
		if( SemBinPacketReceivedUsb == NULL )
		{
			return FALSE; // El semaforo fallo en inicializarse
		}
		xSemaphoreTake( SemBinPacketReceivedUsb, ( TickType_t ) 0);

		// Inicializo semaforo de canal ocupado
		SemBinChannelBusyUsb = xSemaphoreCreateBinary();
		if( SemBinChannelBusyUsb == NULL )
		{
			return FALSE; // El semaforo fallo en inicializarse
		}
		xSemaphoreGive( SemBinChannelBusyUsb);

		// Inicializo semaforo de separacion de paquetes para transmision
		SemBinPacketTxDelayUsb = xSemaphoreCreateBinary();
		if( SemBinPacketTxDelayUsb == NULL )
		{
			return FALSE; // El semaforo fallo en inicializarse
		}
		xSemaphoreGive( SemBinPacketTxDelayUsb);

		/* Inicializar la UART_USB junto con las interrupciones de Tx y Rx */
		uartConfig(UART_USB, driver_actual->uart_speed);
		// Seteo un callback al evento de recepcion y habilito su interrupcion
		uartCallbackSet(UART_USB, UART_RECEIVE, uartUsbReceiveCallback, (void*) driver_actual);
		// Seteo un callback al evento de transmisor libre y habilito su interrupcion
		uartCallbackSet(UART_USB, UART_TRANSMITER_FREE, uartUsbSendCallback, (void*) driver_actual);
		// Habilito todas las interrupciones de UART_USB
		uartInterrupt(UART_USB, TRUE);
		break;

	case UART_232:
		// I create the timer for timeout for UART 232 packet reception
		driver_actual->rxTimer = xTimerCreate ("rx_timeout_timer",driver_actual->rx_callback_ms_delay,pdFALSE,( void * ) 0,Rx232TimerCallback);
		if( driver_actual->rxTimer == NULL )
		{
			return FALSE; // El Timer fallo en inicializarse
		}

		// I create the timer for timeout for UART 232 packet transmision
		driver_actual->txTimer = xTimerCreate ("tx_callback_ms_delay",driver_actual->tx_callback_ms_delay,pdFALSE,( void * ) 0,Tx232TimerCallback);
		if( driver_actual->txTimer == NULL )
		{
			return FALSE; // El Timer fallo en inicializarse
		}

		// Inicializo semaforo de paquete recivido
		SemBinPacketReceived232 = xSemaphoreCreateBinary();
		if( SemBinPacketReceived232 == NULL )
		{
			return FALSE; // El semaforo fallo en inicializarse
		}
		xSemaphoreTake( SemBinPacketReceived232, ( TickType_t ) 0);

		// Inicializo semaforo de canal ocupado
		SemBinChannelBusy232 = xSemaphoreCreateBinary();
		if( SemBinChannelBusy232 == NULL )
		{
			return FALSE; // El semaforo fallo en inicializarse
		}
		xSemaphoreGive( SemBinChannelBusy232);


		/* Inicializar la UART_232 */
		uartConfig(UART_232, driver_actual->uart_speed);
		// Seteo un callback al evento de recepcion y habilito su interrupcion
		uartCallbackSet(UART_232, UART_RECEIVE, uart232ReceiveCallback, (void*) driver_actual);
		// Seteo un callback al evento de transmisor libre y habilito su interrupcion
		uartCallbackSet(UART_232, UART_TRANSMITER_FREE, uart232SendCallback, (void*) driver_actual);
		// Habilito todas las interrupciones de UART_232
		uartInterrupt(UART_232, TRUE);
		break;
	}

	return TRUE;
}

/*==================[end of file]============================================*/
