/*******************************************************************
Copyright (C):    
File name    :    GX104Slaver.c
DESCRIPTION  :
AUTHOR       :
Version      :    1.0
Date         :    2017/07/31
Others       :
History      :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*******************************************************************/

#include "sys.h"
//#include "GX104Slaver.h"

extern TBurstYXRecord YXBrustRecord[MAXYXRECORD];
extern TBurstYXRecord YXSoeRecord[MAXYXRECORD];
extern TBurstYCRecord YCBrustRecord[MAXYCRECORD];
extern TBurstYCRecord DZBrustRecord[MAXYCRECORD];


extern TransTable_T *TransYKTable;

static SendRecvSn_T SendRecvSn;
int GX104SlaverLinkState = 0;
static uint32_t Now_Time_cnt = 0;

	
 /*
 * GLOABLE VARIALBLE
 */

uint8_t             GX104Slaver_Sendbuf[GX104Slaver_MAX_BUF_LEN];
/*
 * STATE
 * */
static uint8_t             GX104Slaver_STATE_FLAG_INIT = GX104Slaver_FLAG_CLOSED;
static uint8_t             GX104Slaver_STATE_FLAG_GROUP = GX104Slaver_FLAG_CLOSED;
static uint8_t             GX104Slaver_STATE_FLAG_CLOCK = GX104Slaver_FLAG_CLOSED;
static uint8_t             GX104Slaver_STATE_FLAG_TESTER = GX104Slaver_FLAG_IDLE;
static uint8_t             GX104Slaver_STATE_FLAG_S_ACK = GX104Slaver_FLAG_CLOSED;

static uint8_t             GX104Slaver_Call_AllQoi = 0;
static uint8_t             GX104Slaver_Call_GroupQoi = 0;


#define GX104Slaver_CYCLE_TIME_MS             100                   /*100ms*/
#define GX104Slaver_RESEND_TIME_MS            (30*1000)             /*30s*/
#define GX104Slaver_S_ACK_TIMEOUT             (5*1000)              /*5s*/
#define GX104Slaver_TESTER_IDLE_TIMEOUT       (1*30*1000)           /*2min*/
static uint32_t            GX104Slaver_TimeCount = 0;
static uint32_t            GX104Slaver_TimeCycle = GX104Slaver_RESEND_TIME_MS;
static uint32_t            GX104Slaver_TimeCycle_S = 0;

static uint32_t            GX104Slaver_Update_SeekAddr = 0;
static uint16_t            GX104Slaver_FirmFlagCount = 0;


int GX104Slaver_Receive(int DevNo, uint8_t *buf, uint16_t len);
static int GX104Slaver_Task(int DevNo);
int GX104Slaver_On_Time_Out(int DevNo);

int Init_GX104Slaver(int DevNo)
{
	gpDevice[DevNo].Receive = GX104Slaver_Receive;
	gpDevice[DevNo].Task = GX104Slaver_Task;
	gpDevice[DevNo].OnTimeOut = GX104Slaver_On_Time_Out;
}
int TimeOn(uint32_t *OldTime, uint32_t *NowTime, uint32_t OverTime)
{
	if((*NowTime - *OldTime) > OverTime){
		*OldTime = *NowTime;
		return 1;
	}
	return 0;
		

}

uint8_t GX104_send(int DevNo,char *buf, int len){
					 
    if(-1 == write(gpDevice[DevNo].fd,buf,len)){
        perror("Send error \n");
		log("DevNo(%d)  fd(%d)\n",DevNo, gpDevice[DevNo].fd);
        return RET_ERROR;
    }

	MonitorTx(monitorData._TX_ID, DevNo, monitorData._fd, buf, len);
	
    return RET_SUCESS;
}

void GX104Slaver_Start_Clear(void){
	GX104Slaver_TimeCycle = 0;
	GX104Slaver_TimeCount = 0;
}

void GX104Slaver_ResetFlag(int DevNo){

    GX104Slaver_STATE_FLAG_GROUP = GX104Slaver_FLAG_IDLE;
    GX104Slaver_STATE_FLAG_CLOCK = GX104Slaver_FLAG_IDLE;
    GX104Slaver_STATE_FLAG_TESTER = GX104Slaver_FLAG_IDLE;

    SendRecvSn.BuildSendSn = 0;
    SendRecvSn.BuildRecvSn = 0;
    SendRecvSn.DealSendSn = -1;
    SendRecvSn.DealRecvSn = 0;
    SendRecvSn.TesterCount = 0;

	
	int i;
	for(i=0;i<gVars.dwDevNum;i++){
		gpDevice[i].ReadYCPtr = gVars.YcWritePtr = 0;
		gpDevice[i].ReadYXPtr = gVars.YxWritePtr = 0;
		gpDevice[i].ReadYXSoePtr = gVars.YxSoeWritePtr = 0;
	}
}


static uint8_t GX104Slaver_Send_Mirror(int DevNo, PGX104Slaver_DATA_T GX101Slaver_68, uint16_t reason){

    uint16_t len, i;
	uint8_t cs_temp = 0;

    /*init struct*/
    PGX104Slaver_DATA_T GX104Slaver = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104Slaver->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
	
	len = GX101Slaver_68->Len + 2;

	memcpy(GX104Slaver_Sendbuf, GX101Slaver_68, len);

	asdu->_reason._reason = reason;

	GX104_send(DevNo,GX104Slaver_Sendbuf, len);

    return RET_SUCESS;
}

uint8_t GX104Slaver_Build_InitFin(int DevNo){

    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;

    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;
    /*build ASDU , COT ,Addr*/
    asdu->_type = Iec10x_M_EI_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason._reason = IEC10X_ASDU_REASON_INIT;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info*/
    ptr = info->_addr;
    Temp32 = 0;
    memcpy(ptr, &Temp32, 3);

    ptr = info->_element;
    //Temp32 = 0;
    //memcpy(ptr, &Temp32, 4);
    info->_element[0] = 0;

    ptr++;

    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    IEC10X_Enqueue(GX104Slaver_Sendbuf, len ,IEC10X_PRIO_INITLINK, NULL,NULL);
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);
	
    return RET_SUCESS;
}


uint8_t GX104Slaver_BuildACKConfrim(int DevNo, uint8_t type, uint8_t qoi){

    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;

    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;
	
	log("SendRecvSn.BuildSendSn (%d)\n",SendRecvSn.BuildSendSn);
    /*build ASDU , COT ,Addr*/
    asdu->_type = type;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason._reason = IEC10X_ASDU_REASON_ACTCON;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info*/
    ptr = info->_addr;
    Temp32 = 0;
    memcpy(ptr, &Temp32, 3);

    ptr = info->_element;
    ptr[0] = qoi;
    ptr+=1;
    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;
    
//    DumpHEX(GX104Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    IEC10X_Enqueue(GX104Slaver_Sendbuf, len ,Prio, NULL,NULL);
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);
    return RET_SUCESS;
}



uint8_t GX104Slaver_BuildResetProcess(int DevNo, uint8_t qoi){

    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;

    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_C_RP_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason._reason = IEC10X_ASDU_REASON_ACTCON;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info*/
    ptr = info->_addr;
    Temp32 = 0;
    memcpy(ptr, &Temp32, 3);

    ptr = info->_element;
    ptr[0] = qoi;
    ptr+=1;
    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;
    
//    DumpHEX(GX104Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    IEC10X_Enqueue(GX104Slaver_Sendbuf, len ,Prio, NULL,NULL);
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);
    return RET_SUCESS;
}

