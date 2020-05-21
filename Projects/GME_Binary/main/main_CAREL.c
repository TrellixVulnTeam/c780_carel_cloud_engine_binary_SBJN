/*
  File  : main_carel.c

  Scope :
  this is ONLY an example of the flow request to make a
  functional system based on the Carel Cloud Library.
  Some routine could be called as a task if an
  operating system is available.
  In case the OS is not available is possible to use the
  routine in a mega-loop but take care that the system are
  able to run without significat jitter.

  Note  :

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main_IS.h"
#include "main_CAREL.h"

#include "gme_types.h"
#include "data_types_CAREL.h"
#include "modbus_IS.h"
#include "utilities_CAREL.h"

#include "sys_CAREL.h"
#include "sys_IS.h"

#include "common.h"

#include "wifi.h"
#include "polling_IS.h"
#include "nvm_CAREL.h"
#include "Led_Manager_IS.h"
#include "http_server_IS.h"
#include "mobile.h"
#include "radio.h"
#include "binary_model.h"

#include "SoftWDT.h"
#include "IO_Port_IS.h"

#ifdef __DEBUG_MAIN_CAREL_LEV_2
#define CAREL_CHECK(res, field)  (res == C_SUCCESS ? printf("OK %s\n", field) : printf("FAIL %s\n", field))
#else
#define CAREL_CHECK(...)
#endif



#define MODBUS_PORT_SELECT(x, port)           (x == 2 ? (port = MB_PORTNUM_TTL) : (port = MB_PORTNUM_485))
/* Functions implementation -------------------------------------------------------*/

//Variables
static gme_sm_t sm = GME_INIT;

void app_main(void)  // main_Carel
{

  Configure_IO_Check_HW_Platform_IS();
  Sys__Delay(50); //just to stabilize the I/O
  hw_platform_detected = Check_HW_Platform_IS();
  Init_Pins();

  Set_Gateway_ID();

  Led_Task_Start();
  Carel_Main_Task_Start();
  //software watchdog
  while(1)
  {
	SoftWDT_Manager();
	Sys__Delay(1000);
  }
}

