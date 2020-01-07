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
#include "nvm.h"
#include "Led_Manager_IS.h"

#ifdef IS_A_GSM_GATEWAY
#include "GSM_Miscellaneous_IS.h"
#endif

#ifdef IS_A_WIFI_GATEWAY

#endif


#define CAREL_CHECK(res, field)  (res == C_SUCCESS ? printf("OK %s\n", field) : printf("FAIL %s\n", field))

/* Functions implementation -------------------------------------------------------*/

//Variables
static gme_sm_t sm;

void app_main(void)  // main_Carel
{
  Led_Task_Start();
  Carel_Main_Task_Start();
}

void Carel_Main_Task(void)
{
  C_RES retval;
  static bool once = false;
  static uint32_t waiting_conf_timer = 0;
  static uint8_t gw_config_status, line_config_status, devs_config_status;
  static uint8_t test3 = 0;

  while(1)
  {
	  switch (sm)
	  {
		  //System Initialization
		  case GME_INIT:
		  {
			  retval = Sys__Init();
			  CAREL_CHECK(retval, "SYSTEM");

			  if(retval != C_SUCCESS){
				  sm = GME_REBOOT;
			  }else{
				  sm = GME_CHECK_FILES;
			  }
			  break;
		  }


	        case GME_CHECK_FILES:
	        {
	        	if(test3 == 0){
					if(C_SUCCESS == FS_CheckFiles()){
						sm = GME_WIFI_CONFIG;
					}else{
						sm = GME_CHECK_FILES;
						printf("Please be sure that the certificates are uploaded correctly under the following paths:\nCert1: %s\nCert2: %s\n\n",CERT1_SPIFFS,CERT2_SPIFFS);
					}
					test3 = 1;
	        	}

	        	break;
	        }


			//Start and configure WiFi interface
			case GME_WIFI_CONFIG:
			{
				printf("SM__Start .... GME_WIFI_CONFIG\n");
				uint8_t config_status;
				config_status = Sys__Config(Sys_GetConfigSM());

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
				if(CONNECTED == WIFI__GetSTAStatus()){
					printf("SM__Start .... GME_WAITING_FOR_INTERNET\n");
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
				printf("Sys__Config .... GME_STRAT_MQTT_NTC\n");
				once = true;
			}

			WiFi__WaitConnection();

			//Init_RTC();
			retval = RTC_Init( WiFi__GetCustomConfig().ntp_server_addr,0);		// Vale: make this call hw independent					// Carel
			CAREL_CHECK(retval, "TIME");

			//Set boot time
			RTC_Set_UTC_Boot_Time();

			Sys__CertAlloc();

			sm = GME_CHECK_GW_CONFIG;

			GME__CheckHTMLConfig();

			break;
		  }


		  //Start and configure WiFi interface
		  case GME_CHECK_GW_CONFIG:
		  {
			//Look for model's file, GW config and Line config
			printf("Sys__Config .... GME_CHECK_GW_CONFIG\n");

			NVM__ReadU8Value(SET_GW_CONFIG_NVM, &gw_config_status);
			NVM__ReadU8Value(SET_LINE_CONFIG_NVM, &line_config_status);
			NVM__ReadU8Value(SET_DEVS_CONFIG_NVM, &devs_config_status);

			if(( CONFIGURED == gw_config_status &&
				 CONFIGURED == line_config_status &&
				 CONFIGURED == devs_config_status))
			{
				sm = GME_SYSTEM_PREPARATION;
			}else{
				printf("gw_config_status = %d \nline_config_status= %d \ndevs_config_status = %d\n", gw_config_status, line_config_status, devs_config_status);
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
          	WiFi__WaitConnection();

            sm = GME_START_POLLING_ENGINE;

  			break;
          }
          case GME_START_POLLING_ENGINE:
          {
          	if(MQTT_GetFlags() == 1){

          	    retval = BinaryModel_Init();		// CAREL
          	    CAREL_CHECK(retval, "MODEL");

          	    retval = Modbus_Init(19200);        // CAREL
          	    CAREL_CHECK(retval, "UART");

          	    Sys__Delay(1000);

          	    Modbus_Task_Start();                // CAREL
          	    Sys__Delay(1000);


          	    PollEngine_MBStart_IS();


  				sm = GME_IDLE_INTERNET_CONNECTED;
  			}else{
  				sm = GME_START_POLLING_ENGINE;
  			}

  			break;
          }


          case GME_IDLE_INTERNET_CONNECTED:
          	//TODO
              WiFi__WaitConnection();

              if(MQTT_GetFlags() == 1)
              	MQTT_PeriodicTasks();			// manage the MQTT subscribes


              //If we received a new WiFi configuration during system running (Re-Configure)
              if(IsConfigReceived()){
              	//sm = GME_CONFIG;
              	Sys_SetConfigSM(WAITING_FOR_HTML_CONF_PARAMETERS);
              }
              break;


          //Reboot GME after 5 seconds
          case GME_REBOOT:
        	  GME__Reboot();
              break;

          default:
              break;
          }

          //If the factory reset button has been pressed for X time (look gme_config.h)
          if(true == Sys__ResetCheck())
          {
          	printf("RESET CHECK DONE STATEMACHINE\n");
          	sm = GME_REBOOT;
          }
      }
}


//********************************************************
//					PRIVATE FUNCTIONS
//********************************************************

//If we received a new WiFi configuration during system running (Re-Configure)
void GME__CheckHTMLConfig(void){
	if(IsConfigReceived()){
		printf("IsConfigReceived\n");
		sm = GME_WIFI_CONFIG;
		Sys_SetConfigSM(WAITING_FOR_HTML_CONF_PARAMETERS);
	}
}

void GME__Reboot(void){
	for(int i=5; i>0; i--)
	{
		printf("Rebooting after %d sec ...\n",i);
		Sys__Delay(1000);
	}
	printf("Rebooting now ...\n");
	fflush(stdout);
	GME_Reboot_IS();
}








