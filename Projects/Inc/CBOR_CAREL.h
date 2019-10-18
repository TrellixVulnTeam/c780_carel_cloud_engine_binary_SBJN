/**
 * @file CBOR_CAREL.h
 * @author Carel
 * @date 12 Sep 2019
 * @brief Carel CBOR glue logic
 * Functions performing encoding need a payload buffer to be passed as reference in order to be filled with required info.
 * This buffer needs to be previously statically allocated (with proper size)
 *      
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CBOR_CAREL_H
#define __CBOR_CAREL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "data_types_CAREL.h" 
#include "cbor.h"

/* Exported constants --------------------------------------------------------*/

#define CBORSTREAM_SIZE			1024

#define TAG_SIZE				3
#define REPLYTO_SIZE			48
#define RESPONSE_SIZE			80
#define ALIAS_SIZE				10	//TODO
#define VAL_SIZE				30
#define A_SIZE					30
#define B_SIZE					30
#define ENTRY_PER_PKT			VLS_NUMBER //TODO

#define REPORT_SLAVE_ID_SIZE	512
#define ADU_SIZE				512

#define CMD_SCAN_LINE_RES		TODO
#define CMD_SEND_MB_ADU			TODO
#define CMD_SET_DEVS_CONFIG		TODO
#define CMD_SET_LINES_CONFIG		TODO
#define CMD_WRITE_VALUES		TODO
#define CMD_READ_VALUES			TODO

#define HEADERREQ_LEN			55			// header of request has fixed size

enum CBOR_CmdResponse{
	INVALID_CMD = -1,
	SUCCESS_CMD,
	ERROR_CMD,
};

typedef enum CloudtoGME_Commands_l{
	SET_GW_CONFIG = 1,
	REBOOT,
	SCAN_DEVICES,
	SET_LINES_CONFIG,
	SET_DEVS_CONFIG,
	READ_VALUES,
	WRITE_VALUES = 7,
	UPDATE_GME_FIRMWARE=10,
	UPDATE_DEV_FIRMWARE,
	FLUSH_VALUES,
	UPDATE_CA_CERTIFICATES,
	SEND_MB_ADU,
	CHANGE_CREDENTIALS,
	START_ENGINE,
	STOP_ENGINE,
}cloud_req_commands_t;


#define CAREL_DEBUG


#ifdef CAREL_DEBUG


#define DEBUG_ENC(err, a) printf("%s: %s %s, error %d\n", __func__, "cannot encode", a, err);
#define DEBUG_ADD(err, a) printf("%s: %s %s, error %d\n", __func__, "cannot add", a, err);
#define DEBUG_DEC(err, a) printf("%s: %s %s, error %d\n", __func__, "cannot decode", a, err);
#else
#define DEBUG_ENC(err, a)
#define DEBUG_ADD(err, a)
#define DEBUG_DEC(err, a)
#endif

/* Exported types ------------------------------------------------------------*/ 

/**
 * @brief C_CBORHREQ
 *
 * Header of a request/response
 * In case of a request, the last element is not populated
 */
#pragma pack(1)
typedef struct C_CBORHREQ{
	C_UINT16 ver;
	C_BYTE rto[REPLYTO_SIZE];
	C_UINT16 cmd;
	C_INT16 res;
} c_cborhreq;
#pragma pack()

/**
 * @brief C_CBORRESWRITEVALUES
 *
 * Response to a write values (without header)
 */
#pragma pack(1)
typedef struct C_CBORREQWRITEVALUES{
	C_CHAR alias[ALIAS_SIZE];
	C_CHAR val[VAL_SIZE];
	C_UINT16 func;
	C_UINT16 addr;
	C_UINT16 dim;
	C_UINT16 pos;
	C_UINT16 len;
	C_CHAR a[A_SIZE];
	C_CHAR b[B_SIZE];
	C_BYTE flags;
	
}c_cborreqwritevalues;
#pragma pack()

/**
 * @brief C_CBORRESREADVALUES
 *
 * Response to a write values (without header)
 */
