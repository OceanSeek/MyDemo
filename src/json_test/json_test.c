#include "sys.h"

MODBUS_ASK_T Modbus_Ask_T;

char* Json_eSDKGetIPInfo()
{
	char *retString;
	//先创建空对象
    cJSON *json = cJSON_CreateObject();
    //在对象上添加键值对
    cJSON_AddStringToObject(json,"token","12345");

	char *timestamp;
	timestamp = sysLocalTime();
    cJSON_AddStringToObject(json,"timestamp",timestamp);
	free(timestamp);
	timestamp = NULL;
    //清理工作
    char *buf = cJSON_Print(json);
//	log(buf);
    cJSON_Delete(json);
    return buf;

}


char* Json_ModbusSendCoilStatus(uint16_t startAddr, uint16_t quantity)
{
	char *retString;
	//先创建空对象
    cJSON *json = cJSON_CreateObject();
    //在对象上添加键值对
//    cJSON_AddStringToObject(json,"devices","12345");

    //添加数组
    cJSON *devices_array = NULL;
    cJSON_AddItemToObject(json,"devices",devices_array=cJSON_CreateArray());
    //在数组上添加对象
    cJSON *obj_1 = NULL;
    cJSON_AddItemToArray(devices_array,obj_1=cJSON_CreateObject());
    cJSON_AddStringToObject(obj_1,"deviceId",CLIENTID);

    //添加数组
    cJSON *services_array = NULL;
    cJSON_AddItemToObject(obj_1,"services",services_array=cJSON_CreateArray());
	
    cJSON *obj_2 = NULL;
    cJSON_AddItemToArray(services_array,obj_2=cJSON_CreateObject());
	
    cJSON *obj_3 = NULL;
    cJSON_AddItemToObject(obj_2,"data",obj_3=cJSON_CreateObject());

    //添加数组
    cJSON *CoilStatus_array = NULL;
	cJSON_AddItemToObject(obj_3,"CoilStatus",CoilStatus_array=cJSON_CreateArray());

    cJSON *obj_4 = NULL;
    cJSON_AddItemToArray(CoilStatus_array,obj_4=cJSON_CreateObject());
	int i;
	char addr[10],*value;
	bool coilStatusValue;
	uint16_t wRealID,nPoint,DevNo;
	
	for(i=0;i<quantity;i++){
		if(startAddr + i >= gVars.TransModbusCoidStatusTableNum)break;
		wRealID = TransModbusCoidStatusTable[startAddr + i].wRealID;
		nPoint = TransModbusCoidStatusTable[startAddr + i].nPoint;
		DevNo = GetDevNo(wRealID);
		if(RET_ERROR == DevNo){
			perror("RET_ERROR == DevNo");
			return NULL;
		}
		sprintf(addr,"addr%d", startAddr + i);
		coilStatusValue = gpDevice[DevNo].ModbusData.pCoilStatus[nPoint];
		if(coilStatusValue) value = "true";
		else value = "false";
    	cJSON_AddStringToObject(obj_4,addr,value);
	}



	char *timestamp;
	timestamp = sysLocalTime();
    cJSON_AddStringToObject(obj_2,"eventTime",timestamp);
	free(timestamp);
	timestamp = NULL;
    cJSON_AddItemToObject(obj_2,"serviceId",cJSON_CreateString("serviceName"));
	
    //清理工作
    char *buf = cJSON_Print(json);
//	log(buf);
    cJSON_Delete(json);
    return buf;

}