uint8_t GX104Slaver_BuildBrustYX(int DevNo, uint16_t asdu_num,uint16_t StartAddr,PTBurstYXRecord asdu_data){

    uint8_t len = 0, i;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PCP56Time2a_T time = NULL;
	uint8_t sq = 0;
	
    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
	PASDU_104INFO_8T info_8 = NULL;
	for(i=0;i<(asdu_num-1);i++){
		if((asdu_data[(StartAddr+i)%MAXYXRECORD].point + 1) == asdu_data[(StartAddr+i+1)%MAXYXRECORD].point){
			sq = 1;
		}
		else{
			sq = 0;
			break;
		}
	}
    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_M_SP_NA_1;
    asdu->_num._sq = sq;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_COT_SPONT;
    asdu->_addr = Iec10x_Sta_Addr;


    /*build info*/
	if(asdu->_num._sq == 1){//连续
		/*info addr*/
	    ptr = info->_addr;
	    Temp32 = asdu_data[StartAddr%MAXYXRECORD].point + STARTSYX;;
	    memcpy(ptr, &Temp32, 3);
		/*info value*/
		ptr = info->_element;
		for(i=0; i<asdu_num; i++){
			
		    *ptr = asdu_data[(StartAddr+i)%MAXYXRECORD].bStatus;
		    ptr++;
		}
	}
	else if(asdu->_num._sq == 0){//不连续
		ptr = info->_addr;
		for(i=0; i<asdu_num; i++){
			/*info addr*/
			info_8 = (PASDU_104INFO_8T)ptr;
			Temp32 = asdu_data[(StartAddr+i)%MAXYXRECORD].point + STARTSYX;
			memcpy(info_8->_addr, &Temp32, 3);
			/*info value*/
			info_8->_Data8 = asdu_data[(StartAddr+i)%MAXYXRECORD].bStatus;//遥信状态
			ptr += sizeof(ASDU_104INFO_8T);
		}
	
	}

    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    IEC10X_Enqueue(GX104Slaver_Sendbuf, len ,IEC10X_PRIO_SPON, NULL,NULL);
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);
	
    return RET_SUCESS;
}


uint8_t GX104Slaver_BuildBrustYXSoe(int DevNo, uint16_t asdu_num,uint8_t reason, uint16_t StartAddr,PTBurstYXRecord asdu_data){

    uint8_t len = 0, i;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PCP56Time2a_T time = NULL;
	uint8_t sq = 0;
	
    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
	PASDU_104SOE_T info_8 = NULL;
    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_M_SP_TB_1;
    asdu->_num._sq = sq;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = reason;
    asdu->_addr = Iec10x_Sta_Addr;


	ptr = info->_addr;
	for(i=0; i<asdu_num; i++){
		/*info addr*/
		info_8 = (PASDU_104SOE_T)ptr;
		Temp32 = asdu_data[(StartAddr+i)%MAXYXRECORD].point + STARTSYX;
		memcpy(info_8->_addr, &Temp32, 3);
		/*info value*/
		info_8->_value = asdu_data[(StartAddr+i)%MAXYXRECORD].bStatus;//遥信状态
		/*time*/
		time = (PCP56Time2a_T)info_8->_element;
		*time = asdu_data[(StartAddr+i)%MAXYXRECORD].stTime;
		ptr += sizeof(ASDU_104SOE_T) + sizeof(CP56Time2a_T)-1;
		
	}

	

    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);
	
    return RET_SUCESS;
}


uint8_t GX104Slaver_BuildBrustDZ(int DevNo, uint16_t asdu_num, uint16_t StartAddr,PTBurstYCRecord asdu_data){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, *ptr = NULL;
	uint8_t sq = 0;
	uint32_t temp32;

    /*init struct*/
    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
	
	for(i=0;i<(asdu_num-1);i++){
		if((asdu_data[(StartAddr+i)%MAXYCRECORD].point + 1) == asdu_data[(StartAddr+i+1)%MAXYCRECORD].point){
			sq = 1;
		}
		else{
			sq = 0;
			break;
		}
	}
    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_TYPE_DZ_YZ;
    asdu->_num._sq = sq;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_COT_SPONT;
    asdu->_addr = Iec10x_Sta_Addr;




    /*Detect value*/
	if(asdu->_num._sq == 1){//连续
		PRINT_FUNLINE;
		/*info addr*/
		temp32 = asdu_data[StartAddr%MAXYCRECORD].point + STARTSDZ;
		memcpy(info->_addr, &temp32, 3);
		/*Detect value*/
		ptr = info->_element;
		for(i=0; i<asdu_num; i++){
		    *(uint32_t *)ptr = asdu_data[(StartAddr+i)%MAXYCRECORD].Value32;
		    ptr += 4;
		}
	}
	else if(asdu->_num._sq == 0){//不连续
		PRINT_FUNLINE;
		ptr = info->_addr;
		for(i=0; i<asdu_num; i++){
			temp32 = asdu_data[(StartAddr+i)%MAXYCRECORD].point + STARTSDZ;
			memcpy(ptr, &temp32, 3);
			ptr += 3;
		    *(uint32_t *)ptr = asdu_data[(StartAddr+i)%MAXYCRECORD].Value32;
		    ptr += 4;
		}
	
	}	
	*ptr = 6;//定值描述符
	ptr++;

    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);
	
    return RET_SUCESS;

}


int GX104Slaver_BuildOldYXSoe(int DevNo, PTBurstYXRecord OldYXSoe){
	int i;
	int sendgroup= 0 ;
	int readptr= 0 ;
	int staradd = 0;

	readptr = gpDevice[DevNo].ReadYXSoePtr;
	if(readptr/100){
		staradd = readptr - 100;
		for(i=0;i<5;i++){
			GX104Slaver_BuildBrustYXSoe(DevNo, 20, IEC10X_COT_GROUP16, staradd + i*20, OldYXSoe);
		}
	}else if(readptr == 0){
//			PRINT_FUNLINE;
//			log("readptr is (%d)\n",readptr);
		return 0;
	}else if((readptr/100) == 0){
		sendgroup = readptr/20;
		for(i=0;i<sendgroup+1;i++){
			if(i == sendgroup){//最后一组
				GX104Slaver_BuildBrustYXSoe(DevNo, readptr%20, IEC10X_COT_GROUP16, i*20, OldYXSoe);
				return RET_SUCESS;
			}
			GX104Slaver_BuildBrustYXSoe(DevNo, 20, IEC10X_COT_GROUP16, i*20, OldYXSoe);
		}
	}
    return RET_SUCESS;
}


uint8_t GX104Slaver_BuildBrustYC(int DevNo, uint16_t asdu_num,uint16_t StartAddr,PTBurstYCRecord asdu_data){

    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PIEC10X_DETECT_T detect = NULL;
	PIEC10X_DETECT_DW_T Value = NULL;
	PGX_104_INFO_YC32_T Value_Addr = NULL;
	uint8_t sq = 0;
	int i;

    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

	for(i=0;i<(asdu_num-1);i++){
		if((asdu_data[(StartAddr+i)%MAXYCRECORD].point + 1) == asdu_data[(StartAddr+i+1)%MAXYCRECORD].point){
			sq = 1;
		}
		else{
			sq = 0;
			break;
		}
	}
    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;
    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_M_ME_NC_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_COT_SPONT;
    asdu->_addr = Iec10x_Sta_Addr;


    /*build info*/
	if(asdu->_num._sq == 1){//连续
		/*info addr*/
	    ptr = info->_addr;
	    Temp32 = asdu_data[StartAddr%MAXYCRECORD].point + STARTSYC;;
	    memcpy(ptr, &Temp32, 3);
		/*info value*/
		ptr = info->_element;
		for(i=0; i<asdu_num; i++){
			Value = (PIEC10X_DETECT_DW_T)ptr;
			Value->_detect = asdu_data[(StartAddr+i)%MAXYCRECORD].Value32;
			Value->_qds = 0x40;
			ptr +=sizeof(IEC10X_DETECT_DW_T);
		}
	}
	else if(asdu->_num._sq == 0){//不连续
		ptr = info->_addr;
		for(i=0; i<asdu_num; i++){
			/*info addr*/
			Value_Addr = (PGX_104_INFO_YC32_T)ptr;
		    Temp32 = asdu_data[(StartAddr+i)%MAXYCRECORD].point + STARTSYC;;
		    memcpy(ptr, &Temp32, 3);
			/*info value*/
			Value_Addr->_detect = asdu_data[(StartAddr+i)%MAXYCRECORD].Value32;
			Value_Addr->_qds = 0x40;
			ptr += sizeof(GX_104_INFO_YC32_T);
		}
	
	}

    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);
	

    return RET_SUCESS;
}


