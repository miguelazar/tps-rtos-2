/*
 * FinalRtosProtocolos.h
 *
 *  Created on: Jun 14, 2019
 *      Author: nacho
 */


// General Includes
#include "stdio.h"
#include "sapi.h"
#include <string.h>
//#include "stringManipulation.h"
#include "board.h"
#include "driver2.h"
#include "oa.h"

// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"


#define MAYUSCULIZAR_FUNC_ID		49 //cooresponde a 1 en ASCII
#define MINUSCULIZAR_FUNC_ID		50 //cooresponde a 2 en ASCII
