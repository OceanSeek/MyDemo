#include "sys.h"

//ȫ�ֱ���
TVars     		gVars;



//======================================================
//	�ⲿ����������
//======================================================
PTDevie gpDevice;
TransTable_T *TransYXTable;
TransTable_T *TransYCTable;
TransTable_T *TransDZTable;
TransTable_T *TransYKTable;
TransTable_T *TransTable_tmp;
Modbus_TransTable_T *TransModbusCoidStatusTable;
Modbus_TransTable_T *TransModbusInputStatusTable;
Modbus_TransTable_T *TransModbusHoldingRegTable;
Modbus_TransTable_T *TransModbusInputRegTable;
Modbus_TransTable_T *TransModbusTable_tmp;

TBurstAISoe *BurstAISoe;
TBurstBISoe *BurstBISoe;

TBurstYXRecord YXBrustRecord[MAXYXRECORD];
TBurstYXRecord DYXBrustRecord[MAXYXRECORD];
TBurstYXRecord YXSoeRecord[MAXYXRECORD];
TBurstYXRecord YXDSoeRecord[MAXYXRECORD];
TBurstYCRecord YCBrustRecord[MAXYCRECORD];
TBurstYCRecord DZBrustRecord[MAXYCRECORD];
TSysTimeAit	AitRecord[MAXYCRECORD];
TSysTimeMr MrRecord[MAXYXRECORD];
TBurstBcd BCDRecord[MAXYCRECORD];



//======================================================
//�ֲ�����������
////======================================================
static uint32_t gdwMaxPortNum; 	//���ݲ����ļ���ȷ�������˿ڸ���



//======================================================
//�ֲ�����������
//======================================================
void InitOtherDbase(void);
int InitPortsTab(void);
int InitDeviceParamter(void);
int InitNetComTab(void);
int InitDeviceTab(void);
int InitDevices(void);
void InitRegulaSOEPtr(uint32_t dwDevID,  TSoeWrap 	*pSoeWrap);
void InitRegulaBurstBIPtr(uint32_t dwDevID, TBurstBIWrap *pBurstBIWrap);
void InitReffer(void);
void WriteData(void);
void ReadData(void);
void Init_Reffer(void);
void Init_ModbusRegAddr(void);
TransTable_T* InitTransTab(char *TableName, uint32_t *tablenum, sqlite3 *db);
Modbus_TransTable_T* InitModbusTransTab(char *TableName, uint32_t *tablenum, sqlite3 *db);




sqlite3* DBase_Open(char *TableName){

	sqlite3 *db = NULL;
	char *zErrMsg = 0;
	int32_t rc;
	char *sql;
	int i=0;


	rc = sqlite3_open(TableName,&db);//�����ݿ⣬��������ھʹ���һ��
	if(rc){
		fprintf(stderr,"can't open database:%s \n",sqlite3_errmsg(db));
		log("can't open database: \n");
		sqlite3_close(db);
		return 0;
	}
	else 
		log("You have open a sqliet3 database named /mnt/internal_storage/dcu/IEC104_data.db successful!\n");
	
	return db;
}

uint8_t ChkSum(uint8_t *buf, int len){
	int i=0;
	uint8_t Sum = 0;
	for(i = 0;i<len;i++){
		Sum += buf[i];
	}
	return Sum;
}



uint8_t GetDevFlag(int DevID,int DevNo){
	return gpDevice[DevID].Flag[DevNo];
}
void ClearDevFlag(int DevID,int DevNo){
	gpDevice[DevID].Flag[DevNo] = FALSE;
}
BOOL SetDevFlag(int DevID, int DevNo){
	if(DevNo>MAX_FLAGNO)
		return FALSE;
	
	gpDevice[DevID].Flag[DevNo] = TRUE;
	return TRUE;
}

void SetDevFlagValue(int    DevID, DWORD dwFlagNo, DWORD dwValue)
{
	if(dwFlagNo >= MAX_DWORDFLAGNO)
		return ;
	gpDevice[DevID].dwFlag[dwFlagNo] = dwValue;
	
}

uint32_t GetDevFlagValue(int    DevID, DWORD dwFlagNo)
{
	if(dwFlagNo >= MAX_DWORDFLAGNO)
		return 0;
	return gpDevice[DevID].dwFlag[dwFlagNo];
	
}

/******************************************************************************/
/** �� ��		GetDev(int DevID)                                           	**/
/** �� �� 	                                                          **/
/***1������ID�ŷ����豸�ṹ��                                                         **/
/** �� ��                                                         **/
/***DevID:                                                       **/
/** ��  ��   2020��11��30��                                                   **/
/******************************************************************************/
PTDevie GetDev(int DevID)
{
	return &gpDevice[DevID];
}

/******************************************************************************/
/** �� ��		GetDevNo(int DevID)                                           	**/
/** �� �� 	                                                          **/
/***1������ID�ŷ����豸���                                                         **/
/** �� ��                                                         **/
/***DevID:                                                       **/
/** ��  ��   2020��11��30��                                                   **/
/******************************************************************************/
int GetDevNo(int DevID)
{
	int i;
	for(i=0;i<gVars.dwDevNum;i++){
		if(DevID == gpDevice[i].ID) return i;
	}
	return RET_ERROR;

}

/******************************************************************************/
/** �� ��		ModifyTransTable(sqlite3 *db, char *TableName)                   **/
/** �� �� 	                                                          	**/
/***1�����ñ�������                                                         	**/
/** �� ��                                                       		  	**/
/***db:��Ҫ�򿪵����ݿ�                                                       	**/
/***TableName:��Ҫ�޸����Եı�����                                     	        **/
/** ��  ��   2020��1��7��                                                    **/
/******************************************************************************/
void ModifyTransTable(sqlite3 *db, char *TableName){
	char *sql;
	char *zErrMsg = 0;
	sql = malloc(200);
	
	sprintf(sql,"begin");
	sqlite3_exec(db,sql,0,0,&zErrMsg);
	
	sprintf(sql,"alter table %s RENAME TO %s_old",TableName,TableName);
	sqlite3_exec(db,sql,0,0,&zErrMsg);
	sprintf(sql,"CREATE TABLE %s(\
				ID INTEGER PRIMARY KEY,\
				wRealID INTEGER,\
				nPoint INTEGER,\
				nIndex INTEGER\
				);",TableName);
	
				
	sqlite3_exec(db,sql,0,0,&zErrMsg);

	sprintf(sql,"INSERT INTO %s(ID,wRealID,nPoint,nIndex) select ID,wRealID,nPoint,nIndex FROM %s_old",TableName,TableName);
	sqlite3_exec(db,sql,0,0,&zErrMsg);
	
	sprintf(sql,"drop table %s_old",TableName);
	sqlite3_exec(db,sql,0,0,&zErrMsg);
	
	sprintf(sql,"commit");
	sqlite3_exec(db,sql,0,0,&zErrMsg);
	
	free(sql);
}

/******************************************************************************/
/** �� ��		ModifyModbusTransTable(sqlite3 *db, char *TableName)                   **/
/** �� �� 	                                                          	**/
/***1�����ñ�������                                                         	**/
/** �� ��                                                       		  	**/
/***db:��Ҫ�򿪵����ݿ�                                                       	**/
/***TableName:��Ҫ�޸����Եı�����                                     	        **/
/** ��  ��   2021��1��6��                                                    **/
/******************************************************************************/
void ModifyModbusTransTable(sqlite3 *db, char *TableName){
	char *sql;
	char *zErrMsg = 0;
	sql = malloc(200);
	
	sprintf(sql,"begin");
	sqlite3_exec(db,sql,0,0,&zErrMsg);
	
	sprintf(sql,"alter table %s RENAME TO %s_old",TableName,TableName);
	sqlite3_exec(db,sql,0,0,&zErrMsg);
	sprintf(sql,"CREATE TABLE %s(\
				ID INTEGER PRIMARY KEY,\
				wRealID INTEGER,\
				nPoint INTEGER,\
				nIndex INTEGER,\
				RegAddr INTEGER,\
				SlaverAddr INTEGER\
				);",TableName);
	
				
	sqlite3_exec(db,sql,0,0,&zErrMsg);

	sprintf(sql,"INSERT INTO %s select * FROM %s_old",TableName,TableName);
//	sprintf(sql,"INSERT INTO %s(ID,wRealID,nPoint,nIndex,RegAddr,SlaverAddr) select ID,wRealID,nPoint,nIndex,RegAddr,SlaverAddr FROM %s_old",TableName,TableName);
	sqlite3_exec(db,sql,0,0,&zErrMsg);
	
	sprintf(sql,"drop table %s_old",TableName);
	sqlite3_exec(db,sql,0,0,&zErrMsg);
	
	sprintf(sql,"commit");
	sqlite3_exec(db,sql,0,0,&zErrMsg);
	
	free(sql);
}

/*******************************************************************  
*���ƣ�      		ModifyTableProperty  
*���ܣ�			����IDΪ������������Ա����Ϊ��Ӧ��������  
*��ڲ�����         
*���ڲ����� 
*�޸����ڣ�2020-6-2
*******************************************************************/ 

void ModifyTableProperty(void){
	sqlite3 *db = NULL;
	db = DBase_Open("/mnt/internal_storage/dcu/IEC104_data.db");
	ModifyTransTable(db, "TransYxTable");
	ModifyTransTable(db, "TransYcTable");
	ModifyTransTable(db, "TransYkTable");
	ModifyTransTable(db, "TransDzTable");
	ModifyModbusTransTable(db, "TransModbusCoidStatusTable");
	ModifyModbusTransTable(db, "TransModbusInputStatusTable");
	ModifyModbusTransTable(db, "TransModbusHoldingRegTable");
	ModifyModbusTransTable(db, "TransModbusInputRegTable");
	sqlite3_close(db);
}

//======================================================
//	��ʼ�����ݿ�
//======================================================
void InitDbase(void)
{
	ModifyTableProperty();
   	InitOtherDbase();
	Init_Reffer();			//��ʼ��ʵ�ʿ�����õ�
	InitReffer();	//5512��ʼ��ʵ�ʿ�����õ�
	Init_ModbusRegAddr();
}
/*******************************************************************  
*���ƣ�      		InitTransTable  
*���ܣ�			��ʼ��ת������  
*��ڲ�����         
*���ڲ����� 
*�޸����ڣ�2020-6-2
*******************************************************************/ 
void InitTransTable(void)
{
	sqlite3 *db = NULL;
	db = DBase_Open("/mnt/internal_storage/dcu/IEC104_data.db");
	TransYXTable = InitTransTab("TransYxTable", &gVars.TransYXTableNum, db);
	TransYCTable = InitTransTab("TransYcTable", &gVars.TransYCTableNum, db);
	TransYKTable = InitTransTab("TransYkTable", &gVars.TransYKTableNum, db);
	TransDZTable = InitTransTab("TransDzTable", &gVars.TransDZTableNum, db);
	TransModbusCoidStatusTable = InitModbusTransTab("TransModbusCoidStatusTable", &gVars.TransModbusCoidStatusTableNum, db);
	TransModbusInputStatusTable = InitModbusTransTab("TransModbusInputStatusTable", &gVars.TransModbusInputStatusTableNum, db);
	TransModbusHoldingRegTable = InitModbusTransTab("TransModbusHoldingRegTable", &gVars.TransModbusHoldingRegTableNum, db);
	TransModbusInputRegTable = InitModbusTransTab("TransModbusInputRegTable", &gVars.TransModbusInputRegTableNum, db);
	
	sqlite3_close(db);
	
//	int i;
//	for(i=0;i<gVars.TransDZTableNum;i++){
//		log("[%d][%d][%d][%d]\n", TransDZTable[i].ID, TransDZTable[i].wRealID, TransDZTable[i].nPoint, TransDZTable[i].nIndex);
//	}

}

