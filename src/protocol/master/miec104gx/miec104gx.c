#include "sys.h"


extern TVars     gVars;

char sss[255];
BYTE DZLogicDevID = 8;//串口模块ID
DWORD DevAddr = 1;
TS104APDU *pRxdFm = NULL;
WORD RecNum_Slave = 0;
WORD SendNum_Slave = 0;
WORD SendNum_Master = 0;
WORD RecNum_Master = 0;	
BYTE bConterW;
BYTE bComP_ReSeNum = 0; 	//是否对帧计数进行判断
uint32_t LocalTime = 0;
uint32_t CurrentTime = 0;
uint32_t BaseTimeLink = 0;
TS104APDU *pTxdFm;
static TCommIO m_Txd;				  //发送缓冲区的处理结构
WORD TimeCnt;
static uint8_t m_dwDevID = 0;
extern pthread_mutex_t mutex;
uint8_t *bRecvBuf; 	//将接收报文保存，处理缓冲区

void Handle_I_Format(int fd, int DevID, uint8_t *buf, uint16_t len);
void Txd_U_Format(int fd, BYTE Command);
static void Edit_call6465(int fd, int DevID, BYTE Style, BYTE Reason, BYTE QOI);
void Deal_RecvDz(int DevID, uint8_t *pbuf);
bool Edit_Deal_DzAo(int fd, int DevID);
static void Txd_Head(int DevID, BYTE bType, BYTE bdata);
static void Txd_Tail(int fd, int DevID, BYTE bReason, BYTE bNum, BYTE EmpFlat);
static void Edit_InfoAddr(BYTE *pBuf,WORD wNo);

BOOL GetDevFromAddr( DWORD dwDevAddr, DWORD *dwDevID)
{
	DWORD dwDevNo;
	for(dwDevNo=0;dwDevNo<gVars.dwDevNum;dwDevNo++)
	{
		if(gpDevice[dwDevNo].Address == dwDevAddr)
		{
			*dwDevID=dwDevNo;
			return TRUE;
		}
	}
        return FALSE;
}


void Deal_Rec104Error(void)
{
	bConterW = 0;
	SendNum_Master = 0;
	RecNum_Master = 0;	
	RecNum_Slave = 0;
	SendNum_Slave =0;	
	return;
}