int GX104Slaver_BuildDetect_Spont(int DevNo,uint8_t TimeFlag, PIEC10X_DETECT_T detectV, uint16_t addrV){

    uint8_t len = 0, asdu_num = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PIEC10X_DETECT_T detect = NULL;
    PCP56Time2a_T time = NULL;

    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /* check Time flag */
    if(TimeFlag != 1 && TimeFlag != 0){
        LOG("-%s-, error time flag(%d) \n",__FUNCTION__,TimeFlag);
        return RET_ERROR;
    }
    /*get value*/
    asdu_num = 1;

    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;
    /*build ASDU , COT ,Addr*/
    if(TimeFlag == 0)
        asdu->_type = IEC10X_M_ME_NA_1;
    else
        asdu->_type = IEC10X_M_ME_TD_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_COT_SPONT;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info addr*/
    ptr = info->_addr;
    Temp32 = addrV;
    memcpy(ptr, &Temp32, 3);

    /*build info value*/
    ptr = info->_element;
    detect = (PIEC10X_DETECT_T)ptr;
    detect->_detect = detectV->_detect;
    detect->_qds = detectV->_qds;
    ptr += sizeof(IEC10X_DETECT_T);
    if(TimeFlag == 1){
        time = (PCP56Time2a_T)ptr;
        IEC10X->GetTime(time);
        ptr += sizeof(CP56Time2a_T);
    }

    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);
	

    return RET_SUCESS;
}

int GX104Slaver_BuildDetectF_Spont(int DevNo,uint8_t TimeFlag, float detectV, uint16_t addrV){

    uint8_t len = 0, asdu_num = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PIEC10X_DETECT_F_T detect = NULL;
    PCP56Time2a_T time = NULL;

    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /* check Time flag */
    if(TimeFlag != 1 && TimeFlag != 0){
        LOG("-%s-, error time flag(%d) \n",__FUNCTION__,TimeFlag);
        return RET_ERROR;
    }
    /*get value*/
    asdu_num = 1;

    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    if(TimeFlag == 0)
        asdu->_type = IEC10X_M_ME_NC_1;
    else
        asdu->_type = IEC10X_M_ME_TF_1;
    asdu->_num._sq = 1;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_COT_SPONT;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info addr*/
    ptr = info->_addr;
    Temp32 = addrV;
    memcpy(ptr, &Temp32, 3);

    /*build info value*/
    ptr = info->_element;
    detect = (PIEC10X_DETECT_F_T)ptr;
    detect->_detect = detectV;
    detect->_qds = 0;
    ptr += sizeof(IEC10X_DETECT_F_T);
    if(TimeFlag == 1){
        time = (PCP56Time2a_T)ptr;
        IEC10X->GetTime(time);
        ptr += sizeof(CP56Time2a_T);
    }

    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;

    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);
	

    return RET_SUCESS;
}
//遥信
uint8_t GX104Slaver_BuildSignal(int DevNo,uint8_t reason, uint8_t DevType){

    uint8_t len = 0, asdu_num = 0;
	int i, offset = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
	uint8_t SendDataNum = 127;
	uint16_t YXNum;
    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*get yx num*/
	YXNum = gVars.TransYXTableNum;
	while(YXNum){
//		LOG("YXNum IS (%d)\n",YXNum);
		if(YXNum > 127){
			asdu_num = SendDataNum;
		}
		else{
			asdu_num = YXNum;
		}

	
	    /*build head*/
	    GX104SlaverData->Head = GX104Slaver_HEAD;

	    /*build control code*/
	    GX104SlaverData->Ctrl.I.Type = 0;
	    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
	    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;

			
	    /*build ASDU , COT ,Addr*/
	    asdu->_type = IEC10X_M_SP_NA_1;
	    asdu->_num._sq = 1;
	    asdu->_num._num = asdu_num;
	    asdu->_reason._reason = reason;
	    asdu->_addr = Iec10x_Sta_Addr;

	    /*build info addr*/
	    ptr = info->_addr;
	    Temp32 = GX104Slaver_INFOADDR_STATE_HXGF + offset;

	    memcpy(ptr, &Temp32, 3);
	    /*build info value*/
	    ptr = info->_element;
	    for(i = 0; i < asdu_num; i++){
			if((i + offset) > gVars.TransYXTableNum){
				perror("(i + OffSet) > gVars.TransYXTableNum\n");
				return 0;
			}
	        *ptr = ReadYx(i + offset);
	        ptr++;
			YXNum--;
	    }
		offset += SendDataNum;
	    /*len*/
	    len = ptr - GX104Slaver_Sendbuf;
	    GX104SlaverData->Len = len - 2;

//	    DumpHEX(GX104Slaver_Sendbuf,len);
	    /* enqueue to the transmisson queue */
		GX104_send(DevNo,GX104Slaver_Sendbuf, len);

	}

    return RET_SUCESS;
}
//遥测
uint8_t GX104Slaver_BuildDetect(int DevNo,uint8_t reason,uint8_t ValueType, uint8_t DevType){

    uint8_t len = 0, asdu_num = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PIEC10X_DETECT_T detect = NULL;
    PIEC10X_DETECT_DW_T detect32 = NULL;
	
	uint16_t YCNum;
	uint8_t SendDataNum = 20;
	int i, offset = 0;
    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*get yc num*/
	YCNum = gVars.TransYCTableNum;
//	LOG("YCNum IS (%d)\n",YCNum);
	while(YCNum){
		if(YCNum > SendDataNum){
			asdu_num = SendDataNum;
		}
		else{
			asdu_num = YCNum;
		}
	
	    /*build head*/
	    GX104SlaverData->Head = GX104Slaver_HEAD;

	    /*build control code*/
	    GX104SlaverData->Ctrl.I.Type = 0;
	    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
	    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;

	    /*build ASDU , COT ,Addr*/
	    asdu->_type = ValueType;
	    asdu->_num._sq = 1;
	    asdu->_num._num = asdu_num;
	    asdu->_reason._reason = reason;
	    asdu->_addr = Iec10x_Sta_Addr;

	    /*build info addr*/
	    ptr = info->_addr;
	    Temp32 = GX104Slaver_INFOADDR_VALUE_HXGF + offset;
	    memcpy(ptr, &Temp32, 3);

	    /*Build Detect value*/
	    ptr = info->_element;
//		LOG("ASDU_NUM IS (%d)\n",asdu_num);
	    for(i=0; i<asdu_num; i++){
			YCNum--;
			if(YCNum == 0)
				break;
	        if(ValueType == IEC10X_M_ME_NA_1){
	            detect = (PIEC10X_DETECT_T)ptr;
	            detect->_detect = ReadYcData16(i + offset);
	            detect->_qds = 0;
	            ptr += sizeof(IEC10X_DETECT_T);
	        }
	        else if(ValueType == IEC10X_M_ME_NC_1){
	            detect32 = (PIEC10X_DETECT_DW_T)ptr;
	            detect32->_detect = ReadYcData32(i + offset);
	            detect32->_qds = 0;
	            ptr += sizeof(IEC10X_DETECT_F_T);
	        }
	    }
		offset += SendDataNum;
	    /*len*/
	    len = ptr - GX104Slaver_Sendbuf;
	    GX104SlaverData->Len = len - 2;

//	    DumpHEX(GX104Slaver_Sendbuf,len);
	    /* enqueue to the transmisson queue */
		GX104_send(DevNo,GX104Slaver_Sendbuf, len);

	}

    return RET_SUCESS;
}

