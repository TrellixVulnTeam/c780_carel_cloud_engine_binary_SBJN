/**
 * @file RTC_IS.h
 * @author carel
 * @date 9 Sep 2019
 * @brief  functions implementations specific related to the managment of the
           real time clock.
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_IS_H
#define __RTC_IS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "data_types_CAREL.h"      
   
   
/* Exported types ------------------------------------------------------------*/ 




/* Exported constants --------------------------------------------------------*/



/* Function prototypes -------------------------------------------------------*/
C_RES RTC_Init(C_URI ntp_server, C_UINT16 ntp_port);
C_TIME RTC_Get_UTC_Current_Time(void);

C_TIME RTC_Get_UTC_Boot_Time(void);
void RTC_Set_UTC_Boot_Time(void);

#ifdef __cplusplus
}
#endif

#endif
