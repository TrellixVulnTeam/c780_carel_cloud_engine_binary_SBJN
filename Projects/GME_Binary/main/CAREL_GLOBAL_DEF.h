/*
  Description : CAREL_GLOBAL_DEF.H

  Scope       : used to customize the library
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAREL_GLOBAL_DEF
#define __CAREL_GLOBAL_DEF

/* ========================================================================== */
/* include                                                                    */
/* ========================================================================== */


/* ========================================================================== */
/* general purpose                                                            */
/* ========================================================================== */
#define FALSE 0
#define TRUE 1





/* ========================================================================== */
/* Device information                                                         */
/* ========================================================================== */
#define GW_TYPE "GME"
#define GW_PARTNUMBER "GTW000M2G0"
#define GW_HW_REV  "100"
#define GW_FW_REV  "100"

#define GW_CLOUDIFACE_TYPE  "wifi"
#define GW_CLOUDIFACE_NAME  "WIFI-1"
#define GW_CLOUDIFACE_SUBTYPE "2.4 GHz"       
#define GW_CLOUDIFACE_MODE "client"

#define GW_CLOUDIFACE_MAC_ADDRESS "3C:71:BF:F7:F4"
/* ========================================================================== */
/* File system related                                                        */
/* ========================================================================== */



/* ========================================================================== */
/* Enum data types MQTT                                                        */
/* ========================================================================== */
#define MQTT_BOOL		0	 
#define MQTT_INT16		1
#define MQTT_UINT16 	2
#define MQTT_INT32 	  	3
#define MQTT_UINT32   	4
#define MQTT_FLOAT	  	5
#define MQTT_VARIANT	6


/* ========================================================================== */
/* Non volatile memory related  NVM#                                          */
/* ========================================================================== */
#define MQTT_CURRENT_BROKER   (tbd)
#define MQTT_CURRENT_PORT     (tbd)
#define MQTT_CURRENT_USER     (tbd)
#define MQTT_CURRENT_PWD      (tbd)


/* ========================================================================== */
/* Cloud related                                                              */
/* ========================================================================== */

#define MQTT_DEFAULT_BROKER "mqtts://mqtt-dev.tera.systems"   // "mqtts://carelmqtt.com"    //
#define MQTT_DEFAULT_PORT   (8883)
#define MQTT_DEFAULT_USER   "admin"
#define MQTT_DEFAULT_PWD    "5Qz*(3_>K&vU!PS^"
#define MQTT_KEEP_ALIVE_DEFAULT_SEC   (600)


/* ========================================================================== */
/* capabilities cloud related                                                 */
/* ========================================================================== */
#define MQTT_COMPRESSION_L 4
#define MQTT_COMPRESSION_W 8

// integer, whether and how values logging is supported,
// -1 unlimited days back | 0 unsupported | >= 1 number of days back
#define GW_CAPAB_LOGVALUES 0

// integer, whether and how alarms logging is supported,
// -1 unlimited days back | 0 unsupported | >= 1 number of days back.
#define GW_CAPAB_LOGALARMS 0

/* ========================================================================== */
/* POLLING ENGINE RELATED                                                     */
/* ========================================================================== */

//polling time in seconds
#define GW_HIGH_POLLING_TIME 300
#define GW_LOW_POLLING_TIME 600


/* ========================================================================== */
/* ALARMS ENGINE RELATED                                                      */
/* ========================================================================== */
#define ALARM_TYPE_NOT_VALID 0
#define ALARM_TYPE_VARIABLE  1
#define ALARM_TYPE_DEVICE    2


/* ========================================================================== */
/* GW MQTT command related                                                    */
/* ========================================================================== */
//command (res)ponse value
#define MQTT_CMD_INVALID -1
#define MQTT_CMD_OK       0
#define MQTT_CMD_ERROR    1

#define PROTOCOL_ID_NOT_VALID  0
#define PROTOCOL_ID_MODBUS     1
#define PROTOCOL_ID_MODBUS_TCP 2
#define PROTOCOL_ID_BACNET     3




/* ========================================================================== */
/* return values / Error values                                               */
/* ========================================================================== */
#define MQTT_CMD_RETURN_SUCCESS 0
#define MQTT_CMD_RETURN_FAIL    1


#endif