/*******************************************************************  
*���ƣ�      		setip  
*���ܣ�			���ñ���IP��ַ  
*��ڲ�����         
*���ڲ�������ȷ����Ϊ0�����󷵻�Ϊ-1 
*�޸����ڣ�2020-6-17
*******************************************************************/ 
int setip(char *ComName, char* ip)
{
    struct ifreq temp;
    struct sockaddr_in *addr;
    int fd = 0;
    int ret = -1;

	if(strcmp(ComName, "NET1") == 0){
		strcpy(temp.ifr_name, "eth0:1");
	}else{
		strcpy(temp.ifr_name, "eth1:1");
	}
    if((fd=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        return -1;
    }
    addr = (struct sockaddr_in *)&(temp.ifr_addr);
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    ret = ioctl(fd, SIOCSIFADDR, &temp);
    close(fd);
    if(ret < 0)
        return -1;
    return 0;
}

void InitOtherDbase(void)
{ 
	InitPortsTab();
	InitDeviceParamter();
	InitTransTable();
	InitDevices();
	InitDeviceTab();
}



/*******************************************************************  
*���ƣ�      		InitPortsTab  
*���ܣ�			��ʼ���豸����  
*��ڲ�����         
*���ڲ�������ȷ����Ϊ0�����󷵻�Ϊ-1 
*�޸����ڣ�2020-6-17
*	���Ӷ�ȡNetComTbale����ȡIP�����ñ���IP��ַ
*******************************************************************/ 
int InitPortsTab(void)
{
	sqlite3 *db = NULL;
	char *zErrMsg = 0;
	int32_t rc;
	char *sql;
	int i=0;
	char ComName[32];
	char ComIP[32];
	db = DBase_Open("/mnt/internal_storage/dcu/IEC104_data.db");

	int32_t nrow=0,ncolumn=0;
	char **azResult;//��ά�����Ž��
	
	sqlite3_exec(db,sql,0,0,&zErrMsg);	

	//READ DeviceTable
	sql = "SELECT * FROM DeviceTable;";
	sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);
	if(nrow == 0){
		perror("DeviceTable is null ,please check DB file path\n");
		exit(-1);
	}
		
	
	gVars.dwDevNum = nrow;
	gpDevice = malloc((sizeof(TDevie)+4)*gVars.dwDevNum);
	memset(gpDevice, 0 ,(sizeof(TDevie)+4)*gVars.dwDevNum);
	if(gpDevice==NULL){
		perror("gpDevice is NULL\n");
		return -1;
	}
	for(i=0;i<nrow;i++){
		gpDevice[i].ID = 			atoi(azResult[(i+1)*ncolumn + DB_ID]);
		gpDevice[i].Address =		atoi(azResult[(i+1)*ncolumn + DB_Address]);
		gpDevice[i].PORT = 			atoi(azResult[(i+1)*ncolumn + DB_PORT]);
		gpDevice[i].SerialBault = 	atoi(azResult[(i+1)*ncolumn + DB_bault]);
		gpDevice[i].SerialDataBit = atoi(azResult[(i+1)*ncolumn + DB_DataBit]);
		gpDevice[i].SerialStopBit = atoi(azResult[(i+1)*ncolumn + DB_StopBit]);
		gpDevice[i].SerialParity = 	atoi(azResult[(i+1)*ncolumn + DB_Parity]);
//		gpDevice[i].MaxSoeBuffer = 	atoi(azResult[(i+1)*ncolumn + DB_MaxSoeBuffer]);
		gpDevice[i].SerialID = 		atoi(azResult[(i+1)*ncolumn + DB_SerialID]);
		gpDevice[i].NetID = 		atoi(azResult[(i+1)*ncolumn + DB_NetID]);
		strcpy(gpDevice[i].Name,azResult[(i+1)*ncolumn + DB_Name]);
		strcpy(gpDevice[i].IP,azResult[(i+1)*ncolumn + DB_IP]);
		strcpy(gpDevice[i].Type,azResult[(i+1)*ncolumn + DB_DevType]);
		strcpy(gpDevice[i].Protocol,azResult[(i+1)*ncolumn + DB_ProtocolType]);
		strcpy(gpDevice[i].Com,azResult[(i+1)*ncolumn + DB_InterfaceType]);
		strcpy(gpDevice[i].TcpType,azResult[(i+1)*ncolumn + DB_TcpType]);
	}
	
//	for(i=0;i<gVars.dwDevNum;i++){
//		LOG("gpDevice[%d].ID is (%d)\n", i, gpDevice[i].ID);
//		LOG("gpDevice[%d].Address is (%d)\n", i, gpDevice[i].Address);
//		LOG("gpDevice[%d].ip is (%s)\n", i, gpDevice[i].IP);
//		LOG("gpDevice[%d].Type is (%s)\n", i, gpDevice[i].Type);
//		LOG("gpDevice[%d].Protocol is (%s)\n", i, gpDevice[i].Protocol);
//		LOG("gpDevice[%d].Com is (%s)\n", i, gpDevice[i].Com);
//		LOG("gpDevice[%d].MaxSoeBuffer is (%d)\n", i, gpDevice[i].MaxSoeBuffer);
//	}
	
	sqlite3_free_table(azResult);

	sqlite3_close(db);
}

void InitWQ900Parameter()
{
	sqlite3 *db = NULL;
	char *zErrMsg = 0;
	int32_t rc;
	char *sql;
	int i=0;
	PTDevie pDevice;
	db = DBase_Open("/mnt/internal_storage/dcu/IEC104_data.db");

	int32_t nrow=0,ncolumn=0;
	char **azResult;//��ά�����Ž��
	sqlite3_exec(db,sql,0,0,&zErrMsg);	
	//READ DeviceTable
	sql = "SELECT * FROM WQ900Table;";
	sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);	
//	log("row:%d,column:%d\n",nrow,ncolumn);
	
	for(i=0;i<nrow;i++){
		TWQ900 WQ900Parameter;
		
//		WQ900Parameter.ID =			atoi(azResult[(i+1)*ncolumn + WQ900_ID]);
		WQ900Parameter.ParentID =	atoi(azResult[(i+1)*ncolumn + WQ900_ParentID]);
		WQ900Parameter.Address =		atoi(azResult[(i+1)*ncolumn + WQ900_Address]);
		WQ900Parameter.AIStart =	atoi(azResult[(i+1)*ncolumn + WQ900_YCStart]);
		WQ900Parameter.BIStart =		atoi(azResult[(i+1)*ncolumn + WQ900_BIStart]);
		WQ900Parameter.DZStart =		atoi(azResult[(i+1)*ncolumn + WQ900_DZStart]);
		WQ900Parameter.DBIStart =		atoi(azResult[(i+1)*ncolumn + WQ900_DBIStart]);
		WQ900Parameter.BOStart =	atoi(azResult[(i+1)*ncolumn + WQ900_YKStart]);
		WQ900Parameter.YMStart =		atoi(azResult[(i+1)*ncolumn + WQ900_YMStart]);
		WQ900Parameter.AINum = 	atoi(azResult[(i+1)*ncolumn + WQ900_AINum]);
		WQ900Parameter.BINum = 		atoi(azResult[(i+1)*ncolumn + WQ900_BINum]);
		WQ900Parameter.BONum = 	atoi(azResult[(i+1)*ncolumn + WQ900_BONum]);
		WQ900Parameter.DZNum = 		atoi(azResult[(i+1)*ncolumn + WQ900_DZNum]);//YM���ɶ�ֵ
		
		int ParentID = WQ900Parameter.ParentID;
		int DevNo;
		DevNo = GetDevNo(ParentID);
		if(DevNo == RET_ERROR){
			perror("DevNo == RET_ERROR,ParentID is %d\n", ParentID);
			return;
		}
		
		pDevice = GetDev(DevNo);
		if(pDevice == NULL){
			perror("ID %d is NULL\n",ParentID);
			return;
		} 
		pDevice->Address = WQ900Parameter.Address;		
		pDevice->AIStart = WQ900Parameter.AIStart;
		pDevice->BIStart = WQ900Parameter.BIStart;
		pDevice->DBIStart = WQ900Parameter.DBIStart;
		pDevice->DZStart = WQ900Parameter.DZStart;
		pDevice->BOStart = WQ900Parameter.BOStart	;
		pDevice->YMStart = WQ900Parameter.YMStart;
		pDevice->AINum = WQ900Parameter.AINum;
		pDevice->BINum = WQ900Parameter.BINum;
		pDevice->BONum = WQ900Parameter.BONum;
		pDevice->DZNum = WQ900Parameter.DZNum;
		
	}
	sqlite3_free_table(azResult);

	sqlite3_close(db);
}

void InitPLCParameter()
{
	sqlite3 *db = NULL;
	char *zErrMsg = 0;
	int32_t rc;
	char *sql;
	int i=0;
	PTDevie pDevice;
	db = DBase_Open("/mnt/internal_storage/dcu/IEC104_data.db");

	int32_t nrow=0,ncolumn=0;
	char **azResult;//��ά�����Ž��
	
	sqlite3_exec(db,sql,0,0,&zErrMsg);	
	//READ DeviceTable
	sql = "SELECT * FROM PLCTable;";
	sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);
//	log("row:%d,column:%d\n",nrow,ncolumn);
	
	for(i=0;i<nrow;i++){
		TPLC PLCParameter;
//		PLCParameter.ID = 		atoi(azResult[(i+1)*ncolumn + PLC_ID]);
		PLCParameter.ParentID =	atoi(azResult[(i+1)*ncolumn + PLC_ParentID]);
		PLCParameter.Address =		atoi(azResult[(i+1)*ncolumn + PLC_Address]);
		PLCParameter.CoiStatusStart =	atoi(azResult[(i+1)*ncolumn + PLC_CoiStatusStart]);
		PLCParameter.InputStatusStart =		atoi(azResult[(i+1)*ncolumn + PLC_InputStatusStart]);
		PLCParameter.HoldingRegStart =		atoi(azResult[(i+1)*ncolumn + PLC_HoldingRegStart]);
		PLCParameter.InputRegStart =	atoi(azResult[(i+1)*ncolumn + PLC_InputRegStart]);
		PLCParameter.CoiStatusNum =		atoi(azResult[(i+1)*ncolumn + PLC_CoiStatusNum]);
		PLCParameter.InputStatusNum =	atoi(azResult[(i+1)*ncolumn + PLC_InputStatusNum]);
		PLCParameter.HoldingRegNum =		atoi(azResult[(i+1)*ncolumn + PLC_HoldingRegNum]);
		PLCParameter.InputRegNum =	atoi(azResult[(i+1)*ncolumn + PLC_InputRegNum]);
		PLCParameter.DestPort =	atoi(azResult[(i+1)*ncolumn + PLC_DestPort]);
		strcpy(PLCParameter.DestIP,azResult[(i+1)*ncolumn + PLC_DestIP]);
		
		int ParentID = PLCParameter.ParentID;
		int DevNo;
		
		DevNo = GetDevNo(ParentID);
		if(DevNo == RET_ERROR){
			perror("DevNo == RET_ERROR");
			return;
		}
		
		pDevice = GetDev(DevNo);
		
		if(pDevice == NULL){
			perror("ID %d is NULL\n",ParentID);
			return;
		} 
		
		pDevice->Address = PLCParameter.Address;		
		pDevice->ModbusData._CoiStatusStart = PLCParameter.CoiStatusStart;
		pDevice->ModbusData._InputStatusStart = PLCParameter.InputStatusStart;
		pDevice->ModbusData._HoldingRegStart = PLCParameter.HoldingRegStart;
		pDevice->ModbusData._InputRegStart = PLCParameter.InputRegStart ;
		pDevice->ModbusData._CoilStatusNum = PLCParameter.CoiStatusNum;
		pDevice->ModbusData._InputStatusNum = PLCParameter.InputStatusNum;
		pDevice->ModbusData._HoldingRegNum = PLCParameter.HoldingRegNum;
		pDevice->ModbusData._InputRegNum = PLCParameter.InputRegNum;
		strcpy(pDevice->UDP_Dest_IP,PLCParameter.DestIP);
		pDevice->UDP_Dest_PORT = PLCParameter.DestPort;

//		log("devid %d  destip:%s dest port:%d \n",ParentID, pDevice->UDP_Dest_IP, pDevice->UDP_Dest_PORT);
		
		
	}
	
	sqlite3_free_table(azResult);
	sqlite3_close(db);

//	for(i=0;i<gVars.dwDevNum;i++){
//		LOG("gpDevice[%d].name is (%s)\n", i, gpDevice[i].Name);
//		LOG("gpDevice[%d].AINum is (%d)\n", i, gpDevice[i].AINum);
//		LOG("gpDevice[%d].BINum is (%d)\n", i, gpDevice[i].BINum);
//		LOG("gpDevice[%d]._CoilStatusNum is (%d)\n", i, gpDevice[i].ModbusData._CoilStatusNum);
//		LOG("gpDevice[%d].Protocol is (%s)\n", i, gpDevice[i].Protocol);
//		LOG("gpDevice[%d].Com is (%s)\n", i, gpDevice[i].Com);
//		LOG("gpDevice[%d].MaxSoeBuffer is (%d)\n\n", i, gpDevice[i].MaxSoeBuffer);
//	}

}


/*******************************************************************  
*���ƣ�      		InitDeviceParamter  
*���ܣ�			��ʼ���豸����  
*��ڲ�����         
*���ڲ�������ȷ����Ϊ0�����󷵻�Ϊ-1 
*�޸����ڣ�2020-11-19
*******************************************************************/ 
int InitDeviceParamter(void)
{
	InitWQ900Parameter();
	InitPLCParameter();
}