char* Json_ModbusSendInputStatus(uint16_t startAddr, uint16_t quantity)
{
	char *retString;
	//先创建空对象
    cJSON *json = cJSON_CreateObject();

    //添加数组
    cJSON *devices_array = NULL;
    cJSON_AddItemToObject(json,"devices",devices_array=cJSON_CreateArray());
    //在数组上添加对象
    cJSON *obj_1 = NULL;
    cJSON_AddItemToArray(devices_array,obj_1=cJSON_CreateObject());
    cJSON_AddStringToObject(obj_1,"deviceId",CLIENTID);

    //添加数组
    cJSON *services_array = NULL;
    cJSON_AddItemToObject(obj_1,"services",services_array=cJSON_CreateArray());
	
    cJSON *obj_2 = NULL;
    cJSON_AddItemToArray(services_array,obj_2=cJSON_CreateObject());
	
    cJSON *obj_3 = NULL;
    cJSON_AddItemToObject(obj_2,"data",obj_3=cJSON_CreateObject());
	
    //添加数组
    cJSON *InputStatus_array = NULL;
	cJSON_AddItemToObject(obj_3,"InputStatus",InputStatus_array=cJSON_CreateArray());

    cJSON *obj_4 = NULL;
    cJSON_AddItemToArray(InputStatus_array,obj_4=cJSON_CreateObject());
	int i;
	char addr[10],*value;
	bool coilStatusValue;
	uint16_t wRealID,nPoint,DevNo;
	
	for(i=0;i<quantity;i++){
		if(startAddr + i >= gVars.TransModbusInputStatusTableNum)break;
		wRealID = TransModbusCoidStatusTable[startAddr + i].wRealID;
		nPoint = TransModbusCoidStatusTable[startAddr + i].nPoint;
		DevNo = GetDevNo(wRealID);
		if(RET_ERROR == DevNo){
			perror("RET_ERROR == DevNo");
			return NULL;
		}
		sprintf(addr,"addr%d", startAddr + i);
		coilStatusValue = gpDevice[DevNo].ModbusData.pInputStatus[nPoint];
		if(coilStatusValue) value = "true";
		else value = "false";
    	cJSON_AddStringToObject(obj_4,addr,value);
	}



	char *timestamp;
	timestamp = sysLocalTime();
    cJSON_AddStringToObject(obj_2,"eventTime",timestamp);
	free(timestamp);
	timestamp = NULL;
    cJSON_AddItemToObject(obj_2,"serviceId",cJSON_CreateString("serviceName"));
	
    //清理工作
    char *buf = cJSON_Print(json);
//	log(buf);
    cJSON_Delete(json);
    return buf;

}


char* Json_ModbusSendHoldingRegs(uint16_t startAddr, uint16_t quantity)
{
	char *retString;
	//先创建空对象
    cJSON *json = cJSON_CreateObject();

    //添加数组
    cJSON *devices_array = NULL;
    cJSON_AddItemToObject(json,"devices",devices_array=cJSON_CreateArray());
    //在数组上添加对象
    cJSON *obj_1 = NULL;
    cJSON_AddItemToArray(devices_array,obj_1=cJSON_CreateObject());
    cJSON_AddStringToObject(obj_1,"deviceId",CLIENTID);

    //添加数组
    cJSON *services_array = NULL;
    cJSON_AddItemToObject(obj_1,"services",services_array=cJSON_CreateArray());
	
    cJSON *obj_2 = NULL;
    cJSON_AddItemToArray(services_array,obj_2=cJSON_CreateObject());
	
    cJSON *obj_3 = NULL;
    cJSON_AddItemToObject(obj_2,"data",obj_3=cJSON_CreateObject());
	
    //添加数组
    cJSON *HoldingRegister_array = NULL;
	cJSON_AddItemToObject(obj_3,"HoldingRegister",HoldingRegister_array=cJSON_CreateArray());

    cJSON *obj_4 = NULL;
    cJSON_AddItemToArray(HoldingRegister_array,obj_4=cJSON_CreateObject());
	int i;
	char addr[10],value[10];
	uint16_t regValue;
	uint16_t wRealID,nPoint,DevNo;

	
	for(i=0;i<quantity;i++){
		if(startAddr + i >= gVars.TransModbusHoldingRegTableNum)break;
		wRealID = TransModbusHoldingRegTable[startAddr + i].wRealID;
		nPoint = TransModbusHoldingRegTable[startAddr + i].nPoint;
		DevNo = GetDevNo(wRealID);
		if(RET_ERROR == DevNo){
			perror("RET_ERROR == DevNo");
			return NULL;
		}
		sprintf(addr,"addr%d", startAddr + i);
		sprintf(value, "%d", gpDevice[DevNo].ModbusData.pHoldingRegister[nPoint]);
    	cJSON_AddStringToObject(obj_4,addr,value);
	}



	char *timestamp;
	timestamp = sysLocalTime();
    cJSON_AddStringToObject(obj_2,"eventTime",timestamp);
	free(timestamp);
	timestamp = NULL;
    cJSON_AddItemToObject(obj_2,"serviceId",cJSON_CreateString("serviceName"));
	
    //清理工作
    char *buf = cJSON_Print(json);
//	log(buf);
    cJSON_Delete(json);
    return buf;

}