int RxdMonitor104(int DevID, uint8_t *buf, uint16_t len)
{
	BYTE RecFormat;
	int fd = gpDevice[DevID].fd;
    pRxdFm = (TS104APDU*)buf;
	BaseTimeLink = LocalTime;	//接收到报文，认为有连接
	RecFormat = (pRxdFm->Control1 & 3);
	switch ( RecFormat )
	{
		case U_FORMAT:
		{
			//清相关的时间等标志
			if ( pRxdFm->Control1 == STARTDT_CON ){
				gpDevice[DevID].bStarFlg |=0x80;
				log("gpDevice[DevID].bStarFlg(%x)\n",gpDevice[DevID].bStarFlg);
			}
				
			if ( pRxdFm->Control1 == STOPDT_CON ){
				log("gpDevice[DevID].bStarFlg(%x)\n",gpDevice[DevID].bStarFlg);
				gpDevice[DevID].bStarFlg=0;	
			}
			

				
			return TRUE;
		}
		
		case S_FORMAT:
		{
			RecNum_Slave = (WORD)pRxdFm->Control4*256+pRxdFm->Control3;
			
			if( bComP_ReSeNum )
			{
				if( (SendNum_Master-RecNum_Slave)/2 >= IEC104_k )
				{
					Deal_Rec104Error();//进行错误处理，重启子站
					return TRUE;
				}
			}
			
			return TRUE;
		}
		
		default: 	//I_FORMAT1.主站对子站发送了STOP，2.子站地址不对使主站对子站的I数据不理睬
		{
			
			if (!(gpDevice[DevID].bStarFlg&0x80))
				return FALSE;
			if(pRxdFm->Address_L!=gpDevice[DevID].Address){
				log("addr(%d),devid(%d)",gpDevice[DevID].Address, DevID);
				return FALSE;
			}
				
			SendNum_Slave = (WORD)pRxdFm->Control2*256+pRxdFm->Control1;
			RecNum_Slave = (WORD)pRxdFm->Control4*256+pRxdFm->Control3;
			
			if( bComP_ReSeNum )
			{
				if( (SendNum_Master == RecNum_Slave)
					&&(SendNum_Slave-RecNum_Master)<IEC104_k)
				{
					RecNum_Master +=2;
					bConterW++;
					
//					if( !BaseTimeS )
//						BaseTimeS = m_dwTaskRunCount;	//用于判断主站发送S报文的时间T值
					
					Handle_I_Format(fd, DevID, buf, len);
//					return RxdMonitor104(fd, DevID, buf, len);
				}
				else
				{
					Deal_Rec104Error();//错误处理
					return TRUE;
				}
			}
			else
			{
				bConterW++;
				RecNum_Master += 2;
//				
//				if( !BaseTimeS )
//					BaseTimeS = m_dwTaskRunCount;	//用于判断主站发送S报文的时间T值
					
				Handle_I_Format(fd, DevID, buf, len);
//				return RxdMonitor104(fd, DevID, buf, len);
			}
		}	//end of default
	}	//end of switch(RecFormat)

}
bool TXD_DZ_ACT(int fd, int DevID)
{	
	BYTE Style;
	BYTE Reason=0x6;
	BYTE* pBuf;
	int i;
	
	pBuf = m_Txd.Buf;
	Style = 0x71;
	
	
	Txd_Head(DevID, Style, 0); 
	Edit_InfoAddr(m_Txd.Buf,0);
	
    m_Txd.Buf[ m_Txd.WritePtr++ ] = 1;
	Txd_Tail(fd, DevID, Reason, 0, 1);
    return TRUE;
}
//===================================================================
//  函数名称: CM104gx::Edit_RstProsess        
//  函数功能: 复位进程
//  输入参数:                 
//  返回值：	  TRUE/FALSE                
//===================================================================
bool Edit_RstProsess(int fd, int DevID)
{	
	BYTE Style;
	BYTE Reason=0x6;
	BYTE bSelFlg = 0x01;	//选择标志
	BYTE* pBuf;
	WORD wNo;

	Style = 0x69;

	pBuf = m_Txd.Buf;
	
	Txd_Head(DevID, Style, 0);	
	
  	wNo = 0;
  		
	Edit_InfoAddr(pBuf, wNo);

	
    m_Txd.Buf[ m_Txd.WritePtr++ ] = bSelFlg;   
    
	Txd_Tail(fd,  DevID, Reason, 1, 1);
    return TRUE;
}

static BOOL TimerOn(DWORD TimeOut) 
{ 
	return (TimeCnt%TimeOut)==0; 
}; //检查是否到了TimeOut间隔定时


BOOL TxdOnTimer(int DevID)
{
//	if(TimerOn(5)){
//		Edit_call6465(0x64,0x6,0x14);//总召唤
//		return TRUE;
//	}
//	if ( CheckClearDevFlag(SF_CallAll) )
//	{
//		Edit_call6465(0x64,0x6,0x14);//总召唤
//		return TRUE;
//	}
//
//	if ( CheckClearDevFlag(SF_CloSyn) )
//	{
//		Txd_67_SetTime();//时钟同步
//		return TRUE;
//	}
//	if ( CheckClearDevFlag(SF_CallAllDZ) )
//	{
//		Edit_call6465(0x6c,0x6,0x14);
//		return TRUE;
//	}
//	
	
// 	if ( CheckClearDevFlag(SF_CallAllCI) )
// 	{
// 		Edit_call6465(0x65,0x6,0x5);//总召唤电度
// 		return TRUE;
// 	}

	if(LocalTime%10 == 0){//定值总召唤
//		Edit_call6465(DevID, 0x6c,0x6,0x14);
		return TRUE;
	}



//	if(TimerOn(10)){
//		LogError(MLE_NORMAL, "m104_txdontimer");
//		Deal_RecvDztest();
//		return TRUE;
//	}

	
//	if ( (gpDevice[DevID].bStarFlg&0x80)&&(bTUFlg==1) )
//	{
//		bTUFlg = 0;
//		Txd_U_Format(TESTFR_ACT);
//		return TRUE; 
//	}
//	if(gpDevice[0]._flag == 1){
//		gpDevice[0]._flag = 0;
//		LogError(MLE_NORMAL, "m104_txdontimer");
//	}
	


	return TRUE;
}
BYTE CallStep = 0;