#pragma pack(1)
typedef struct C_CBORREQREADVALUES{
	C_CHAR alias[ALIAS_SIZE];
	C_UINT16 func;
	C_UINT16 addr;
	C_UINT16 dim;
	C_UINT16 pos;
	C_UINT16 len;
	C_CHAR a[A_SIZE];
	C_CHAR b[B_SIZE];
	C_BYTE flags;
	
}c_cborreqreadvalues;
#pragma pack()

/**
 * @brief C_CBORREQSETGWCONFIG
 *
 * Request set gw config
 */
#pragma pack(1)
typedef struct C_CBORREQSETGWCONFIG{
	C_UINT16 pva;		// values message will be sent every pva seconds
	C_UINT16 pst;		// status message will be sent every pst seconds
	C_UINT16 mka;		// mqtt keep alive interval
	C_UINT16 lss;		// low speed sampling period
	C_UINT16 hss;		// high speed sampling period
}c_cborreqsetgwconfig;
#pragma pack()

/**
 * @brief C_CBORALARMS
 *
 * Alarms
 */
#pragma pack(1)
typedef struct C_CBORALARMS{
	C_BYTE aty;
	C_CHAR ali[ALIAS_SIZE];
	C_BYTE aco;
	C_TIME st;
	C_TIME et;
}c_cboralarms;
#pragma pack()

#define VLS_NUMBER 		3
typedef struct C_CBORVALS{
	C_CHAR alias[ALIAS_SIZE];
	C_CHAR values[ALIAS_SIZE];
}c_cborvals;

typedef struct db_values{
	C_TIME t;
	C_UINT32 cnt;
	c_cborvals vls[VLS_NUMBER];
}db_values;

/*----------------------------------------------------------------------------------------*/
size_t CBOR_Alarms(C_CHAR* cbor_stream, c_cboralarms cbor_alarms);
void CBOR_SendAlarms(c_cboralarms cbor_alarms);
size_t CBOR_Hello(C_CHAR* cbor_stream);
void CBOR_SendHello(void);
size_t CBOR_Status(C_CHAR* cbor_stream);
void CBOR_SendStatus(void);
size_t CBOR_Values(C_CHAR* cbor_stream, C_UINT16 index, C_UINT16 number, C_INT16 frame);
void CBOR_SendFragmentedValues(C_UINT16 index, C_UINT16 number);
size_t CBOR_Mobile(C_CHAR* cbor_stream);

void CBOR_ResHeader(C_CHAR* cbor_stream, c_cborhreq* cbor_req, CborEncoder* encoder, CborEncoder* mapEncoder);
size_t CBOR_ResSimple(C_CHAR* cbor_response, c_cborhreq* cbor_req);
size_t CBOR_ResScanLine(C_CHAR* cbor_response, c_cborhreq* cbor_req, C_UINT16 device, C_CHAR* answer);
size_t CBOR_ResSendMbAdu(C_CHAR* cbor_response, c_cborhreq* cbor_req, C_UINT16 seq, C_CHAR* val);

CborError CBOR_ReqHeader(C_CHAR* cbor_stream, C_UINT16 cbor_len, c_cborhreq* cbor_req, CborValue* it, CborValue* recursed);
CborError CBOR_ReqSetLinesConfig(CborValue* recursed, C_UINT32* new_baud_rate, C_BYTE* new_connector);
CborError CBOR_ReqSetDevsConfig(CborValue* recursed, C_SCHAR* usr, C_SCHAR* pwd, C_SCHAR* uri, C_UINT16* cid);
CborError CBOR_ReqChangeCredentials(CborValue* recursed, C_SCHAR* usr, C_SCHAR* pwd);
CborError CBOR_ReqWriteValues(CborValue* recursed, c_cborreqwritevalues* cbor_wv);
CborError CBOR_ReqSetGwConfig(CborValue* recursed, c_cborreqsetgwconfig* cbor_setgwconfig);
CborError CBOR_ReqSendMbAdu(CborValue* recursed, C_UINT16* seq, C_CHAR* adu);
int CBOR_ReqTopicParser(C_CHAR* cbor_stream, C_UINT16 cbor_len);

C_INT16 CBOR_ExtractInt(CborValue* recursed, int64_t* read);
#ifdef __cplusplus
}
#endif

#endif
