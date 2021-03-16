#ifndef _json_test_H
#define _json_test_H

typedef struct{
	bool _ask;
	int _function; 
	uint16_t _startAddr; 
	uint16_t _quantity;
}MODBUS_ASK_T;
extern MODBUS_ASK_T Modbus_Ask_T;

int Init_Json_Test();
int Json_ParseData(char *string);
int Json_DealModbusData(MODBUS_ASK_T askParameter);
char* Json_eSDKGetIPInfo();





#endif