/*******************************************************************  
*���ƣ�      		ReadColumnFromDB  
*���ܣ�			��ȡ��������Ԫ�أ�����ת������  
*��ڲ�����         
*	@db 			���ݿ��ļ�������
*	@RowNum 		��ȡ��Ԫ��������
*	@TransTable_tmp	��ʱ�洢�ռ�
*	@ColumnName		������
*	@TableName		��������
*���ڲ�������ȷ����Ϊ0�����󷵻�Ϊ-1 
*˵����	
*	�����е�ID��ű����0��ʼ������������������ȡ����
*******************************************************************/ 

int ReadColumnFromDB(sqlite3 *db,uint32_t RowNum, PTransTable_T TransTable, char *ColumnName, char *TableName)
{

	uint32_t Len = 0;
	uint32_t i = 0;
	char *zErrMsg = 0;
	uint32_t RowTemp;
	int32_t nrow=0,ncolumn=0;
	char **azResult;//��ά�����Ž��
	char *sql;
	uint32_t j = 0;//ѭ������
	int MaxIDNum;
	RowTemp = RowNum;
	sql = malloc(100);
	
	if(RowTemp > MAX_ROW){
		
		do{
			j++;
			RowTemp -= MAX_ROW;
		
			sprintf(sql,"SELECT %s FROM %s WHERE ID BETWEEN %d AND %d;", ColumnName, TableName, Len, Len+MAX_ROW-1);
//			log("%s\n",sql);
			sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);
			for(i=0;i<MAX_ROW;i++){
				if(azResult[i+1] == NULL){
						perror("tablename(%s)value is null (%d)\n", TableName, i+Len);
						return -1;
				}
				if(strcmp("wRealID", ColumnName) == 0){
					TransTable[i+Len].wRealID = atoi(azResult[i+1]);
//					log("i[%d],wRealID(%d)\n",i+Len,TransTable[i+Len].wRealID);
				}else if(strcmp("nPoint", ColumnName) == 0){
					TransTable[i+Len].nPoint = atoi(azResult[i+1]);
				}else if(strcmp("nIndex", ColumnName) == 0){
					TransTable[i+Len].nIndex = atoi(azResult[i+1]);
//					log("i[%d],nIndex(%d)\n",i+Len,TransTable[i+Len].nIndex);
				}else{
					PERROR("ERROR___________FUNCTION(%s)LINE(%d)\n",__FUNCTION__,__LINE__);
					return -1;
				}
			}
			sqlite3_free_table(azResult);
			Len += MAX_ROW;
		}while(RowTemp > MAX_ROW);

	}
	sprintf(sql,"SELECT MAX(ID) FROM %s;", TableName);
	sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);
	MaxIDNum = atoi(azResult[(1)*ncolumn+0]) + 1;
	
	sprintf(sql,"SELECT %s FROM %s WHERE ID BETWEEN %d AND %d;", ColumnName, TableName, Len, MaxIDNum);
//	log("%s\n",sql);
	sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);

	for(i=0;i<RowTemp;i++){
		if(azResult[i+1] == NULL){
				perror("value is null (%d)\n",i+Len);
				return -1;
		}
		if(strcmp("wRealID", ColumnName) == 0){
			TransTable[i+Len].wRealID = atoi(azResult[i+1]);
		}else if(strcmp("nPoint", ColumnName) == 0){
			TransTable[i+Len].nPoint = atoi(azResult[i+1]);
		}else if(strcmp("nIndex", ColumnName) == 0){
			TransTable[i+Len].nIndex = atoi(azResult[i+1]);
		}else{
			PERROR("ERROR___________FUNCTION(%s)LINE(%d)\n",__FUNCTION__,__LINE__);
			return -1;
		}
	}

	sqlite3_free_table(azResult);

	free(sql);
	return 0;
}

int ReadModbusColumnFromDB(sqlite3 *db,uint32_t RowNum, PModbus_TransTable_T TransTable, char *ColumnName, char *TableName)
{

	uint32_t Len = 0;
	uint32_t i = 0;
	char *zErrMsg = 0;
	uint32_t RowTemp;
	int32_t nrow=0,ncolumn=0;
	char **azResult;//��ά�����Ž��
	char *sql;
	uint32_t j = 0;//ѭ������
	int MaxIDNum;
	RowTemp = RowNum;
	sql = malloc(100);
	
	if(RowTemp > MAX_ROW){
		
		do{
			j++;
			RowTemp -= MAX_ROW;
		
			sprintf(sql,"SELECT %s FROM %s WHERE ID BETWEEN %d AND %d;", ColumnName, TableName, Len, Len+MAX_ROW-1);
//			log("%s\n",sql);
			sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);
			for(i=0;i<MAX_ROW;i++){
				if(azResult[i+1] == NULL){
						perror("tablename(%s)value is null (%d)\n", TableName, i+Len);
						return -1;
				}
				if(strcmp("wRealID", ColumnName) == 0){
					TransTable[i+Len].wRealID = atoi(azResult[i+1]);
//					log("i[%d],wRealID(%d)\n",i+Len,TransTable[i+Len].wRealID);
				}
				else if(strcmp("nPoint", ColumnName) == 0){
					TransTable[i+Len].nPoint = atoi(azResult[i+1]);
				}
				else if(strcmp("nIndex", ColumnName) == 0){
					TransTable[i+Len].nIndex = atoi(azResult[i+1]);
//					log("i[%d],nIndex(%d)\n",i+Len,TransTable[i+Len].nIndex);
				}
				else if(strcmp("RegAddr", ColumnName) == 0){
					TransTable[i+Len].RegAddr = atoi(azResult[i+1]);
//					log("i[%d],RegAddr(%d)\n",i+Len,TransTable[i+Len].RegAddr);
				}
				else if(strcmp("SlaverAddr", ColumnName) == 0){
					TransTable[i+Len].SlaverAddr = atoi(azResult[i+1]);
				}
				else{
					PERROR("ERROR___________FUNCTION(%s)LINE(%d)\n",__FUNCTION__,__LINE__);
					return -1;
				}
			}
			sqlite3_free_table(azResult);
			Len += MAX_ROW;
		}while(RowTemp > MAX_ROW);

	}
	sprintf(sql,"SELECT MAX(ID) FROM %s;", TableName);
	sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);
	MaxIDNum = atoi(azResult[(1)*ncolumn+0]) + 1;
	
	sprintf(sql,"SELECT %s FROM %s WHERE ID BETWEEN %d AND %d;", ColumnName, TableName, Len, MaxIDNum);
//	log("%s\n",sql);
	sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);

	for(i=0;i<RowTemp;i++){
		if(azResult[i+1] == NULL){
				perror("value is null (%d)\n",i+Len);
				return -1;
		}
		if(strcmp("wRealID", ColumnName) == 0){
			TransTable[i+Len].wRealID = atoi(azResult[i+1]);
		}
		else if(strcmp("nPoint", ColumnName) == 0){
			TransTable[i+Len].nPoint = atoi(azResult[i+1]);
		}
		else if(strcmp("nIndex", ColumnName) == 0){
			TransTable[i+Len].nIndex = atoi(azResult[i+1]);
		}
		else if(strcmp("RegAddr", ColumnName) == 0){
			TransTable[i+Len].RegAddr = atoi(azResult[i+1]);
		}
		else if(strcmp("SlaverAddr", ColumnName) == 0){
			TransTable[i+Len].SlaverAddr = atoi(azResult[i+1]);
		}
		
		else{
			PERROR("ERROR___________FUNCTION(%s)LINE(%d)\n",__FUNCTION__,__LINE__);
			return -1;
		}
	}

	sqlite3_free_table(azResult);

	free(sql);
	return 0;
}


/*******************************************************************  
*���ƣ�      		ReadColumn  
*���ܣ�			��ȡ��������Ԫ�أ�����ת������  
*��ڲ�����         
*	@db 			���ݿ��ļ�������
*	@RowNum 		��ȡ��Ԫ��������
*	@TransTable_tmp	�����洢ָ��
*	@ColumnName		������
*	@TableName		��������
*���ڲ�������ȷ����Ϊ0�����󷵻�Ϊ-1 
*˵����	
*	�����е�ID��ű����0��ʼ������������������ȡ����
*******************************************************************/ 
int ReadColumn(sqlite3 *db,uint32_t RowNum, PTransTable_T TransTable, char *TableName)
{
	int ret = 0;
	ret = ReadColumnFromDB(db, RowNum, TransTable, "wRealID", TableName);
	ret = ReadColumnFromDB(db, RowNum, TransTable, "nPoint", TableName);
	ret = ReadColumnFromDB(db, RowNum, TransTable, "nIndex", TableName);
	return ret;
}

int ReadModbusColumn(sqlite3 *db,uint32_t RowNum, PModbus_TransTable_T TransTable, char *TableName)
{
	int ret = 0;
	ret = ReadModbusColumnFromDB(db, RowNum, TransTable, "wRealID", TableName);
	ret = ReadModbusColumnFromDB(db, RowNum, TransTable, "nPoint", TableName);
	ret = ReadModbusColumnFromDB(db, RowNum, TransTable, "nIndex", TableName);
	ret = ReadModbusColumnFromDB(db, RowNum, TransTable, "RegAddr", TableName);
	ret = ReadModbusColumnFromDB(db, RowNum, TransTable, "SlaverAddr", TableName);
	return ret;
}

/*******************************************************************  
*���ƣ�				InitPortsTab  
*���ܣ�			��ʼ��ת������  
*��ڲ�����		
*	TableName:	������
*	tablenum:	�������index��
*	db:			���ݿ���
*���ڲ�����
*	PTableName:	���ر���ָ��
*�޸����ڣ�2020-5-2
*	ת����������<���͵������������������͵��������ת�����ռ� 
*�޸����ڣ�2020-6-2
* 	���Ӳ��� PTableNameָ�룬�洢����ָ��
*******************************************************************/ 
TransTable_T* InitTransTab(char *TableName, uint32_t *tablenum, sqlite3 *db)
{
	char *zErrMsg = 0;
	int32_t rc;
	char *sql;
	uint32_t i=0;
	uint32_t ret_rownum = 0;//ת����������
	uint32_t ret_tablenum = 0;//ת����index����
	int ret = 0;
	TransTable_T *PTableName;
	
	int32_t nrow=0,ncolumn=0;
	char **azResult;//��ά�����Ž��
	
	sql = malloc(100);
//	log("TableName %s____ \n",TableName);
	sprintf(sql,"SELECT COUNT(*) FROM %s;", TableName);
	sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);
//	log("nrow %d \n",nrow);
	if(nrow == 0){
		perror("%s is null ,please check DB file path\n", TableName);
		exit(-1);
	}
	ret_rownum = atoi(azResult[(1)*ncolumn+0]);
//	log("ret_rownum %d \n",ret_rownum);
	if(ret_rownum == 0){
		perror("%s is null ,please check Table \n", TableName);
		exit(-1);
	}
	
	sprintf(sql,"SELECT MAX(nIndex) FROM %s;", TableName);
	sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);
	ret_tablenum = atoi(azResult[(1)*ncolumn+0]) + 1;
//	log("ret_tablenum is %d ret_rownum is %d\n",ret_tablenum, ret_rownum);

	
	if(ret_tablenum < ret_rownum){
		perror("%s cannot found primary key ,please go to function (ModifyTransTable)\n",TableName);//����ȱ����������Ҫȥ������ModifyTransTable���������¼���ı���
		exit(-1);
	}
	TransTable_tmp = (TransTable_T *)malloc(sizeof(TransTable_T)*ret_rownum);
	memset(TransTable_tmp, 0, sizeof(TransTable_T)*ret_rownum); 
	PTableName = (TransTable_T *)malloc(sizeof(TransTable_T)*ret_tablenum);
	if(PTableName == NULL || PTableName == 0){
		perror("P%s = NULL\n", TableName);
		exit(-1);
	}
	memset(PTableName, 0, sizeof(TransTable_T)*ret_tablenum); 
	ReadColumn(db, ret_rownum, TransTable_tmp, TableName);
	for(i=0;i<ret_rownum;i++){
		PTableName[TransTable_tmp[i].nIndex].wRealID = TransTable_tmp[i].wRealID;
		PTableName[TransTable_tmp[i].nIndex].nPoint = TransTable_tmp[i].nPoint;
		PTableName[TransTable_tmp[i].nIndex].nIndex = TransTable_tmp[i].nIndex;
	}
	
	for(i=0;i<ret_tablenum;i++){
		PTableName[i].ID = i;
	}
	*tablenum = ret_tablenum;
	free(TransTable_tmp);
	free(sql);
	sqlite3_free_table(azResult);
	return PTableName;

}

