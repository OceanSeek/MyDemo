/*******************************************************************
Copyright (C):    
File name    :    GX104Master.c
DESCRIPTION  :
AUTHOR       :
Version      :    1.0
Date         :    2017/07/31
Others       :
History      :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*******************************************************************/

#include "sys.h"
//#include "GX104Master.h"

//#include "queue.h"
extern uint32_t sys_time_cnt;
static SendRecvSn_T SendRecvSn;
uint32_t Old_Time_cnt = 0;
uint32_t tcp_master_time_cnt_old = 0;
	
extern pthread_mutex_t mutex;
 /*
 * GLOABLE VARIALBLE
 */

uint8_t             GX104Master_Sendbuf[GX104Master_MAX_BUF_LEN];

uint8_t             GX104Master_Call_AllQoi = 0;
uint8_t             GX104Master_Call_GroupQoi = 0;


#define GX104Master_CYCLE_TIME_MS             100                   /*100ms*/
#define GX104Master_RESEND_TIME_MS            (30*1000)             /*30s*/
#define GX104Master_S_ACK_TIMEOUT             (5*1000)              /*5s*/
#define GX104Master_TESTER_IDLE_TIMEOUT       (1*30*1000)           /*2min*/
uint32_t            GX104Master_TimeCount = 0;
uint32_t            GX104Master_TimeCycle = GX104Master_RESEND_TIME_MS;
uint32_t            GX104Master_TimeCycle_S = 0;
uint32_t            GX104Master_TimeCycle_TesterIdle = 0;

uint32_t            GX104Master_Update_SeekAddr = 0;
uint16_t            GX104Master_FirmFlagCount = 0;

int GX104Master_Receive(int DevNO, uint8_t *buf, uint16_t len);
int GX104Master_BuildYk(int DevNO, uint8_t Type, uint16_t reason, uint8_t YKData);
int GX104Master_BuildSetDZ(int DevNO);
int GX104Master_Task(int DevNO);
int GX104Master_On_Time_Out(int DevNO);

int Init_GX104Master(int DevNO)
{
	gpDevice[DevNO].SetYK = GX104Master_BuildYk;
	gpDevice[DevNO].SetDZ = GX104Master_BuildSetDZ;
	gpDevice[DevNO].Receive = GX104Master_Receive;
	gpDevice[DevNO].Task = GX104Master_Task;
	gpDevice[DevNO].OnTimeOut = GX104Master_On_Time_Out;
	
	
}


void GX104Master_Start_Clear(void){
	GX104Master_TimeCycle = 0;
	GX104Master_TimeCount = 0;
}
void GX104Master_GetSysTime(CP56Time2a_T* Time){

	time_t timep;
	struct tm *p;
	time(&timep);
	p=gmtime(&timep);
	
	Time->_year._year 		=	(p->tm_year -100);
	Time->_month._month 	=	1+p->tm_mon;
	Time->_day._dayofmonth 	=	p->tm_mday;
	Time->_hour._hours	=	p->tm_hour;
	Time->_min._minutes	=	p->tm_min;
	Time->_milliseconds	=	p->tm_sec;
}

uint8_t GX104Master_send(int DevNO,char *buf, int len){

	if(strcmp("udp", gpDevice[DevNO].TcpType) == 0){
		struct sockaddr_in dest_addr = {};
		dest_addr.sin_family = AF_INET;//ipv4
		dest_addr.sin_port = htons(gpDevice[DevNO].UDP_Dest_PORT);
		dest_addr.sin_addr.s_addr = inet_addr(gpDevice[DevNO].UDP_Dest_IP);
		sendto(gpDevice[DevNO].fd, buf, len, 0, (struct sockaddr *)&dest_addr,sizeof(dest_addr)); 
	}
    else if(-1 == write(gpDevice[DevNO].fd, buf,len)){
        perror("Send error fd ,fd is (%d)\n",gpDevice[DevNO].fd);
        return RET_ERROR;
    }
    gpDevice[DevNO].ReSendNum++;

	MonitorTx(monitorData._TX_ID, DevNO, monitorData._fd, buf, len);
    return RET_SUCESS;
}

/*****************************************************************
函数名称: GX104Master_Deal_RecvSYx
函数功能: 处理单点遥信入库 
输入参数: 
返回值：  无
修改日期：2019-11-22
1、未定义的点不发送，即nIndex=0且ID=0的突发遥信会发送，其他的不发送  
*****************************************************************/
void GX104Master_Deal_RecvSYx(int m_dwDevID,PGX104Master_DATA_T GX104MasterData)
{
	uint16_t wBINo;
	uint16_t i;	
	uint8_t *pData,BINum;

    uint8_t Type,COT;
    uint16_t RecvSn,SendSn;
    uint32_t FirmwareType = 0;
	CP56Time2a_T SoeTime;
	TSysTimeSoe YxValue; 


	/**test**/
//	uint8_t test_buf[20]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
//	PGX104Master_DATA_T buf_tmp = (PGX104Master_DATA_T)test_buf;
//    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(buf_tmp->Asdu);
	/**test**/
	
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);

    SendSn = GX104MasterData->Ctrl.I.SendSn;
    RecvSn = GX104MasterData->Ctrl.I.RecvSn;

    
    /* deal the receive and send serial number */
//    if(GX104Master_Deal_SN(SendSn, RecvSn) == RET_ERROR){
//        return RET_ERROR;
//    }
//    /* Start tester timer */
//    GX104Master_Tester_Timer();




	
//	if ( ((asdu->_reason<0x14) || (asdu->_reason>=0x14+bGroupYx))
//		&&(asdu->_reason!=3) && (asdu->_reason!=0x0b) )
//		return;
		
	BINum = asdu->_num._num; //可变结构限定词
	pData = asdu->_info;
	Type = asdu->_type;
	COT = asdu->_reason._reason;
	if ( asdu->_num._sq )//连续
	{	
		asdu->_num._sq ^= 1;
		
		wBINo = MAKEWORD(pData[DEALSTATRT], pData[DEALSTATRT+1])-STARTSYX;
		pData += DEALSTATRT+BINFOADDR;
		for(i=0; i<BINum; i++)
		{
			if (wBINo+i > wAllSYxNum){
				perror("wBINo+i > wAllSYxNum");
				return;
			}
			if ( pData[i]&1 )
				WriteYx(m_dwDevID,i+wBINo,PBI_ON);
			else 
				WriteYx(m_dwDevID,i+wBINo,PBI_OFF);

			if(COT == IEC10X_COT_SPONT){//突发遥信写进队列
				gpDevice[m_dwDevID].pBurstBI[i+wBINo].flag = Flag_Spon;
			}

		}
		
//		for(i=0; i<BINum;i++){
//			log("yx npoint is (%d) value is (%d)\n",i,ReadYx(m_dwDevID,i));;
//		}
//		
		return;
	}
	
	//不连续
	pData += DEALSTATRT;
	for(i=0; i<BINum; i++,pData+=BINFOADDR+1)
	{
	
		wBINo = MAKEWORD(pData[0], pData[1]) - STARTSYX;
		
		if ( wBINo > wAllSYxNum )
			return;
//		log("____pData[BINFOADDR](%d)\n",pData[BINFOADDR]);
		if ( pData[BINFOADDR]&1 )
			WriteYx(m_dwDevID,wBINo,PBI_ON);
		else 
			WriteYx(m_dwDevID,wBINo,PBI_OFF);
		if(COT == IEC10X_COT_SPONT){
			gpDevice[m_dwDevID].pBurstBI[wBINo].flag = Flag_Spon;
		}
	}