void Carel_Main_Task(void)
{
  C_RES retval;
  static bool once = false;
  static uint32_t waiting_conf_timer = 0;
  static uint8_t gw_config_status, line_config_status, devs_config_status;
  static uint8_t test3 = 0;

  static C_UINT32 NVMBaudrate;
  static C_BYTE   NVMConnector;

  static C_BYTE gsm_on_1_shoot;
  static C_BYTE gsm_start_delay;

  SoftWDT_Init(SWWDT_MAIN_DEVICE, SWWDT_DEFAULT_TIME);

  while(1)
  {
	  Sys__Delay(10);
      SoftWDT_Reset(SWWDT_MAIN_DEVICE);
	  IsTimerForAPConnectionExpired();

	  switch (sm)
	  {
		  //System Initialization
		  case GME_INIT:
		  {
			  Init_IO_IS();

			  if PLATFORM(PLATFORM_DETECTED_2G)
		      {
				GSM_Module_Pwr_Supply_On_Off(GSM_POWER_SUPPLY_ON);
                gsm_on_1_shoot = 0;
                gsm_start_delay = 0;
			  }

			  retval = Sys__Init();
			  CAREL_CHECK(retval, "SYSTEM");

			  PRINTF_DEBUG("Version V47 \n");


			  if(retval != C_SUCCESS){
				  sm = GME_REBOOT;
			  }else{
				  sm = GME_CHECK_FILES;
			  }
			  break;
		  }


	        case GME_CHECK_FILES:
	        {
	        	if (test3 == 0){

					if(C_SUCCESS == FS_CheckFiles()){
						sm = GME_RADIO_CONFIG;

						if PLATFORM(PLATFORM_DETECTED_2G)
					    {
						  if ((gsm_on_1_shoot == 0) && (gsm_start_delay == 0))
						  {
							GSM_Module_PwrKey_On_Off(GSM_PWRKEY_ON);
						    gsm_on_1_shoot = 1;
						  }
						  else
						  {
							//wait at least 3 second
							if (gsm_start_delay < 3)
							{
							   gsm_start_delay++;
							   Sys__Delay(1000);
							}
						  }
					    }
					}else{
						sm = GME_CHECK_FILES;
						PRINTF_DEBUG("Please be sure that the certificates are uploaded correctly under the following paths:\nCert1: %s\nCert2: %s\n\n",CERT1_SPIFFS,CERT2_SPIFFS);
					}

					test3 = 1;
	        	}

	        	break;
	        }

	        //Start and configure Radio interface
			case GME_RADIO_CONFIG:
			{
				PRINTF_DEBUG("SM__Start .... GME_RADIO_CONFIG\n");
				uint8_t config_status;

				config_status = Radio__Config();

				if(GME_REBOOT == config_status){
					sm = GME_REBOOT;
				}
				else if(GME_WAITING_FOR_INTERNET == config_status){
					sm = GME_WAITING_FOR_INTERNET;
				}
			}
			break;


	        case GME_WAITING_FOR_INTERNET:
	        {
				if(CONNECTED == Radio__GetStatus()){
					PRINTF_DEBUG("SM__Start .... GME_WAITING_FOR_INTERNET\n");
					sm = GME_STRAT_NTC;
				}
				GME__CheckHTMLConfig();

	        	break;
	        }

		  //Starting the main functionalities of the GME....TO MODIFY!!!
		  case GME_STRAT_NTC:
		  {
			//Start all modules, mqtt client, https client, modbus master, ..etc
			if(false == once){
				PRINTF_DEBUG("Radio__Config .... GME_STRAT_MQTT_NTC\n");
				once = true;
			}

			Radio__WaitConnection();
            //NB. the esp library use always the default port 123...so the file system contain the ntp port value but is not used!!!
			retval = RTC_Init( WiFi__GetCustomConfig().ntp_server_addr, NTP_DEFAULT_PORT);
			retval = RTC_Sync();
			CAREL_CHECK(retval, "TIME");

			//Set boot time
			RTC_Set_UTC_Boot_Time();

			Sys__CertAlloc();

			sm = GME_CHECK_GW_CONFIG;

			GME__CheckHTMLConfig();

			break;
		  }


		  //Start and configure Radio interface
		  case GME_CHECK_GW_CONFIG:
		  {
			//Look for model's file, GW config and Line config
            PRINTF_DEBUG("Radio__Config .... GME_CHECK_GW_CONFIG\n");

			NVM__ReadU8Value(SET_GW_CONFIG_NVM, &gw_config_status);
			NVM__ReadU8Value(SET_LINE_CONFIG_NVM, &line_config_status);
			NVM__ReadU8Value(SET_DEVS_CONFIG_NVM, &devs_config_status);


            #ifdef CHINESE_HW_TEST
				/* this row force the device to be active immediately without a
				 * remote configuration */
				NVM__WriteU8Value(SET_GW_CONFIG_NVM, CONFIGURED);
				NVM__WriteU8Value(SET_LINE_CONFIG_NVM, CONFIGURED);
				NVM__WriteU8Value(SET_DEVS_CONFIG_NVM, CONFIGURED);
				NVM__WriteU32Value(MB_BAUDRATE_NVM, 19200);
				NVM__WriteU8Value(MB_CONNECTOR_NVM, 2);   //write "1" for rs485/ "2" for ttl
				NVM__WriteU32Value(MB_DELAY_NVM, 0);      // no polling delay
            #endif

			if(( CONFIGURED == gw_config_status &&
				 CONFIGURED == line_config_status &&
				 CONFIGURED == devs_config_status))
			{
				sm = GME_SYSTEM_PREPARATION;
			}else{
                PRINTF_DEBUG("gw_config_status = %d \nline_config_status= %d \ndevs_config_status = %d\n", gw_config_status, line_config_status, devs_config_status);
				sm = GME_WAITING_FOR_CONFIG_FROM_MQTT;
			}

			Utilities__Init();
			MQTT_Start();

			GME__CheckHTMLConfig();
			break;
		  }



        case GME_WAITING_FOR_CONFIG_FROM_MQTT:
        {
        	if(RTC_Get_UTC_Current_Time() > waiting_conf_timer + WAITING_CONF_COUNTER){
        		NVM__ReadU8Value(SET_GW_CONFIG_NVM, &gw_config_status);
        		NVM__ReadU8Value(SET_LINE_CONFIG_NVM, &line_config_status);
        		NVM__ReadU8Value(SET_DEVS_CONFIG_NVM, &devs_config_status);

				if(	CONFIGURED == gw_config_status &&			//Set_GW_config
					CONFIGURED == line_config_status &&			//Set_Line_config
					CONFIGURED == devs_config_status)			//Set_devs_config
				{
					sm = GME_REBOOT;
				}else{
					waiting_conf_timer = RTC_Get_UTC_Current_Time();
				}
        	}

        	GME__CheckHTMLConfig();

        }
        	break;


          case GME_SYSTEM_PREPARATION:
          {
          	Radio__WaitConnection();

            sm = GME_START_POLLING_ENGINE;

  			break;
          }


          case GME_START_POLLING_ENGINE:
          {
          	if(MQTT_GetFlags() == 1){

          	    if(CheckModelValidity() == FALSE) {
          	    	// this means loaded model is not valid
          	    	// hence, clear SET_DEVS_CONFIG_NVM flag in nvm and go waiting for a new configuration
          	    	NVM__WriteU8Value(SET_DEVS_CONFIG_NVM, DEFAULT);
          	    	sm = GME_WAITING_FOR_CONFIG_FROM_MQTT;
          	    	break;
          	    }
          	    
          	    NVM__ReadU32Value(MB_BAUDRATE_NVM, &NVMBaudrate);		// read the baudrate from nvm
          	    NVM__ReadU8Value(MB_CONNECTOR_NVM, &NVMConnector);		// read which uart to use (for rs485 or ttl) from nvm

          	    MODBUS_PORT_SELECT(NVMConnector, modbusPort);

          	    // in case of bcu only use ttl, pass proper parameter or force it somehow... TODO
          	    retval = Modbus_Init(NVMBaudrate, GME__GetHEaderInfo()->Rs485Parity, GME__GetHEaderInfo()->Rs485Stop, modbusPort);


          	    CAREL_CHECK(retval, "UART");

          	    Sys__Delay(1000);

          	    Modbus_Task_Start();
          	    Sys__Delay(1000);


         	    PollEngine_MBStart_IS();


  				sm = GME_IDLE_INTERNET_CONNECTED;
  			}else{
  				sm = GME_START_POLLING_ENGINE;
  				GME__CheckHTMLConfig();			// this is needed in case MQTT is not connected
  												// and we want a new config from web to reboot gme (differently reboot won't happen)
  			}

  			break;
          }


          case GME_IDLE_INTERNET_CONNECTED:
          	//TODO
              Radio__WaitConnection();

              if(MQTT_GetFlags() == 1)
              	MQTT_PeriodicTasks();			// manage the MQTT subscribes

              GME__CheckHTMLConfig();

              break;


          //Reboot GME after 5 seconds
          case GME_REBOOT:
        	  GME__Reboot();
              break;

          default:
              break;
          }

          //Check reboot/factory reset button
          if (Get_Button_Pin() >= 0)
              Sys__ResetCheck();

      }
}


