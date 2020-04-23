/*
  Description : CAREL_GLOBAL_DEF.H

  Scope       : used to customize the library
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAREL_GLOBAL_DEF
#define __CAREL_GLOBAL_DEF


#include "data_types_CAREL.h"


/* ========================================================================== */
/* DEVELOPMENT / TARGET PLATFORM                                              */
/* ========================================================================== */
/**
 * @brief DEVELOPMENT / TARGET PLATFORM
 *        define which target platform do you want to use
 *        REMEMBER Only one.
 */

//TODO BILATO Work in progress

/* The platform is determined by an I/O pin on 2G e WiFi model so that
 * the same FW runs on both platforms.
 * ONLY for the bCU the platform must be set manually and generate a
 * dedicated FW.
 * */

#define PLATFORM_DETECTED_WIFI  1
#define PLATFORM_DETECTED_2G    2
#define PLATFORM_DETECTED_BCU   3
#define PLATFORM_DETECTED_ESP_WROVER_KIT  4

/* ========================================================================== */
/* development platform                                                       */
/* ========================================================================== */
//#define __USE_ESP_WROVER_KIT

/* ========================================================================== */
/*  OFFICIAL HW PLATFORM                                                      */
/* ========================================================================== */
//#define __USE_CAREL_BCU_HW
#define __USE_USR_WIFI_HW
//#define __USE_USR_2G_HW


/**
 * @brief INCLUDE_PLATFORM_DEPENDENT
 *        you MUST enable this define as soon you have implemented all the
 *        platform dependent routines.
 *        undef it to test you specific compiler and understand if all right.
 *        WARNING! this define MUST be DEFINED in the release version of the FW
 */
#define INCLUDE_PLATFORM_DEPENDENT 1

/* ========================================================================== */
/* include                                                                    */
/* ========================================================================== */

/* ========================================================================== */
/* general purpose                                                            */
/* ========================================================================== */

/* ========================================================================== */
/* debugging purpose                                                          */
/* ========================================================================== */
/**
 * @brief __CCL_DEBUG_MODE 
 *        if defined enable print on the debug console some debugger message 
 *        take care to enable the _DEBUG_filename locally in each file you want
 *        to debug. 
 *        WARNING! remember that the debug output take elaboration time 
 *        WARNING! this define MUST be DISABLE in the release version of the FW
 */
//#define __CCL_DEBUG_MODE

#ifdef __CCL_DEBUG_MODE
	#define	PRINTF_DEBUG	printf
#else
	#define	PRINTF_DEBUG(...)
#endif



/* ONLY TO TEST THE HW IN CHINA */
//#define CHINESE_HW_TEST


/* ========================================================================== */
/* general purpose                                                            */
/* ========================================================================== */
#define FALSE 0
#define TRUE 1

/* ========================================================================== */
/* Device information                                                         */
/* ========================================================================== */
#define GW_WIFI_PARTNUMBER 	"GTW000MWT0"
#define GW_GSM_PARTNUMBER 	"GTW000MGP0"

#define GW_HW_REV  256 //0x100
#define GW_FW_REV  256 //0x100

/* ========================================================================== */
/* OS related                                                                 */
/* ========================================================================== */

/** @brief SYSTEM_TIME_TICK  system tick of the OS expressed in ms
 *                           if used in non OS system this is the incremet of 
 *                           tick time 
*/

#define SYSTEM_TIME_TICK	1

/* ========================================================================== */
/* Cloud related                                                              */
/* ========================================================================== */

#define MQTT_DEFAULT_BROKER "mqtts://mqtt-dev.tera.systems"   // "mqtts://carelmqtt.com"    //
#define MQTT_DEFAULT_PORT   (8883)
#define MQTT_DEFAULT_USER   "admin"
#define MQTT_DEFAULT_PWD    "5Qz*(3_>K&vU!PS^"
#define MQTT_KEEP_ALIVE_DEFAULT_SEC   (600)


#define NTP_DEFAULT_SERVER  "pool.ntp.org"
#define NTP_DEFAULT_PORT  	123

#define GW_MOBILE_TIME		300
#endif