//响应定值召唤
uint8_t GX104Slaver_BuildDZ(int DevNo,uint8_t reason,uint8_t ValueType){

    uint8_t len = 0, asdu_num = 0;
	int i, offset = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PIEC10X_DETECT_T detect = NULL;
    PIEC10X_DETECT_DW_T detect32 = NULL;
	
	Data_Value_T DataValue;
	uint32_t value;
	uint16_t DZNum;
	uint8_t SendDataNum = 20;
    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*get yc num*/
	DZNum = gVars.TransDZTableNum;
//	LOG("DZNum IS (%d)\n",DZNum);
	while(DZNum){
		if(DZNum > SendDataNum){
			asdu_num = SendDataNum;
		}
		else{
			asdu_num = DZNum;
		}
	
	    /*build head*/
	    GX104SlaverData->Head = GX104Slaver_HEAD;

	    /*build control code*/
	    GX104SlaverData->Ctrl.I.Type = 0;
	    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
	    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;

	    /*build ASDU , COT ,Addr*/
	    asdu->_type = ValueType;
	    asdu->_num._sq = 1;
	    asdu->_num._num = asdu_num;
	    asdu->_reason._reason = reason;
	    asdu->_addr = Iec10x_Sta_Addr;

	    /*build info addr*/
	    ptr = info->_addr;
	    Temp32 = STARTSDZ + offset;
	    memcpy(ptr, &Temp32, 3);

	    /*Build Detect value*/
	    ptr = info->_element;
//		LOG("ASDU_NUM IS (%d)\n",asdu_num);
	    for(i=0; i<asdu_num; i++){
			DZNum--;
			if(DZNum == 0)
				break;
			
            *(uint32_t *)ptr = ReadDzData32(i + offset);
            ptr += 4;

			value = ReadDzData32(i + offset);
			DataValue._int32 = value;
			log("dz(%d) value is %f\n", i + offset, DataValue._float);
	    }
		offset += SendDataNum;
	    /*len*/
	    len = ptr - GX104Slaver_Sendbuf;
	    GX104SlaverData->Len = len - 2;

//	    DumpHEX(GX104Slaver_Sendbuf,len);
	    /* enqueue to the transmisson queue */
		GX104_send(DevNo,GX104Slaver_Sendbuf, len);

	}

    return RET_SUCESS;
}

uint8_t GX104Slaver_BuildActFinish(int DevNo,uint8_t qoi){

    uint8_t len = 0, asdu_num = 1;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;

    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_C_IC_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_ASDU_REASON_ACTFIN;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info addr*/
    ptr = info->_addr;
    Temp32 = IEC10X_INFO_ADDR_SIG_BASE+IEC10X_INFO_ADDR_SIG_TEMP_HX_OFF;
    Temp32 = 0;
    memcpy(ptr, &Temp32, 3);

    /*Build Detect value*/
    ptr = info->_element;
    ptr[0] = qoi;

    ptr+=1;
    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;

//    DumpHEX(GX104Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);

    return RET_SUCESS;
}
uint8_t GX104Slaver_BuildDZActFinish(int DevNo,uint8_t qoi){

    uint8_t len = 0, asdu_num = 1;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;

    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_TYPE_DZ_CALL;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_ASDU_REASON_ACTFIN;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info addr*/
    ptr = info->_addr;
    Temp32 = IEC10X_INFO_ADDR_SIG_BASE+IEC10X_INFO_ADDR_SIG_TEMP_HX_OFF;
    Temp32 = 0;
    memcpy(ptr, &Temp32, 3);

    /*Build Detect value*/
    ptr = info->_element;
    ptr[0] = qoi;

    ptr+=1;
    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;

//    DumpHEX(GX104Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);

    return RET_SUCESS;
}

uint8_t GX104Slaver_BuildAskClock(int DevNo, uint16_t rec_ms){
	uint16_t len = 0;
    uint8_t cs_temp = 0, i, asdu_num = 0, *ptr = NULL;

    /*init struct*/
    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
    PCP56Time2a_T time = (PCP56Time2a_T)(info->_element);

    /*get value*/
    asdu_num = 1;
    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*Ctrol*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_C_CS_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_COT_ACTCON;
    asdu->_addr = IEC10X_INFO_ADDR_NONE;
	
    /*clock value*/
    ptr = info->_element;
    IEC10X->GetTime(time);//获取系统时间，有待完善
	time->_milliseconds = rec_ms;//接收毫秒

    ptr+=sizeof(CP56Time2a_T);

    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;

	LOG("%s\n",__FUNCTION__);
    DumpHEX(GX104Slaver_Sendbuf,len);
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);

    return RET_SUCESS;
}


uint8_t GX104Slaver_BuildYkReturn(int DevNo,uint8_t qoi,uint8_t reason, YK_INFO_T Yk_info){

    uint8_t len = 0, asdu_num = 1;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;
    /*build ASDU , COT ,Addr*/
    asdu->_type = qoi;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = reason;
    asdu->_addr = Iec10x_Sta_Addr;
    /*build info addr*/
    ptr = info->_addr;
    Temp32 = Yk_info._addr + STARTSYK; 
    memcpy(ptr, &Temp32, 3);
    /*Build Detect value*/
    ptr = info->_element;
	memcpy(ptr, &Yk_info._YKData, 1);
    ptr+=1;
    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;

//    /* enqueue to the transmisson queue */
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);

    return RET_SUCESS;
}

uint8_t GX104Slaver_Build_DZ_Return(int DevNo)
{
	int i, len, num = 0, old = -1, sq = 1;
	int offset;
	uint32_t Temp32;
	PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
	PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
	PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
	uint8_t *ptr = NULL;

	
	for(i = 0; i < gVars.TransDZTableNum; i++){
		if(ReadDzFlag(i) == Flag_DZ_RET){
			if(old == -1){
				old = i;
			}else if((old+1) != i){
				sq = 0;
			}
			old = i;
			num++;
		}
	}
	if(num == 0)
		return 0;

    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;
    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_TYPE_DZ_DATA;
    asdu->_num._sq = sq;
    asdu->_num._num = num;
    asdu->_reason._reason = IEC10X_COT_ACTCON;
    asdu->_addr = Iec10x_Sta_Addr;

	if(sq){//连续
		for(i = 0; i < gVars.TransDZTableNum; i++){
			if(ReadDzFlag(i) == Flag_DZ_RET){
				break;
			}
		}
		offset = i;
		Temp32 = i + STARTSDZ;

		memcpy(info->_addr, &Temp32, 3);
		
		ptr = info->_element;
		for(i = 0 ; i < num; i++){
			*(int32_t *)ptr = ReadDzYZData32(offset+i);
			log("dz sq1 point is (%d)\n", offset+i);
			SetDZFlag(offset + i, Flag_Clear);
			ptr += 4;
		}
		
	}else{//不连续
		ptr = info->_addr;
		
		for(i = 0; i < gVars.TransDZTableNum; i++){
			if(ReadDzFlag(i) == Flag_DZ_RET){
				
				SetDZFlag(i, Flag_Clear);
				Temp32 = i + STARTSDZ;
				memcpy(ptr, &Temp32, 3);
				ptr += 3;
				*(int32_t *)ptr = ReadDzYZData32(i);
				ptr += 4;
			}
		}
	}
	
	*ptr = 9;
	ptr++;
    /*len*/
	len = ptr - GX104Slaver_Sendbuf;
	GX104SlaverData->Len = len - 2;
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);
	return RET_SUCESS;

}


uint8_t GX104Slaver_Build_U(int DevNo, uint8_t UType, uint8_t mode){

    uint8_t len = 0, Tester, Start, Stop;
    uint8_t *ptr = NULL;

    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;

    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

	GX104SlaverData->Ctrl.Func.Func = UType;
	GX104SlaverData->Ctrl.Func.Reserve = 0;
    /*build ASDU , COT ,Addr*/
    ptr=GX104SlaverData->Asdu;

    /*build info*/

    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;
//    DumpHEX(GX104Slaver_Sendbuf,len);
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);
    return RET_SUCESS;
}