//********************************************************
//					PRIVATE FUNCTIONS
//********************************************************

//If we received a new WiFi configuration during system running (Re-Configure)
void GME__CheckHTMLConfig(void){
	if(IsConfigReceived() || IsWpsMode()){
		PRINTF_DEBUG("IsConfigReceived\n");
		sm = GME_RADIO_CONFIG; //GME_WIFI_CONFIG;
		WiFi_SetConfigSM(WAITING_FOR_HTML_CONF_PARAMETERS);
	}
}

void GME__Reboot(void){

	if (PLATFORM(PLATFORM_DETECTED_2G)) {
		PRINTF_DEBUG("Powering down 2G module... power key\n");
		GSM_Module_PwrKey_On_Off(GSM_PWRKEY_OFF);
		Sys__Delay(12000);
		PRINTF_DEBUG("Powering down 2G module... power down\n");
		GSM_Module_Pwr_Supply_On_Off(GSM_POWER_SUPPLY_OFF);
	}

	for(int i=5; i>0; i--)
	{
		PRINTF_DEBUG("Rebooting after %d sec ...\n",i);
		Sys__Delay(1000);
	}

	PRINTF_DEBUG("Rebooting now ...\n");
	fflush(stdout);
	GME_Reboot_IS();
}



static struct HeaderModel mHeaderModel;

void GME__ExtractHeaderInfo(H_HeaderModel *pt)
{
	mHeaderModel = *pt;
}


H_HeaderModel * GME__GetHEaderInfo(void)
{
   return &mHeaderModel;
}






