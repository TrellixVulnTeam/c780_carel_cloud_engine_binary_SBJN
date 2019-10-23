/**
 * @file File_System_IS.c
 * @author carel
 * @date 9 Sep 2019
 * @brief  functions implementations specific related to the managment of the
 *         file system.
 *         Note  : 
 *         we assume that the stdio.h function are supported 
 *         ie. fopen/fclose/fread/fwrite/fseek 
 */






/* Includes ------------------------------------------------------------------*/
#include "File_System_IS.h"   

#ifdef INCLUDE_PLATFORM_DEPENDENT
#include "file_system.h"
#endif

/* Functions Implementation --------------------------------------------------*/

/**
 * @brief File_System_Init
 *        Initialize the file system
 * 
 * @param none
 * @return C_SUCCESS or C_FAIL
 */
C_RES File_System_Init(void)
{  /* TO BE implemented */
   
	/* call the routine to initialize the file system */
	#ifdef INCLUDE_PLATFORM_DEPENDENT
	return init_spiffs();
	#endif
	
	return C_FAIL;
}


