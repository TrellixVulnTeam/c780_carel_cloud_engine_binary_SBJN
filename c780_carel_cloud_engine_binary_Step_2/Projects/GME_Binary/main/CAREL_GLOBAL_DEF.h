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

/* The platform is determined by an I/O pin on 2G e WiFi model so that
 * the same FW runs on both platforms.
 * ONLY for the bCU the platform must be set manually and generate a
 * dedicated FW.
 * */

#define PLATFORM_DETECTED_WIFI  1
#define PLATFORM_DETECTED_2G    2
#define PLATFORM_DETECTED_BCU   3
#define PLATFORM_DETECTED_ESP_WROVER_KIT  4
#define PLATFORM_DETECTED_TEST_MODE 0x80

/* ========================================================================== */
/* uncomment for development platform                                         */
/* ========================================================================== */
//#define __USE_ESP_WROVER_KIT

/* ========================================================================== */
/*  uncomment for bcu PLATFORM                                                */
/* ========================================================================== */
//#define __USE_CAREL_BCU_HW

/* ========================================================================== */
/*  for WiFi ad 2G platforms comment both above defines                       */
/*  model recognition will happen automatically	                              */
/* ========================================================================== */

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

//#define DEBUG_MODE 1

#if DEBUG_MODE == 1
	#define __CCL_DEBUG_MODE
#endif

#ifdef __CCL_DEBUG_MODE
	#define	PRINTF_DEBUG	printf
#else
	#define	PRINTF_DEBUG(...)
#endif




/*
 * The below row are used to perform a coverage test, not all the part are covered
 * but is possible to modulate the memory occupation selectively enable the coverage
 * for some file simply put #define__CCL_COVERAGE_MODE 1 locally to the files
 * the below enable globally.
 * REMEMBER disable the DEBUG_MODE to obtain a better result and reduce the num. of
 * printf
 * */
//#define __CCL_COVERAGE_MODE

#define COV_MARK "!#!"
#ifdef __CCL_COVERAGE_MODE
    #define	P_COV_LN  printf("%s|%s|%d|\r\n",COV_MARK,__FILE__, __LINE__)
#else
    #define P_COV_LN ;
#endif


//#define	P_COV_LN_OFF(var)  printf("offline %s\r\n", var)


/* ========================================================================== */
/* general purpose                                                            */
/* ========================================================================== */
#define FALSE 0
#define TRUE 1

/* ========================================================================== */
/* Device information                                                         */
/* ========================================================================== */
#define GW_WIFI_PARTNUMBER 		"GTW000MWT0"
#define GW_GSM_PARTNUMBER 		"GTW000MGP0"
#define GW_GSM_WIFI_PARTNUMBER 	"GTW000MGW0"
#define GW_GSM_THIRD_PARTNUMBER	"GTW000MGT0"
/*
 * When there is some changes and a new GME FW version are made,
 * remember to check also the spiffs version
 * generated by the tool "SpiffsSoftwareGenerator"
 *
 */
#define GW_HW_REV  "100"
#define GW_FW_REV  "103"

#define GW_SPIFFS_REV  "101"

// Uncomment following line to enable esp32 bypass:
// everything that will be sent to ttl uart will be
// forwarded to M95 module
// This feature allows to send AT commands to M95 module
// directly from ttl (@115200)
//#define GW_BYPASS_ESP32 1

// Uncomment following line to compile FW for GTW000MGW0 model
// (2G HW model, management of both 2G connection and WiFi AP in FW)
// THIS IS JUST A DRAFT VERSION FOR HOMOLOGATION TESTS!
// FOR FINAL PRODUCT SOME WORK TO DO...
//#define GW_GSM_WIFI

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

#define MQTT_KEEP_ALIVE_DEFAULT_SEC   (60)

#define NTP_DEFAULT_PORT  	123

// period for mobile payload transmission
#define GW_MOBILE_TIME		7200	//600 (10 minutes, stress test), 7200 (2 hours)
#define GW_SAMPLES_MOBILE	4
#define GW_CSQ_TIME			(GW_MOBILE_TIME/GW_SAMPLES_MOBILE) //150 seconds (for stress test) or 30 minutes
//period for status payload transmission
#define GW_STATUS_TIME		7200
// comment below if no periodic power query is required
//
//#define GW_QUERY_RSSI
#endif