BOOL TxdMonitor104(int fd, int DevID)
{
	LocalTime++;
	CurrentTime = LocalTime;
		
//	if ( (gpDevice[DevID].bStarFlg==0)||((DWORD)(CurrentTime-BaseTimeLink)>IEC104_t1) )
	if (gpDevice[DevID].bStarFlg==0)
	{
		gpDevice[DevID].bStarFlg = 0;
		Txd_U_Format(fd, STARTDT_act);
		return TRUE;
	}
//	
//	if ( TxdUrgence() )		//发送紧急发送的报文
//		return TRUE;
//
//	if ( (bConterW>=IEC104_k)
//		||(BaseTimeS&&((DWORD)(CurrentTime-BaseTimeS)>T_NUM))&&(gpDevice[DevID].bStarFlg&0x80) )
//	{
//		bConterW = 0;
//		BaseTimeS = 0;
//		Txd_S_Format();		//要清t2标志，并且时间和计数都要清零
//		return TRUE;
//	}
//		
	if ( TxdOnTimer(DevID) )
		return TRUE;//发送定时发送的报文
		
	return TRUE; 
}


//================================================================
//  函数名称: CM104gx::Deal_RecvFloatYc
//  函数功能: 处理短浮点型遥测入库 
//  输入参数: 信息字节bYcFlg区分是否带时标
//  返回值：  无
//================================================================
void Deal_RecvFloatYc(BYTE bYcFlg)
{
	BYTE i;
	BYTE *pData;
	WORD wAINo;
	WORD wAINum;		
	DWORD dwTmp;
	DWORD dwValue;
	//WORD wMSecond;
	//TSysTime sysTime;
	if ( bYcFlg!=5 && bYcFlg!=12 )
		return;
	
	if( ((pRxdFm->Reason_L<0x14) )
		&&(pRxdFm->Reason_L!=3) )
		return;
	
	wAINum = pRxdFm->Definitive; //可变结构限定词
	
	if ( (bYcFlg==12) && (wAINum&0x80) )
		return;
		
	pData = bRecvBuf;
	
	if ( wAINum & 0x80 )//连续
	{	
		wAINum ^= 0x80;
		wAINo = MAKEWORD(pData[DEALSTATRT],pData[DEALSTATRT+1])-STARTYC104;
			
		pData += DEALSTATRT+BINFOADDR;
		
		for (i=0; i<wAINum; i++,pData+=bYcFlg)
		{
			if ( wAINo+i > gVars.TransYCTableNum )
				return;

			
			dwTmp = MAKEDWORD(pData[BINFOADDR], pData[BINFOADDR+1], 
						pData[BINFOADDR+2], pData[BINFOADDR+3]);
			
			dwValue = dwTmp;

			if(pRxdFm->Reason_L == 3)//突发遥测
			{
				gpDevice[m_dwDevID].pAIStatus[wAINo+i] |= OVDEAD_BIT;
				WriteOneLogicBurstAI(gpDevice[m_dwDevID].pRealBase->pRealAI[wAINo+i].pReflex, OVDEAD_BIT, dwValue);
			}
			
			WriteAI(m_dwDevID, wAINo+i, dwValue);			
		}
		
		return;
	}
	
	//不连续
	bYcFlg += BINFOADDR;
	pData += DEALSTATRT;
	
	for(i=0; i<wAINum; i++,pData+=bYcFlg)
	{
		wAINo = MAKEWORD(pData[0], pData[1])-STARTYC104;//遥测点号
		
		if (wAINo > gVars.TransYCTableNum)
			return;
			
		dwTmp = MAKEDWORD(pData[BINFOADDR], pData[BINFOADDR+1], 
					pData[BINFOADDR+2], pData[BINFOADDR+3]);
		
		dwValue = dwTmp;
		if(pRxdFm->Reason_L == 3)//突发遥测
		{
			gpDevice[m_dwDevID].pAIStatus[wAINo] |= OVDEAD_BIT;
			WriteOneLogicBurstAI(gpDevice[m_dwDevID].pRealBase->pRealAI[wAINo].pReflex, OVDEAD_BIT, dwValue);
		}

		/*wMSecond = MAKEWORD(pData[BINFOADDR+4],pData[BINFOADDR+5]);
		
		sysTime.Second = (wMSecond/1000);
		sysTime.MSecond = (wMSecond%1000);
		
		sysTime.Minute = (pData[BINFOADDR+6] & 0x3F);
		sysTime.Hour = (pData[BINFOADDR+7] & 0x1F);
		sysTime.Day = (pData[BINFOADDR+8] & 0x1F);
		sysTime.Month = (pData[BINFOADDR+9] & 0x0F);
		sysTime.Year = (pData[BINFOADDR+10] & 0x7F) + 2000;*/
		WriteAI(m_dwDevID, wAINo, dwValue);
		//WriteAISoe(m_dwDevID, wAINo, );
	}    
	return;
}