//	log("queue len is (%d)\n",LengthQueue(&YxQueue));
	
	return;
}

/*****************************************************************
函数名称: CM104gx::GX104Master_Deal_RecvSSoe
函数功能: 处理单点SOE入库 
输入参数: 
返回值：  
修改日期：2019/09/29
	待增加写遥信标志位，单独开一个线程用来写遥信soe入数据库，方便使用事务方式写数据库
	遥信存储到遥信转发表中（RAM），再到线程中读取遥信转发表的读状态，未读的写进
	数据库，并清除未读状态。
*****************************************************************/
void GX104Master_Deal_RecvSSoe(uint16_t m_dwDevID,PGX104Master_DATA_T GX104MasterData)
{
	uint16_t i,val;	
	uint16_t wSoeNo;
	uint16_t wMSecond;
	uint8_t bSoeNum;
	uint8_t *pData;

	uint8_t SoeDataWith = 11;
	uint8_t TimeStartAddr = 4;
	uint8_t ValueStartAddr = 3;
	
    uint8_t Type;
    uint16_t RecvSn,SendSn;
    uint32_t FirmwareType = 0;
	
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);
	PCP56Time2a_T time = NULL;
    SendSn = GX104MasterData->Ctrl.I.SendSn;
    RecvSn = GX104MasterData->Ctrl.I.RecvSn;
		
	if ( (asdu->_reason._reason!=3) && (asdu->_reason._reason!=0x0b) )
		return;

	pData = asdu->_info;
	
	bSoeNum = asdu->_num._num; 
	log("bSoeNum(%d)\n",bSoeNum);
	for(i=0; i<bSoeNum; i++,pData+=SoeDataWith)
	{
    	wSoeNo=MAKEDWORD(pData[0], pData[1], pData[2], 0) - STARTSYX;
		if ( wSoeNo > wAllSYxNum ){
			return;
		}
		time = (PCP56Time2a_T)(&pData[TimeStartAddr]);
		WriteYxSoe(m_dwDevID, wSoeNo, pData[ValueStartAddr], time);
    }
	return;

}

void GX104Master_Deal_RecvSSoe_TEST(uint16_t m_dwDevID,PGX104Master_DATA_T GX104MasterData)
{
	uint16_t i,val;	
	uint16_t wSoeNo;
	uint16_t wMSecond;
	uint8_t bSoeNum;
	uint8_t *pData;

	uint8_t SoeDataWith = 11;
	uint8_t TimeStartAddr = 4;
	uint8_t ValueStartAddr = 3;
	
    uint8_t Type;
    uint16_t RecvSn,SendSn;
    uint32_t FirmwareType = 0;
	
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);
	PCP56Time2a_T time = NULL;
    SendSn = GX104MasterData->Ctrl.I.SendSn;
    RecvSn = GX104MasterData->Ctrl.I.RecvSn;
		
	if ( (asdu->_reason._reason!=3) && (asdu->_reason._reason!=0x0b) )
		return;

	pData = asdu->_info;
	
	bSoeNum = asdu->_num._num; 
	log("bSoeNum(%d)\n",bSoeNum);
	for(i=0; i<bSoeNum; i++,pData+=SoeDataWith)
	{
    	wSoeNo=MAKEDWORD(pData[0], pData[1], pData[2], 0) - STARTSYX;
		log("pData[0](%d),pData[1](%d),pData[2]%d\n",pData[0], pData[1], pData[2]);
		if ( wSoeNo > wAllSYxNum ){
			log("wSoeNo(%d)\n",wSoeNo);
			return;
		}
		log("wSoeNo(%d)\n",wSoeNo);
		time = (PCP56Time2a_T)(&pData[TimeStartAddr]);
		WriteYxSoe(m_dwDevID, wSoeNo, pData[ValueStartAddr], time);
		PRINT_FUNLINE;
    }
	return;
}


/*****************************************************************
函数名称: GX104Master_Deal_RecvSYc
函数功能: 处理遥测入库
输入参数: 
返回值：  无
修改日期：2019-11-22
	1、未定义的点不发送，即nIndex=0且ID=0的突发遥信会发送，其他的不发送  

*****************************************************************/
void GX104Master_Deal_RecvSYc(uint16_t DevNO,PGX104Master_DATA_T GX104MasterData)
{
	uint16_t wBINo;
	uint16_t i;	
	uint8_t *pData,AINum;

    uint8_t Type,COT;
    uint16_t RecvSn,SendSn;
    uint32_t FirmwareType = 0;
	CP56Time2a_T SoeTime;
	TSysTimeSoe YcValue; 
	uint16_t value16;
	uint32_t value32;
	Data_Value_T DataValue;
	
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);
    SendSn = GX104MasterData->Ctrl.I.SendSn;
    RecvSn = GX104MasterData->Ctrl.I.RecvSn;

    
    /* deal the receive and send serial number */
//    if(GX104Master_Deal_SN(SendSn, RecvSn) == RET_ERROR){
//        return RET_ERROR;
//    }
    /* Start tester timer */
