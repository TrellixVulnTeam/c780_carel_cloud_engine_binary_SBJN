/**
 * @file modbus_IS.c
 * @author carel
 * @date 9 Sep 2019
 * @brief  this is ONLY an example of the flow request to make a
 *         functional system based on the Carel Cloud Library.
 *         Some routine could be called as a task if an
 *         operating system is available.
 *         In case the OS is not available is possible to use the
 *         routine in a mega-loop but take care that the system are
 *         able to run without significat jitter.
 */
#include "stdint.h"

#include "modbus_IS.h"
#include "data_types_CAREL.h"
#include "data_types_IS.h"
#include "polling_CAREL.h"

#include "gme_config.h"
#include "nvm_CAREL.h"
#include "port.h"
#include "mb_m.h"
//#include "mbcontroller.h"
#include "mbconfig.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "sys_IS.h"
#include "SoftWDT.h"

#include "IO_Port_IS.h"

#define  MODBUS_TIME_OUT     100

static C_SBYTE GetStopBitTable(C_SBYTE stp);
static C_SBYTE GetParityTable(C_SBYTE prt);

// TO DO...implementatiotion depend on the sistem chip in use!!!
extern BOOL xMBMasterPortSerialTxPoll(void);


static TaskHandle_t MODBUS_TASK = NULL;
static uint32_t MB_Device = 0;
static uint16_t MB_Delay = 0;



/**
 * @brief Use brief, otherwise the index won't have a brief explanation.
 *
 * Detailed explanation.
 */
//#define __DEBUG_MODBUS_CAREL




/**
 * @brief Modbus_Init
 *        Initialize the modbus protocol
 *
 * @param none
 * @return C_SUCCESS or C_FAIL
 */
C_RES Modbus_Init(C_INT32 baud, C_SBYTE parity, C_SBYTE stopbit, C_BYTE port)  // TODO stop bit da capire
{
     eMBErrorCode eStatus;
     esp_err_t err;

     // translate into the esp constant
     C_SBYTE mParity = GetParityTable(parity);

     if(port == MB_PORTNUM_485)
     {
   	    err = uart_set_pin(port, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, -1);   // MB_PORTNUM
   	    //printf("\n\r");
   	    //printf("RS485 selected\n");
   	    //printf("\n\r");
     }
   	 else
   	 {
    	err = uart_set_pin(port, TTL_TXD, TTL_RXD, TTL_RTS, -1);   					// MB_PORTNUM
    	//printf("\n\r");
    	//printf("TTL selected\n");
    	//printf("\n\r");
   	 }

   	 if(err != 0)
   	   printf("Setting UART pin fail\n");

	 eStatus = eMBMasterInit(MB_RTU, port, baud, mParity);
	 Sys__Delay(50);

     if (0 == eStatus)
     {
    	eStatus = eMBMasterEnable();
    	if (0 == eStatus){

    		// Set driver mode to Half Duplex
    		if(port == MB_PORTNUM_485)
    		  err = uart_set_mode(port, UART_MODE_RS485_HALF_DUPLEX);
    		else
    		  err = uart_set_mode(port, UART_MODE_RS485_HALF_DUPLEX);

    		return C_SUCCESS;
    	}
    	else{
    		printf("MODBUS initialize fail\n");
    		return C_FAIL;
    	}
     }
     else{
		 printf("MODBUS initialize fail\n");
		 return C_FAIL;
     }

     return C_FAIL;
}


//
// table translation for esp32 uart setting
//

static C_SBYTE GetStopBitTable(C_SBYTE stp)
{
   C_SBYTE val;

   switch(stp)
   {
   	   default:
   	   case 0:
   	   case 1:
   		   val =  UART_STOP_BITS_1;
   		   break;

   	   case 2:
   		   val =  UART_STOP_BITS_2;
   		break;

   	   case 3:
   		   val =  UART_STOP_BITS_1_5;
   		break;
   }
   return val;
}

static C_SBYTE GetParityTable(C_SBYTE prt)
{
	   C_SBYTE val;

	   switch(prt)
	   {
	   	   default:
	   	   case 0:
	   		   val =  UART_PARITY_DISABLE;
	   		   break;

	   	   case 1:
	   		   val =  UART_PARITY_EVEN;
	   		break;

	   	   case 2:
	   		   val =  UART_PARITY_ODD;
	   		break;
	   }
	   return val;
}


/**
 * @brief Modbus_Task
 *        Start the modbus comunication
 *
 * @param none
 * @return none
 */

void Modbus_Task(void)
{
	SoftWDT_Init(SWWDT_MODBUS_RTU , SWWDT_DEFAULT_TIME);

#ifdef INCLUDE_PLATFORM_DEPENDENT
	while(1)
	{
			SoftWDT_Reset(SWWDT_MODBUS_RTU );

			eMBMasterPoll();

			BOOL xSentState = xMBMasterPortSerialTxPoll();

			if (xSentState) {
				// Let state machine know that response was transmitted out
				(void)xMBMasterPortEventPost(EV_MASTER_FRAME_TRANSMITTED);
			}
		}
#endif
}


/**
 * @brief Modbus_Task_Start
 *        if we have a OS we let's start the task here
 *
 * @param none
 * @return none
 */