//================================================================
//  函数名称: CM104gx::Deal_RecvSSoe
//  函数功能: 处理单点SOE入库 
//  输入参数: 
//  返回值：  无
//================================================================
void Deal_RecvSSoe(int DevID)
{
	WORD i;	
	WORD wSoeNo;
	WORD wMSecond;
	BYTE bSoeNum;
	BYTE *pData;
	BYTE bDelNum;
	TSysTimeSoe sysTimeSoe; 
	TPtr *pPtr;	
	DWORD TempDevID;
	DWORD dwPtr;	
	
	if ( (pRxdFm->Reason_L!=3) && (pRxdFm->Reason_L!=0x0b)  && (pRxdFm->Reason_L!=0x24))
		return;
	if(pRxdFm->Reason_L == 3){
		pData = bRecvBuf+DEALSTATRT;	
		bDelNum = BINFOADDR+8;	
		bSoeNum = pRxdFm->Definitive; //可变结构限定词
		
		for(i=0; i<bSoeNum; i++,pData+=bDelNum)
		{
			wSoeNo=MAKEWORD(pData[0], pData[1])-STARTSYX104;
			
			if ( wSoeNo > wAllSYxNum )
				return;
		
			sysTimeSoe.nNo = wSoeNo;
					
			sysTimeSoe.bStatus = pData[BINFOADDR];	//分合状态
			wMSecond = MAKEWORD(pData[BINFOADDR+1], pData[BINFOADDR+2]);
		
			sysTimeSoe.stTime._milliseconds	= wMSecond;
			sysTimeSoe.stTime._min._minutes	= (pData[BINFOADDR+3] & 0x3F);
			sysTimeSoe.stTime._hour._hours		= (pData[BINFOADDR+4]);
			
//			WriteSysTimeSoe(m_dwDevID, &sysTimeSoe);
		}

	}

	if(pRxdFm->Reason_L == 0x24){
		pData = bRecvBuf+DEALSTATRT;	
		bDelNum = BINFOADDR+8;	
		bSoeNum = pRxdFm->Definitive; //可变结构限定词
		
		for(i=0; i<bSoeNum; i++,pData+=bDelNum)
		{
			wSoeNo=MAKEWORD(pData[0], pData[1])-STARTSYX104;
			
			if ( wSoeNo > wAllSYxNum )
				return;
		
			sysTimeSoe.nNo = wSoeNo;
					
			sysTimeSoe.bStatus = pData[BINFOADDR];	//分合状态
			wMSecond = MAKEWORD(pData[BINFOADDR+1], pData[BINFOADDR+2]);
		
			sysTimeSoe.stTime._milliseconds	= wMSecond;
			sysTimeSoe.stTime._min._minutes	= (pData[BINFOADDR+3] & 0x3F);
			sysTimeSoe.stTime._hour._hours		= (pData[BINFOADDR+4]);
			sysTimeSoe.stTime._day._dayofmonth		= (pData[BINFOADDR+5]);
			sysTimeSoe.stTime._month._month		= (pData[BINFOADDR+6]);
			sysTimeSoe.stTime._year._year		= (pData[BINFOADDR+7]);

			switch(gpDevice[DevID].Address)
			{
				case 1:
					GetDevFromAddr(4,&TempDevID);
					pPtr = &gpDevice[TempDevID].pSoeWrap->Ptr;
					dwPtr = pPtr->dwWritePtr%100;
					gpDevice[TempDevID].pSysTimeSoe[dwPtr] = sysTimeSoe;
					pPtr->dwWritePtr++;
					break;
				case 2:
					GetDevFromAddr(5,&TempDevID);
					pPtr = &gpDevice[TempDevID].pSoeWrap->Ptr;
					dwPtr = pPtr->dwWritePtr%100;
					gpDevice[TempDevID].pSysTimeSoe[dwPtr] = sysTimeSoe;
					pPtr->dwWritePtr++;
					break;
					
				case 3:
					GetDevFromAddr(6,&TempDevID);
					pPtr = &gpDevice[TempDevID].pSoeWrap->Ptr;
					dwPtr = pPtr->dwWritePtr%100;
					gpDevice[TempDevID].pSysTimeSoe[dwPtr] = sysTimeSoe;
					pPtr->dwWritePtr++;
					break;
				default:
					break;
			}

		}

	}

	return;
}

