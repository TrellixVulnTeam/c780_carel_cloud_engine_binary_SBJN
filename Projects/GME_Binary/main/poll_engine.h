/*
 * poll_engine.h
 *
 *  Created on: Jul 25, 2019
 *      Author: ataayoub
 */

#ifndef MAIN_POLL_ENGINE_H_
#define MAIN_POLL_ENGINE_H_


#include "common.h"
#include "binary_model.h"
#include "mbcontroller.h"


#include "mb_m.h"
//#include "mb_device_params.h"



#define T_LOW_POLL	(30)   //(120)   //120
#define TSEND		(10*60)
#define T_HIGH_POLL	(10)   //(65)


enum{
	CURRENT = 0,
	PREVIOUS,
};


enum{
	ERROR = 0,
	CHANGED,
};

enum{
	DEACTIVATED = 0,
	ACTIVETED,
};




enum{
	STOPPED = 0,
	INITIALIZED,
	RUNNING,
};

enum{
	NOT_RECEIVED = 0,
	RECEIVED,
	IN_PROGRESS,
	EXECUTED,
};

//Register: Coil and DI low polling and high polling
#pragma pack(1)
typedef struct coil_di_low_high_s{
	r_coil_di		 	info;
	uint8_t 			c_value:1;
	uint8_t 			p_value:1;
	uint8_t				error:3;
}coil_di_low_high_t;
#pragma pack()



//Table: Coil and DI low polling and high polling tables
#pragma pack(1)
typedef struct coil_di_poll_tables_s{
	coil_di_low_high_t 	*reg;
	//uint32_t 			cur_time_stamp;
	//uint32_t			prev_time_stamp;
}coil_di_poll_tables_t;
#pragma pack()


//Register: Coil and DI alarm polling tables
#pragma pack(1)
typedef struct alarm_read_s{
	uint8_t 	value:1;
	uint8_t		error:3;
	uint8_t 	send_flag:1;
	// TODO...DA PADDARE OCIO AI BUSI
	uint32_t	start_time;
	uint32_t	stop_time;
}alarm_read_t;
#pragma pack()

//Table: Coil and DI alarm polling tables
#pragma pack(1)
typedef struct alarm_tables_s{
	r_coil_di_alarm 	info;
	alarm_read_t		data;
}coil_di_alarm_tables_t;
#pragma pack()


//struct for HR and IR low polling and high polling

#pragma pack(1)
typedef enum{
	TYPE_A = 0,
	TYPE_B,
	TYPE_C_SIGNED,
	TYPE_C_UNSIGNED,
	TYPE_D,
	TYPE_E,
	TYPE_F_SIGNED,
	TYPE_F_UNSIGNED,
	MAX_TYPES,
}hr_ir_read_type_t;
#pragma pack()


#pragma pack(1)
typedef union hr_ir_low_high_value_s{
	int32_t value;
	struct{
		int16_t low;
		int16_t high;
	}reg;
}hr_ir_low_high_value_t;
#pragma pack()


#pragma pack(1)
typedef struct hr_ir_low_high_poll_s{
	r_hr_ir 				info;
	hr_ir_low_high_value_t c_value;
	hr_ir_low_high_value_t p_value;
	hr_ir_read_type_t	   read_type;
	uint8_t					error;
}hr_ir_low_high_poll_t;
#pragma pack()

//struct for HR and IR low polling and high polling tables
#pragma pack(1)
typedef struct hr_ir_poll_tables_s{
	hr_ir_low_high_poll_t 	*tab;

	//uint32_t 				cur_time_stamp;
	//uint32_t				prev_time_stamp;
}hr_ir_poll_tables_t;
#pragma pack()

//struct for HR and IR alarm polling
#pragma pack(1)
typedef struct hr_ir_alarm_s{
	uint8_t 	value:1;
	uint8_t		error:3;
	uint8_t 	send_flag:1;
	uint32_t	start_time;
	uint32_t	stop_time;
}hr_ir_alarm_t;
#pragma pack()

//Table: Coil and DI alarm polling tables
#pragma pack(1)
typedef struct hr_ir_alarm_tables_s{
	r_hr_ir_alarm 		info;
	hr_ir_alarm_t		data;
}hr_ir_alarm_tables_t;
#pragma pack()

#pragma pack(1)
typedef struct poll_req_num_s{
	uint8_t coil;
	uint8_t di;
	uint8_t hr;
	uint8_t ir;
	uint16_t total;
}poll_req_num_t;
#pragma pack()



#pragma pack(1)
typedef struct mb_param_char_s{
	char p_ch[6];
}mb_param_char_t;
#pragma pack()

#pragma pack(1)
typedef struct sampling_tstamp{
	uint32_t current_alarm;
	uint32_t previous_alarm;
	uint32_t current_high;
	uint32_t previous_high;
	uint32_t current_low;
	uint32_t previous_low;
}sampling_tstamp_t;
#pragma pack()


#pragma pack(1)
typedef struct values_buffer_s{
	uint16_t 	index;
	uint16_t 	alias;
	long double value;
	uint8_t		info_err;
}values_buffer_t;
#pragma pack()



#pragma pack(1)
typedef struct values_buffer_timing_s{
	uint32_t 	t_start;
	uint32_t 	t_stop;
	uint16_t 	index;
}values_buffer_timing_t;
#pragma pack()



#pragma pack(1)
typedef struct poll_engine_flags_s{
	 uint8_t engine;
	 uint8_t polling;
	 uint8_t passing_mode;
}poll_engine_flags_t;
#pragma pack()




void create_tables(void);
void create_modbus_tables(void);
mb_parameter_descriptor_t* PollEngine__GetParamVectPtr(void);
uint16_t PollEngine__GetParamNum(void);


void CarelEngineMB_Init(void);



#endif /* MAIN_POLL_ENGINE_H_ */