//    GX104Master_Tester_Timer();
//	if ( ((asdu->_reason<0x14) || (asdu->_reason>=0x14+bGroupYx))
//		&&(asdu->_reason!=3) && (asdu->_reason!=0x0b) )
//		return;
		
	AINum = asdu->_num._num; //可变结构限定词
	pData = asdu->_info;
	Type = asdu->_type;
	COT = asdu->_reason._reason;
	
	switch (Type){
		case IEC10X_M_ME_NA_1://归一化遥测
			if ( asdu->_num._sq )//连续
			{	
				PRINT_FUNLINE;
				asdu->_num._sq ^= 1;
				
				wBINo = MAKEWORD(pData[DEALSTATRT], pData[DEALSTATRT+1])-STARTSYC;
				pData += DEALSTATRT+BINFOADDR;
				for(i=0; i<AINum; i++)
				{
					if (wBINo+i > wAllSYcNum)
						return;
					
					value16 =  *(uint16_t *)&pData[0];
					WriteYc16(DevNO,wBINo+i,value16);
					
					if(COT == IEC10X_COT_SPONT){//突发遥测写进队列
//						gpDevice[DevNO].pBurstAI[i+wBINo].flag = Flag_Spon;
						WriteOneLogicBurstAI(gpDevice[DevNO].pRealBase->pRealAI[wBINo+i].pReflex, OVDEAD_BIT, value16);
					}
					pData+=3;//信息体长度3,2个字节遥测,1个品质描述
		
				}
				
		//		for(i=0; i<BINum;i++){
		//			log("yx npoint is (%d) value is (%d)\n",i,ReadYx(DevNO,i));;
		//		}
		//		
				return;
			}
			
			//不连续
			pData += DEALSTATRT;
			for(i=0; i<AINum; i++,pData+=BINFOADDR+3)
			{
				PRINT_FUNLINE;
				wBINo = MAKEWORD(pData[0], pData[1])-STARTSYC;
				
				if ( wBINo > wAllSYcNum )
					return;
					
				value16 =  *(uint16_t *)&pData[BINFOADDR];
				WriteYc16(DevNO,wBINo,value16);
				
				if(COT == IEC10X_COT_SPONT){//突发遥测写进队列
//					gpDevice[DevNO].pBurstAI[wBINo].flag = Flag_Spon;
					WriteOneLogicBurstAI(gpDevice[DevNO].pRealBase->pRealAI[wBINo].pReflex, OVDEAD_BIT, value16);
				}
			}
			break;
		case IEC10X_M_TI_BD_1://标度化遥测
			if ( asdu->_num._sq )//连续
			{	
				PRINT_FUNLINE;
				asdu->_num._sq ^= 1;
				
				wBINo = MAKEWORD(pData[DEALSTATRT], pData[DEALSTATRT+1])-STARTSYC;
				pData += DEALSTATRT+BINFOADDR;
				
				for(i=0; i<AINum; i++)
				{
					if (wBINo+i > wAllSYcNum)
						return;
					value32 =  *(uint32_t *)&pData[0];
					WriteYc32(DevNO,wBINo+i,value32);
					
					if(COT == IEC10X_COT_SPONT){//突发遥测写进队列
//						gpDevice[DevNO].pBurstAI[wBINo+i].flag = Flag_Spon;
						WriteOneLogicBurstAI(gpDevice[DevNO].pRealBase->pRealAI[wBINo+i].pReflex, OVDEAD_BIT, value32);
					}
					pData+=5;//信息体长度5,4个字节遥测,1个品质描述
		
				}
				
		//		
				return;
			}
//			//不连续
//			pData += DEALSTATRT;
//			for(i=0; i<gVars.TransYCTableNum; i++){
//				log("___i[%d]nindex(%d)\n",i,gpDevice[DevNO].pBurstAI[i].nNo);
//			}
//			return;
			for(i=0; i<AINum; i++,pData+=BINFOADDR+5)
			{
				wBINo = MAKEWORD(pData[0], pData[1])-STARTSYC;
				if ( wBINo > wAllSYcNum )
					return;
				value32 =  *(uint32_t *)&pData[BINFOADDR];
				WriteYc32(DevNO,wBINo,value32);
				if(COT == IEC10X_COT_SPONT){//突发遥测写进队列
//					gpDevice[DevNO].pBurstAI[wBINo].flag = Flag_Spon;
					WriteOneLogicBurstAI(gpDevice[DevNO].pRealBase->pRealAI[wBINo].pReflex, OVDEAD_BIT, value32);
				}
			}
		
			break;
		case IEC10X_M_ME_NC_1://浮点型遥测
			if ( asdu->_num._sq )//连续
			{	
				asdu->_num._sq ^= 1;
				
				wBINo = MAKEWORD(pData[DEALSTATRT], pData[DEALSTATRT+1])-STARTSYC;
				pData += DEALSTATRT+BINFOADDR;
				for(i=0; i<AINum; i++)
				{
					if (wBINo+i > wAllSYcNum)
						return;
					
					value32 =  *(uint32_t *)&pData[0];

//					DataValue._int32 = value32;
//					log("pdata[%d](%f)\n",i,DataValue._float);
					WriteYc32(DevNO,wBINo+i,value32);
					
					if(COT == IEC10X_COT_SPONT){//突发遥测写进队列
//						gpDevice[DevNO].pBurstAI[wBINo+i].flag = Flag_Spon;
						WriteOneLogicBurstAI(gpDevice[DevNO].pRealBase->pRealAI[wBINo+i].pReflex, OVDEAD_BIT, value32);
					}
					pData+=5;//信息体长度5,4个字节遥测,1个品质描述
		
				}
				
				return;
			}
			
			//不连续
			pData += DEALSTATRT;
			for(i=0; i<AINum; i++,pData+=BINFOADDR+5)
			{
				wBINo = MAKEWORD(pData[0], pData[1])-STARTSYC;
				
				if ( wBINo > wAllSYcNum )
					return;

				value32 =  *(uint32_t *)&pData[BINFOADDR];
				WriteYc32(DevNO,wBINo,value32);
		
				if(COT == IEC10X_COT_SPONT){//突发遥测写进队列
//					gpDevice[DevNO].pBurstAI[wBINo].flag = Flag_Spon;
					WriteOneLogicBurstAI(gpDevice[DevNO].pRealBase->pRealAI[wBINo].pReflex, OVDEAD_BIT, value32);
				}
			}
		
			break;
		
	
	}

	
	return;
}
void GX104Master_Deal_DZ_YZ(uint16_t DevNO,PGX104Master_DATA_T GX104MasterData)
{
	uint16_t wBINo;
	uint16_t i,DZNum;	
	uint8_t *pData;
	uint8_t COT;
	TSysTimeSoe YcValue; 
	uint32_t value32;
	Data_Value_T DataValue;
	uint8_t flag;
	
	DZNum = gpDevice[DevNO].DZNum;
	PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);
	COT = asdu->_reason._reason;

	flag = (COT == IEC10X_COT_ACTCON)?Flag_DZ_RET:Flag_DZ_REFUSE;
	
	/*info addr*/
	pData = asdu->_info;
	
	if ( asdu->_num._sq )//连续
	{	
		asdu->_num._sq ^= 1;
		
		wBINo = MAKEWORD(pData[0], pData[1])-STARTSDZ;
		pData += BINFOADDR;
		for(i=0; i<asdu->_num._num; i++)
		{
			if (wBINo+i > DZNum)
				return;
			
			value32 = *(uint32_t *)&pData[0];
			
			gpDevice[DevNO].pBurstDZ[wBINo+i].YZ_value = value32;
			gpDevice[DevNO].pBurstDZ[wBINo+i].flag = flag;
			pData+=4;//信息体长度4
			DataValue._int32 = value32;
			log("receive dz(%d) value is %f flag(%d)\n", wBINo+i, DataValue._float, flag);
		}
		return;
	}
	
	//不连续
	pData = asdu->_info;
	for(i=0; i<asdu->_num._num; i++)
	{
		wBINo = MAKEWORD(pData[0], pData[1])-STARTSDZ;
		if ( wBINo > DZNum )
			return;
			
		value32 = *(uint32_t *)&pData[BINFOADDR];
		gpDevice[DevNO].pBurstDZ[wBINo].YZ_value = value32;
		gpDevice[DevNO].pBurstDZ[wBINo].flag = flag;
		SetDZFlag(wBINo, flag);
		pData += BINFOADDR + 4;
		DataValue._int32 = value32;
		log("receive dz(%d) value is %f flag(%d)\n", wBINo, DataValue._float, flag);
	}
	return;

}
void GX104Master_Deal_DZ_Call(uint16_t DevNO,PGX104Master_DATA_T GX104MasterData)
{
	uint16_t wBINo;
	uint16_t i,DZNum;	
	uint8_t *pData;
    uint8_t COT;
	TSysTimeSoe YcValue; 
	uint32_t value32;
	Data_Value_T DataValue;
	
	DZNum = gpDevice[DevNO].DZNum;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);
	COT = asdu->_reason._reason;
	
	/*info addr*/
	pData = asdu->_info;
	if ( asdu->_num._sq )//连续
	{	
		asdu->_num._sq ^= 1;
		
		wBINo = MAKEWORD(pData[0], pData[1])-STARTSDZ;
		pData += BINFOADDR;
		for(i=0; i<asdu->_num._num; i++)
		{
			if (wBINo+i > DZNum)
				return;
			
			value32 = *(uint32_t *)&pData[0];
			WriteRealDz(DevNO,wBINo+i,value32);
//			if(DevNO == 0){
//				DataValue._int32 = value32;
//				log("dz(%d) value is %f\n", wBINo+i, DataValue._float);
//			}
			if(COT == IEC10X_COT_SPONT){//突发定值写进队列
				SetRealDZFlag(DevNO, wBINo+i, Flag_Spon);
			}
			pData+=4;//信息体长度4
		}
		
		return;
	}
	
	//不连续
	pData = asdu->_info;
	for(i=0; i<DZNum; i++)
	{
		wBINo = MAKEWORD(pData[0], pData[1])-STARTSDZ;
		if ( wBINo > wAllSDzNum )
			return;
			
		value32 = *(uint32_t *)&pData[BINFOADDR];
		WriteRealDz(DevNO,wBINo,value32);
		
		if(COT == IEC10X_COT_SPONT){//突发定值写进队列
			SetRealDZFlag(DevNO, wBINo, Flag_Spon);
		}
		pData += BINFOADDR + 4;
	}

	return;
}