/*******************************************************************  
*���ƣ�				InitModbusTransTab  
*���ܣ�			��ʼ��ת������  
*��ڲ�����		
*	TableName:	������
*	tablenum:	�������index��
*	db:			���ݿ���
*���ڲ�����
*	PTableName:	���ر���ָ��
*******************************************************************/ 
Modbus_TransTable_T* InitModbusTransTab(char *TableName, uint32_t *tablenum, sqlite3 *db)
{
	char *zErrMsg = 0;
	int32_t rc;
	char *sql;
	uint32_t i=0;
	uint32_t ret_rownum = 0;//ת����������
	uint32_t ret_tablenum = 0;//ת����index����
	int ret = 0;
	Modbus_TransTable_T *PTableName;
	
	int32_t nrow=0,ncolumn=0;
	char **azResult;//��ά�����Ž��
	
	sql = malloc(100);
//	log("TableName %s____ \n",TableName);
	sprintf(sql,"SELECT COUNT(*) FROM %s;", TableName);
	sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);
//	log("nrow %d \n",nrow);
	if(nrow == 0){
		perror("%s is null ,please check DB file path\n", TableName);
		exit(-1);
	}
	ret_rownum = atoi(azResult[(1)*ncolumn+0]);
//	log("ret_rownum %d \n",ret_rownum);
	if(ret_rownum == 0){
		perror("%s is null ,please check Table \n", TableName);
		exit(-1);
	}
	
	sprintf(sql,"SELECT MAX(nIndex) FROM %s;", TableName);
	sqlite3_get_table(db,sql,&azResult,&nrow,&ncolumn,&zErrMsg);
	ret_tablenum = atoi(azResult[(1)*ncolumn+0]) + 1;
//	log("ret_tablenum is %d ret_rownum is %d\n",ret_tablenum, ret_rownum);

	
	if(ret_tablenum < ret_rownum){
		perror("%s cannot found primary key ,please go to function (ModifyTransTable)\n",TableName);//����ȱ����������Ҫȥ������ModifyTransTable���������¼���ı���
		exit(-1);
	}
	TransModbusTable_tmp = (Modbus_TransTable_T *)malloc(sizeof(Modbus_TransTable_T)*ret_rownum);
	memset(TransModbusTable_tmp, 0, sizeof(Modbus_TransTable_T)*ret_rownum); 
	PTableName = (Modbus_TransTable_T *)malloc(sizeof(Modbus_TransTable_T)*ret_tablenum);
	if(PTableName == NULL || PTableName == 0){
		perror("P%s = NULL\n", TableName);
		exit(-1);
	}
	memset(PTableName, 0, sizeof(Modbus_TransTable_T)*ret_tablenum); 
	ReadModbusColumn(db, ret_rownum, TransModbusTable_tmp, TableName);
	for(i=0;i<ret_rownum;i++){
		PTableName[TransModbusTable_tmp[i].nIndex].wRealID = TransModbusTable_tmp[i].wRealID;
		PTableName[TransModbusTable_tmp[i].nIndex].nPoint = TransModbusTable_tmp[i].nPoint;
		PTableName[TransModbusTable_tmp[i].nIndex].nIndex = TransModbusTable_tmp[i].nIndex;
		PTableName[TransModbusTable_tmp[i].nIndex].RegAddr = TransModbusTable_tmp[i].RegAddr;
		PTableName[TransModbusTable_tmp[i].nIndex].SlaverAddr = TransModbusTable_tmp[i].SlaverAddr;
	}
	
	for(i=0;i<ret_tablenum;i++){
		PTableName[i].ID = i;
	}
	*tablenum = ret_tablenum;
	free(TransModbusTable_tmp);
	free(sql);
	sqlite3_free_table(azResult);
	return PTableName;

}



int InitDeviceTab(void)
{
	
	return 1;
}

void Init_ModbusMemery(int DevNo)
{
	//�Ĵ���״ֵ̬
	gpDevice[DevNo].ModbusData.pCoilStatus = (bool*)malloc(gpDevice[DevNo].ModbusData._CoilStatusNum);
	gpDevice[DevNo].ModbusData.pInputStatus = (bool*)malloc(gpDevice[DevNo].ModbusData._InputStatusNum);
	gpDevice[DevNo].ModbusData.pHoldingRegister = (uint16_t*)malloc(gpDevice[DevNo].ModbusData._HoldingRegNum*sizeof(uint16_t));
	gpDevice[DevNo].ModbusData.pInputResgister = (uint16_t*)malloc(gpDevice[DevNo].ModbusData._InputRegNum*sizeof(uint16_t));
	memset(gpDevice[DevNo].ModbusData.pCoilStatus, 0, gpDevice[DevNo].ModbusData._CoilStatusNum*sizeof(bool));
	memset(gpDevice[DevNo].ModbusData.pInputStatus, 0, gpDevice[DevNo].ModbusData._InputStatusNum*sizeof(bool));
	memset(gpDevice[DevNo].ModbusData.pHoldingRegister, 0, gpDevice[DevNo].ModbusData._HoldingRegNum*sizeof(uint16_t));
	memset(gpDevice[DevNo].ModbusData.pInputResgister, 0, gpDevice[DevNo].ModbusData._InputRegNum*sizeof(uint16_t));
	//�ٻ���־
	gpDevice[DevNo].ModbusData.pFlag_AskCoilStatus = (bool*)malloc(gpDevice[DevNo].ModbusData._CoilStatusNum);
	gpDevice[DevNo].ModbusData.pFlag_AskInputStatus = (bool*)malloc(gpDevice[DevNo].ModbusData._InputStatusNum);
	gpDevice[DevNo].ModbusData.pFlag_AskHoldingRegister = (bool*)malloc(gpDevice[DevNo].ModbusData._HoldingRegNum);
	gpDevice[DevNo].ModbusData.pFlag_AskInputResgister = (bool*)malloc(gpDevice[DevNo].ModbusData._InputRegNum);
	memset(gpDevice[DevNo].ModbusData.pFlag_AskCoilStatus, 0, gpDevice[DevNo].ModbusData._CoilStatusNum*sizeof(bool));
	memset(gpDevice[DevNo].ModbusData.pFlag_AskInputStatus, 0, gpDevice[DevNo].ModbusData._InputStatusNum*sizeof(bool));
	memset(gpDevice[DevNo].ModbusData.pFlag_AskHoldingRegister, 0, gpDevice[DevNo].ModbusData._HoldingRegNum*sizeof(bool));
	memset(gpDevice[DevNo].ModbusData.pFlag_AskInputResgister, 0, gpDevice[DevNo].ModbusData._InputRegNum*sizeof(bool));
	//�Ĵ�����ַ
	gpDevice[DevNo].ModbusData.pCoiStatus_T = (MODBUSDEVICEDATA_T*)malloc(gpDevice[DevNo].ModbusData._CoilStatusNum*sizeof(MODBUSDEVICEDATA_T));
	gpDevice[DevNo].ModbusData.pInputStatus_T = (MODBUSDEVICEDATA_T*)malloc(gpDevice[DevNo].ModbusData._InputStatusNum*sizeof(MODBUSDEVICEDATA_T));
	gpDevice[DevNo].ModbusData.pHoldingRegister_T = (MODBUSDEVICEDATA_T*)malloc(gpDevice[DevNo].ModbusData._HoldingRegNum*sizeof(MODBUSDEVICEDATA_T));
	gpDevice[DevNo].ModbusData.pInputResgister_T = (MODBUSDEVICEDATA_T*)malloc(gpDevice[DevNo].ModbusData._InputRegNum*sizeof(MODBUSDEVICEDATA_T));
	memset(gpDevice[DevNo].ModbusData.pCoiStatus_T, 0, gpDevice[DevNo].ModbusData._CoilStatusNum*sizeof(MODBUSDEVICEDATA_T));
	memset(gpDevice[DevNo].ModbusData.pInputStatus_T, 0, gpDevice[DevNo].ModbusData._InputStatusNum*sizeof(MODBUSDEVICEDATA_T));
	memset(gpDevice[DevNo].ModbusData.pHoldingRegister_T, 0, gpDevice[DevNo].ModbusData._HoldingRegNum*sizeof(MODBUSDEVICEDATA_T));
	memset(gpDevice[DevNo].ModbusData.pInputResgister_T, 0, gpDevice[DevNo].ModbusData._InputRegNum*sizeof(MODBUSDEVICEDATA_T));
}


/***************
���ܣ�
	�����ڴ�ռ�
������
����ֵ��
	0:�ɹ�,����:����
�޸����ڣ�
2020/02/10
	�ƻ���վ�豸ң��ң��������Ϊ0�����������ڴ�ʱ���ɰ�����ȡ�������豸�������ڴ棬����վ�豸ң����Ϊ0ʱ����ʼ��ת�����ʱ
��վ���豸�ͱ��������޸ģ�Ӧ��ΪֻΪ�ն��豸�����ڴ�
***************/
int InitDevices(void)
{
//	PTDevie Device;
	int dwDevNo;
	int i;
	
	if(gVars.dwDevNum > MAX_DEVEICE_NUM){
		PERROR("DeviceNum > MAX_DEVEICE_NUM\n");
		return -1;
	}
	
	for(dwDevNo=0;dwDevNo<gVars.dwDevNum;dwDevNo++){

		if(gpDevice[dwDevNo].BINum)
		{
//			log("____sizeof(TBurstBI)(%d)\n",sizeof(TBurstBI));
			gpDevice[dwDevNo].pBurstBI=(TBurstBI*)malloc(gpDevice[dwDevNo].BINum* (sizeof(TBurstBI)));
			memset(gpDevice[dwDevNo].pBurstBI,0,gpDevice[dwDevNo].BINum * (sizeof(TBurstBI)));
			gpDevice[dwDevNo].pSysTimeSoe=(TSysTimeSoe*)malloc(gpDevice[dwDevNo].BINum* sizeof(TSysTimeSoe));
			memset(gpDevice[dwDevNo].pSysTimeSoe,0,gpDevice[dwDevNo].BINum * sizeof(TSysTimeSoe));
			gpDevice[dwDevNo].pSoeWrap=(TSoeWrap*)malloc(sizeof(TSoeWrap));
			memset(gpDevice[dwDevNo].pSoeWrap,0,sizeof(TSoeWrap));
			

			
		}

//		if(strcmp("zhuzhan", gpDevice[dwDevNo].Type) == 0){
//			gpDevice[dwDevNo].pLogicBase=(TLogicBase*)malloc(sizeof(TLogicBase));
//			memset(gpDevice[dwDevNo].pLogicBase,0,sizeof(TLogicBase));
//			
//			gpDevice[dwDevNo].pLogicBase->pLogicBI=(TLogicBI*)malloc(gVars.TransYXTableNum * sizeof(TLogicBI));
//			memset(gpDevice[dwDevNo].pLogicBase->pLogicBI, 0, gVars.TransYXTableNum * sizeof(TLogicBI));
//			if(gpDevice[dwDevNo].pLogicBase->pLogicBI == NULL)
//				perror("gpDevice[DevIDNum].pLogicBase->pLogicBI == NULL");
//			
//			gpDevice[dwDevNo].pLogicBase->pLogicAI=(TLogicAI*)malloc(gVars.TransYCTableNum * sizeof(TLogicAI));
//			memset(gpDevice[dwDevNo].pLogicBase->pLogicAI, 0, gVars.TransYCTableNum * sizeof(TLogicAI));
//			
//			log("gVars.TransYXTableNum(%d) TransYCTableNum(%d)devid(%d)\n",gVars.TransYXTableNum, gVars.TransYCTableNum, dwDevNo);
//
//		}

		
		if(gpDevice[dwDevNo].AINum)
		{
			gpDevice[dwDevNo].pBurstAI=(TBurstAI*)malloc(gpDevice[dwDevNo].AINum * sizeof(TBurstAI));//ң��
			memset(gpDevice[dwDevNo].pBurstAI,0,gpDevice[dwDevNo].AINum * sizeof(TBurstAI));
			gpDevice[dwDevNo].pAI=(DWORD*)malloc(gpDevice[dwDevNo].AINum * sizeof(DWORD));//ң��
			memset(gpDevice[dwDevNo].pAI,0,gpDevice[dwDevNo].AINum * sizeof(int32_t));
			gpDevice[dwDevNo].pAIStatus=(uint16_t*)malloc(gpDevice[dwDevNo].AINum * sizeof(uint16_t));
			memset(gpDevice[dwDevNo].pAIStatus,0,gpDevice[dwDevNo].AINum * sizeof(uint16_t));
			
			gpDevice[dwDevNo].pLogicBase=(TLogicBase*)malloc(sizeof(TLogicBase));
			memset(gpDevice[dwDevNo].pLogicBase,0,sizeof(TLogicBase));
			
			gpDevice[dwDevNo].pLogicBase->pLogicAI=(TLogicAI*)malloc(gpDevice[dwDevNo].AINum * sizeof(TLogicAI));
			memset(gpDevice[dwDevNo].pLogicBase->pLogicAI, 0, gpDevice[dwDevNo].AINum * sizeof(TLogicAI));

			gpDevice[dwDevNo].pRealBase=(TRealBase*)malloc(sizeof(TRealBase));
			memset(gpDevice[dwDevNo].pRealBase,0,sizeof(TRealBase));

			gpDevice[dwDevNo].pRealBase->pRealAI=(TRealAI*)malloc(gpDevice[dwDevNo].AINum * sizeof(TRealAI));
			memset(gpDevice[dwDevNo].pRealBase->pRealAI, 0, gpDevice[dwDevNo].AINum * sizeof(TRealAI));
	
			
		}

		if(gpDevice[dwDevNo].DZNum)
		{
			gpDevice[dwDevNo].pDZ=(uint32_t*)malloc(gpDevice[dwDevNo].DZNum * sizeof(uint32_t));
			memset(gpDevice[dwDevNo].pDZ,0,gpDevice[dwDevNo].DZNum * sizeof(uint32_t));
			
			gpDevice[dwDevNo].pBurstDZ=(TBurstDZ*)malloc(gpDevice[dwDevNo].DZNum * sizeof(TBurstDZ));
			memset(gpDevice[dwDevNo].pBurstDZ,0,gpDevice[dwDevNo].DZNum * sizeof(TBurstDZ));
//			log("sizeof(TBurstDZ) (%d)\n",sizeof(TBurstDZ));
		}

		if(strcmp("PLC", gpDevice[dwDevNo].Name) == 0){
			Init_ModbusMemery(dwDevNo);
		}
		
	}
	return 0;

}