void Handle_I_Format(int fd, int DevID, uint8_t *buf, uint16_t len)
{
	bRecvBuf = buf;
	m_dwDevID = DevID;
	switch ( pRxdFm->Style )
	{
		case 1:		//M_SP_NA_1 
		{   
//			Deal_RecvSYx( );  		
			break;		
		}
		
		case 3:		//M_DP_NA_1 	
		{   
//			Deal_RecvDYx( );  		
			break;		
		}
		
		case 21:	//M_ME_ND_1 
		{   
//			Deal_RecvYc(2); 		
			break;		
		}
		
		case 9:		//M_ME_NA_1 
		case 11:	//M_ME_NB_1 	
		{   
//			Deal_RecvYc(3); 		
			break;		
		}
		
		case 34:	//M_ME_TD_1 
		case 35:	//M_ME_TE_1 	
		{   
//			Deal_RecvYc(10); 		
			break;		
		}
		
		case 13:	//M_ME_NC_1 
		{   
			Deal_RecvFloatYc(5);  		
			break;		
		}
		
		case 36:	//M_ME_TF_1 
		{   
//			Deal_RecvFloatYc(12);  		
			break;		
		}
		
		case 15:	//M_ME_NA_1 
		{   
//			Deal_RecvMc(5);  		
			break;		
		}
		
		case 37:	//M_ME_NB_1 
		{   
//			Deal_RecvMc(12);  		
			break;		
		}
		
		case 30:	//M_SP_TB_1 
		{   
			Deal_RecvSSoe(DevID);  		
			break;		
		}
		
		case 31:	//M_DP_TB_1 
		{   
//			Deal_RecvDSoe();  		
			break;		
		}	
		case 45:	
		case 46:	
//			Deal_RecvDo();
			break;		
		case 60:
		{
//			Deal_RecvDo();
			break;
		}
		case 70://初始化结束
		{
//			if(Flag_SendOneYX == 1){
//				if(::GetDevFromAddr(DevAddr,&DevID)){//找到模块地址是m_dwDzID的设备
//					WriteBI(DevID, 0, PBI_OFF,0,0,0,0,0);
//					Flag_SendOneYX = 0;
//					sprintf(sss,"M104---DevID(%d)  m_dwDevID(%d) devtype(%d)", DevID, m_dwDevID, gpDevice[DevID].dwType);
//					LogError(MLE_NORMAL,sss);
//				}
//				
//			}
			
			break;
		}
	
		case 0x70:	//定值总召唤
			Deal_RecvDz(DevID, buf);
			break;
		case 0x71:	//定值激活确认
			break;
			
		default:	//未知的类型标识
			break;
	
	}
	return;
}

void Txd_U_Format(int fd, BYTE Command)
{
	m_Txd.ReadPtr = 0;
	m_Txd.WritePtr = 6;
	
	pTxdFm=(TS104APDU *)m_Txd.Buf;
	
	pTxdFm->Start = STARTHEAD;
	pTxdFm->Length = 4;
	pTxdFm->Control1 = Command;
	pTxdFm->Control2 = 0;
	pTxdFm->Control3 = 0;
	pTxdFm->Control4 = 0;
	IEC10X->Send(fd, m_Txd.Buf, m_Txd.WritePtr);
	return;
}
static void Txd_Head(int DevID, BYTE bType, BYTE bdata)
{
	BYTE* pBuf;

	pBuf = m_Txd.Buf;
	m_Txd.ReadPtr = 0;
	
	pBuf[0] = STARTHEAD;	//报文头
  										//报文长度，先不处理
  	pBuf[2] = LOBYTE(SendNum_Master);    //控制字节1，发送序列号高
  	pBuf[3] = HIBYTE(SendNum_Master);  	//控制字节2
  	pBuf[4] = LOBYTE(RecNum_Master);    //控制字节3，接收序列号高
  	pBuf[5] = HIBYTE(RecNum_Master);  	//控制字节4
  	pBuf[6] = bType;					//类型表标识
										//传送数目，先不处理
										//传送原因，先不处理
  	pBuf[9] = 0x00;
  	pBuf[10] = gpDevice[DevID].Address;					//公共地址，虚拟RTU
  	pBuf[11] = 0x00;					//公共地址改为1个字节
	
	SendNum_Master += 2;				//发送序列号加2
	m_Txd.WritePtr = 12;				//加上信息体地址前的12个字节
	return;
}
static void Txd_Tail(int fd, int DevID, BYTE bReason, BYTE bNum, BYTE EmpFlat)
{
	BYTE* pBuf;	

	pBuf = m_Txd.Buf;
	pBuf[1] = m_Txd.WritePtr-2;		//发送报文长度
	pBuf[7] = bNum;					//传送个数
	pBuf[8] = bReason;				//传送原因

	IEC10X->Send(fd, m_Txd.Buf, m_Txd.WritePtr);
	if(1){
//	if(m_Txd.Buf[15] != 0x14){
		log("\nm104send--DevID(%d):",DevID);
		DumpHEX(m_Txd.Buf, m_Txd.WritePtr);
	}
	return;
}