uint8_t GX104Slaver_Build_S_Ack(int DevNo){

    uint8_t len = 0;
    uint8_t *ptr = NULL;

    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;

    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.S.Type1 = 1;
    GX104SlaverData->Ctrl.S.Type2 = 0;

    GX104SlaverData->Ctrl.S.Reserve = 0;
    GX104SlaverData->Ctrl.S.RecvSn = SendRecvSn.BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    ptr=GX104SlaverData->Asdu;

    /*build info*/

    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;
    DumpHEX(GX104Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);

    return RET_SUCESS;
}
int GX104Slaver_ASDU_Call(int DevNo, PIEC10X_ASDU_T Iec10x_Asdu){

    PASDU_INFO_T asdu_info = (PASDU_INFO_T)(Iec10x_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];

    uint32_t InfoAddr = 0;
	int fd = gpDevice[DevNo].fd;
	
    /* check info addrest */
    memcpy(&InfoAddr, asdu_info->_addr, 3);
    if(InfoAddr != 0){
        LOG("-%s- call cmd active error addr(%x) \n" ,__FUNCTION__ ,InfoAddr);
        return RET_ERROR;
    }
//	log("reanson (%d)  qoi(%d)\n",Iec10x_Asdu->_reason._reason,qoi);
    switch(Iec10x_Asdu->_reason._reason){

        case IEC10X_ASDU_REASON_ACT:
            switch(qoi){
                case IEC10X_CALL_QOI_TOTAL:
					GX104Slaver_BuildACKConfrim(DevNo, IEC10X_C_IC_NA_1, qoi);
					GX104Slaver_BuildSignal(DevNo,IEC10X_COT_INTROGEN, DATA_TYPE_YX);
					GX104Slaver_BuildDetect(DevNo,IEC10X_COT_INTROGEN,IEC10X_M_ME_NC_1, DATA_TYPE_YC);
					GX104Slaver_BuildActFinish(DevNo,qoi);
					gpDevice[DevNo].Flag_Brust_Send_Enable = ENABLE;
                    break;
                case IEC10X_CALL_QOI_GROUP1:
                case IEC10X_CALL_QOI_GROUP2:
                case IEC10X_CALL_QOI_GROUP9:
                case IEC10X_CALL_QOI_GROUP10:
					GX104Slaver_BuildACKConfrim(DevNo, IEC10X_C_IC_NA_1, qoi);
					GX104Slaver_BuildActFinish(DevNo,qoi);
					break;
				case IEC10X_CALL_QOI_GROUP16:
					GX104Slaver_BuildACKConfrim(DevNo, IEC10X_C_IC_NA_1, qoi);
					GX104Slaver_BuildOldYXSoe(DevNo, YXSoeRecord);
					GX104Slaver_BuildActFinish(DevNo,qoi);
                    break;
				case IEC10X_CALL_QOI_M_TOTAL:
					break;
                default:
                    LOG("-%s- call cmd error qoi(%d) \n", __FUNCTION__,qoi);
                    return RET_ERROR;
            }

            break;
        default:
            LOG("-%s- call cmd error reason(%d) \n", __FUNCTION__,Iec10x_Asdu->_reason._reason);
            break;
    }
    return RET_SUCESS;
}

int GX104Slaver_Deal_DZ_YZ(int DevNo, PIEC10X_ASDU_T GX104Slaver_Asdu)
{
	uint8_t *pData;
	uint8_t sq;
	uint8_t num;
	int i;
	int info_addr;
	uint32_t value;
	Data_Value_T DataValue;
	
	sq = GX104Slaver_Asdu->_num._sq;
	num = GX104Slaver_Asdu->_num._num;
	pData = GX104Slaver_Asdu->_info;

	if(sq){//连续
		info_addr = MAKEWORD(pData[0], pData[1]) - IEC10X_DZ_ADDR;
		pData += BINFOADDR;
		for(i = 0; i < num ; i++){
			value = MAKEDWORD(pData[0], pData[1], pData[2], pData[3]);
			WriteDzYz(DevNo, info_addr + i, value);	
			SetDZFlag(info_addr + i, Flag_DZ_YZ);
			pData += 4;
			gpDevice[DevNo].DZ_YZ_DEVID = GetDZRealID(info_addr + i);
//			DataValue._int32 = value;
//			log("dz(%d) value is %f\n", info_addr + i, DataValue._float);
		}
	}else{//不连续
		for(i = 0; i < num ; i++){
			info_addr = MAKEWORD(pData[0], pData[1]) - IEC10X_DZ_ADDR;
			pData += BINFOADDR;
			value = MAKEDWORD(pData[0], pData[1], pData[2], pData[3]);
			WriteDzYz(DevNo, info_addr, value);
			pData += 4;
			SetDZFlag(info_addr, Flag_DZ_YZ);
			gpDevice[DevNo].DZ_YZ_DEVID = GetDZRealID(info_addr);
//			DataValue._int32 = value;
//			log("dz(%d) value is %f\n", info_addr, DataValue._float);
		}
	}
	gpDevice[DevNo].Flag_Select_DZ = ACTIVATE;
}

int GX104Slaver_ASDU_DZ_JH(int DevNo, PIEC10X_ASDU_T Iec10x_Asdu){

    PASDU_INFO_T asdu_info = (PASDU_INFO_T)(Iec10x_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];
	int Dest_DevNo;//预置对象ID号
	
    switch(Iec10x_Asdu->_reason._reason){
        case IEC10X_ASDU_REASON_ACT:
			GX104Slaver_BuildACKConfrim(DevNo, IEC10X_TYPE_DZ_JH, qoi);
			Dest_DevNo = gpDevice[DevNo].DZ_YZ_DEVID;
			if(gpDevice[Dest_DevNo].SetDZ != NULL)
				gpDevice[Dest_DevNo].SetDZ(Dest_DevNo);
            break;


        default:
            perror(" call cmd error reason(%d) \n", Iec10x_Asdu->_reason._reason);
            break;
    }
    return RET_SUCESS;
}


int GX104Slaver_ASDU_DZCall(int DevNo, PIEC10X_ASDU_T Iec10x_Asdu){

    PASDU_INFO_T asdu_info = (PASDU_INFO_T)(Iec10x_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];
    uint32_t InfoAddr = 0;
	
    /* check info addrest */
    memcpy(&InfoAddr, asdu_info->_addr, 3);
    if(InfoAddr != 0){
        LOG("-%s- call cmd active error addr(%x) \n" ,__FUNCTION__ ,asdu_info->_addr);
        return RET_ERROR;
    }
//	LOG("Iec10x_Asdu->_reason is (%02x)\n",Iec10x_Asdu->_reason);
	LOG("qoi (%02x)\n",qoi);
	
    switch(Iec10x_Asdu->_reason._reason){

        case IEC10X_ASDU_REASON_ACT:
            switch(qoi){
                case IEC10X_CALL_QOI_TOTAL:
					LOG("IEC10X_CALL_QOI_TOTAL\n");
//					SetCallDZAllFlag();
					GX104Slaver_BuildACKConfrim(DevNo, IEC10X_TYPE_DZ_CALL, qoi);
					GX104Slaver_BuildDZ(DevNo,IEC10X_COT_INTROGEN,IEC10X_TYPE_DZ_YZ);
					GX104Slaver_BuildDZActFinish(DevNo,qoi);
					gpDevice[DevNo].Flag_Brust_Send_Enable = ENABLE;
                    break;

                default:
					perror("wrong asdu reason (0x:%2x)\n", qoi);
            }
            break;

        default:
            LOG("-%s- call cmd error reason(%d) \n", __FUNCTION__,Iec10x_Asdu->_reason._reason);
            break;
    }
    return RET_SUCESS;
}


int GX104Slaver_Aadu_Clock(int DevNo, PIEC10X_ASDU_T Iec10x_Asdu){

    PASDU_INFO_T asdu_info = (PASDU_INFO_T)(Iec10x_Asdu->_info);

    memcpy(&IEC10X_Cp56time2a,asdu_info->_element, sizeof(CP56Time2a_T));
    if(asdu_info->_addr[0] != 0 || asdu_info->_addr[1] != 0 || asdu_info->_addr[2] != 0){
        LOG("-%s- Clock cmd error addr(0x%02x:%02x:%02x) \n", __FUNCTION__,asdu_info->_addr[0],asdu_info->_addr[2],asdu_info->_addr[2]);
        return RET_ERROR;
    }

    switch(Iec10x_Asdu->_reason._reason){

        case IEC10X_COT_ACT:
            LOG("-%s- Clock SetTime cmd (20%d-%d-%d %d %d:%d:%d) \n", __FUNCTION__,IEC10X_Cp56time2a._year._year,IEC10X_Cp56time2a._month._month,IEC10X_Cp56time2a._day._dayofmonth,
                    IEC10X_Cp56time2a._day._dayofweek,IEC10X_Cp56time2a._hour._hours,IEC10X_Cp56time2a._min._minutes,IEC10X_Cp56time2a._milliseconds);
            /*get time*/
            /*...*/
            IEC10X->SetTime(&IEC10X_Cp56time2a);
			GX104Slaver_BuildAskClock(DevNo, IEC10X_Cp56time2a._milliseconds);

            break;
        case IEC10X_COT_SPONT:
            LOG("-%s- Clock cmd spont \n", __FUNCTION__);
            break;
        default:
            LOG("-%s- Clock cmd error reason(%d) \n", __FUNCTION__,Iec10x_Asdu->_reason._reason);
            break;
    }
    return RET_SUCESS;
}