void InitRegulaSOEPtr(uint32_t dwDevID,  TSoeWrap 	*pSoeWrap)
{
	TPtr *pPtr=&pSoeWrap->Ptr;
	uint32_t Diff=pPtr->dwWritePtr - pPtr->dwReadPtr;//pSoeWrap->dwSoeReadPtr;

	if(Diff >= gpDevice[dwDevID].MaxSoeBuffer)
	{
		pPtr->dwWritePtr=0;
		pPtr->dwReadPtr=0;
	}
}

void InitRegulaBurstBIPtr(uint32_t dwDevID, TBurstBIWrap *pBurstBIWrap)
{
	TPtr *pPtr=&pBurstBIWrap->Ptr;
	uint32_t Diff=pPtr->dwWritePtr - pPtr->dwReadPtr;

	if(Diff >= gpDevice[dwDevID].MaxSoeBuffer)
	{
		pPtr->dwWritePtr=0;
		pPtr->dwReadPtr=0;
	}
}

//��ʼ��ת�����
void Init_Reffer(void)
{
	int i;
	int DevIDNum;

	PTDevie pDevice;
	
//	for(i=0;i<gVars.TransYXTableNum;i++){//
//		gpDevice[TransYXTable[i].wRealID].pBurstBI[TransYXTable[i].nPoint].nNo = TransYXTable[i].nIndex;//�������ڵ�Ź���
//		gpDevice[TransYXTable[i].wRealID].pBurstBI[TransYXTable[i].nPoint].ID = TransYXTable[i].ID;
//		gpDevice[TransYXTable[i].wRealID].DeviceID= TransYXTable[i].wRealID;//�豸�ų�ʼ��
//		LOG("[%d].nflag(%d)nPoint(%d)nidex(%d)\n",i,TransYXTable[i].wRealID,TransYXTable[i].nPoint,\
//			TransYXTable[i].nIndex);
//	}
//	log("gVars.TransYCTableNum is %d \n",gVars.TransYCTableNum);
	for(i=0;i<gVars.TransYCTableNum;i++){
		int RealID = TransYCTable[i].wRealID;
		int nPoint = TransYCTable[i].nPoint;
		if(RealID >= gVars.dwDevNum) continue;
		int DevNo;
		DevNo = GetDevNo(RealID);
		pDevice = GetDev(DevNo);
		if(pDevice == NULL){
			log("i %d  DevNo %d\n", i, DevNo);
			perror("pDevice == NULL\n");
			continue;
		}
			
		if(strcmp("WQ900", pDevice->Name) != 0) continue;
		
		if(pDevice->pBurstAI == NULL){
			perror("YC is NULL,please check DevType or YCNum");
			return;
		}
		
		pDevice->pBurstAI[nPoint].nNo = TransYCTable[i].nIndex;//�豸�ڵ�Ŷ�Ӧ��ת�����
		pDevice->pBurstAI[nPoint].nIndex= TransYCTable[i].ID;//�������ڵ�Ź���

	}
	for(i=0;i<gVars.TransDZTableNum;i++){
		int RealID = TransDZTable[i].wRealID;
		int nPoint = TransDZTable[i].nPoint;
	
		if(RealID >= gVars.dwDevNum) continue;
		pDevice = GetDev(RealID);
		if(pDevice == NULL){
//			perror("pDevice == NULL\n");
			continue;
		}
		
		if(strcmp("WQ900", pDevice->Name) != 0) continue;
		pDevice->pBurstDZ[nPoint].nNo = TransDZTable[i].nIndex;//�豸�ڵ�Ŷ�Ӧ��ת�����

	}
	//��ʼ����վģ��ת����
	for(DevIDNum=0;DevIDNum<gVars.dwDevNum;DevIDNum++){
		if(strcmp("Logic", gpDevice[DevIDNum].Type) == 0){
			if(strcmp("WQ900", gpDevice[DevIDNum].Name) != 0) continue;
			
			/*�����ڴ�*/
			/*�߼�����ַ*/
			gpDevice[DevIDNum].pLogicBase=(TLogicBase*)malloc(sizeof(TLogicBase));
			memset(gpDevice[DevIDNum].pLogicBase,0,sizeof(TLogicBase));
			/*ң��*/
			gpDevice[DevIDNum].pLogicBase->pLogicBI=(TLogicBI*)malloc(gVars.TransYXTableNum * sizeof(TLogicBI));
			memset(gpDevice[DevIDNum].pLogicBase->pLogicBI, 0, gVars.TransYXTableNum * sizeof(TLogicBI));
			//˫��ң��
			gpDevice[DevIDNum].pLogicBase->pLogicDBI=(TLogicDBI*)malloc(gVars.TransYXTableNum * sizeof(TLogicDBI));
			memset(gpDevice[DevIDNum].pLogicBase->pLogicDBI, 0, gVars.TransYXTableNum * sizeof(TLogicDBI));
			/*ң��*/
			gpDevice[DevIDNum].pLogicBase->pLogicAI=(TLogicAI*)malloc(gVars.TransYCTableNum * sizeof(TLogicAI));
			memset(gpDevice[DevIDNum].pLogicBase->pLogicAI, 0, gVars.TransYCTableNum * sizeof(TLogicAI));
			//�����
			gpDevice[DevIDNum].dwCINum = 100;
			gpDevice[DevIDNum].pLogicBase->pLogicCI=(TLogicCI*)malloc(gpDevice[DevIDNum].dwCINum * sizeof(TLogicCI));
			memset(gpDevice[DevIDNum].pLogicBase->pLogicCI, 0, gpDevice[DevIDNum].dwCINum * sizeof(TLogicCI));
			//BCD
			gpDevice[DevIDNum].dwBCDNum = 100;
			gpDevice[DevIDNum].pLogicBase->pLogicBCD=(TLogicBCD*)malloc(gpDevice[DevIDNum].dwBCDNum * sizeof(TLogicBCD));
			memset(gpDevice[DevIDNum].pLogicBase->pLogicBCD, 0, gpDevice[DevIDNum].dwBCDNum * sizeof(TLogicBCD));
			/*��ֵ*/
			gpDevice[DevIDNum].pLogicBase->pLogicDZ=(TLogicDZ*)malloc(gVars.TransDZTableNum * sizeof(TLogicDZ));
			memset(gpDevice[DevIDNum].pLogicBase->pLogicDZ, 0, gVars.TransDZTableNum * sizeof(TLogicDZ));
			
			/*��վ�߼��豸��ʼ�����������豸���*/
			for(i=0;i<gVars.TransYCTableNum;i++){
				int wRealID = TransYCTable[i].wRealID;
				int nPoint = TransYCTable[i].nPoint;
				if(strcmp("WQ900", gpDevice[wRealID].Name) != 0) continue;
//				log("i %d\n",i);
				gpDevice[DevIDNum].pLogicBase->pLogicAI[i].wRealID = wRealID;
				gpDevice[DevIDNum].pLogicBase->pLogicAI[i].wOffset = nPoint;
			}
			for(i=0;i<gVars.TransYXTableNum;i++){
				int wRealID = TransYXTable[i].wRealID;
				int nPoint = TransYXTable[i].nPoint;
				if(strcmp("WQ900", gpDevice[wRealID].Name) != 0) continue;
//				log("i %d\n",i);
				gpDevice[DevIDNum].pLogicBase->pLogicBI[i].wRealID = wRealID;
				gpDevice[DevIDNum].pLogicBase->pLogicBI[i].wOffset = nPoint;
			}

			for(i=0;i<gVars.TransDZTableNum;i++){
				int wRealID = TransDZTable[i].wRealID;
				int nPoint = TransDZTable[i].nPoint;
				if(strcmp("WQ900", gpDevice[wRealID].Name) != 0) continue;
//				log("i %d\n",i);
				gpDevice[DevIDNum].pLogicBase->pLogicDZ[i].wRealID = wRealID;
				gpDevice[DevIDNum].pLogicBase->pLogicDZ[i].wOffset = nPoint;
			}
		}
	}


	
}
/***************
���ܣ�
	����豸ID������Ƿ�Խ��
������
	DevID,�ն�ID
	nPoint,�ڵ��
	MaxDevNum ����豸��
	MaxPointNum	������
����ֵ��
	0:�ɹ�,����:����
�޸����ڣ�
***************/

int CheckError(uint8_t DevID,uint32_t nPoint, uint8_t MaxDevNum, uint32_t MaxPointNum)
{
	if(DevID > MaxDevNum){//�豸�Ŵ�������豸���򱨴�
		perror("ERROR:-------> DevID > max DeviceNum");
		return -1;
	}
	if(nPoint > MaxPointNum){//��Ŵ���������򱨴�
		perror("ERROR:-------> nPoint > max nPoint");
		return -1;
	}

	return 0;

}

/***************
���ܣ�
	д1��ң��soe���ڴ��
������
	DevID,�ն�ID
	nPoint,�ڵ��
	bValue,ң��ֵ
����ֵ��
	0:�ɹ�,����:����
�޸����ڣ�
2019/09/24
	time�����ʹ��޸�,��Ϊ�����ͻ��߽ṹ��
2019/09/25
	ת����0�ŵ㴦�������š��ڵ����ת����0�ŵ�һ�µ�ʱ�򣬲Ÿ�ֵ��
***************/

int WriteYx(uint8_t DevNo,uint32_t nPoint,uint8_t bStatus)
{

	if(CheckError(DevNo, nPoint, gVars.dwDevNum, gpDevice[DevNo].BINum) < 0)
		return -1;
	
	if(gpDevice[DevNo].pBurstBI == NULL){
		perror("ERROR:-------> gpDevice[DevID].pBurstBI is NULL");
		return -1;
	}
	gpDevice[DevNo].pBurstBI[nPoint].bStatus = bStatus;

	return 0;

}

/***************
���ܣ�
	д1��ң��soe���ڴ��
������
	DevID,�ն�ID
	nPoint,�ڵ��
	bValue,ң��ֵ
	CP56Time2a_T,
����ֵ��
	0:�ɹ�,����:����
�޸����ڣ�
2019/09/25
	ת����0�ŵ㴦�������š��ڵ����ת����0�ŵ�һ�µ�ʱ�򣬲Ÿ�ֵ��
2019/09/29
	���Ӳ�����ʱ��ṹ��
***************/

int WriteYxSoe(uint8_t DevID,uint32_t nPoint,uint8_t bStatus,PCP56Time2a_T _Time)
{
	if(CheckError(DevID, nPoint, gVars.dwDevNum, gpDevice[DevID].BINum) < 0)
		return -1;
	
	if(gpDevice[DevID].pBurstBI == NULL){
		perror("ERROR:-------> gpDevice[DevID].pBurstBI is null");
		return -1;
	}
	gpDevice[DevID].pBurstBI[nPoint].bStatus = bStatus;
	gpDevice[DevID].pBurstBI[nPoint].stTime = *_Time;
	gpDevice[DevID].pBurstBI[nPoint].FlagSoe = 1;
	return 0;

}


/***************
���ܣ�
	д1��ң��soe�����ݿ�
������
	DevID,�ն�ID
	nPoint,�ڵ��
	bStatus,״ֵ̬
	bResdStatus,��ȡ״̬,0�Ѷ�,1δ��
	time,ң�ŷ���ʱ��
����ֵ��
	0:�ɹ�,����:����
�޸����ڣ�
2019/09/29
	д�����ݿ�
	���ݿ�ң��������޸ģ��������У��ꡢ�¡��ա�ʱ���֡�����
***************/