/*******************************************************************  
*名称：      		check_DZ_YZ  
*功能：			检查定值预置标志，并发送定值预置参数  
*入口参数：
*	DevNO 				设备ID号
*	GX104MasterData		输入缓冲区 
*******************************************************************/ 
void GX104Master_Deal_RecvDZ(uint16_t DevNO,PGX104Master_DATA_T GX104MasterData)
{
	uint16_t COT;
	
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);

	COT = asdu->_reason._reason;
	
	switch (COT){
		case IEC10X_CALL_QOI_TOTAL://响应定值召唤
		case IEC10X_COT_SPONT://定值突发
			GX104Master_Deal_DZ_Call(DevNO, GX104MasterData);
			break;
		case IEC10X_COT_ACTCON://定值预置返回
		case IEC10X_COT_REFUSE://定值预置拒绝
			GX104Master_Deal_DZ_YZ(DevNO, GX104MasterData);
			break;
	}
	
	return;
}

uint8_t GX104Master_BuildCall(int DevNO, uint8_t type, uint8_t qoi){

    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;

    PGX104Master_DATA_T GX104MasterData = (PGX104Master_DATA_T)GX104Master_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);

    /*build head*/
    GX104MasterData->Head = GX104Master_HEAD;

    /*build control code*/
    GX104MasterData->Ctrl.I.Type = 0;
    GX104MasterData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104MasterData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    asdu->_type = type;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason._reason = IEC10X_ASDU_REASON_ACT;
    asdu->_addr = gpDevice[DevNO].Address;
    /*build info*/
    ptr = info->_addr;
    Temp32 = 0;
    memcpy(ptr, &Temp32, 3);

    ptr = info->_element;
    ptr[0] = qoi;

    ptr+=1;
    /*len*/
    len = ptr - GX104Master_Sendbuf;
    GX104MasterData->Len = len - 2;
    
	GX104Master_send(DevNO,(char *)GX104Master_Sendbuf,len);
    return RET_SUCESS;
}

/*******************************************************************  
*名称：      		GX104Master_BuildYk  
*功能：			处理遥控任务  
*入口参数：         
*	@DevNO		设备编号
*	@Type 		遥控类型
*	@reason 	传输原因
*	@YKData 	遥控命令
*出口参数：正确返回为0，错误返回为-1 
*******************************************************************/ 
int GX104Master_BuildYk(int DevNO, uint8_t Type, uint16_t reason, uint8_t YKData){

    uint8_t len = 0, asdu_num = 1;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PGX104Master_DATA_T GX104MasterData = (PGX104Master_DATA_T)GX104Master_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
	uint8_t YKCommand = 0;
	
    /*build head*/
    GX104MasterData->Head = GX104Master_HEAD;

    /*build control code*/
    GX104MasterData->Ctrl.I.Type = 0;
    GX104MasterData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104MasterData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;
    /*build ASDU , COT ,Addr*/
    asdu->_type = Type;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = reason;
    asdu->_addr = gpDevice[DevNO].Address;
    /*build info addr*/
    ptr = info->_addr;
    Temp32 = 0+STARTSYK;
    memcpy(ptr, &Temp32, 3);
	ptr += 3;
    /*Build Detect value*/
	*ptr = YKData;
    ptr++;
    /*len*/
    len = ptr - GX104Master_Sendbuf;
    GX104MasterData->Len = len - 2;
	GX104Master_send(DevNO,(char *)GX104Master_Sendbuf,len);

    return RET_SUCESS;
}