int GX104Slaver_ASDU_SetAct(PIEC10X_ASDU_T Iec10x_Asdu, uint8_t Type){

    PASDU_INFO_T asdu_info = (PASDU_INFO_T)(Iec10x_Asdu->_info);
    uint8_t *ptr = NULL;
    uint8_t n = Iec10x_Asdu->_num._num, Sq = Iec10x_Asdu->_num._sq, i;
    float Value = 0.0;
    uint32_t InfoAddr = 0;
            
    /* if sq == 1 */
    PIEC10X_DETECT_T detect = NULL;
    PIEC10X_DETECT_F_T detect_f = NULL;
  
    /* if sq == 0 */  
    PIEC10X_DETECT_SQ0_T detect_Sq0 = NULL;
    PIEC10X_DETECT_SQ0_F_T detect_Sq0_f = NULL;

    /* check info addrest */
    memcpy(&InfoAddr, asdu_info->_addr, 3);

    switch(Iec10x_Asdu->_reason._reason){

        case IEC10X_COT_ACT:
        
            switch(Type){
                case IEC10X_C_SE_NA_1:
                
                    if(Sq == 1){
                        ptr = asdu_info->_element;
                        for(i=0; i<n; i++){

                            detect = (PIEC10X_DETECT_T)ptr;
                            Value = (float)(detect->_detect);
                            ptr += sizeof(IEC10X_DETECT_T);
                            IEC10X->SetConfig(Value, InfoAddr+i);
                        }
                    }else if(Sq == 0){
                        ptr = Iec10x_Asdu->_info;
                        for(i=0; i<n; i++){

                            detect_Sq0 = (PIEC10X_DETECT_SQ0_T)ptr;
                            Value = (float)(detect_Sq0->_detect);
                            InfoAddr = 0;
                            memcpy(&InfoAddr, detect_Sq0->_addr, 3);
                            IEC10X->SetConfig(Value, InfoAddr);
                            ptr += sizeof(IEC10X_DETECT_SQ0_T);   
                        }
                        
                    }
                    break;
                case IEC10X_C_SE_NC_1:
                    if(Sq == 1){
                        ptr = asdu_info->_element;
                        for(i=0; i<n; i++){
                   
                            detect_f = (PIEC10X_DETECT_F_T)ptr;
                            Value = detect_f->_detect;
                            ptr += sizeof(IEC10X_DETECT_F_T);
                            IEC10X->SetConfig(Value, InfoAddr+i);
                        }
                    }else if(Sq == 0){
                        ptr = Iec10x_Asdu->_info;
                        for(i=0; i<n; i++){
                            detect_Sq0_f = (PIEC10X_DETECT_SQ0_F_T)ptr;
                            Value = (float)(detect_Sq0_f->_detect);
                            memcpy(&InfoAddr, detect_Sq0_f->_addr, 3);
                            IEC10X->SetConfig(Value, InfoAddr);
                            ptr += sizeof(IEC10X_DETECT_SQ0_F_T); 
                        }
                    }
                    break;
                default:
                    LOG("-%s-, Type error !",__FUNCTION__);
                    return RET_ERROR;
            }
            break;

        default:
            perror("error reason \n");
            return RET_ERROR;
    }
    return RET_SUCESS;
}


int GX104Slaver_ASDU_DZ_YZ(int DevNo, PGX104Slaver_DATA_T GX104SlaverData){

    PIEC10X_ASDU_T GX104Slaver_Asdu = NULL;
    GX104Slaver_Asdu = (PIEC10X_ASDU_T)GX104SlaverData->Asdu;
    PASDU_INFO_T asdu_info = (PASDU_INFO_T)(GX104Slaver_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];

    switch(GX104Slaver_Asdu->_reason._reason){
        case IEC10X_ASDU_REASON_ACT:
			GX104Slaver_Deal_DZ_YZ(DevNo, GX104Slaver_Asdu);
            break;
        case IEC10X_COT_STOPACT:
			GX104Slaver_Send_Mirror(DevNo, GX104SlaverData, IEC10X_ASDU_REASON_REFUSEACT);
            break;

        default:
            perror("call cmd error reason(%d) \n", GX104Slaver_Asdu->_reason._reason);
            break;
    }
    return RET_SUCESS;
}

uint8_t GX104Slaver_Build_SetAck(int DevNo,uint8_t Prio, uint8_t Type){

    uint8_t len = 0;
    uint8_t *ptr = NULL;

    PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*build head*/
    GX104SlaverData->Head = GX104Slaver_HEAD;

    /*build control code*/
    GX104SlaverData->Ctrl.I.Type = 0;
    GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = Type;
    asdu->_num._sq = 0;
    asdu->_num._num = 0;
    asdu->_reason._reason = IEC10X_ASDU_REASON_ACTFIN;
    asdu->_addr = Iec10x_Sta_Addr;

    /*build info*/
    ptr = info->_addr;

    /*len*/
    len = ptr - GX104Slaver_Sendbuf;
    GX104SlaverData->Len = len - 2;
    
    //DumpHEX(GX104Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);
	

    return RET_SUCESS;
}