int WriteYxSoeDB(uint8_t DevID,uint32_t nPoint,uint8_t bStatus,uint8_t bResdStatus,CP56Time2a_T *_Time)//time�ĸ�ʽҪ�ģ���ʱ������uint16_t
{
	if(CheckError(DevID, nPoint, gVars.dwDevNum, gpDevice[DevID].BINum) < 0)
		return -1;
	if(gpDevice[DevID].pBurstDBI == NULL){
		perror("ERROR:-------> gpDevice[DevID].pBurstDBI is null");
		return -1;
	}
	gpDevice[DevID].pBurstDBI[nPoint].bStatus = bStatus;
	return 0;
	
}

/***************
���ܣ�
	д1��16λң��soe���ڴ��
������
	DevID,�ն�ID
	nPoint,�ڵ��
	bValue,ң��ֵ
����ֵ��
	0:�ɹ�,����:����
�޸����ڣ�
2019/09/24
	time�����ʹ��޸�,��Ϊ�����ͻ��߽ṹ��
2019/09/25
	����ת�����в������ĵ���ڳ�ʼ����ʱ�����Ŷ�Ϊ�㡣����Щ�ڵ�Ų���ʱ����������Ž��и�ֵ��

	�������Ϊ�㣬�ݲ���������ת��������Ϊ0���ڵ����Ҫ���⴦����
***************/

int WriteYc16(uint8_t DevID,uint32_t nPoint, uint16_t bValue)
{
	if(CheckError(DevID, nPoint, gVars.dwDevNum, gpDevice[DevID].BINum) < 0)
		return -1;

	if(gpDevice[DevID].pBurstAI == NULL){
		perror("ERROR:-------> gpDevice[DevID].pBurstAI is null");
		return -1;
	}

	gpDevice[DevID].pBurstAI[nPoint].detect16._detect = bValue;
	
	return 0;

}

/***************
���ܣ�
	д1��32λң��soe���ڴ��
������
	DevID,�ն�ID
	nPoint,�ڵ��
	bValue,ң��ֵ
����ֵ��
	0:�ɹ�,����:����
�޸����ڣ�
2019/09/24
	time�����ʹ��޸�,��Ϊ�����ͻ��߽ṹ��
2019/09/25
	����ת�����в������ĵ���ڳ�ʼ����ʱ�����Ŷ�Ϊ�㡣����Щ�ڵ�Ų���ʱ����������Ž��и�ֵ��

	�������Ϊ�㣬�ݲ���������ת��������Ϊ0���ڵ����Ҫ���⴦����
***************/

int WriteYc32(uint8_t DevID,uint32_t nPoint, uint32_t bValue)
{
	if(CheckError(DevID, nPoint, gVars.dwDevNum, gpDevice[DevID].BINum) < 0)
		return -1;
	
	if(gpDevice[DevID].pBurstAI == NULL){
		perror("ERROR:-------> gpDevice[DevID].pBurstAI is null");
		return -1;
	}
	gpDevice[DevID].pBurstAI[nPoint].detect32._detect = bValue;
	
	return 0;

}

int WriteYcFloat(uint8_t DevID,uint32_t nPoint, float bValue)
{
	if(CheckError(DevID, nPoint, gVars.dwDevNum, gpDevice[DevID].BINum) < 0)
		return -1;
	
	if(gpDevice[DevID].pBurstAI == NULL){
		perror("ERROR:-------> gpDevice[DevID].pBurstAI is null");
		return -1;
	}
	gpDevice[DevID].pBurstAI[nPoint].detect32_F._detect = bValue;
	
	return 0;

}
int WriteRealDz(uint8_t DevID,uint32_t nPoint, uint32_t bValue)
{
	if(CheckError(DevID, nPoint, gVars.dwDevNum, gpDevice[DevID].DZNum) < 0)
		return -1;
	if(gpDevice[DevID].pBurstDZ == NULL){
		perror("ERROR:-------> gpDevice[DevID].pBurstDZ is null");
		return -1;
	}
	gpDevice[DevID].pBurstDZ[nPoint].value = bValue;
	return 0;
}

int WriteDzYz(int DevID,uint32_t nIndex, uint32_t bValue)
{
	int RealID,nPoint;
	if(nIndex > gVars.TransDZTableNum){
		perror("error:nIndex > gVars.TransDZTableNum\n");
		return -1;
	}
	RealID = TransDZTable[nIndex].wRealID;
	nPoint = TransDZTable[nIndex].nPoint;
	if(RealID == 0 && nPoint == 0 && TransDZTable[nIndex].ID != 0){
		return 0;
	}
	
	gpDevice[RealID].pBurstDZ[nPoint].YZ_value = bValue;
	gpDevice[RealID].pBurstDZ[nPoint].YZ_Caller = DevID;
	return 0;
}



/***************
���ܣ�
	д1��ң��soe���ڴ��
������
	DevID,�ն�ID
	nPoint,�ڵ��
	bValue,ң��ֵ
����ֵ��
	0:�ɹ�,����:����
�޸����ڣ�
2019/09/24
	time�����ʹ��޸�,��Ϊ�����ͻ��߽ṹ��
2019/09/25
	����ת�����в������ĵ���ڳ�ʼ����ʱ�����Ŷ�Ϊ�㡣����Щ�ڵ�Ų���ʱ����������Ž��и�ֵ��

	�������Ϊ�㣬�ݲ���������ת��������Ϊ0���ڵ����Ҫ���⴦����
2019/11-01
	���ݲ�����YCת�����ȫ������gpDevice������ת������gpDevice��Ѱ������
***************/

int WriteYcSoe(uint8_t DevID,uint32_t nPoint,uint16_t bValue,CP56Time2a_T *_time)
{

	gpDevice[DevID].pBurstAI[nPoint].detect16._detect = bValue;
	
//	log("yc index is(%d)   bValue is(%d)\n",gpDevice[DevID].pBurstAI[nPoint].nNo,TransYCTable[gpDevice[DevID].pBurstAI[nPoint].nNo].Value);
	return 0;

}

/***************
���ܣ�
	д1��ң��soe�����ݿ�
������
	DevID,�ն�ID
	nPoint,�ڵ��
	bStatus,״ֵ̬
	bResdStatus,��ȡ״̬,0�Ѷ�,1δ��
	bValue,ң��ֵ
	time,ң�ŷ���ʱ��
����ֵ��
	0:�ɹ�,����:����
�޸����ڣ�
2019/09/24
	time�����ʹ��޸�,��Ϊ�����ͻ��߽ṹ��
2019/09/29
	ʱ���޸�Ϊ�ṹ��CP56Time2a_T
***************/

int WriteYcSoeDB(uint8_t DevID,uint32_t nPoint,uint8_t bStatus,uint8_t bResdStatus,uint16_t bValue,CP56Time2a_T *_time)
{
//	sqlite3 *db = NULL;
//	char *zErrMsg = 0;
//	int32_t rc;
//	char *sql;
//	int i=0;
//	char *TableName;
//
//	
//	TableName = "TransYcSoeTable";
//	
//	rc = sqlite3_open("/mnt/internal_storage/dcu/IEC104_data.db",&db);//�����ݿ⣬��������ھʹ���һ��
//	if(rc){
//		fprintf(stderr,"can't open database:%s \n",sqlite3_errmsg(db));
//		log("can't open database: \n");
//		sqlite3_close(db);
//		return -1;
//	}
//	else 
//		log("You have open a sqliet3 database named /mnt/internal_storage/dcu/IEC104_data.db successful!\n");
//
//	int32_t nrow=0,ncolumn=0;
//	char **azResult;//��ά�����Ž��
//
//
//	//������������ñ������򲻴�������������ʾ��Ϣ���洢��zErrMsg��
//	sql = "CREATE TABLE TransYcSoeTable(\
//			ID INTEGER PRIMARY KEY,\
//			nFlag TEXT,\
//			bStatus INTEGER,\
//			bResdStatus INTEGER,\
//			bValue INTEGER,\
//			nPoint INTEGER,\
//			nIndex INTEGER, \
//			stTime TEXT \
//			);";
//	sqlite3_exec(db,sql,0,0,&zErrMsg);
//	
//	//��������
//	sql = malloc(strlen("INSERT INTO ")+strlen(TableName)+strlen("VALUES(%d,%d,%d,%d,%d,%d,%d,%d)")+sizeof(TBurstBISoe));
//	sprintf(sql,"INSERT INTO %s VALUES(null,%d,%d,%d,%d,%d,%d,%d);",TableName,gpDevice[DevID].DeviceID,bStatus,\
//		bResdStatus,bValue,nPoint,gpDevice[DevID].pBurstBI[nPoint].nNo,_time->_year._year);
//
//		
//	log("%s\n",sql);
//	sqlite3_exec(db,sql,0,0,&zErrMsg);
//
//
//	log("ERORR_MSG is %s\n",zErrMsg);
//	sqlite3_close(db);
//	return 0;

}


int ReadRealYx(uint8_t DevID,uint32_t nPoint)
{
	if(gpDevice[DevID].pBurstBI == NULL) return 0;
	return gpDevice[DevID].pBurstBI[nPoint].bStatus;
}
uint32_t ReadRealYcData32(uint8_t DevID,uint32_t nPoint)
{
	if(gpDevice[DevID].pBurstAI == NULL) return 0;
	return gpDevice[DevID].pBurstAI[nPoint].detect32._detect;
}
uint16_t ReadRealYcData16(uint8_t DevID,uint32_t nPoint)
{
	if(gpDevice[DevID].pBurstAI == NULL) return 0;
	return gpDevice[DevID].pBurstAI[nPoint].detect16._detect;
}

uint32_t ReadRealDzData32(uint8_t DevID,uint32_t nPoint)
{
	if(gpDevice[DevID].pBurstDZ == NULL) return 0;
	return gpDevice[DevID].pBurstDZ[nPoint].value;
}
uint32_t ReadRealDzYZData32(uint8_t DevID,uint32_t nPoint)
{
	if(gpDevice[DevID].pBurstDZ == NULL) return 0;
	return gpDevice[DevID].pBurstDZ[nPoint].YZ_value;
}


PTBurstBI ReadRealYxSoe(int DevID, uint16_t Point)
{
	if(gpDevice[DevID].pBurstBI == NULL) return 0;
	return &gpDevice[DevID].pBurstBI[Point];
}
uint8_t ReadRealYxFlag(uint16_t DevID, uint16_t Point)
{
	if(gpDevice[DevID].pBurstBI == NULL) return 0;
	return gpDevice[DevID].pBurstBI[Point].flag;
}
uint8_t ReadRealYxSoeFlag(uint16_t DevID, uint16_t Point)
{
	if(gpDevice[DevID].pBurstBI == NULL) return 0;
	return gpDevice[DevID].pBurstBI[Point].FlagSoe;

}
uint8_t ReadRealYcFlag(uint16_t DevID, uint16_t Point)
{
	if(gpDevice[DevID].pBurstAI == NULL) return 0;
	return gpDevice[DevID].pBurstAI[Point].flag;
}

uint8_t ReadRealdDzFlag(uint16_t DevID, uint16_t Point)
{
	if(gpDevice[DevID].pBurstDZ == NULL) return 0;
	return gpDevice[DevID].pBurstDZ[Point].flag;
}

uint8_t ReadRealdDzCallFlag(uint16_t DevID, uint16_t Point)
{
	if(gpDevice[DevID].pBurstDZ == NULL) return 0;
	return gpDevice[DevID].pBurstDZ[Point].flag_call;
}


int ReadYx(uint32_t nIndex)
{
	int DevID,nPoint;
	if(nIndex > gVars.TransYXTableNum){
		perror("error:nIndex > gVars.TransYXTableNum\n");
		return -1;
	}
	DevID = TransYXTable[nIndex].wRealID;
	nPoint = TransYXTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransYXTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	return ReadRealYx(DevNo, nPoint);
}


uint32_t ReadYcData32(uint32_t nIndex)
{
	int DevID,nPoint;
	if(nIndex > gVars.TransYCTableNum){
		perror("error:nIndex > gVars.TransYCTableNum\n");
//		PRINT_FUNLINE;
		return -1;
	}
	DevID = TransYCTable[nIndex].wRealID;
	nPoint = TransYCTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransYCTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	
	return ReadRealYcData32(DevNo, nPoint);
}


uint16_t ReadYcData16(uint32_t nIndex)
{
	int DevID,nPoint;
	if(nIndex > gVars.TransYCTableNum){
		perror("error:nIndex > gVars.TransYCTableNum\n");
		PRINT_FUNLINE;
		return -1;
	}
	DevID = TransYCTable[nIndex].wRealID;
	nPoint = TransYCTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransYCTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	return ReadRealYcData16(DevNo, nPoint);
}