/*******************************************************************  
*名称：      		GX104Master_BuildSetDZ  
*功能：			处理遥控任务  
*入口参数：         
*	@DevNO		设备ID号
*	@Type 		遥控类型
*	@reason 	传输原因
*	@YKData 	遥控命令
*出口参数：正确返回为0，错误返回为-1 
*******************************************************************/ 
int GX104Master_BuildSetDZ(int DevNO){

    uint8_t len = 0, asdu_num = 1;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PGX104Master_DATA_T GX104MasterData = (PGX104Master_DATA_T)GX104Master_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
	
    /*build head*/
    GX104MasterData->Head = GX104Master_HEAD;

    /*build control code*/
    GX104MasterData->Ctrl.I.Type = 0;
    GX104MasterData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104MasterData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;
    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_TYPE_DZ_JH;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_COT_ACT;
    asdu->_addr = gpDevice[DevNO].Address;
    /*build info addr*/
    ptr = info->_addr;
    Temp32 = 0;
    memcpy(ptr, &Temp32, 3);
	ptr += 3;
    /*Build Detect value*/
	*ptr = 1;
    ptr++;
    /*len*/
    len = ptr - GX104Master_Sendbuf;
    GX104MasterData->Len = len - 2;
	GX104Master_send(DevNO,(char *)GX104Master_Sendbuf,len);

    return RET_SUCESS;
}



/*******************************************************************  
*名称：      		GX104Master_BuildClock  
*功能：			对时  
*入口参数：         
*	@fd 		文件描述符
*	@COT		传输类型
*	@Type 		遥控类型
*出口参数：正确返回为0，错误返回为-1 
*******************************************************************/ 
uint8_t GX104Master_BuildClock(int DevNO){

    uint8_t len = 0, asdu_num = 1;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PGX104Master_DATA_T GX104MasterData = (PGX104Master_DATA_T)GX104Master_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
	uint8_t YKCommand = 0;
	PCP56Time2a_T TimeTemp = NULL;
	
    /*build head*/
    GX104MasterData->Head = GX104Master_HEAD;

    /*build control code*/
    GX104MasterData->Ctrl.I.Type = 0;
    GX104MasterData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104MasterData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;
    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_C_CS_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason._reason = IEC10X_COT_ACT;
    asdu->_addr = gpDevice[DevNO].Address;
    /*build info addr*/
    ptr = info->_addr;
    Temp32 = 0;
    memcpy(ptr, &Temp32, 3);
    /*Build Detect value*/
    ptr = info->_element;
	TimeTemp = (PCP56Time2a_T)ptr;
	IEC10X->GetTime(TimeTemp);
    ptr+=sizeof(CP56Time2a_T);
    /*len*/
    len = ptr - GX104Master_Sendbuf;
    GX104MasterData->Len = len - 2;
	

	GX104Master_send(DevNO, (char *)GX104Master_Sendbuf,len);

    return RET_SUCESS;
}

uint8_t GX104Master_Build_U(int DevNO, uint8_t UType){

    uint8_t len = 0, Tester, Start, Stop;
    uint8_t *ptr = NULL;

    PGX104Master_DATA_T GX104MasterData = (PGX104Master_DATA_T)GX104Master_Sendbuf;


    /*build head*/
    GX104MasterData->Head = GX104Master_HEAD;

    /*build control code*/
//    if(mode){
//        Tester = GX104Master_U_FUNC_TESTER;
//        Start = GX104Master_U_FUNC_STARTDT;
//        Stop = GX104Master_U_FUNC_STOPDT;
//    }else{
//
//        Tester = GX104Master_U_FUNC_TESTER_ACK;
//        Start = GX104Master_U_FUNC_STARTDT_ACK;
//        Stop = GX104Master_U_FUNC_STOPDT_ACK;
//    }
//
//    switch(UType){
//
//        case GX104Master_U_FUNC_STARTDT:
//            GX104MasterData->Ctrl.Func.Func = Start;
//            break;
//        case GX104Master_U_FUNC_STOPDT:
//            GX104MasterData->Ctrl.Func.Func = Stop;
//            break;
//        case GX104Master_U_FUNC_TESTER:
//        case GX104Master_U_FUNC_TESTER_ACK:
//            GX104MasterData->Ctrl.Func.Func = Tester;
//			break;
//		case GX104Master_U_FUNC_STARTDT_ACK:
//			GX104MasterData->Ctrl.Func.Func = Start;
//			break;
//			
//        default:
//            LOG(">%s<, U Type Error(%d) !\n",__FUNCTION__,UType);
//            return RET_ERROR;
//    }

	GX104MasterData->Ctrl.Func.Func = UType;
	GX104MasterData->Ctrl.Func.Reserve = 0;
    /*build ASDU , COT ,Addr*/
    ptr=GX104MasterData->Asdu;

    /*build info*/

    /*len*/
    len = ptr - GX104Master_Sendbuf;
    GX104MasterData->Len = len - 2;
//    DumpHEX(GX104Master_Sendbuf,len);
//	LOG("sfsfadfdaf\n");
    /* enqueue to the transmisson queue */
//    IEC10X_Enqueue(GX104Master_Sendbuf, len ,IEC10X_PRIO_INITLINK, NULL,NULL);
	
	GX104Master_send(DevNO,(char *)GX104Master_Sendbuf,len);
    return RET_SUCESS;
}


uint8_t GX104Master_Build_S_Ack(int DevNO){

    uint8_t len = 0;
    uint8_t *ptr = NULL;

    PGX104Master_DATA_T GX104MasterData = (PGX104Master_DATA_T)GX104Master_Sendbuf;

    /*build head*/
    GX104MasterData->Head = GX104Master_HEAD;

    /*build control code*/
    GX104MasterData->Ctrl.S.Type1 = 1;
    GX104MasterData->Ctrl.S.Type2 = 0;

    GX104MasterData->Ctrl.S.Reserve = 0;
    GX104MasterData->Ctrl.S.RecvSn = SendRecvSn.BuildRecvSn;

    /*build ASDU , COT ,Addr*/
    ptr=GX104MasterData->Asdu;

    /*build info*/

    /*len*/
    len = ptr - GX104Master_Sendbuf;
    GX104MasterData->Len = len - 2;
    DumpHEX(GX104Master_Sendbuf,len);
    /* enqueue to the transmisson queue */
	GX104Master_send(DevNO,(char *)GX104Master_Sendbuf,len);

    return RET_SUCESS;
}

int GX104Master_ASDU_SetAct(PIEC10X_ASDU_T Iec10x_Asdu, uint8_t Type){

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
            LOG("-%s- , error reason(%d) \n", __FUNCTION__,Iec10x_Asdu->_reason._reason);
            return RET_ERROR;
    }
    return RET_SUCESS;
}