int GX104Slaver_Deal_SN(uint16_t SendSn, uint16_t RecvSn){

    
#if 0
    if(SendSn > SendRecvSn.DealSendSn+1){
        LOG("-%s-, error,send last(%d),now(%d) \n",__FUNCTION__,SendRecvSn.DealSendSn,SendSn);
        GX104Slaver_STATE_FLAG_INIT = GX104Slaver_FLAG_SEND_CLOSED;
        return RET_ERROR;
    }else if(SendSn < SendRecvSn.DealSendSn+1){
        LOG("-%s-, Retransmit,send last(%d),now(%d) \n",__FUNCTION__,SendRecvSn.DealSendSn,SendSn);
        return RET_ERROR;
    }
    if(RecvSn != SendRecvSn.BuildSendSn){
        LOG("-%s-, error,receive last(%d),now(%d) \n",__FUNCTION__,SendRecvSn.BuildSendSn,RecvSn);
        GX104Slaver_STATE_FLAG_INIT = GX104Slaver_FLAG_SEND_CLOSED;
        return RET_ERROR;
    }
    if(RecvSn < SendRecvSn.DealRecvSn){
        LOG("-%s-, error,receive2 last(%d),now(%d) \n",__FUNCTION__,SendRecvSn.DealRecvSn,RecvSn);
        return RET_ERROR;
    }
#endif

    if(SendSn < SendRecvSn.DealSendSn || RecvSn < SendRecvSn.DealRecvSn){

        perror("error:send receive num\n");
        return RET_ERROR;
    }
    SendRecvSn.BuildRecvSn = SendSn+1;

    SendRecvSn.DealSendSn = SendSn;
    SendRecvSn.DealRecvSn = RecvSn;

    //SendRecvSn.BuildRecvSn++;

    /* return S ACK */
    GX104Slaver_STATE_FLAG_S_ACK = GX104Slaver_FLAG_S_ACK;
    GX104Slaver_TimeCycle_S = 0;

    return RET_SUCESS;
}
int GX104Slaver_Deal_YK(int DevNo, PGX104Slaver_DATA_T GX104SlaverData){

	uint32_t YKDevNo = 0;//遥控目标设备
	uint32_t YKIndex = 0;

    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);



	YKIndex = MAKEDWORD(info->_addr[0], info->_addr[1], info->_addr[2], 0)-STARTSYK;
	//本地遥控地址
	gpDevice[DevNo].YKInfo._addr = YKIndex;
	gpDevice[DevNo].YKInfo._YKData = *(YK_DATA_T *)info->_element;
	
	if(YKIndex >= gVars.TransYKTableNum){
		perror("YKIndex > gVars.TransYKTableNum\n");
		gpDevice[DevNo].YKInfo._Return._refuse = 1;
		sprintf(gpDevice[DevNo].logbuf, "GX104Slaver YKIndex > gVars.TransYKTableNum \n");
		write_log(gpDevice[DevNo].logbuf, LOGMARNING, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	YKDevNo = TransYKTable[YKIndex].wRealID;
	if(YKDevNo >= MAX_SLAVE_DEVICE_NUM ){
		gpDevice[DevNo].YKInfo._Return._refuse = 1;
		PERROR("YKID(%d) > MAX_SLAVE_DEVICE_NUM(%d)!\n", YKDevNo, MAX_SLAVE_DEVICE_NUM);
		return -1 ;
	}
	
	if(YKDevNo > gVars.dwDevNum){
		PERROR("YKID(%d) > gVars.dwDevNum(%d)!\n", YKDevNo, gVars.dwDevNum);
		return -1;
	}
	if(gpDevice[YKDevNo].SetYK == NULL){
		perror("gpDevice[YKDevNo].SetYK == NULL, protocol(%s)\n", gpDevice[DevNo].Protocol);
		return -1;
	}
	gpDevice[YKDevNo].SetYK(YKDevNo, asdu->_type, asdu->_reason._reason, info->_element[0]);
	gpDevice[YKDevNo].YK_FROM_ID = DevNo;
	
	return RET_SUCESS;
}
static int Check_Brust_Yc(void)
{
	int YCNum = 0;
	uint16_t i;
	uint32_t Value;

	
	for(i=0;i<gVars.TransYCTableNum;i++){
		if(ReadYcFlag(i) == Flag_Spon){
			Value = ReadYcData32(i);
//			SetYCFlag(i, Flag_Clear);
			YCNum++;
		}
	}
	return YCNum;
}



uint8_t GX104laver_BuildBrustYC(int DevNo, uint8_t ValueType){
	

	uint8_t len = 0, asdu_num = 0, i,j = 0;
	uint8_t *ptr = NULL;
	uint32_t Temp32 = 0;
	PIEC10X_DETECT_T detect = NULL;
	PIEC10X_DETECT_DW_T detect32 = NULL;
	uint8_t tempbuf[50];
	int YCNum = 0;
	uint8_t SendDataNum = 20;
	uint8_t offset = 0;
	PGX104Slaver_DATA_T GX104SlaverData = (PGX104Slaver_DATA_T)GX104Slaver_Sendbuf;
	PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);
	PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
	uint8_t sq = 0;
	TBurstAI burstAI;
	int YCSendNum;
	/*get yc num*/
//	YCNum = Check_Brust_Yc();
  	YCNum= GetBurstAINum(DevNo);

	if(YCNum == 0){
		return 0;
	}
	if(YCNum > YCSendNum){
		YCSendNum = 20;
	}else{
		YCSendNum = YCNum;
	}
	
	/*build head*/
	GX104SlaverData->Head = GX104Slaver_HEAD;
	
	/*build control code*/
	GX104SlaverData->Ctrl.I.Type = 0;
	GX104SlaverData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
	GX104SlaverData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;
	
	/*build ASDU , COT ,Addr*/
	asdu->_type = ValueType;
	asdu->_num._sq = sq;
	asdu->_num._num = YCSendNum;
	asdu->_reason._reason = IEC10X_COT_SPONT;
	asdu->_addr = gpDevice[DevNo].Address;
	
	/*build info addr*/
	ptr = info->_addr;
	
	for(i=0;i<YCSendNum;i++){
		if(ReadBurstAI(DevNo, &burstAI) != TRUE)
			break;
		Temp32 = GX104Slaver_INFOADDR_VALUE_HXGF + burstAI.dwNo;
		memcpy(ptr, &Temp32, 3);
		ptr += 3;
		detect32 = (PIEC10X_DETECT_DW_T)ptr;
		detect32->_detect = burstAI.sValue;
		detect32->_qds = 0;
		ptr += sizeof(IEC10X_DETECT_F_T);
	}

	/*len*/
	len = ptr - GX104Slaver_Sendbuf;
	GX104SlaverData->Len = len - 2;
//		DumpHEX(GX104Slaver_Sendbuf,len);
	/* enqueue to the transmisson queue */
	GX104_send(DevNo,GX104Slaver_Sendbuf, len);

    return RET_SUCESS;
}




int GX104Slaver_Deal_I(int DevNo, PGX104Slaver_DATA_T GX104SlaverData, uint16_t len){

    uint8_t Type,mode;
    uint16_t RecvSn,SendSn;
    uint32_t FirmwareType = 0;
	
	uint16_t DeviceID;
	
	DeviceID = DevNo;
	
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104SlaverData->Asdu);

    SendSn = GX104SlaverData->Ctrl.I.SendSn;
    RecvSn = GX104SlaverData->Ctrl.I.RecvSn;
    /* check asdu addrest */
//    if(Iec10x_Sta_Addr != asdu->_addr){
//        LOG("-%s-, error asdu addr(%x)(%x) \n" ,__FUNCTION__ ,Iec10x_Sta_Addr,asdu->_addr);
//        return RET_ERROR;
//    }
    
    /* deal the receive and send serial number */
	
    if(GX104Slaver_Deal_SN(SendSn, RecvSn) == RET_ERROR){
        return RET_ERROR;
    }
	
    Type = asdu->_type;
	
	
	switch(Type){

		case IEC10X_C_IC_NA_1://总召唤
			LOG("++++Asdu Type Call cmd... \n");
			GX104Slaver_ASDU_Call(DevNo, asdu);
			break;
		case IEC10X_TYPE_DZ_CALL://定值召唤
			LOG("++++asdu type DZ call cmd... \n");
			GX104Slaver_ASDU_DZCall(DevNo, asdu);
			break;
		case IEC10X_TYPE_DZ_YZ://定值预置
			LOG("++++asdu type DZ yuzhi cmd... \n");
			GX104Slaver_ASDU_DZ_YZ(DevNo, GX104SlaverData);
			break;
		case IEC10X_TYPE_DZ_JH://定值激活
			LOG("++++asdu type DZ jihuo cmd... \n");
			GX104Slaver_ASDU_DZ_JH(DevNo, asdu);
			gpDevice[DevNo].Flag_Select_DZ ==  Flag_Clear;
			break;
			
		case IEC10X_C_CS_NA_1://对时
			LOG("++++Asdu Type Clock syc cmd... \n");
			GX104Slaver_Aadu_Clock(DevNo, asdu);
			break;
		case IEC10X_C_RP_NA_1://复位进程
			LOG("++++Asdu Reset Process cmd... \n");
			GX104Slaver_BuildResetProcess(DevNo, 1);
			GX104Slaver_ResetFlag(DevNo);
			break;
			
		case IEC10X_C_SC_NA_1://单点遥控
			LOG("++++Asdu YK... \n");
			GX104Slaver_Deal_YK(DevNo, GX104SlaverData);
		
			break;
		case IEC10X_C_SE_NA_1://双点遥控
//				GX104Slaver_Deal_YK_return(GX104SlaverData, &Client_info->YK_Return, IEC10X_C_SE_NA_1);
			break;

			
		default:
			LOG("-%s-, error Type(%d) \n", __FUNCTION__,Type);
			return RET_ERROR;
	}
	return RET_SUCESS;

}