uint32_t ReadDzData32(uint32_t nIndex)
{
	int DevID,nPoint;
	if(nIndex > gVars.TransDZTableNum){
		perror("error:nIndex > gVars.TransDZTableNum\n");
//		PRINT_FUNLINE;
		return -1;
	}
	DevID = TransDZTable[nIndex].wRealID;
	nPoint = TransDZTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransDZTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	
	return ReadRealDzData32(DevNo, nPoint);

}

uint32_t ReadDzYZData32(uint32_t nIndex)
{
	int DevID,nPoint;
	if(nIndex > gVars.TransDZTableNum){
		perror("error:nIndex > gVars.TransDZTableNum\n");
//		PRINT_FUNLINE;
		return -1;
	}
	DevID = TransDZTable[nIndex].wRealID;
	nPoint = TransDZTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransDZTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	
	return ReadRealDzYZData32(DevNo, nPoint);

}




/***************
���ܣ�
	��YXSOE���ݿ��е�SOE
������
	DevID,�ն�ID
	nPoint,�ڵ��
	bStatus,״ֵ̬
	bResdStatus,��ȡ״̬,0�Ѷ�,1δ��
	time,ң�ŷ���ʱ��
����ֵ��
	0:�ɹ�,����:����
�޸����ڣ�
2019/09/24
	time�����ʹ��޸�,��Ϊ�����ͻ��߽ṹ��
2019/09/25
	sqlite3_get_table��sqlite3_free_table��Ҫ����ʹ��,����ᱨ�ڴ����
	sqlite3_exec����ڵ�sql�ַ�����Ҫ�ӡ�;���š�
***************/

PTBurstBI ReadYxSoe(uint16_t nIndex)
{
	int DevID,nPoint;
	if(nIndex > gVars.TransYXTableNum){
		perror("error:nIndex > gVars.TransYXTableNum\n");
		return 0;
	}
	DevID = TransYXTable[nIndex].wRealID;
	nPoint = TransYXTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransYXTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return NULL;
	}
	return ReadRealYxSoe(DevNo, nPoint);
		
}
uint8_t ReadYxFlag(uint16_t nIndex)
{
	uint16_t DevID,nPoint;
	if(nIndex > gVars.TransYXTableNum){
		perror("error:nIndex > gVars.TransYXTableNum\n");
		PRINT_FUNLINE;
		return -1;
	}
	DevID = TransYXTable[nIndex].wRealID;
	nPoint = TransYXTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransYXTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);

//	log("i is %d devno is %d, nPoint %d\n",nIndex, DevNo, nPoint);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	return ReadRealYxFlag(DevNo, nPoint);
}
int ReadYcFlag(uint16_t nIndex)
{
	uint16_t DevID,nPoint;
	if(nIndex > gVars.TransYCTableNum){
		perror("error:nIndex > gVars.TransYXTableNum\n");
		PRINT_FUNLINE;
		return -1;
	}
	DevID = TransYCTable[nIndex].wRealID;
	nPoint = TransYCTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransYCTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}

	return ReadRealYcFlag(DevNo, nPoint);

}

int ReadDzFlag(uint16_t nIndex)
{
	uint16_t DevID,nPoint;
	if(nIndex > gVars.TransDZTableNum){
		perror("error:nIndex > gVars.TransDZTableNum\n");
		PRINT_FUNLINE;
		return -1;
	}
	DevID = TransDZTable[nIndex].wRealID;
	nPoint = TransDZTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransDZTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	return ReadRealdDzFlag(DevNo, nPoint);

}

int ReadDzCallFlag(uint16_t nIndex)
{
	uint16_t DevID,nPoint;
	if(nIndex > gVars.TransDZTableNum){
		perror("error:nIndex > gVars.TransDZTableNum\n");
		PRINT_FUNLINE;
		return -1;
	}
	DevID = TransDZTable[nIndex].wRealID;
	nPoint = TransDZTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransDZTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	return ReadRealdDzCallFlag(DevNo, nPoint);

}



uint8_t ReadYxSoeFlag(uint16_t nIndex)
{
	uint16_t DevID,nPoint;
	if(nIndex > gVars.TransYXTableNum){
		perror("error:nIndex > gVars.TransYXTableNum\n");
		PRINT_FUNLINE;
		return -1;
	}
	DevID = TransYXTable[nIndex].wRealID;
	nPoint = TransYXTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransYXTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	return ReadRealYxSoeFlag(DevNo, nPoint);

}
int SetRealYXFlag(uint16_t DevID, uint16_t nPoint, uint8_t Value)
{
	gpDevice[DevID].pBurstBI[nPoint].flag = Value;
	return 0;
}
int SetRealYXSoeFlag(uint16_t DevID, uint16_t nPoint, uint8_t flag)
{
	gpDevice[DevID].pBurstBI[nPoint].FlagSoe = flag;
	return 0;
}
int SetRealYCFlag(uint16_t DevID, uint16_t nPoint, uint8_t flag)
{
	gpDevice[DevID].pBurstAI[nPoint].flag =	flag;
	return 0;
}

int SetRealDZFlag(uint16_t DevID, uint16_t nPoint, uint8_t flag)
{
	gpDevice[DevID].pBurstDZ[nPoint].flag =	flag;
	return 0;
}

int SetRealDZCallFlag(uint16_t DevID, uint16_t nPoint, uint8_t flag)
{
	gpDevice[DevID].pBurstDZ[nPoint].flag_call = flag;
	return 0;
}