int GX104Master_Deal_SN(uint16_t SendSn, uint16_t RecvSn){

    LOG("Receive Pakage I(%d,%d), Send(%d,%d)\n",SendSn,RecvSn, SendRecvSn.BuildSendSn,SendRecvSn.BuildRecvSn);
    
#if 0
    if(SendSn > SendRecvSn.DealSendSn+1){
        LOG("-%s-, error,send last(%d),now(%d) \n",__FUNCTION__,SendRecvSn.DealSendSn,SendSn);
        gpDevice[DevNO].STATE_FLAG_INIT = GX104Master_FLAG_SEND_CLOSED;
        return RET_ERROR;
    }else if(SendSn < SendRecvSn.DealSendSn+1){
        LOG("-%s-, Retransmit,send last(%d),now(%d) \n",__FUNCTION__,SendRecvSn.DealSendSn,SendSn);
        return RET_ERROR;
    }
    if(RecvSn != SendRecvSn.BuildSendSn){
        LOG("-%s-, error,receive last(%d),now(%d) \n",__FUNCTION__,SendRecvSn.BuildSendSn,RecvSn);
        gpDevice[DevNO].STATE_FLAG_INIT = GX104Master_FLAG_SEND_CLOSED;
        return RET_ERROR;
    }
    if(RecvSn < SendRecvSn.DealRecvSn){
        LOG("-%s-, error,receive2 last(%d),now(%d) \n",__FUNCTION__,SendRecvSn.DealRecvSn,RecvSn);
        return RET_ERROR;
    }
#endif

    if(SendSn < SendRecvSn.DealSendSn || RecvSn < SendRecvSn.DealRecvSn){

        LOG("-%s-, error,send last(%d),now(%d). recv last(%d),now(%d) \n",__FUNCTION__,
                        SendRecvSn.DealSendSn,SendSn, SendRecvSn.DealRecvSn, RecvSn);
        return RET_ERROR;
    }
    SendRecvSn.BuildRecvSn = SendSn+1;

    SendRecvSn.DealSendSn = SendSn;
    SendRecvSn.DealRecvSn = RecvSn;

    //SendRecvSn.BuildRecvSn++;

    /* return S ACK */
    GX104Master_TimeCycle_S = 0;

    return RET_SUCESS;
}




uint8_t GX104Master_Deal_YK_return(int DevNO, PGX104Master_DATA_T GX104MasterData){

    uint8_t len = 0;
    uint8_t *ptr = NULL;
	uint8_t reason;
	int YK_From_ID;//遥控来源ID号

    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
	reason = asdu->_reason._reason;
	
	YK_From_ID = gpDevice[DevNO].YK_FROM_ID;
	switch(reason){
		case IEC10X_COT_ACTCON:
			gpDevice[YK_From_ID].YKInfo._Return._ackconfirm = 1;
			break;
		case IEC10X_COT_STOPACTCONFIRM:
			gpDevice[YK_From_ID].YKInfo._Return._stopackconfirm = 1;
			break;
		case IEC10X_COT_ACTFIN:
			gpDevice[YK_From_ID].YKInfo._Return._finish = 1;
			break;
		case IEC10X_COT_REFUSE:
			gpDevice[YK_From_ID].YKInfo._Return._refuse = 1;
			break;
		case IEC10X_ASDU_REASON_REFUSE_FIN:
			gpDevice[YK_From_ID].YKInfo._Return._refusefinish = 1;
			break;
		default:
			return -1;
	}
    return RET_SUCESS;
}


//uint8_t GX104Master_Deal_I(int fd, PGX104Master_DATA_T GX104MasterData, uint16_t len){
int GX104Master_Deal_I(int DevNO, PGX104Master_DATA_T GX104MasterData, uint16_t len){

    uint8_t Type;
    uint16_t RecvSn,SendSn;
    uint32_t FirmwareType = 0;
	
	
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);

    SendSn = GX104MasterData->Ctrl.I.SendSn;
    RecvSn = GX104MasterData->Ctrl.I.RecvSn;
    /* check asdu addrest */
//    if(Iec10x_Sta_Addr != asdu->_addr){
//        LOG("-%s-, error asdu addr(%x)(%x) \n" ,__FUNCTION__ ,Iec10x_Sta_Addr,asdu->_addr);
//        return RET_ERROR;
//    }
    
    /* deal the receive and send serial number */
	
//    if(GX104Master_Deal_SN(SendSn, RecvSn) == RET_ERROR){//判断接收序列号
//        return RET_ERROR;
//    }
	
    Type = asdu->_type;
	
	switch(Type){
		
		case IEC10X_C_IC_NA_1:
//			LOG("++++Asdu Type Call cmd... \n");
			gpDevice[DevNO].STATE_FLAG_CALLALL = GX104Master_FLAG_IDLE;
			break;
		
		case IEC10X_TYPE_DZ_CALL:
//			LOG("++++Asdu Type DZ Call cmd... \n");
			gpDevice[DevNO].STATE_FLAG_DZCALLALL = GX104Master_FLAG_IDLE;
			break;
			
		case IEC10X_C_SC_NA_1://单点遥控
			GX104Master_Deal_YK_return(DevNO, GX104MasterData);
			break;
		
		case IEC10X_C_SE_NA_1://双点遥控
//				GX104Master_Deal_YK_return(DevNO, GX104MasterData);
			break;

		case Iec10x_M_EI_NA_1:
//			gpDevice[DevNO].STATE_FLAG_INIT = GX104Master_FLAG_INIT_FIN;
			LOG("++++Asdu Type Iint_Finishd... ip(%s)\n", gpDevice[DevNO].IP);
			break;
		
		case IEC10X_M_ME_NA_1://归一化遥测
		case IEC10X_M_TI_BD_1://标度化遥测
		case IEC10X_M_ME_NC_1://浮点型遥测
			GX104Master_Deal_RecvSYc(DevNO,GX104MasterData);
			break;

		case IEC10X_M_SP_NA_1://单点信息
//			gpDevice[DevNO].STATE_FLAG_INIT = GX104Master_M_FLAG_REC_YX;
			GX104Master_Deal_RecvSYx(DevNO,GX104MasterData);
			break;
		case IEC10X_M_SP_TB_1://带CP56Time2a时标的单点信息
//			gpDevice[DevNO].STATE_FLAG_INIT = ;
			GX104Master_Deal_RecvSSoe(DevNO,GX104MasterData);
			LOG("++++Asdu IEC10X_M_SP_TB_1... \n");
			break;
		case IEC10X_C_CS_NA_1://时钟同步确认
			gpDevice[DevNO].STATE_FLAG_CLOCK = GX104Master_FLAG_IDLE;
			break;

		case IEC10X_TYPE_DZ_DATA://定值响应召唤
			GX104Master_Deal_RecvDZ(DevNO,GX104MasterData);
			gpDevice[DevNO].STATE_FLAG_DZCALLALL = GX104Master_FLAG_IDLE;
			break;
		case IEC10X_TYPE_DZ_JH://定值激活确认
			break;
			
		default:
			LOG("-%s-, error Type(%d) \n", __FUNCTION__,Type);
			return RET_ERROR;
	}
	return RET_SUCESS;


}