uint8_t GX104Slaver_Deal_S(int DevNo, PGX104Slaver_DATA_T GX104SlaverData, uint16_t len){


    return RET_SUCESS;
}
uint8_t GX104Slaver_Deal_U(int DevNo, PGX104Slaver_DATA_T GX104SlaverData, uint16_t len){

    switch(GX104SlaverData->Ctrl.Func.Func){

    	case GX104Slaver_U_FUNC_STARTDT:
//            LOG(">%s<, function STARTDT \n",__FUNCTION__);
			gpDevice[DevNo].Flag_Brust_Send_Enable = DISABLE;
			GX104Slaver_ResetFlag(DevNo);
            GX104Slaver_Build_U(DevNo,GX104Slaver_U_FUNC_STARTDT_ACK,1);
            GX104Slaver_Build_InitFin(DevNo);
			sprintf(gpDevice[DevNo].logbuf, "GX104Slaver Start link \n");
			write_log(gpDevice[DevNo].logbuf, LOGDEBUG, __FILE__, __FUNCTION__, __LINE__);
			
            break;

        case GX104Slaver_U_FUNC_STOPDT:
//            LOG(">%s<, function STOPDT \n",__FUNCTION__);
            IEC10X->CloseLink();
            GX104Slaver_Build_U(DevNo,GX104Slaver_U_FUNC_STOPDT,1);
            GX104Slaver_STATE_FLAG_INIT = GX104Slaver_FLAG_RECV_CLOSED;
            break;
        case GX104Slaver_U_FUNC_TESTER:
//            LOG(">%s<, function TESTER \n",__FUNCTION__);
//            GX104Slaver_Build_U(DevNo,GX104Slaver_U_FUNC_TESTER_ACK,1);
			SendRecvSn.TesterCount=0;
            break;

        /* U ACK */
        case GX104Slaver_U_FUNC_STOPDT_ACK:
//            LOG(">%s<, function STOPDT ACK\n",__FUNCTION__);
            GX104Slaver_STATE_FLAG_INIT = GX104Slaver_FLAG_RECV_CLOSED;
            break;
        case GX104Slaver_U_FUNC_TESTER_ACK:
//            LOG(">%s<, function TESTER ACK\n",__FUNCTION__);
			GX104Slaver_Build_U(DevNo,GX104Slaver_U_FUNC_TESTER,0);
            SendRecvSn.TesterCount = 0;
            break;
        default:
            LOG(">%s<, function ERROR \n",__FUNCTION__);
            break;
    }
    return RET_SUCESS;
}

int GX104Slaver_Receive(int DevNo, uint8_t *buf, uint16_t len){

    uint8_t *BufTemp = NULL;
    int16_t LenRemain,LenTmp;
    PGX104Slaver_DATA_T GX104SlaverData = NULL;
	

    if(buf == NULL){
        LOG("-%s-,buffer (null)",__FUNCTION__);
        return RET_ERROR;
    }
#if 0
    if(len <= 0 || len>GX104Slaver_MAX_BUF_LEN || len<BufTemp[0]+2){
        LOG("-%s-,buffer len error(%d) \n",__FUNCTION__,len);
        return;
    }
#endif

    BufTemp = buf;
    LenRemain = len;

    while(BufTemp<buf+len){

        GX104SlaverData = (PGX104Slaver_DATA_T)BufTemp;
        Iec10x_Lock();
        if(GX104SlaverData->Head == GX104Slaver_HEAD){

            LenTmp = GX104SlaverData->Len + 2;
            if(LenRemain < GX104Slaver_HEAD_LEN){
                LOG("_%s_,len error(%d) \n",__FUNCTION__,len);
                Iec10x_UnLock();
                return RET_ERROR;
            }
            if(GX104SlaverData->Ctrl.Type.Type1 == 0){
//                LOG("-%s-,Frame Type I \n",__FUNCTION__);
                GX104Slaver_Deal_I(DevNo, GX104SlaverData, LenTmp);

            }else if(GX104SlaverData->Ctrl.Type.Type1 == 1 && GX104SlaverData->Ctrl.Type.Type2 == 0){
//                LOG("-%s-,Frame Type S \n",__FUNCTION__);
                GX104Slaver_Deal_S(DevNo, GX104SlaverData, LenTmp);

            }else if(GX104SlaverData->Ctrl.Type.Type1 == 1 && GX104SlaverData->Ctrl.Type.Type2 == 1){
//                LOG("-%s-,Frame Type U \n",__FUNCTION__);
                GX104Slaver_Deal_U(DevNo, GX104SlaverData, LenTmp);
            }
        }else{
            LOG("-%s-,head type error(%d) \n",__FUNCTION__,BufTemp[0]);
            Iec10x_UnLock();
            return RET_ERROR;
        }
        Iec10x_UnLock();
        BufTemp+=LenTmp;
        LenRemain-=LenTmp;
    }
    return RET_SUCESS;
}

int CheckYKReturn104(int DevNo)
{
	
	if(gpDevice[DevNo].YKInfo._Return._ackconfirm == 1){
		GX104Slaver_BuildYkReturn(DevNo, IEC10X_C_SC_NA_1, IEC10X_ASDU_REASON_ACTCON, gpDevice[DevNo].YKInfo);
		gpDevice[DevNo].YKInfo._Return._ackconfirm = 0;
	}else if(gpDevice[DevNo].YKInfo._Return._stopackconfirm == 1){
		 GX104Slaver_BuildYkReturn(DevNo, IEC10X_C_SC_NA_1, IEC10X_ASDU_REASON_REFUSEACT, gpDevice[DevNo].YKInfo);
		 gpDevice[DevNo].YKInfo._Return._stopackconfirm = 0;
	}else if(gpDevice[DevNo].YKInfo._Return._finish == 1){
		 GX104Slaver_BuildYkReturn(DevNo, IEC10X_C_SC_NA_1, IEC10X_ASDU_REASON_ACTFIN, gpDevice[DevNo].YKInfo);
		 gpDevice[DevNo].YKInfo._Return._finish = 0;
	}else if(gpDevice[DevNo].YKInfo._Return._refuse == 1){
		 GX104Slaver_BuildYkReturn(DevNo, IEC10X_C_SC_NA_1, IEC10X_ASDU_REASON_REFUSE, gpDevice[DevNo].YKInfo);
		 gpDevice[DevNo].YKInfo._Return._refuse = 0;
	}else if(gpDevice[DevNo].YKInfo._Return._refusefinish == 1){
		 GX104Slaver_BuildYkReturn(DevNo, IEC10X_C_SC_NA_1, IEC10X_ASDU_REASON_REFUSE_FIN, gpDevice[DevNo].YKInfo);
		 gpDevice[DevNo].YKInfo._Return._refusefinish = 0;
		 gpDevice[DevNo].Respon_Confirm = 0;
	}

}

int CheckDZYZReturn104()
{
	int i, num = 0;

	for(i = 0; i < gVars.TransDZTableNum; i++){
		if(ReadDzFlag(i) == Flag_DZ_RET) num++;
	}
	if(num == 0)
		return 0;
	return num;
}

int GX104ScanBrustData(int DevNo)
{
	uint16_t StartAddr = 0,BrustYxNum = 0, BrustYxSoeNum = 0, BrustYcNum = 0, BrustDzNum = 0;
	if(gpDevice[DevNo].Flag_Brust_Send_Enable == DISABLE){
		return 0;
	}
		
	if(ScanBrustYx(DevNo, &gpDevice[DevNo].ReadYXPtr, &StartAddr, &BrustYxNum)){
		GX104Slaver_BuildBrustYX(DevNo, BrustYxNum, StartAddr, YXBrustRecord);
		return 1;
	}
	if(ScanBrustYxSoe(DevNo, &gpDevice[DevNo].ReadYXSoePtr, &StartAddr, &BrustYxSoeNum)){
		GX104Slaver_BuildBrustYXSoe(DevNo, BrustYxSoeNum,IEC10X_COT_SPONT , StartAddr, YXSoeRecord);
		return 1;
	}
	if(ScanBrustDZ(DevNo, &gpDevice[DevNo].ReadDZPtr, &StartAddr, &BrustDzNum)){
		GX104Slaver_BuildBrustDZ(DevNo, BrustDzNum, StartAddr, DZBrustRecord);
		return 1;
	}


}


static int GX104Slaver_Task(int DevNo)
{
	/*突发信息*/
	GX104ScanBrustData(DevNo);

	/* 遥控 */
	CheckYKReturn104(DevNo);

	return 0;
}

int GX104Slaver_On_Time_Out(int DevNo){

	Now_Time_cnt++;
	if(Now_Time_cnt%5 == 0){
		if(gpDevice[DevNo].Flag_Brust_Send_Enable == DISABLE){
			return 0;
		}
		GX104laver_BuildBrustYC(DevNo, IEC10X_M_ME_NC_1);
	}
	if(gpDevice[DevNo].Flag_Select_DZ ==  ACTIVATE){
		if(CheckDZYZReturn104()){
			GX104Slaver_Build_DZ_Return(DevNo);
			return 0;
		}
	}
	
}