void Modbus_Task_Start(void)
{
	// to be implemented by USR
#ifdef INCLUDE_PLATFORM_DEPENDENT
	xTaskCreate(&Modbus_Task, "MODBUS_START", 2*2048, NULL, 10, MODBUS_TASK );
#endif

}



// 0x01 //single or multi-coils
int app_coil_read(const uint8_t addr, const int func, const int index, const int num)
{
	C_RES result = C_SUCCESS;
#ifdef INCLUDE_PLATFORM_DEPENDENT
    const long timeout = MODBUS_TIME_OUT;
    eMBMasterReqErrCode errorCode = MB_MRE_NO_ERR;
    const USHORT saddr = index;
    errorCode = eMBMasterReqReadCoils(addr, saddr, num, timeout);
    result = errorCode;
#endif
    Modbus__Delay();
    return result;
}


//  0x02 //single or multi-coils
int app_coil_discrete_input_read(const uint8_t addr, const int func, const int index, const int num)
{
	C_RES result = C_SUCCESS;

#ifdef INCLUDE_PLATFORM_DEPENDENT
    const long timeout = MODBUS_TIME_OUT;
    eMBMasterReqErrCode errorCode = MB_MRE_NO_ERR;
    const USHORT saddr = index;
    errorCode = eMBMasterReqReadDiscreteInputs(addr, saddr, num, timeout);
    result = errorCode;
#endif
    Modbus__Delay();
    return result;
}


//  0x03 //single or multi-coils
int app_holding_register_read(const uint8_t addr, const int func, const int index, const int num)
{
	C_RES result = C_SUCCESS;

#ifdef INCLUDE_PLATFORM_DEPENDENT
    const long timeout = MODBUS_TIME_OUT;
    eMBMasterReqErrCode errorCode = MB_MRE_NO_ERR;
    const USHORT saddr = index;
    errorCode = eMBMasterReqReadHoldingRegister(addr, saddr, num, timeout);
    result = errorCode;
#endif
    Modbus__Delay();
    return result;
}


//  0x04 //single or multi-coils
int app_input_register_read(const uint8_t addr, const int func, const int index, const int num)
{
	C_RES result = C_SUCCESS;

#ifdef INCLUDE_PLATFORM_DEPENDENT
    const long timeout = MODBUS_TIME_OUT;
    eMBMasterReqErrCode errorCode = MB_MRE_NO_ERR;
    const USHORT saddr = index;
    errorCode = eMBMasterReqReadInputRegister(addr, saddr, num, timeout);
    result = errorCode;
#endif
    Modbus__Delay();
    return result;
}


// app_coil_write
int app_coil_write(const uint8_t addr, const int index, short newData)
{
	C_RES result = C_SUCCESS;

#ifdef INCLUDE_PLATFORM_DEPENDENT
    const long timeout = MODBUS_TIME_OUT;
    eMBMasterReqErrCode errorCode = MB_MRE_NO_ERR;

    errorCode = eMBMasterReqWriteCoil( addr, index, newData,timeout);

    result = errorCode;
#endif
    Modbus__Delay();
    return result;
}



// app_hr_write
int app_hr_write(const uint8_t addr, const int index, C_CHAR num_of , C_UINT16 * newData)
{
	C_RES result = C_SUCCESS;

#ifdef INCLUDE_PLATFORM_DEPENDENT
    const long timeout = MODBUS_TIME_OUT;
    eMBMasterReqErrCode errorCode = MB_MRE_NO_ERR;
    const USHORT saddr = index;

    errorCode = eMBMasterReqWriteMultipleHoldingRegister( addr, index, num_of, newData, timeout );

    result = errorCode;
#endif
    Modbus__Delay();
    return result;
}





// app_report_slave_id_read
C_RES app_report_slave_id_read(const uint8_t addr)
{
   C_RES result = C_SUCCESS;

#ifdef INCLUDE_PLATFORM_DEPENDENT
    const long timeout = MODBUS_TIME_OUT;
    eMBMasterReqErrCode errorCode = MB_MRE_NO_ERR;

    errorCode = eMBMAsterReqReportSlaveId(addr, timeout);
    result = errorCode;
#endif
    Modbus__Delay();
    return result;
}

void Modbus_Disable(void)
{
#ifdef INCLUDE_PLATFORM_DEPENDENT
	eMBMasterDisable();
#endif
}

void Modbus_Enable(void)
{
#ifdef INCLUDE_PLATFORM_DEPENDENT
	ClearQueueMB();
	eMBMasterEnable();
#endif
}

void Modbus__ReadAddressFromNVM(void){
	C_UINT32 dev_addr;
	if(C_SUCCESS == NVM__ReadU32Value(MB_DEV_NVM, &dev_addr))
	    MB_Device = dev_addr;
	else
		MB_Device = 1;
}

void Modbus__ReadDelayFromNVM(void){
	C_UINT32 delay;
	if(C_SUCCESS == NVM__ReadU32Value(MB_DELAY_NVM, &delay))
	    MB_Delay = delay;
	else
	    MB_Delay = 0;
	printf("MB_Delay %d\n", MB_Delay);
}

C_UINT16 Modbus__GetAddress(void){
	return MB_Device;
}

void Modbus__Delay(void){
	Sys__Delay(MB_Delay);
}