uint8_t GX104Master_Deal_S(int DevNO, PGX104Master_DATA_T GX104MasterData, uint16_t len){

    return RET_SUCESS;
}
uint8_t GX104Master_Deal_U(int DevNO, PGX104Master_DATA_T GX104MasterData, uint16_t len){

    switch(GX104MasterData->Ctrl.Func.Func){


        case GX104Master_U_FUNC_STOPDT:
            LOG(">%s<, function STOPDT \n",__FUNCTION__);
            IEC10X->CloseLink();
            GX104Master_Build_U(DevNO, GX104Master_U_FUNC_STOPDT);
            gpDevice[DevNO].STATE_FLAG_INIT = GX104Master_FLAG_RECV_CLOSED;
            break;
        case GX104Master_U_FUNC_TESTER:
//            LOG(">%s<, function TESTER \n",__FUNCTION__);
//            GX104Master_Build_U(DevNO, GX104Master_U_FUNC_TESTER_ACK);
			SendRecvSn.TesterCount=0;
            break;

        /* U ACK */
		case GX104Master_U_FUNC_STARTDT_ACK:
//			LOG(">%s<, function STARTDT \n",__FUNCTION__);
			GX104Master_TimeCycle = 0;
			GX104Master_TimeCount = 0;
			gpDevice[DevNO].STATE_FLAG_INIT = GX104Master_FLAG_LINK_INIT;
			gpDevice[DevNO].Flag_start_link = LINK_SUCESS;
			
			break;
        case GX104Master_U_FUNC_STOPDT_ACK:
            LOG(">%s<, function STOPDT ACK\n",__FUNCTION__);
            gpDevice[DevNO].STATE_FLAG_INIT = GX104Master_FLAG_RECV_CLOSED;
            break;

        default:
            LOG(">%s<, function ERROR \n",__FUNCTION__);
            break;
    }
    return RET_SUCESS;
}