static void Edit_InfoAddr(BYTE *pBuf,WORD wNo) 
{                       
	
	if (BINFOADDR == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //信息体地址Hi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //信息体地址Hi
	}
	return;
}


static void Edit_call6465(int fd, int DevID, BYTE Style, BYTE Reason, BYTE QOI)
{
	
	Txd_Head(DevID, Style,0);
	Edit_InfoAddr(m_Txd.Buf,0);
    m_Txd.Buf[ m_Txd.WritePtr++ ] = QOI;
	Txd_Tail(fd, DevID, Reason, 1, 1);
	return;
}
void Deal_RecvDz(int DevID, uint8_t *pbuf)
{
	BYTE i;
	BYTE *pData;
	WORD wDzNo;
	WORD wDzNum;
	DWORD dwDzValue;
	BYTE wAllDzNum = 114;
	BYTE Data_width = 4;
	DWORD TempDevID = 0;
	BYTE *dzbuf;
	Data_Value_T DataValue;
	
	GetDevFromAddr(5,&TempDevID);
	
	wDzNum = pRxdFm->Definitive; //可变结构限定词
	pData = pbuf;
	if(pRxdFm->Reason_L == 3){
		PRINT_FUNLINE;
	
		gpDevice[TempDevID].pDZ[1] = gpDevice[DevID].Address;//突发的设备的地址号          pDZ[1]给s104用
		gpDevice[TempDevID].pDZ[2] = gpDevice[DevID].Address;//突发的设备的地址号 pDZ[2]给s101用
		gpDevice[TempDevID].pDZ[3] = 1;//定值突发-----pDZ[3]给s101用
		gpDevice[TempDevID].pDZ[4] = 1;//定值突发-----pDZ[3]给s104用
		log("gpDevice[DevID].Address(%d)\n",gpDevice[DevID].Address);
		dzbuf = (BYTE *)&gpDevice[TempDevID].pDZ[10];//从地址10开始缓存数据
		memcpy(dzbuf,pData,pRxdFm->Length +2);

		if ( wDzNum & 0x80 )//连续
		{	
			wDzNum ^= 0x80;
			wDzNo = MAKEWORD(pData[DEALSTATRT],pData[DEALSTATRT+1])-STARTDZ104;
				
			pData += DEALSTATRT+BINFOADDR;
			
			for(i=0; i<wDzNum; i++,pData+=Data_width)
			{
				if ( wDzNo+i > wAllDzNum )
					return;
				dwDzValue = MAKEDWORD(pData[0], pData[1],pData[2], pData[3]);//定值
//				gpDevice[m_dwDevID].pDZ[i] = dwDzValue;
				gpDevice[DZLogicDevID].pDZ[(gpDevice[DevID].Address-1)*114 + i] = dwDzValue;
				
			}
			
			return;
		}
		
		//NO不连续
		Data_width += BINFOADDR;
		pData += DEALSTATRT;
		
		for(i=0; i<wDzNum; i++,pData+=Data_width)
		{
			wDzNo = MAKEWORD(pData[0], pData[1])-STARTDZ104;//定值点号
			
			if ( wDzNo > wAllDzNum )
				return;
				
			dwDzValue = MAKEDWORD(pData[BINFOADDR],pData[BINFOADDR+1],pData[BINFOADDR+2],pData[BINFOADDR+3]);//定值
//			gpDevice[m_dwDevID].pDZ[wDzNo] = dwDzValue;
			gpDevice[DZLogicDevID].pDZ[(gpDevice[DevID].Address-1)*114 + wDzNo] = dwDzValue;
		}	

		
		return;
	}
	

	if((pRxdFm->Reason_L & 0x0f) == 7){
		GetDevFromAddr(4,&TempDevID);
		log("gpDevice[TempDevID].pDZ[5](%d)\n",gpDevice[TempDevID].pDZ[5]);
		if(gpDevice[TempDevID].pDZ[5] == Type_104){//104
			GetDevFromAddr(6,&TempDevID);
		}
		gpDevice[TempDevID].pDZ[5] = Type_clear;
		
		gpDevice[TempDevID].pDZ[0] = pRxdFm->Reason_L;
		switch (gpDevice[DevID].Address){
			case 1:
				gpDevice[TempDevID].pDZ[1] = 1;
				dzbuf = (BYTE *)&gpDevice[TempDevID].pDZ[Dev1_DZ_Start];
				memcpy(dzbuf,pData,pRxdFm->Length +2);
				break;
			case 2:
				gpDevice[TempDevID].pDZ[2] = 1;
				dzbuf = (BYTE *)&gpDevice[TempDevID].pDZ[Dev2_DZ_Start];
				memcpy(dzbuf,pData,pRxdFm->Length +2);
				break;
			case 3:
				gpDevice[TempDevID].pDZ[3] = 1;
				dzbuf = (BYTE *)&gpDevice[TempDevID].pDZ[Dev3_DZ_Start];
				memcpy(dzbuf,pData,pRxdFm->Length +2);
				break;
			default :
				break;
		
		}


		return;
	}

	
	pData = pbuf;
	if(pRxdFm->Reason_L == 0x14){
		if ( wDzNum & 0x80 )//连续
		{	
			wDzNum ^= 0x80;
			wDzNo = MAKEWORD(pData[DEALSTATRT],pData[DEALSTATRT+1])-STARTDZ104;
			pData += DEALSTATRT+BINFOADDR;
			for(i=0; i<wDzNum; i++,pData+=Data_width)
			{
				if ( wDzNo+i > wAllDzNum )
					return;
				dwDzValue = MAKEDWORD(pData[0], pData[1],pData[2], pData[3]);//定值
//				if(dwDzValue){
//					DataValue._int32 = dwDzValue;
//					log("addr(%d)value(%x)value(%f)\n", wDzNo+i, dwDzValue, DataValue._float);
//				}
//				DataValue._int32 = dwDzValue;
//				log("addr(%d)value(%x)value(%f)\n", wDzNo+i, dwDzValue, DataValue._float);
				gpDevice[DZLogicDevID].pDZ[(gpDevice[DevID].Address-1)*114 + i + wDzNo] = dwDzValue;
				
			}
			
			return;
		}
		
		//NO不连续
		Data_width += BINFOADDR;
		pData += DEALSTATRT;
		for(i=0; i<wDzNum; i++,pData+=Data_width)
		{
			wDzNo = MAKEWORD(pData[0], pData[1])-STARTDZ104;//遥测点号
			
			if ( wDzNo > wAllDzNum )
				return;
				
			dwDzValue = MAKEDWORD(pData[BINFOADDR],pData[BINFOADDR+1],pData[BINFOADDR+2],pData[BINFOADDR+3]);//定值
			gpDevice[DZLogicDevID].pDZ[(gpDevice[DevID].Address-1)*114 + wDzNo] = dwDzValue;
		}	 
		return;
	}
	
}
//===================================================================
//  函数名称: CM104gx::Edit_Deal_DzAo        
//  函数功能: 参数预置
//  输入参数:                 
//  返回值：	  TRUE/FALSE                
//===================================================================
bool Edit_Deal_DzAo(int fd,int DevID)
{	
	BYTE Style;
	BYTE Reason=0x6;
	BYTE* pBuf;
	WORD wNo;
	BYTE* dzBuf;
	BYTE* Data;
	int i;
	BYTE info_num;
	int info_sq;
	int Flag_sq = 0;
	DWORD Dz_DevID;
	BYTE ProtocolStyle;//0:104,1:101
	BYTE info_addr_start;
	BYTE info_start = 12;
	DWORD dwValue;
	DWORD protocol_type = 0;
	S104DZVALUE_T *DZvalue = NULL;
	S104DZVALUE_T *DZvaluetmp = NULL;

	
	GetDevFromAddr(4,&Dz_DevID);
	protocol_type = gpDevice[Dz_DevID].pDZ[5];

	if(protocol_type == Type_104){//104用地址6缓存定值预置数据，101用地址4缓存
		GetDevFromAddr(6,&Dz_DevID);
		PRINT_FUNLINE;
	}
	
	pBuf = m_Txd.Buf;
	Style = 0x70;
	info_num = gpDevice[Dz_DevID].pDZ[0];
	
	info_addr_start = 10;
	
	Data = (BYTE*)&gpDevice[Dz_DevID].pDZ[info_addr_start];

	for(i=0;i<(info_num-1);i++){
		DZvalue = (S104DZVALUE_T*)Data;
		DZvaluetmp = (S104DZVALUE_T*)(Data+6);
		if((DZvalue->addr +1) == (DZvaluetmp->addr)){
			Flag_sq = 0x80;
		}
		Data += 6;
	}
	Data = (BYTE*)&gpDevice[Dz_DevID].pDZ[info_addr_start];
	
	if(Flag_sq){//连续
		Txd_Head(DevID, Style, 0); 
		
		wNo = MAKEWORD(Data[0], Data[1]);
		wNo %= 114;
		wNo += 0x5001;
		m_Txd.WritePtr = DEALSTATRT;
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //信息体地址Hi
		pBuf[m_Txd.WritePtr++] = 0;
		for(i=0;i<info_num;i++){
			pBuf[m_Txd.WritePtr++] = Data[2]; //信息体地址Lo
			pBuf[m_Txd.WritePtr++] = Data[3]; //信息体地址Hi
			pBuf[m_Txd.WritePtr++] = Data[4];
			pBuf[m_Txd.WritePtr++] = Data[5];
			Data += 6;
		}
	
	}else{//计划使用连接4的定值区做定值预置的的缓冲区
		Txd_Head(DevID, Style, 0); 
		m_Txd.WritePtr = DEALSTATRT;
		for(i=0;i<info_num;i++){
			wNo = MAKEWORD(Data[0], Data[1]);
			wNo %= 114;
			wNo += 0x5001;
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //信息体地址Hi
			pBuf[m_Txd.WritePtr++] = 0;
			pBuf[m_Txd.WritePtr++] = Data[2]; 
			pBuf[m_Txd.WritePtr++] = Data[3]; 
			pBuf[m_Txd.WritePtr++] = Data[4];
			pBuf[m_Txd.WritePtr++] = Data[5];
			Data += 6;

		}
	}

	
	Txd_Tail(fd, DevID, Reason, info_num | Flag_sq, 1);
    return TRUE;
}