char* Json_ModbusSendInputRegs(uint16_t startAddr, uint16_t quantity)
{
	char *retString;
	//先创建空对象
    cJSON *json = cJSON_CreateObject();

    //添加数组
    cJSON *devices_array = NULL;
    cJSON_AddItemToObject(json,"devices",devices_array=cJSON_CreateArray());
    //在数组上添加对象
    cJSON *obj_1 = NULL;
    cJSON_AddItemToArray(devices_array,obj_1=cJSON_CreateObject());
    cJSON_AddStringToObject(obj_1,"deviceId",CLIENTID);

    //添加数组
    cJSON *services_array = NULL;
    cJSON_AddItemToObject(obj_1,"services",services_array=cJSON_CreateArray());
	
    cJSON *obj_2 = NULL;
    cJSON_AddItemToArray(services_array,obj_2=cJSON_CreateObject());
	
    cJSON *obj_3 = NULL;
    cJSON_AddItemToObject(obj_2,"data",obj_3=cJSON_CreateObject());
	
    //添加数组
    cJSON *InputRegister_array = NULL;
	cJSON_AddItemToObject(obj_3,"InputRegister",InputRegister_array=cJSON_CreateArray());

    cJSON *obj_4 = NULL;
    cJSON_AddItemToArray(InputRegister_array,obj_4=cJSON_CreateObject());
	int i;
	char addr[10],value[10];
	uint16_t regValue;
	uint16_t wRealID,nPoint,DevNo;

	
	for(i=0;i<quantity;i++){
		if(startAddr + i >= gVars.TransModbusInputRegTableNum)break;
		wRealID = TransModbusHoldingRegTable[startAddr + i].wRealID;
		nPoint = TransModbusHoldingRegTable[startAddr + i].nPoint;
		DevNo = GetDevNo(wRealID);
		if(RET_ERROR == DevNo){
			perror("RET_ERROR == DevNo");
			return NULL;
		}
		sprintf(addr,"addr%d", startAddr + i);
		sprintf(value, "%d", gpDevice[DevNo].ModbusData.pInputResgister[nPoint]);
    	cJSON_AddStringToObject(obj_4,addr,value);
	}



	char *timestamp;
	timestamp = sysLocalTime();
    cJSON_AddStringToObject(obj_2,"eventTime",timestamp);
	free(timestamp);
	timestamp = NULL;
    cJSON_AddItemToObject(obj_2,"serviceId",cJSON_CreateString("serviceName"));
	
    //清理工作
    char *buf = cJSON_Print(json);
//	log(buf);
    cJSON_Delete(json);
    return buf;

}


int Json_GetAskData(char *name, char* value)
{
	int function, quantity,startAddr;
	if(strcmp(name, "function") == 0){
		if(strcmp(value, "CoilStatus") == 0){
			function = ReadCoilStatus;
		}
		else if(strcmp(value, "InputStatus") == 0){
			function = ReadInputStatus;
		}
		else if(strcmp(value, "HoldingRegister") == 0){
			function = ReadHoldingRegister;
		}
		else if(strcmp(value, "InputRegister") == 0){
			function = ReadInputRegister;
		}
		Modbus_Ask_T._function = function;
	}
	if(strcmp(name, "modbusAsk") == 0){
		if(strcmp(value, "true") == 0){
			Modbus_Ask_T._ask = true;
		}
		else Modbus_Ask_T._ask = false;
		
	}
	if(strcmp(name, "quantity") == 0){
		quantity = atoi(value);
		log("__quantity is %d \n",quantity);
		Modbus_Ask_T._quantity = quantity;
	}
	if(strcmp(name, "startAddr") == 0){
		startAddr = atoi(value);
		log("__startAddr is %d \n",startAddr);
		Modbus_Ask_T._startAddr = startAddr;
	}
	
	
}