int GX104Master_Receive(int DevNO, uint8_t *buf, uint16_t len){

    uint8_t *BufTemp = NULL;
    int16_t LenRemain,LenTmp;
    PGX104Master_DATA_T GX104MasterData = NULL;
	
    if(buf == NULL){
        LOG("-%s-,buffer (null)",__FUNCTION__);
        return RET_ERROR;
    }
#if 0
    if(len <= 0 || len>GX104Master_MAX_BUF_LEN || len<BufTemp[0]+2){
        LOG("-%s-,buffer len error(%d) \n",__FUNCTION__,len);
        return;
    }
#endif

    BufTemp = buf;
    LenRemain = len;

    while(BufTemp<buf+len){
		gpDevice[DevNO].ReSendNum = 0;
        GX104MasterData = (PGX104Master_DATA_T)BufTemp;
        Iec10x_Lock();
        if(GX104MasterData->Head == GX104Master_HEAD){
			tcp_master_time_cnt_old = sys_time_cnt;
            LenTmp = GX104MasterData->Len + 2;
            if(LenRemain < GX104Master_HEAD_LEN){
                LOG("_%s_,len error(%d) \n",__FUNCTION__,len);
                Iec10x_UnLock();
                return RET_ERROR;
            }
            if(GX104MasterData->Ctrl.Type.Type1 == 0){
//                LOG("-%s-,Frame Type I \n",__FUNCTION__);
                GX104Master_Deal_I(DevNO, GX104MasterData, LenTmp);

            }else if(GX104MasterData->Ctrl.Type.Type1 == 1 && GX104MasterData->Ctrl.Type.Type2 == 0){
//                LOG("-%s-,Frame Type S \n",__FUNCTION__);
                GX104Master_Deal_S(DevNO, GX104MasterData, LenTmp);

            }else if(GX104MasterData->Ctrl.Type.Type1 == 1 && GX104MasterData->Ctrl.Type.Type2 == 1){
//                LOG("-%s-,Frame Type U \n",__FUNCTION__);
                GX104Master_Deal_U(DevNO, GX104MasterData, LenTmp);
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
void GX104Master_Start_Link(int DevNO){
	gpDevice[DevNO].STATE_FLAG_INIT = GX104Master_FLAG_START_LINK;
	gpDevice[DevNO].STATE_FLAG_CALLALL = GX104Master_FLAG_CLOSED;
	gpDevice[DevNO].STATE_FLAG_DZCALLALL = GX104Master_FLAG_CLOSED;
	gpDevice[DevNO].STATE_FLAG_TESTER = GX104Master_FLAG_CLOSED;
	gpDevice[DevNO].STATE_FLAG_CLOCK = GX104Master_FLAG_CLOSED;
	gpDevice[DevNO].Flag_start_link = LINK_COLSE;
}
void GX104Master_ResetFlag(int DevNO){
    gpDevice[DevNO].STATE_FLAG_CALLALL = GX104Master_FLAG_IDLE;
    gpDevice[DevNO].STATE_FLAG_DZCALLALL = GX104Master_FLAG_IDLE;
    gpDevice[DevNO].STATE_FLAG_GROUP = GX104Master_FLAG_IDLE;
    gpDevice[DevNO].STATE_FLAG_CLOCK = GX104Master_FLAG_IDLE;
    gpDevice[DevNO].STATE_FLAG_TESTER = GX104Master_FLAG_IDLE;
	gpDevice[DevNO].STATE_FLAG_INIT = GX104Master_FLAG_IDLE;


	
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

uint32_t GX104Master_TestCount_Temp = 0;

//建立链路
uint8_t GX104Master_Build_Link(int DevNO)
{
	switch (gpDevice[DevNO].STATE_FLAG_INIT){
			
		case GX104Master_FLAG_CLOSED:
			gpDevice[DevNO].STATE_FLAG_INIT = GX104Master_FLAG_START_LINK;
			break;
	
		case GX104Master_FLAG_START_LINK:
			GX104Master_Build_U(DevNO, GX104Master_U_FUNC_STARTDT);
		
			break;
		
		case GX104Master_FLAG_LINK_INIT:
//					IEC10X_ClearQ();
			GX104Master_ResetFlag(DevNO);
			gpDevice[DevNO].STATE_FLAG_INIT = GX104Master_FLAG_INIT_FIN;
			break;
			
			break;
		case GX104Master_FLAG_INIT_FIN:
			gpDevice[DevNO].STATE_FLAG_CALLALL = GX104Master_FLAG_CALL_ALLDATA;
			gpDevice[DevNO].STATE_FLAG_DZCALLALL = GX104Master_FLAG_CALL_ALLDATA;
			gpDevice[DevNO].STATE_FLAG_INIT = GX104Master_FLAG_IDLE;
			gpDevice[DevNO].STATE_FLAG_CLOCK = GX104Master_FLAG_CLOCK_SYS;
		
			break;
			
		case GX104Master_FLAG_IDLE:
			
			break;
			
		default:
			break;
	}
	
	if(gpDevice[DevNO].Flag_start_link != LINK_SUCESS)
		return 0;
	
	switch (gpDevice[DevNO].STATE_FLAG_CALLALL){//总召唤
			
			case GX104Master_FLAG_CLOSED:
				break;
		
			case GX104Master_FLAG_CALL_ALLDATA:
				GX104Master_BuildCall(DevNO, IEC10X_C_IC_NA_1, IEC10X_CALL_QOI_TOTAL);
				break;
			case GX104Master_FLAG_IDLE:
				break;
				
			default:
				break;
	}

	switch (gpDevice[DevNO].STATE_FLAG_DZCALLALL){//定值召唤
			
			case GX104Master_FLAG_CLOSED:
				break;
		
			case GX104Master_FLAG_CALL_ALLDATA:
				GX104Master_BuildCall(DevNO, IEC10X_TYPE_DZ_CALL, IEC10X_CALL_QOI_TOTAL);
				break;
			
			case GX104Master_FLAG_IDLE:
				break;
				
			default:
				break;
	}
	
	switch (gpDevice[DevNO].STATE_FLAG_CLOCK){//时钟同步
			
			case GX104Master_FLAG_CLOSED:
				break;
			
			case GX104Master_FLAG_CLOCK_SYS:
				GX104Master_BuildClock(DevNO);
				break;
			
			case GX104Master_FLAG_IDLE:
				break;
				
			default:
				break;
	}

	switch(gpDevice[DevNO].STATE_FLAG_TESTER){
		
			case GX104Master_FLAG_CLOSED:
				break;
	
			case GX104Master_FLAG_TESTER:
				GX104Master_Build_U(DevNO, GX104Master_U_FUNC_TESTER_ACK);
				gpDevice[DevNO].STATE_FLAG_TESTER = GX104Master_FLAG_IDLE;
				break;
				
			case GX104Master_FLAG_TESTER_STOP:
				break;
				
			case GX104Master_FLAG_IDLE:
				if(sys_time_cnt - tcp_master_time_cnt_old > 40){
					tcp_master_time_cnt_old = sys_time_cnt;
					gpDevice[DevNO].STATE_FLAG_TESTER = GX104Master_FLAG_TESTER;
				}
				break;
				
			default:
				break;
	}
	
	return 0;

}

/*******************************************************************  
*名称：      		check_DZ_YZ  
*功能：			检查定值预置标志，并发送定值预置参数  
*入口参数：
*	DevNO 	设备ID号
*******************************************************************/ 
void check_DZ_YZ(int DevNO)
{
	int i,num;
    uint8_t len = 0;
    uint8_t *ptr = NULL;
    uint32_t Temp32 = 0;
    PIEC10X_DETECT_T detect = NULL;
	PIEC10X_DETECT_DW_T Value = NULL;
	PGX_104_INFO_YC32_T Value_Addr = NULL;
	uint8_t sq = 1;
	int old = -1;
	int offset;
    PGX104Master_DATA_T GX104MasterData = (PGX104Master_DATA_T)GX104Master_Sendbuf;
    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);
    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
	
    /*build head*/
    GX104MasterData->Head = GX104Master_HEAD;

    /*build control code*/
    GX104MasterData->Ctrl.I.Type = 0;
    GX104MasterData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
    GX104MasterData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;
    /*build ASDU , COT ,Addr*/
    asdu->_type = IEC10X_TYPE_DZ;
	
    asdu->_reason._reason = IEC10X_COT_ACT;
    asdu->_addr = gpDevice[DevNO].Address;
	
	num = 0;
	for(i = 0; i < gpDevice[DevNO].DZNum; i++){
		if(gpDevice[DevNO].pBurstDZ[i].flag == Flag_DZ_YZ){
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
		return;
	if(sq){//连续
		for(i = 0; i < gpDevice[DevNO].DZNum; i++){
			if(gpDevice[DevNO].pBurstDZ[i].flag == Flag_DZ_YZ){
				break;
			}
		}
		offset = i;
		Temp32 = i + STARTSDZ;
	    memcpy(info->_addr, &Temp32, 3);
		
		ptr = info->_element;
		for(i = 0 ; i < num; i++){
			*(int32_t *)ptr = gpDevice[DevNO].pBurstDZ[offset + i].YZ_value;
			SetRealDZFlag(DevNO, offset + i, Flag_Clear);
			ptr += 4;
		}
		
	}else{//不连续
		ptr = info->_addr;
		for(i = 0; i < gpDevice[DevNO].DZNum; i++){
			if(gpDevice[DevNO].pBurstDZ[i].flag == Flag_DZ_YZ){
				SetRealDZFlag(DevNO, i, Flag_Clear);
				Temp32 = i + STARTSDZ;
				memcpy(ptr, &Temp32, 3);
				ptr += 3;
				*(int32_t *)ptr = gpDevice[DevNO].pBurstDZ[i].YZ_value;
				ptr += 4;
			}
		}
	}
	
    asdu->_num._sq = sq;
    asdu->_num._num = num;
	
    len = ptr - GX104Master_Sendbuf;
    GX104MasterData->Len = len - 2;
	GX104Master_send(DevNO,(char *)GX104Master_Sendbuf,len);
	
}

/*****************************************************************
函数名称: GX104Master_Task
函数功能: 快速响应任务
输入参数: 
	fd:文件描述符
	DevNO:设备ID号
返回值：  无
修改日期：2020-3-31
*****************************************************************/
int GX104Master_Task(int DevNO)
{
	if(gpDevice[DevNO].ReSendNum > 10){
		gpDevice[DevNO].ReSendNum = 0;
		GX104Master_Start_Link(DevNO);
	}

	check_DZ_YZ(DevNO);
	return 0;
}
/*****************************************************************
函数名称: GX104Master_On_Time_Out
函数功能: 定时响应任务,1秒响应一次
输入参数: 
	DevNO:设备ID号
返回值：  无
修改日期：2020-6-17
	去掉参数fd，改为1秒响应一次，10秒一次总召，30秒一次对时
*****************************************************************/
int GX104Master_On_Time_Out(int DevNO){
	gpDevice[DevNO].TimeCnt++;
	
	if(gpDevice[DevNO].TimeCnt%10 == 0){
		gpDevice[DevNO].STATE_FLAG_CALLALL = GX104Master_FLAG_CALL_ALLDATA;
	}
	if(gpDevice[DevNO].TimeCnt%30 == 0){
		gpDevice[DevNO].STATE_FLAG_CLOCK = GX104Master_FLAG_CLOCK_SYS;
	}
	if(gpDevice[DevNO].TimeCnt%20 == 0){
		gpDevice[DevNO].STATE_FLAG_DZCALLALL = GX104Master_FLAG_CALL_ALLDATA;
	}

	GX104Master_Build_Link(DevNO);
	
}