int OnTimeOut_m104(int DevID)
{	

	return RET_SUCESS;
}

int miec104_BuildYk(int DevID, uint8_t Type, uint16_t reason, uint8_t YKData){

//    uint8_t len = 0, asdu_num = 1;
//    uint8_t *ptr = NULL;
//    uint32_t Temp32 = 0;
//    PGX104Master_DATA_T GX104MasterData = (PGX104Master_DATA_T)GX104Master_Sendbuf;
//    PIEC10X_ASDU_T asdu = (PIEC10X_ASDU_T)(GX104MasterData->Asdu);
//    PASDU_INFO_T info = (PASDU_INFO_T)(asdu->_info);
//	uint8_t YKCommand = 0;
//	
//    /*build head*/
//    GX104MasterData->Head = GX104Master_HEAD;
//
//    /*build control code*/
//    GX104MasterData->Ctrl.I.Type = 0;
//    GX104MasterData->Ctrl.I.SendSn = SendRecvSn.BuildSendSn++;
//    GX104MasterData->Ctrl.I.RecvSn = SendRecvSn.BuildRecvSn;
//    /*build ASDU , COT ,Addr*/
//    asdu->_type = Type;
//    asdu->_num._sq = 0;
//    asdu->_num._num = asdu_num;
//    asdu->_reason._reason = reason;
//    asdu->_addr = gpDevice[DevID].Address;
//    /*build info addr*/
//    ptr = info->_addr;
//    Temp32 = 0+STARTSYK;
//    memcpy(ptr, &Temp32, 3);
//	ptr += 3;
//    /*Build Detect value*/
//	*ptr = YKData;
//    ptr++;
//    /*len*/
//    len = ptr - GX104Master_Sendbuf;
//    GX104MasterData->Len = len - 2;
//	GX104Master_send(DevID,(char *)GX104Master_Sendbuf,len);

    return RET_SUCESS;
}

/*****************************************************************
函数名称: miec104_Task
函数功能: 快速响应任务
输入参数: 
	DevID:设备ID号
返回值：  无
修改日期：2020-3-31
*****************************************************************/
int miec104_Task(int DevID)
{
	return 0;
}

int Init_miec104gx(int DevID)
{
	gpDevice[DevID].Receive = RxdMonitor104;
	gpDevice[DevID].Task = miec104_Task;
	gpDevice[DevID].SetYK = miec104_BuildYk;
	gpDevice[DevID].OnTimeOut = OnTimeOut_m104;

}


