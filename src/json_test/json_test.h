#ifndef _json_test_H
#define _json_test_H

int Init_Json_Test();
int Json_ParseData(char *string);
char* Json_eSDKGetIPInfo();

typedef struct{
	bool _ask;
	int _function; 
	uint16_t _startAddr; 
	uint16_t _quantity;
}MODBUS_ASK_T;

#endif