int SetYXFlag(uint16_t nIndex, uint8_t Value)
{
	uint16_t DevID,nPoint;
	if(nIndex > gVars.TransYXTableNum){
		perror("error:nIndex > gVars.TransYXTableNum\n");
		PRINT_FUNLINE;
		return -1;
	}
	DevID = TransYXTable[nIndex].wRealID;
	nPoint = TransYXTable[nIndex].nPoint;
	
	if(DevID == 0 && nPoint == 0 && TransYXTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	if(gpDevice[DevNo].pBurstBI == NULL){
		perror("gpDevice[DevNo].pBurstBI is NULL please check TransYxTable\n");
		return -1;
	}
	return SetRealYXFlag(DevNo, nPoint, Value);
}

int SetYXSoeFlag(uint16_t nIndex, uint8_t Value)
{
	uint16_t DevID,nPoint;
	if(nIndex > gVars.TransYXTableNum){
		perror("error:nIndex > gVars.TransYXTableNum\n");
		PRINT_FUNLINE;
		return -1;
	}
	DevID = TransYXTable[nIndex].wRealID;
	nPoint = TransYXTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransYXTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	
	return SetRealYXSoeFlag(DevNo, nPoint, Value);
}


int SetYCFlag(uint32_t nIndex, uint8_t Value)
{
	uint16_t DevID,nPoint;
	if(nIndex > gVars.TransYCTableNum){
		perror("error:nIndex > gVars.TransYCTableNum\n");
		PRINT_FUNLINE;
		return RET_ERROR;
	}
	DevID = TransYCTable[nIndex].wRealID;
	nPoint = TransYCTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransYCTable[nIndex].ID != 0){
		return RET_SUCESS;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	if(gpDevice[DevNo].pBurstAI == NULL){
		perror("gpDevice[DevNo].pBurstAI is NULL please check TransYcTable\n");
		return RET_ERROR;
	}
	return SetRealYCFlag(DevNo, nPoint, Value);
}

int SetDZFlag(uint32_t nIndex, uint8_t flag)
{
	uint16_t DevID,nPoint;
	if(nIndex > gVars.TransDZTableNum){
		perror("error:nIndex > gVars.TransDZTableNum\n");
		PRINT_FUNLINE;
		return -1;
	}
	DevID = TransDZTable[nIndex].wRealID;
	nPoint = TransDZTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransDZTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	
	return SetRealDZFlag(DevNo, nPoint, flag);

}

int SetDZCallFlag(uint32_t nIndex, uint8_t flag)
{
	uint16_t DevID,nPoint;
	if(nIndex > gVars.TransDZTableNum){
		perror("error:nIndex > gVars.TransDZTableNum\n");
		PRINT_FUNLINE;
		return -1;
	}
	DevID = TransDZTable[nIndex].wRealID;
	nPoint = TransDZTable[nIndex].nPoint;
	if(DevID == 0 && nPoint == 0 && TransDZTable[nIndex].ID != 0){
		return 0;
	}
	int DevNo;
	DevNo = GetDevNo(DevID);
	if(RET_ERROR == DevNo){
		perror("RET_ERROR == DevNo");
		return RET_ERROR;
	}
	
	
	return SetRealDZCallFlag(DevNo, nPoint, flag);

}

int GetDZRealID(uint32_t nIndex)
{
	return TransDZTable[nIndex].wRealID;;
}


void SetCallAllFlag(void)
{
	int i;
	uint32_t YXNum,YCNum;
	YXNum = gVars.TransYXTableNum;
	YCNum = gVars.TransYCTableNum;
	
	for(i=0;i<YXNum;i++){
		SetYXFlag(i,Flag_CallAll);
	}
	for(i=0;i<YCNum;i++){
		SetYCFlag(i,Flag_CallAll);
	}
}

void SetCallDZAllFlag(void)
{
	int i;
	uint32_t DZNum;
	DZNum = gVars.TransDZTableNum;
	
	for(i=0;i<DZNum;i++){
		SetDZCallFlag(i,Flag_CallAll);
	}

}

int WriteBrustYXRecord(uint16_t Point, uint8_t *Value)
{

    IEC10X->GetTime(&YXBrustRecord[gVars.YxWritePtr%MAXYXRECORD].stTime);//��ȡϵͳʱ��
	YXBrustRecord[gVars.YxWritePtr%MAXYXRECORD].bStatus = *Value;
	YXBrustRecord[gVars.YxWritePtr%MAXYXRECORD].point = Point;
	gVars.YxWritePtr++;
}

int WriteBrustYXSoeRecord(uint16_t Point, PTBurstBI SOE_T)
{

	YXSoeRecord[gVars.YxSoeWritePtr%MAXYXRECORD].bStatus = SOE_T->bStatus;
	YXSoeRecord[gVars.YxSoeWritePtr%MAXYXRECORD].stTime = SOE_T->stTime;
	YXSoeRecord[gVars.YxSoeWritePtr%MAXYXRECORD].point = Point;
	gVars.YxSoeWritePtr++;
}

int WriteBrustDZRecord(uint16_t Point, uint32_t *Value)
{

    IEC10X->GetTime(&DZBrustRecord[gVars.DZWritePtr%MAXYXRECORD].stTime);//��ȡϵͳʱ��
	DZBrustRecord[gVars.DZWritePtr%MAXYXRECORD].Value32 = *Value;
	DZBrustRecord[gVars.DZWritePtr%MAXYXRECORD].point = Point;
	gVars.DZWritePtr++;
}


/*******************************************************************  
*���ƣ�      		CheckBrustYx  
*���ܣ�			���ͻ��ң��  
*��ڲ����� 
*���ڲ�������ȷ����Ϊ0�����󷵻�Ϊ-1 
*******************************************************************/ 
int CheckBrustYx(void)
{
	uint32_t YXNum;
	uint16_t i;
	uint8_t Value;
	YXNum = gVars.TransYXTableNum;

	for(i=0;i<YXNum;i++){
		if(ReadYxFlag(i) == Flag_Spon){
			Value = ReadYx(i);
			SetYXFlag(i, Flag_Clear);
			WriteBrustYXRecord(i, &Value);
		}
	}
}
/*******************************************************************  
*���ƣ�      		CheckBrustYxSoe  
*���ܣ�			���ͻ��ң��  
*��ڲ����� 
*���ڲ�������ȷ����Ϊ0�����󷵻�Ϊ-1 
*******************************************************************/ 
int CheckBrustYxSoe(void)
{
	uint32_t YXNum;
	uint16_t i;
	PTBurstBI Soe_T = NULL;
	YXNum = gVars.TransYXTableNum;
	for(i=0;i<YXNum;i++){
		if(ReadYxSoeFlag(i) == Flag_Spon){
			SetYXSoeFlag(i, Flag_Clear);
			Soe_T = ReadYxSoe(i);
			WriteBrustYXSoeRecord(i, Soe_T);
		}
	}
}

/*******************************************************************  
*���ƣ�      		CheckBrustYx  
*���ܣ�			���ͻ��ң��  
*��ڲ����� 
*���ڲ�������ȷ����Ϊ0�����󷵻�Ϊ-1 
*******************************************************************/ 
int CheckBrustDZ(void)
{
	uint32_t DZNum;
	uint16_t i;
	uint32_t Value;
	DZNum = gVars.TransDZTableNum;

	for(i=0;i<DZNum;i++){
		if(ReadDzFlag(i) == Flag_Spon){
			Value = ReadDzData32(i);
			SetDZFlag(i, Flag_Clear);
			WriteBrustDZRecord(i, &Value);
		}
	}
}

/*******************************************************************  
*���ƣ�      		ScanBrustYx  
*���ܣ�			����ͻ��ң��  
*��ڲ�����         
*	@ReadPtr 	��ָ��

*���ڲ�����0:��ͻ��ң�� 1:��ͻ��ң�� 
*******************************************************************/ 
int ScanBrustYx(int DevID, uint32_t *ReadPtr, uint16_t *StartAddr, uint16_t *BrustYXNum)
{
	uint8_t Value;
	uint8_t Flag_Brust = 0;
	uint8_t Flag_Start = 1;
	
	while(*ReadPtr != gVars.YxWritePtr){
//		log("*ReadPtr(%d)gVars.YxWritePtr(%d)\n",*ReadPtr,gVars.YxWritePtr); 
		if(Flag_Start){
			Flag_Start = 0;
			*StartAddr = *ReadPtr%MAXYXRECORD;
		}
//		log("DevID(%d) YXBrustPoint(%d)  value(%d),readptr(%d)\n",DevID, YXBrustRecord[(*ReadPtr)%MAXYXRECORD].point\
//			,YXBrustRecord[(*ReadPtr)%MAXYXRECORD].bStatus, *ReadPtr);
		(*ReadPtr)++;
		(*BrustYXNum)++;
		Flag_Brust = 1;
		if((*BrustYXNum) >= 40)
			return Flag_Brust;
	}
	return Flag_Brust;
}

/*******************************************************************  
*���ƣ�      		ScanBrustYxSoe  
*���ܣ�			����ͻ��ң���¼� 
*��ڲ�����         
*	@ReadPtr 	��ָ��

*���ڲ�����0:��ͻ��ң�� 1:��ͻ��ң�� 
*******************************************************************/ 
int ScanBrustYxSoe(int DevID, uint32_t *ReadPtr, uint16_t *StartAddr, uint16_t *BrustYXNum)
{
	TBurstBI Value;
	uint8_t Flag_Brust = 0;
	uint8_t Flag_Start = 1;
	
	while(*ReadPtr != gVars.YxSoeWritePtr){
		if(Flag_Start){
			Flag_Start = 0;
			*StartAddr = *ReadPtr%MAXYXRECORD;
		}
		(*ReadPtr)++;
		(*BrustYXNum)++;
		Flag_Brust = 1;
		if((*BrustYXNum) >= 20)
			return Flag_Brust;
	}
	return Flag_Brust;
}

int ScanBrustDZ(int DevID, uint32_t *ReadPtr, uint16_t *StartAddr, uint16_t *BrustDZNum)
{
	uint8_t Value;
	uint8_t Flag_Brust = 0;
	uint8_t Flag_Start = 1;
	
	while(*ReadPtr != gVars.DZWritePtr){
//		log("*ReadPtr(%d)gVars.YxWritePtr(%d)\n",*ReadPtr,gVars.YxWritePtr); 
		if(Flag_Start){
			Flag_Start = 0;
			*StartAddr = *ReadPtr%MAXYXRECORD;
		}
//		log("DevID(%d) YXBrustPoint(%d)  value(%d),readptr(%d)\n",DevID, YXBrustRecord[(*ReadPtr)%MAXYXRECORD].point\
//			,YXBrustRecord[(*ReadPtr)%MAXYXRECORD].bStatus, *ReadPtr);
		(*ReadPtr)++;
		(*BrustDZNum)++;
		Flag_Brust = 1;
		if((*BrustDZNum) >= 40)
			return Flag_Brust;
	}
	return Flag_Brust;
}

int WriteBrustYCRecord(uint16_t Point, uint32_t *Value)
{

    IEC10X->GetTime(&YCBrustRecord[gVars.YcWritePtr%MAXYCRECORD].stTime);//��ȡϵͳʱ��
	YCBrustRecord[gVars.YcWritePtr%MAXYCRECORD].Value32 = *Value;
	YCBrustRecord[gVars.YcWritePtr%MAXYCRECORD].point = Point;
	gVars.YcWritePtr++;
	log("gVars.YcWritePtr(%d)\n",gVars.YcWritePtr);
}

/*******************************************************************  
*���ƣ�      		CheckBrustYc  
*���ܣ�			���ͻ��ң��  
*��ڲ����� 
*���ڲ�������ȷ����Ϊ0�����󷵻�Ϊ-1 
*******************************************************************/ 
int CheckBrustYc(void)
{
	uint32_t YCNum;
	uint16_t i;
	uint32_t Value;
	YCNum = gVars.TransYCTableNum;

	for(i=0;i<YCNum;i++){
		if(ReadYcFlag(i) == Flag_Spon){
			Value = ReadYcData32(i);
			log("Value is (%d)\n",Value);
			SetYCFlag(i, Flag_Clear);
			WriteBrustYCRecord(i, &Value);
		}
	}
}
/*******************************************************************  
*���ƣ�      		ScanBrustYc  
*���ܣ�			����ͻ��ң��  
*��ڲ�����         
*	@ReadPtr 	��ָ��

*���ڲ�����0:��ͻ��ң�� 1:��ͻ��ң�� 
*******************************************************************/ 
int ScanBrustYc(int fd, int DevID, uint16_t *ReadPtr, uint16_t *StartAddr, uint16_t *BrustYCNum)
{
	uint32_t Value;
	uint8_t Flag_Start = 1;
	uint8_t Flag_Brust = 0;
	

	while(*ReadPtr != gVars.YcWritePtr){
		if(Flag_Start){
			Flag_Start = 0;
			*StartAddr = *ReadPtr%MAXYCRECORD;
		}
		log("DevID(%d) YCBrustPoint(%d)  value(%d),readptr(%d)\n",DevID, YCBrustRecord[(*ReadPtr)%MAXYCRECORD].point\
			,YCBrustRecord[(*ReadPtr)%MAXYCRECORD].Value32, *ReadPtr);
		(*ReadPtr)++;
		(*BrustYCNum)++;
		Flag_Brust = 1;
		if((*BrustYCNum) > 40)
			return Flag_Brust;
	}
	return Flag_Brust;

}

//��ȡʵ�ʿ�ָ��
TRealBase * pGetRealBase( DWORD dwDevNO)
{
	TDevie *pDevice;
	pDevice = pGetDevice(dwDevNO);
    if( pDevice==NULL ) // ����豸���Ƿ�Ϸ�
		return NULL;
    
    return pDevice->pRealBase;
}
/***************
������	
	void InitLogicAitReffer(DWORD dwDevID)
���ܣ�
	��ʼ��ת��������ʼ�������豸���ݵ�����߼��豸���е�ƫ����
������
	dwDevID���߼�ģ��ID��
����ֵ��
�޸����ڣ�
***************/
void InitLogicAitReffer(DWORD dwDevNo)
{
	TDevie *pDevice;
	TLogicBase *pLogicBase;
	TLogicAI *pLogicAI;
	TRealAI  *pRealAI;
	TRealBase *pRealBase;

	DWORD dwAINo;
	DWORD dwRealID;   	//��AI���������豸��ID
	DWORD dwRealAINum;  //��AI���������豸��ID
	DWORD dwOffset;    	//��AI�����������豸ʵʱ���е�ƫ����

	TReflex *pReflex;
	uint16_t DevNo;

      pDevice = &gpDevice[dwDevNo];
    
	  pLogicBase = pDevice->pLogicBase;
//	  log("dwDevID(%d)\n",dwDevID);
	  if(!pLogicBase){
	  	perror("!pLogicBase");
			return;
	  }
      		

      pLogicAI = pLogicBase->pLogicAI; 

      if(pLogicAI == NULL){ //new  ����
		  perror("pLogicAI == NULL");
      		return;
      	}
	for(dwAINo=0;dwAINo<gVars.TransYCTableNum;dwAINo++)
	{
//		dwRealID = pLogicAI[dwAINo].wRealID;
//        dwOffset = pLogicAI[dwAINo].wOffset;
		dwRealID = TransYCTable[dwAINo].wRealID;
		dwOffset = TransYCTable[dwAINo].nPoint;
		DevNo = GetDevNo(dwRealID);
		
       	pRealBase = pGetRealBase( DevNo );//dwRealID�Ϸ��Լ��

       	if(!pRealBase)
       		continue;

       	dwRealAINum = gpDevice[DevNo].AINum;

       	if(dwOffset >= dwRealAINum)
       		continue; //dwOffset�Ϸ��Լ��

       	pRealAI = &pRealBase->pRealAI[dwOffset];
       	pReflex = (TReflex *)malloc(sizeof(TReflex));

       	if(!pReflex)
       		return ;

       	pReflex->dwDevNo = DevNo;
       	pReflex->dwNo = dwAINo;
       	pReflex->pNext = 0;

//����������
		TReflex *pRef;
		   	
		if(pRealAI->pReflex == 0)
	   	{
	  		pRealAI->pReflex = pReflex;
	   	}
		else
		{
			pRef = pRealAI->pReflex;

			while(pRef->pNext != 0)//�ҵ����һ��
				pRef = pRef->pNext;

	       	pRef->pNext = pReflex;
		}
	}
}
//��ʼ��ת�����
void InitReffer(void)//5512
{
	DWORD dwDevNo;
	for(dwDevNo=0;dwDevNo<gVars.dwDevNum;dwDevNo++)
	{
		if(strcmp("Logic", gpDevice[dwDevNo].Type) == 0)
		{
			if(strcmp("WQ900", gpDevice[dwDevNo].Name) != 0) continue;
			
//			InitLogicBiReffer(dwDevNo);
//			InitLogicDbiReffer(dwDevNo);
			InitLogicAitReffer(dwDevNo);
//			InitLogicMRReffer(dwDevNo);
//			InitLogicDZReffer(dwDevNo);//lgh
		}
	}
}

void Init_ModbusRegAddr(void)
{
	int i, wRealID, nPoint, RegAddr, DevNo, SlaverAddr;
	
	for( i = 0 ; i < gVars.TransModbusCoidStatusTableNum ; i++){
		wRealID = TransModbusCoidStatusTable[i].wRealID;
		nPoint = TransModbusCoidStatusTable[i].nPoint;
		RegAddr = TransModbusCoidStatusTable[i].RegAddr;
		SlaverAddr = TransModbusCoidStatusTable[i].SlaverAddr;
		DevNo = GetDevNo(wRealID);
		if(DevNo == -1) return;
		gpDevice[DevNo].ModbusData.pCoiStatus_T[nPoint]._RegAddr = RegAddr;
		gpDevice[DevNo].ModbusData.pCoiStatus_T[nPoint]._SlaverAddr = SlaverAddr;
		
	}
	for( i = 0 ; i < gVars.TransModbusInputStatusTableNum ; i++){
		wRealID = TransModbusInputStatusTable[i].wRealID;
		nPoint = TransModbusInputStatusTable[i].nPoint;
		RegAddr = TransModbusInputStatusTable[i].RegAddr;
		SlaverAddr = TransModbusInputStatusTable[i].SlaverAddr;
		DevNo = GetDevNo(wRealID);
		if(DevNo == -1) return;
		gpDevice[DevNo].ModbusData.pInputStatus_T[nPoint]._RegAddr = RegAddr;
		gpDevice[DevNo].ModbusData.pInputStatus_T[nPoint]._SlaverAddr = SlaverAddr;
	}
	for( i = 0 ; i < gVars.TransModbusHoldingRegTableNum ; i++){
		wRealID = TransModbusHoldingRegTable[i].wRealID;
		nPoint = TransModbusHoldingRegTable[i].nPoint;
		RegAddr = TransModbusHoldingRegTable[i].RegAddr;
		SlaverAddr = TransModbusHoldingRegTable[i].SlaverAddr;
		DevNo = GetDevNo(wRealID);
		if(DevNo == -1) return;
		gpDevice[DevNo].ModbusData.pHoldingRegister_T[nPoint]._RegAddr = RegAddr;
		gpDevice[DevNo].ModbusData.pHoldingRegister_T[nPoint]._SlaverAddr = SlaverAddr;
//		log("DevNo is %d RegAddr is %d SlaverAddr is %d  i is %d\n", DevNo, RegAddr, SlaverAddr, i);
	}
	for( i = 0 ; i < gVars.TransModbusInputRegTableNum ; i++){
		wRealID = TransModbusInputRegTable[i].wRealID;
		nPoint = TransModbusInputRegTable[i].nPoint;
		RegAddr = TransModbusInputRegTable[i].RegAddr;
		SlaverAddr = TransModbusInputRegTable[i].SlaverAddr;
		DevNo = GetDevNo(wRealID);
		if(DevNo == -1) return;
		gpDevice[DevNo].ModbusData.pInputResgister_T[nPoint]._RegAddr = RegAddr;
		gpDevice[DevNo].ModbusData.pInputResgister_T[nPoint]._SlaverAddr = SlaverAddr;
	}
	
	
}