int Json_Send_Data()
{
	//先创建空对象
    cJSON *json = cJSON_CreateObject();
    //在对象上添加键值对
    cJSON_AddStringToObject(json,"country","china");
    //添加数组
    cJSON *array = NULL;
    cJSON_AddItemToObject(json,"stars",array=cJSON_CreateArray());
    //在数组上添加对象
    cJSON *obj = NULL;
    cJSON_AddItemToArray(array,obj=cJSON_CreateObject());
    cJSON_AddItemToObject(obj,"name",cJSON_CreateString("Faye"));
    cJSON_AddStringToObject(obj,"address","beijing");
    //在对象上添加键值对
    cJSON_AddItemToArray(array,obj=cJSON_CreateObject());
    cJSON_AddItemToObject(obj,"name",cJSON_CreateString("andy"));
    cJSON_AddItemToObject(obj,"address",cJSON_CreateString("HK"));

    cJSON_AddItemToArray(array,obj=cJSON_CreateObject());
    cJSON_AddStringToObject(obj,"name","eddie");
    cJSON_AddStringToObject(obj,"address","TaiWan");

    //清理工作
    FILE *fp = fopen("create.json","w");
    char *buf = cJSON_Print(json);
	log(buf);
    fwrite(buf,strlen(buf),1,fp);
    fclose(fp);
    cJSON_Delete(json);
    return 0;

}
void Json_PrintItemValue(cJSON *Item)
{
	switch(Item->type)
	{
		case cJSON_Object:
			printf("%s:%s\n", Item->string, Item->valuestring);
			break;
		case cJSON_String:
			printf("%s:%s\n", Item->string, Item->valuestring);
			Json_GetAskData(Item->string, Item->valuestring);
			break;
		case cJSON_Number:
			printf("%s:%d\n", Item->string, Item->valueint);
			break;
		case cJSON_False:
			printf("%s:%s\n", Item->string, Item->valuestring);
			break;
		case cJSON_Array:
			printf("%s:%s\n", Item->string, Item->valuestring);
			break;
			
		default:
			perror("Item->type is %d\n", Item->type);
			break;
	}

}

//递归读取JSON数据
int Json_ParseData_recursion(cJSON *parentNode)
{
	int size,i;
	cJSON *tnode = NULL;
	size = cJSON_GetArraySize(parentNode);
	for(i=0;i<size;i++)
	{
		tnode = cJSON_GetArrayItem(parentNode,i);
		int num;
		num = cJSON_GetArraySize(tnode); 
		if(num > 0)
		{
			if(tnode->type == cJSON_Array) printf("Array-%s\n", tnode->string);
			Json_ParseData_recursion(tnode);
		}
		else Json_PrintItemValue(tnode);
	}

}

int Json_DealModbusData(MODBUS_ASK_T askParameter)
{
	char *topic,*playload;
	switch (askParameter._function)
	{
		case ReadCoilStatus:
			playload = Json_ModbusSendCoilStatus(askParameter._startAddr,askParameter._quantity);
			break;
		case ReadInputStatus:
			playload = Json_ModbusSendInputStatus(askParameter._startAddr,askParameter._quantity);
			break;
		case ReadHoldingRegister:
			playload = Json_ModbusSendHoldingRegs(askParameter._startAddr,askParameter._quantity);
			break;
		case ReadInputRegister:
			playload = Json_ModbusSendInputRegs(askParameter._startAddr,askParameter._quantity);
			break;
		default:
			playload = NULL;
			break;
	}
	topic = "/v1/devices/device_03/datas";
	Mqtt_Client_public(topic, playload);

	
}


int Json_ParseData(char *string)
{
    cJSON *json = cJSON_Parse(string);
    cJSON *node = NULL;
	int size;

	if(json == NULL)
	{
		perror("playload is not json type\n");
		return RET_ERROR;
	}
	log("start to parse json data:\n");
	size = cJSON_GetArraySize(json);
	if(size > 0) Json_ParseData_recursion(json);
	else Json_PrintItemValue(json);

	if(Modbus_Ask_T._ask){
		Json_DealModbusData(Modbus_Ask_T);
		Modbus_Ask_T._ask = false;
	}
	
    return RET_SUCESS;

}



int Init_Json_Test()
{
//	Json_Send_Data();
//	Json_Parse_Data();
//	Json_ModbusSendCoilStatus(0,4);
//	Json_ModbusSendInputStatus(0,4);
//	Json_ModbusSendHoldingRegs(2,9);
//	Json_ModbusSendInputRegs(38,9);
//	Json_eSDKGetIPInfo();

	return 0;

}


