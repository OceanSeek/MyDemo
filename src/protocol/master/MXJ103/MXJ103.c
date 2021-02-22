#include "sys.h"

//�ⲿ����������
extern TBurstYXRecord YXBrustRecord[MAXYXRECORD];
extern TBurstYXRecord YXSoeRecord[MAXYXRECORD];
extern TBurstYCRecord YCBrustRecord[MAXYCRECORD];
extern TransTable_T *TransYKTable;


//����������
int call_all_cou ;
int call_data_cou ;
int rccou ;
bool init_flag1;
bool init_flag2;
//	bool auto_flag;
bool call_all;
bool call_ai_flag;
WORD wCount;
WORD wAICount;
BYTE bCount;
BYTE bDZCount;
bool Call_complete;
bool bAct;
bool bAlarm;
bool bSetTime;
//	TFlagInfo FlagInfo[maxDevNum];/*����װ������*/
WORD wBINum;
WORD wRDZNum;
WORD wDzValue[50];
bool time_Flag;
BYTE bCallCount;

TTxdFrameHead * pTxdFrameHead;     //����֡ͷָ��
TRxdFrameHead * pRxdFrameHead;     //����֡ͷָ��

TInfo * pInfo[MAX_SUBDEVNO];       //����ʱ��Ҫ�洢����Ϣ�ṹ
BYTE DeviceAddress[maxDevNum];
WORD wSlaveNum;

BYTE CurrentDevice;		//��ǰ�ٻ���װ����� ע�� ���ǵ�ַ ��ʼ��Ϊ0

TGenData GenData;                  //ͨ�÷������ݽṹ
//wsnr
bool  YKHAVESELECT;
WORD dwYkAddress;                  //�ӿ�ң���ٶȣ���סԭ����ң��Ԥ�õ�ģ���ַ
//wsnr
BYTE bYKNo;
BYTE bYKAttribte;


static TCommIO m_Txd;				  //���ͻ������Ĵ���ṹ


uint8_t XJ103_Master_Sendbuf[XJ103_Slaver_MAX_BUF_LEN];


//����������
bool SetAsduHead(BYTE byTYP,BYTE byVSQ,BYTE byCOT,BYTE byADDRESS,BYTE byFUN,BYTE byINF);
bool Txd68_Head(int DevNo, BYTE byFC,BYTE byFCV);
bool Txd68_Tail(int DevNo);
bool Txd68_GData_10(int DevNo, BYTE byINF);
bool Txd68_GCommand_21(int DevNo, BYTE byINF);
bool Txd68_Call_YC(int DevNo);
bool Txd68_Call_7(int DevNo);
bool Txd68_SST_6(int DevNo);
bool Txd10_BaseFrame(int DevNo, BYTE byFC,BYTE byFCV);
bool Txd_OnTimer_Data(int DevNo);




//int RxdMonitor_XJ103(int DevNo, uint8_t *buf, uint16_t len);
static int OnTimeOut_XJ103(int DevNo);
static int TxdMonitor(int DevNo);

int Init_XJ103Master(int DevNo)
{
//	gpDevice[DevNo].Receive = RxdMonitor_XJ103; 	
	gpDevice[DevNo].OnTimeOut = OnTimeOut_XJ103; 	
	gpDevice[DevNo].Task = TxdMonitor;
	
//	wBINum = gVars.TransYXTableNum; 			//����ң����Ŀ
//	wAINum = gVars.TransYCTableNum; 			//ң����Ŀ

	// ========  ��д��Լ��Ϣ��	========
	//WORD wBINum;
	
	FILE* fileID;
	BYTE addr;
	WORD wNo;
//	TRealBI* RealBI;
	

	pTxdFrameHead = (TTxdFrameHead *)m_Txd.Buf;

	CurrentDevice = 0;


	init_flag1 = false;
	init_flag2 = false;
	call_ai_flag = true;
	call_all = false;
	wCount = 0;
	wAICount = 0;
	wRDZNum = 0;
	bCount = 0;
	call_all_cou =0;
	call_data_cou =0;
	rccou =0;
	Call_complete = false;
	time_Flag = false;	 //��ʱ��־
	bCallCount = 0;
	bAct = false;		 //�ж����ź�
	bAlarm = false; 	 //�и澯�ź�
	bSetTime = false;	//

	//��ʼ����Ҫ��ɵĴ���
//	SetAllDevFlag(SF_LINKSTATUS_ERROR); 		//��ʼ��������·״̬Ϊ����
//	SetAllDevFlag(SF_CALLLINK); 				//��������·״̬��־
//	SetAllDevFlag(SF_RESETCU);					//�ø�λͨ�ŵ�Ԫ��־
//	SetAllDevFlag(SF_CALLALL);					//�����ٻ���־
//	SetAllDevFlag(SF_SST);						//�ö�ʱ��־
//	ClearAllDevFlag(SF_HaveAO); 



	return TRUE;

}

//*******************************************************************************************//
//  ��������: CheckSum
//  ��������: ʵ���ۼӺ�У��
//  �������: BYTE * pBuf, ���뻺����
//            BYTE byLength, ����������
//  ����ֵ:   BYTE bySum, �ۼӺ�
//*******************************************************************************************//
static BYTE CheckSum(BYTE * pBuf,BYTE byLength)
{
	BYTE bySum=0;
	int i;
	for(i=0;i<byLength;i++)
		bySum+=pBuf[i];
	return bySum;
}

static uint8_t XJ103_send(int DevNo,char *buf, int len){
					 
    if(-1 == write(gpDevice[DevNo].fd,buf,len)){
        perror("Send error \n");
		log("DevNo(%d)  fd(%d)\n",DevNo, gpDevice[DevNo].fd);
        return RET_ERROR;
    }

	MonitorTx(monitorData._TX_ID, DevNo, monitorData._fd, buf, len);
	
    return RET_SUCESS;
}


//===================================================================
//
//		��ʱ���ݴ�����
//
//===================================================================
//  ��������: OnTimeOut
//  ��������: ��ʱʱ�䵽
//  �������:                         
//  ����ֵ����                        
//===================================================================
static int OnTimeOut_XJ103(int DevNo)
{
	gpDevice[DevNo].TimeCnt++;

	if(gpDevice[DevNo].TimeCnt%3 == 0){
		Txd68_SST_6(DevNo);
	}

//			if( GetRetryFlag() )											//��ʱ�ط�����
//				return TxdRetry();
//		  
//			if(TxdUrgence())			//���ͽ������͵ı���
//				return true;
		
			
	if(call_all_cou >= 20)
	{
		call_all_cou =1;
		call_ai_flag = false;
		call_data_cou ++;
//			rccou ++;
//			if(rccou >=50)
//			{
//				call_all_cou =0;
//				rccou =0;
//			}
	}
	PRINT_FUNLINE;
	if(call_data_cou >20)
	{
		call_all = false;
		call_data_cou =0;
	}
	PRINT_FUNLINE;
	if(call_all_cou ==0)
	{
		init_flag2 =false;
		call_all_cou ++;
	}
	PRINT_FUNLINE;
	if(init_flag2 ==true && rccou ==0)
	{
		call_all = false;
		rccou =1;
		call_all_cou ++;
	}
	PRINT_FUNLINE;
	if(call_all ==true )
		call_all_cou ++;
	
	if(init_flag1==false)
	{
		Txd10_BaseFrame(DevNo, FC_M_CALLLINK,FCV_INVALID);
		CurrentDevice++;
		if(CurrentDevice==wSlaveNum)
		{
			CurrentDevice=0;
			init_flag1 = true;
		}
		return true;
	}
	PRINT_FUNLINE;
	if(init_flag2==false)
	{
		Txd10_BaseFrame(DevNo, FC_M_RESETCU,FCV_INVALID);
		CurrentDevice++;
		if(CurrentDevice==wSlaveNum)
		{
			CurrentDevice=0;
			init_flag2 = true;
		}
		return true;
	}
	PRINT_FUNLINE;
	if(bSetTime==false)
	{
		Txd68_SST_6(DevNo);
		bSetTime = true;
		return true;
	}
	PRINT_FUNLINE;
	if(call_all==false)
	{
		Txd68_Call_7(DevNo);//���ٻ�					
			CurrentDevice++;
		if(CurrentDevice==wSlaveNum)
		{
			CurrentDevice=0;
//				bCount++;
//				if(bCount>1)
//				{
//					bCount = 0;
				call_all = true;
//				}
		}
		return true;
	}
	PRINT_FUNLINE;
	if(call_ai_flag==false) 				//�ٻ�ң��ֵ
	{
	//	Txd68_GCommand_21(INF_M_GEN_RGROUPVALUE);
	//	Txd10_CallClass2_11();
		Txd68_Call_YC(DevNo);
		CurrentDevice++;
		if(CurrentDevice==wSlaveNum)
		{
			CurrentDevice=0;
			call_ai_flag = true;
		}
		
		return true;
	}
	PRINT_FUNLINE;
	Txd10_BaseFrame(DevNo, FC_M_CALLCLASS1,FCV_VALID);
	//Txd10_CallClass2_11();
//	taskDelay(200);
	CurrentDevice++;
	if(CurrentDevice==wSlaveNum)
	{
		CurrentDevice=0;
		wCount++;
		if(wCount==50)	//50
		{
			wCount=0;
			//wAICount++;
			//if(wAICount==5)//50
			//{
				call_all=false;
			//	call_ai_flag = true;
			//	wAICount=0;
			//}
		//	else
			//	call_ai_flag = false;
		}
	}
	PRINT_FUNLINE;
	return TRUE;




	
	return RET_SUCESS;			
}

//*******************************************************************************************//
//  ��������: TxdMonitor
//  ��������: ʵ��MNR103��Լ�ķ��ʹ���
//  �������:
//  ����ֵ:   TRUE,�ɹ�;
//            FALSE,ʧ��
//*******************************************************************************************//

static int TxdMonitor(int DevNo)
{
	
////	bCount++;
////	if(bCount==2)
////	{
////		bCount=0;
////		if( GetRetryFlag() )											//��ʱ�ط�����
////	  		return TxdRetry();
//	  
////		if(TxdUrgence())			//���ͽ������͵ı���
////			return true;
	
        
//		if(call_all_cou >= 20)
//		{
//            call_all_cou =1;
//			call_ai_flag = false;
//			call_data_cou ++;
////			rccou ++;
//// 			if(rccou >=50)
//// 			{
//// 				call_all_cou =0;
//// 				rccou =0;
//// 			}
//		}
//		PRINT_FUNLINE;
//		if(call_data_cou >20)
//		{
//			call_all = false;
//			call_data_cou =0;
//		}
//		PRINT_FUNLINE;
// 		if(call_all_cou ==0)
//		{
// 			init_flag2 =false;
//			call_all_cou ++;
//		}
//		PRINT_FUNLINE;
// 		if(init_flag2 ==true && rccou ==0)
//		{
// 			call_all = false;
//			rccou =1;
//			call_all_cou ++;
//		}
//		PRINT_FUNLINE;
//        if(call_all ==true )
//            call_all_cou ++;
//		
//		if(init_flag1==false)
//		{
//			Txd10_BaseFrame(DevNo, FC_M_CALLLINK,FCV_INVALID);
//			CurrentDevice++;
//			if(CurrentDevice==wSlaveNum)
//			{
//				CurrentDevice=0;
//				init_flag1 = true;
//			}
//			return true;
//		}
//		PRINT_FUNLINE;
//		if(init_flag2==false)
//		{
//			Txd10_BaseFrame(DevNo, FC_M_RESETCU,FCV_INVALID);
//			CurrentDevice++;
//			if(CurrentDevice==wSlaveNum)
//			{
//				CurrentDevice=0;
//				init_flag2 = true;
//			}
//			return true;
//		}
//		PRINT_FUNLINE;
//		if(bSetTime==false)
//		{
//			Txd68_SST_6(DevNo);
//			bSetTime = true;
//			return true;
//		}
//		PRINT_FUNLINE;
//		if(call_all==false)
//		{
//			Txd68_Call_7(DevNo);   					
//		    	CurrentDevice++;
//			if(CurrentDevice==wSlaveNum)
//			{
//				CurrentDevice=0;
////				bCount++;
////				if(bCount>1)
////				{
////					bCount = 0;
//					call_all = true;
////				}
//			}
//			return true;
//		}
//		PRINT_FUNLINE;
//		if(call_ai_flag==false)					//�ٻ�ң��ֵ
//		{
//		//	Txd68_GCommand_21(INF_M_GEN_RGROUPVALUE);
//		//	Txd10_CallClass2_11();
//			Txd68_Call_YC(DevNo);
//			CurrentDevice++;
//			if(CurrentDevice==wSlaveNum)
//			{
//				CurrentDevice=0;
//				call_ai_flag = true;
//			}
//			
//			return true;
//		}
//		PRINT_FUNLINE;
//		Txd10_BaseFrame(DevNo, FC_M_CALLCLASS1,FCV_VALID);
//		PRINT_FUNLINE;
//		//Txd10_CallClass2_11();
//	//	taskDelay(200);
//		CurrentDevice++;
//		if(CurrentDevice==wSlaveNum)
//		{
//			CurrentDevice=0;
//			wCount++;
//			if(wCount==50)  //50
//			{
//				wCount=0;
//				//wAICount++;
//				//if(wAICount==5)//50
//				//{
//					call_all=false;
//				//	call_ai_flag = true;
//				//	wAICount=0;
//				//}
//			//	else
//				//	call_ai_flag = false;
//			}
//		}
//		return TRUE;
}

//*******************************************************************************************//
//  ��������: ProcessFCB
//  ��������: ��������ǰ,FCBλ�Ĵ���
//  �������: BYTE byFCV,FCVλ
//  ����ֵ:   ��
//*******************************************************************************************//
void ProcessFCB(BYTE byFCV)
{
	//FCB�Ĵ���,���FCV��Ч,��FCBȡ��
//	if(byFCV==FCV_VALID)
//		pInfo[DevNo]->byCurrentFCB=( ~(pInfo[DevNo]->byCurrentFCB))&0x1;
}


//===================================================================
//  ��������: RxdMonitor        
//  ��������: ���ݽ�����Ϣ
//  �������:                         
//  ����ֵ����                        
//===================================================================
//static int RxdMonitor_XJ103(int DevNo, uint8_t *buf, uint16_t len)
//{
//	//	BYTE* pBuf;
//		bool bReturn=FALSE;
//	
//		if(SearchFrame()!=TRUE)
//			return FALSE;
//		pRxdFrameHead = (TRxdFrameHead *)m_RxdFrame.Buf;
//	
//		//����������еı�־
//		if( ReceiveControlFlag(pRxdFrameHead->FrameFix_10.byStart) != TRUE )
//			return FALSE;
//	
//		switch(pRxdFrameHead->FrameFix_10.byStart)
//		{
//		case SYN_START_10:	   //�̶�֡������
//	/*		if(m_RxdFrame.Buf[1]==0x29)
//			{
//				auto_flag=true;
//			}*/
//			switch(pRxdFrameHead->FrameFix_10.Control.FC)
//			{
//			case FC_S_ACK:			bReturn=Rxd10_Ack_0();			break;	   //ȷ��֡
//			case FC_S_BUSY: 		bReturn=Rxd10_Busy_1(); 		break;	   //��·æ֡
//			case FC_S_NODATA:		bReturn=Rxd10_NoData_9();		break;	   //�����ٻ�������
//			case FC_S_LINKOK:		bReturn=Rxd10_LinkOk_11();		break;	   //��·��������
//			case FC_S_LINKERROR:
//			case FC_S_LINKUNFIN:	bReturn=Rxd10_LinkError_14();	break;	   //��·δ����
//			default:	break;
//			}
//			break;
//	
//		case SYN_START_68:	   //�ɱ�֡������
//	
//			switch(pRxdFrameHead->FrameVar_68.Control.FC)
//			{
//			case FC_S_DATA: 		bReturn=Rxd68();				break;	   //������Ӧ֡
//			default:	break;
//			}
//			break;
//	
//		default:			   //����֡
//			break;
//		}
//	
//		return bReturn;
//
//}


//===================================================================
//  ��������: SearchOneFrame        
//  ��������: �жϽ��յ��������Ƿ���ȷ
//  �������:                         
//  ����ֵ����                        
//===================================================================
static DWORD SearchOneFrame(int DevNo, uint8_t* pBuf,short sLen)
{
//	WORD wFrameLength=5;
//
//	if(wLength<MIN_RXDFRAME_SIZE)
//		return FM_LESS;
//
//	pRxdFrameHead = (TRxdFrameHead *)pBuf;
//
//	switch(pRxdFrameHead->FrameFix_10.byStart)
//	{
//	case SYN_START_10:	   //�̶�֡������
//		{
//		//��ַ����ж�
//		if(SwitchToAddress((pRxdFrameHead->FrameFix_10).byAddress)==FALSE)
//			return FM_ERR|1;
////			return FM_ERR|wFrameLength;
//
//		//У��͵��ж�
//		if( (pRxdFrameHead->FrameFix_10.byChecksum) !=
//			  CheckSum((BYTE *)&(pRxdFrameHead->FrameFix_10.Control),2) )
//			return FM_ERR | 1;
////			return FM_ERR|wFrameLength;
//
//		//�����ַ����ж�
//		if( pRxdFrameHead->FrameFix_10.byStop != SYN_STOP )
//			return FM_ERR|1;
////			return FM_ERR|wFrameLength;
//
//		//֡У����ȷ
//		return FM_OK|wFrameLength;
//		}
//
//	case SYN_START_68:	   //�ɱ�֡������
//		{
//		wFrameLength=(pRxdFrameHead->FrameVar_68).byLength1+6;
//
//		//�����ַ����ж�
//		if( pRxdFrameHead->FrameVar_68.byStart1 != pRxdFrameHead->FrameVar_68.byStart2 )
//			return FM_ERR|1;
////			return FM_ERR|wFrameLength;
//
//		//���ȵ��ж�
//		if( pRxdFrameHead->FrameVar_68.byLength1 != pRxdFrameHead->FrameVar_68.byLength2 )
//			return FM_ERR|1;
////			return FM_ERR|wFrameLength;
//
//		//���ձ��ĳ��Ȳ����ڱ�֡���ĳ���
//		if(wLength< wFrameLength)
//		{
//			return FM_LESS;
//		}
//		if(wFrameLength != wLength)
//			return FM_ERR|1;
////			return FM_ERR|wFrameLength;
//
//		//��ַ����ж�
//		if(SwitchToAddress((pRxdFrameHead->FrameVar_68).byAddress)==FALSE)
//			return FM_ERR|1;
////			return FM_ERR|wFrameLength;
//
//		//У��͵��ж�
//		if( pBuf[wFrameLength-2] !=
//			  CheckSum((BYTE *)&(pRxdFrameHead->FrameVar_68.Control),pRxdFrameHead->FrameVar_68.byLength1) )
//		return FM_ERR|1;
////			return FM_ERR|wFrameLength;
//
//		//�����ַ����ж�
//		if( pBuf[wFrameLength-1] != SYN_STOP )
//			return FM_ERR|1;
////			return FM_ERR|wFrameLength;
//
//		//֡У����ȷ
//		return FM_OK|wFrameLength;
//		}
//
//	default:			   //����֡
//		return FM_ERR|1;
//	}


}


bool TxdUrgence()
{
//	if( SwitchClearDevFlag( SF_HaveBO )) 
//		return Txd68_BO_40();
//
//	//lgh--
//	if( SwitchClearDevFlag( SF_HaveDZ )) 
//		return Txd_DZ_Proc();

	//lgh

	return false;
}

//*******************************************************************************************//
//  ��������: Txd_OnTimer_Data
//  ��������: ʵ�ֶ�ʱ���ݵ��ٻ�����
//  �������:
//  ����ֵ:   TRUE,�ɹ�;
//           FALSE,ʧ��
//*******************************************************************************************//
bool Txd_OnTimer_Data(int DevNo)
{
	if(time_Flag)
	{
		Txd68_SST_6(DevNo);
		time_Flag = false;
		return true;
	}
	return false;
	
}

//*******************************************************************************************//
//  ��������: Txd10_BaseFrame
//  ��������: ʵ�̶ֹ�֡��֡�ķ��ʹ���
//  �������: BYTE byFC,������
//            BYTE byFCV,FCVλ
//  ����ֵ:   TRUE,�ɹ�;
//            FALSE,ʧ��
//*******************************************************************************************//
bool Txd10_BaseFrame(int DevNo, BYTE byFC,BYTE byFCV)
{
	m_Txd.ReadPtr=0;

	//FCB�Ĵ���,���FCV��Ч,��FCBȡ��
	ProcessFCB(byFCV);

	pTxdFrameHead->FrameFix_10.byStart=SYN_START_10;
	pTxdFrameHead->FrameFix_10.Control.RESERVED=0;
	pTxdFrameHead->FrameFix_10.Control.PRM=PRM_MASTER;
	pTxdFrameHead->FrameFix_10.Control.FCV=byFCV;
	pTxdFrameHead->FrameFix_10.Control.FCB=gpDevice[DevNo].fcb&0x01;
	pTxdFrameHead->FrameFix_10.Control.FC=byFC;
	pTxdFrameHead->FrameFix_10.byAddress=gpDevice[DevNo].Address;
	pTxdFrameHead->FrameFix_10.byChecksum=CheckSum((BYTE *)&(pTxdFrameHead->FrameFix_10.Control),2);
	pTxdFrameHead->FrameFix_10.byStop=SYN_STOP;

	m_Txd.WritePtr=sizeof(TTxdFrameFix);

//	WriteToComm(pTxdFrameHead->FrameFix_10.byAddress,0);

	return TRUE;
}
//*******************************************************************************************//
//  ��������: Txd68_SST_6
//  ��������: ʵ�ֶ�ʱ���ĵĴ���(��Ҫ��Ӧ��,��Ϊ�����ӱ�����֧�ַǹ㲥��ʽ)
//  �������:
//  ����ֵ:   TRUE,�ɹ�;
//            FALSE,ʧ��
//*******************************************************************************************//
bool Txd68_SST_6(int DevNo)
{
	BYTE byTYP,byVSQ,byCOT,byADDRESS,byFUN,byINF;
	TSysTime SysTime;
	WORD wMSecond;

	Txd68_Head(DevNo, FC_M_BROADCAST,FCV_INVALID);

	pTxdFrameHead->FrameVar_68.byAddress=MASTER_BROADCAST_ADDRESS;

	byTYP=TYP_M_SST;
	byVSQ=0x81;
	byCOT=COT_M_SST;
	byADDRESS=pTxdFrameHead->FrameVar_68.byAddress;
	byFUN=FUN_GLB;
	byINF=0;

	SetAsduHead(byTYP,byVSQ,byCOT,byADDRESS,byFUN,byINF);

//	::ReadSysTime(&SysTime);
	wMSecond=SysTime.MSecond;

	m_Txd.Buf[m_Txd.WritePtr++]=LOBYTE(wMSecond);
	m_Txd.Buf[m_Txd.WritePtr++]=HIBYTE(wMSecond);
	m_Txd.Buf[m_Txd.WritePtr++]=SysTime.Minute;
	m_Txd.Buf[m_Txd.WritePtr++]=SysTime.Hour;
	m_Txd.Buf[m_Txd.WritePtr++]=SysTime.Day;
	m_Txd.Buf[m_Txd.WritePtr++]=SysTime.Month;

	//��ʱ��Ϊ����λ��
	m_Txd.Buf[m_Txd.WritePtr++]=(BYTE)(SysTime.Year % 100);

	Txd68_Tail(DevNo);

	return TRUE;
}

//*******************************************************************************************//
//  ��������: Txd68_Call_7
//  ��������: ʵ�����ٻ��Ĵ���
//  �������:
//  ����ֵ:   TRUE,�ɹ�;
//            FALSE,ʧ��
//*******************************************************************************************//
bool Txd68_Call_7(int DevNo)
{
	BYTE byTYP,byVSQ,byCOT,byADDRESS,byFUN,byINF;

	Txd68_Head(DevNo, FC_M_DATA,FCV_VALID);

	byTYP=TYP_M_CALL;
	byVSQ=0x81;
	byCOT=COT_M_CALL;
	byADDRESS=pTxdFrameHead->FrameVar_68.byAddress;
	byFUN=FUN_GLB;
	byINF=0;

	SetAsduHead(byTYP,byVSQ,byCOT,byADDRESS,byFUN,byINF);

	m_Txd.Buf[m_Txd.WritePtr++]=(pInfo[DevNo]->byScanNumber)++;

	Txd68_Tail(DevNo);

	return TRUE;
}

bool Txd68_Call_YC(int DevNo)
{
	BYTE byTYP,byVSQ,byCOT,byADDRESS,byFUN,byINF;

	Txd68_Head(DevNo, FC_M_DATA,FCV_VALID);

	byTYP=TYP_M_GCOMMAND;
	byVSQ=0x81;
	byCOT=COT_M_GREAD;
	byADDRESS=pTxdFrameHead->FrameVar_68.byAddress;
	byFUN=FUN_GEN;
	byINF=0xf1;

	SetAsduHead(byTYP,byVSQ,byCOT,byADDRESS,byFUN,byINF);

//	m_Txd.Buf[m_Txd.WritePtr++]=(pInfo[DevNo]->byScanNumber)++;
	
	m_Txd.Buf[m_Txd.WritePtr++] = 0x01;
	m_Txd.Buf[m_Txd.WritePtr++] = 0x01;
	m_Txd.Buf[m_Txd.WritePtr++] = 0x01;
	m_Txd.Buf[m_Txd.WritePtr++] = 0x01;
	
	Txd68_Tail(DevNo);

	return TRUE;
}

static int CheckClearDevFlag()
{
	return FALSE;
}


//*******************************************************************************************//
//  ��������: Txd_General_Data
//  ��������: ʵ��ͨ�÷������ݵķ��ʹ���
//  �������:
//  ����ֵ:   TRUE,�ɹ�;
//            FALSE,ʧ��
//*******************************************************************************************//
bool Txd_General_Data(int DevNo)
{
	//��ͨ�÷������ݵ��ܲ�ѯ
	if( CheckClearDevFlag(SF_GEN_CALLALL) )    
	{ 
		Txd68_GCommand_21(DevNo, INF_M_GEN_CALLALL);  
		return TRUE; 
	}

	//�����б�������ı���
	if( CheckClearDevFlag(SF_GEN_RGROUPITEM) )    
	{ 
		Txd68_GCommand_21(DevNo, INF_M_GEN_RGROUPITEM);  
		return TRUE; 
	}

	//��һ�����ȫ����Ŀ��ֵ������
	if( CheckClearDevFlag(SF_GEN_RGROUPVALUE) )    
	{ 
		Txd68_GCommand_21(DevNo, INF_M_GEN_RGROUPVALUE);  
		return TRUE; 
	}

	//��������Ŀ��Ŀ¼
	if( CheckClearDevFlag(SF_GEN_RENTRYITEM) )
	{ 
		Txd68_GCommand_21(DevNo, INF_M_GEN_RENTRYITEM);  
		return TRUE; 
	}

	//��������Ŀ��ֵ������
	if( CheckClearDevFlag(SF_GEN_RENTRYVALUE) )    
	{ 
		Txd68_GCommand_21(DevNo, INF_M_GEN_RENTRYVALUE);  
		return TRUE; 
	}

	//д��Ŀ
	if( CheckClearDevFlag(SF_GEN_WENTRY) )
	{ 
		Txd68_GData_10(DevNo, INF_M_GEN_WENTRY);		
		return TRUE; 
	}

	//��ȷ�ϵ�д��Ŀ
	if( CheckClearDevFlag(SF_GEN_WENTRYACK) )       
	{ 
		Txd68_GData_10(DevNo, INF_M_GEN_WENTRYACK);		
		return TRUE; 
	}

	//��ִ�е�д��Ŀ
	if( CheckClearDevFlag(SF_GEN_WENTRYEXEC) )
                
	{ 
		Txd68_GData_10(DevNo, INF_M_GEN_WENTRYEXEC);		
		return TRUE; 
  }

	//д��Ŀ��ֹ
	if( CheckClearDevFlag(SF_GEN_WENTRYCANCEL) )       
	{ 
		Txd68_GData_10(DevNo, INF_M_GEN_WENTRYCANCEL);		
		return TRUE; 
	}

	return FALSE;
}

//*******************************************************************************************//
//  ��������: Txd68_GCommand_21
//  ��������: ʵ��ͨ�÷�������Ĵ���
//  �������: BYTE byINF,��Ϣ���
//  ����ֵ:   TRUE,�ɹ�;
//            FALSE,ʧ��
//*******************************************************************************************//
bool Txd68_GCommand_21(int DevNo, BYTE byINF)
{
	BYTE byTYP,byVSQ,byCOT,byADDRESS,byFUN;

	Txd68_Head(DevNo, FC_M_DATA,FCV_VALID);

	byTYP=TYP_M_GCOMMAND;
	byVSQ=0x81;

	if(byINF == INF_M_GEN_CALLALL)
		byCOT=COT_M_CALL;
	else
		byCOT=COT_M_GREAD;

	byADDRESS=pTxdFrameHead->FrameVar_68.byAddress;
	byFUN=FUN_GEN;

	SetAsduHead(byTYP,byVSQ,byCOT,byADDRESS,byFUN,byINF);

	m_Txd.Buf[m_Txd.WritePtr++]=(pInfo[DevNo]->byScanNumber)++;

	switch(byINF)
	{
	case INF_M_GEN_CALLALL:
	case INF_M_GEN_RGROUPITEM:
		m_Txd.Buf[m_Txd.WritePtr++]=0;
		m_Txd.Buf[m_Txd.WritePtr++]=0;
		break;

	case INF_M_GEN_RGROUPVALUE:
	case INF_M_GEN_RENTRYITEM:
	case INF_M_GEN_RENTRYVALUE:
		m_Txd.Buf[m_Txd.WritePtr++]=1;
		m_Txd.Buf[m_Txd.WritePtr++]=1;
		m_Txd.Buf[m_Txd.WritePtr++]=1;
		m_Txd.Buf[m_Txd.WritePtr++]=1;
		//m_Txd.Buf[m_Txd.WritePtr++]=1;
		break;

	default:
		break;
	}

	Txd68_Tail(DevNo);

	return TRUE;
}

//*******************************************************************************************//
//  ��������: Txd68_GData_10
//  ��������: ʵ��ͨ�÷������ݵĴ���
//  �������: BYTE byINF,��Ϣ���
//  ����ֵ:   TRUE,�ɹ�;
//            FALSE,ʧ��
//*******************************************************************************************//
bool Txd68_GData_10(int DevNo, BYTE byINF)
{
	BYTE byTYP,byVSQ,byCOT,byADDRESS,byFUN;

	Txd68_Head(DevNo, FC_M_DATA,FCV_VALID);

	//byTYP=TYP_M_CALL;
	byTYP=TYP_M_GDATA;
	byVSQ=0x81;
	//byCOT=COT_M_CALL;
	byCOT=COT_S_GWACK;
	byADDRESS=pTxdFrameHead->FrameVar_68.byAddress;
	byFUN=FUN_GLB;

	SetAsduHead(byTYP,byVSQ,byCOT,byADDRESS,byFUN,byINF);

	m_Txd.Buf[m_Txd.WritePtr++]=(pInfo[DevNo]->byScanNumber)++;

	Txd68_Tail(DevNo);

	return TRUE;
}

//*******************************************************************************************//
//  ��������: Txd68_Head
//  ��������: ʵ�ֿɱ�֡��֡ͷ�Ĵ���
//  �������: BYTE byFC,������
//            BYTE byFCV,FCVλ
//  ����ֵ:   TRUE,�ɹ�;
//            FALSE,ʧ��
//*******************************************************************************************//
bool Txd68_Head(int DevNo, BYTE byFC,BYTE byFCV)
{
	m_Txd.ReadPtr=0;

	//FCB�Ĵ���,���FCV��Ч,��FCBȡ��
	ProcessFCB(byFCV);
		
	pTxdFrameHead->FrameVar_68.byStart1=SYN_START_68;
	pTxdFrameHead->FrameVar_68.byStart2=SYN_START_68;
	pTxdFrameHead->FrameVar_68.Control.RESERVED=0;
	pTxdFrameHead->FrameVar_68.Control.PRM=PRM_MASTER;
	pTxdFrameHead->FrameVar_68.Control.FCV=byFCV;
	pTxdFrameHead->FrameVar_68.Control.FCB=gpDevice[DevNo].fcb&0x1;
	pTxdFrameHead->FrameVar_68.Control.FC=byFC;
	pTxdFrameHead->FrameVar_68.byAddress=gpDevice[DevNo].Address;

	m_Txd.WritePtr=(uint8_t *)&(pTxdFrameHead->FrameVar_68.Asdu)-(uint8_t *)&(pTxdFrameHead->FrameVar_68.byStart1);

	return TRUE;
}

//*******************************************************************************************//
//  ��������: Txd68_Tail
//  ��������: ʵ�ֿɱ�֡��֡β�Ĵ���
//  �������:
//  ����ֵ:   TRUE,�ɹ�;
//            FALSE,ʧ��
//*******************************************************************************************//
bool Txd68_Tail(int DevNo)
{
	pTxdFrameHead->FrameVar_68.byLength1=m_Txd.WritePtr-4;
	pTxdFrameHead->FrameVar_68.byLength2=m_Txd.WritePtr-4;

	m_Txd.Buf[m_Txd.WritePtr++]=CheckSum((BYTE *)&(pTxdFrameHead->FrameVar_68.Control),pTxdFrameHead->FrameVar_68.byLength1);
	m_Txd.Buf[m_Txd.WritePtr++]=SYN_STOP;
	

	XJ103_send(DevNo, m_Txd.Buf, m_Txd.WritePtr);

	return TRUE;
}

//*******************************************************************************************//
//  ��������: SetAsduHead
//  ��������: ����Ӧ�÷������ݵ�Ԫ�ı�ʶ��Ϣͷ
//  �������:
//  ����ֵ:   TRUE,�ɹ�;
//            FALSE,ʧ��
//*******************************************************************************************//
bool SetAsduHead(BYTE byTYP,BYTE byVSQ,BYTE byCOT,BYTE byADDRESS,BYTE byFUN,BYTE byINF)
{
	pTxdFrameHead->FrameVar_68.Asdu.byTYP=byTYP;
	pTxdFrameHead->FrameVar_68.Asdu.byVSQ=byVSQ;
	pTxdFrameHead->FrameVar_68.Asdu.byCOT=byCOT;
	pTxdFrameHead->FrameVar_68.Asdu.byADDRESS=byADDRESS;
	pTxdFrameHead->FrameVar_68.Asdu.byFUN=byFUN;
	pTxdFrameHead->FrameVar_68.Asdu.byINF=byINF;

	m_Txd.WritePtr+=6;

	return TRUE;
}



//*******************************************************************************************//
//  ��������: Txd68_FuGui_14(void);
//  ��������: ���͸��鱨��
//  �������: 
//  ����ֵ:   ��
//*******************************************************************************************//
bool Txd68_FuGui_14(int DevNo)
{
	BYTE byTYP,byVSQ,byCOT,byADDRESS,byFUN,byINF;
	
	//Txd68_Head(DevNo, FC_M_DATA,FCV_VALID);

        Txd68_Head(DevNo, FC_M_BROADCAST,FCV_INVALID);

	pTxdFrameHead->FrameVar_68.byAddress=MASTER_BROADCAST_ADDRESS;

	byTYP=TYP_M_FUGUI;
	byVSQ=0x81;
	byCOT=COT_M_COMMAND;
	byADDRESS=pTxdFrameHead->FrameVar_68.byAddress;
	byFUN=0xC6;//198fun 
	byINF=0x13;//19inf 	

	SetAsduHead(byTYP,byVSQ,byCOT,byADDRESS,byFUN,byINF);
	
	m_Txd.Buf[m_Txd.WritePtr++]=0x10;
	m_Txd.Buf[m_Txd.WritePtr++]=0x00;//������Ϣ��ʶ�����������

	Txd68_Tail(DevNo);

	return TRUE;

}

//*******************************************************************************************//
//  ��������: Txd68_BO_40
//  ��������: ����ң�ش���
//  �������: 
//  ����ֵ:   ��
//*******************************************************************************************//
bool Txd68_BO_40()
{
	BYTE bAttrib,bCommand,bType,bGinh;
	DWORD dwBoNo;
	
//	TBO* pBO;
//	pBO = pGetBO();
//
//	dwBoNo =pBO->dwNo;
//
//
//		switch(pBO->dwCommand)
//		{
//		case TP_YKSELECT:
//			bCommand = 249;
//			break;
//		case TP_YKOPERATE:
//			bCommand = 250;
//			break;
//		case TP_YKCANCEL:
//			bCommand = 251;
//			break;
//		default:
//			SetBOReturn(PBORET_FAIL);
//			return false;
//		}
//	
//		switch(pBO->dwAttrib)					//�����ֺ�״̬
//		{
//		case TP_CLOSE: 
//			bAttrib = 0x02;
//			break;
//		case TP_OPEN:  
//			bAttrib = 0x01; 
//			break;	
//		default:
//			SetBOReturn(PBORET_FAIL);
//			return FALSE;
//		}
//	
//		if(dwBoNo>0 && dwBoNo<=50)
//		{
//			bType = 1;			//ң�ؿ���
//		}
//		else if(dwBoNo>50 && dwBoNo<100)
//		{
//			bType = 2;			//ң��ѹ��
//		}
//		else if(dwBoNo == 100)
//		{
//			bType = 3;			//���齡
//		}
//		else
//			bType = 0;
//	
//		switch(bType)
//		{
//		case 1:
//			bGinh = 2;        //ң��
//			break;
//		case 2:
//			bGinh = 3;       //��ѹ��
//			break;
//		case 3:
//			bGinh = 9;      //����
//			break;
//		case 0:
//			return false;
//		}
//	
//		Txd68_Head(DevNo, FC_M_DATA,FCV_VALID);
//	
//		m_Txd.Buf[m_Txd.WritePtr++]=0x0a;
//		m_Txd.Buf[m_Txd.WritePtr++]=0x81;
//		m_Txd.Buf[m_Txd.WritePtr++]=0x28;
//		m_Txd.Buf[m_Txd.WritePtr++]=1;
//		m_Txd.Buf[m_Txd.WritePtr++]=0xFE;
//		m_Txd.Buf[m_Txd.WritePtr++]=bCommand;
//		m_Txd.Buf[m_Txd.WritePtr++]=0;
//		m_Txd.Buf[m_Txd.WritePtr++]=1;
//		m_Txd.Buf[m_Txd.WritePtr++]=bGinh;
//		m_Txd.Buf[m_Txd.WritePtr++]=(BYTE)dwBoNo;
//		m_Txd.Buf[m_Txd.WritePtr++]=0x01;
//		m_Txd.Buf[m_Txd.WritePtr++]=0x09;
//		m_Txd.Buf[m_Txd.WritePtr++]=0x01;
//		m_Txd.Buf[m_Txd.WritePtr++]=0x01;
//		m_Txd.Buf[m_Txd.WritePtr++]=bAttrib;
//	
//		Txd68_Tail(DevNo);
//	
//		bYKNo = (BYTE)dwBoNo;
//		bYKAttribte = bCommand;
//	
//		return true;

}

//lgh--
//*******************************************************************************************//
//  ��������: Txd_DZ_Proc
//  ��������: ��ֵ���ʹ���
//*******************************************************************************************//
bool Txd_DZ_Proc(void)
{
//        TDZ* pDZ;
//		
//	pDZ = pGetDZ();
//
//	switch(pDZ->dwCmd)
//	{
//	case TP_DZ_CALL_SEL:  //��ֵ�ٻ�Ԥ���·�
//		Txd68_DZ_41(DZ_SELECT);
////		SetDZReturn(PDZ_WAITOPERATE);
//		return true;
//	case TP_DZ_CALL_EXE:  //��ֵ�ٻ�ִ���·�
//		Txd68_DZ_41(DZ_OPERATE);
//		SetDZReturn(PDZ_OK);
//		return true;
//	case TP_DZ_HAVE_DATA:  //��ֵ�ٻ�һ������
//		Txd10_BaseFrame(DevNo, FC_M_CALLCLASS1,FCV_VALID);
////		taskDelay(50);
//		return true;
//	case TP_DZ_MODI_QH:  //��ֵ�޸������·�
//		Txd_Current_quhao();
//		return true;
//	case TP_DZ_MODI_SEL:  //��ֵ�޸�Ԥ���·�
//		Txd_ChgDZ_Select();
//		return true;
//	case TP_DZ_MODI_EXE:  //��ֵ�޸�ִ���·�
//		Txd_ChgDZ_Operate();
//		return true;
//	default:
//		SetDZReturn(PDZ_ERR);
//		return false;
//	}
	
}
//lgh


//====================================================================================
//
//  ��ֵ������غ���  2007.11.17     ����÷
//
//====================================================================================
//��ֵ�ٻ�
bool Txd68_DZ_41(int DevNo, BYTE bType)
{
	Txd68_Head(DevNo, FC_M_DATA,FCV_VALID);

	if(bType==DZ_SELECT)
	{
		m_Txd.Buf[m_Txd.WritePtr++]=0x0a;
		m_Txd.Buf[m_Txd.WritePtr++]=0x81;
		m_Txd.Buf[m_Txd.WritePtr++]=0x28;
		m_Txd.Buf[m_Txd.WritePtr++]=1;
		m_Txd.Buf[m_Txd.WritePtr++]=0xFE;
		m_Txd.Buf[m_Txd.WritePtr++]=0xFA;    //��ִ�е�д��Ŀ
		m_Txd.Buf[m_Txd.WritePtr++]=0;
		m_Txd.Buf[m_Txd.WritePtr++]=1;
		m_Txd.Buf[m_Txd.WritePtr++]=5;
		m_Txd.Buf[m_Txd.WritePtr++]=1;
		m_Txd.Buf[m_Txd.WritePtr++]=1;
		m_Txd.Buf[m_Txd.WritePtr++]=1;
		m_Txd.Buf[m_Txd.WritePtr++]=1;
		m_Txd.Buf[m_Txd.WritePtr++]=1;
		m_Txd.Buf[m_Txd.WritePtr++]=0;
		
	}
	else if(bType==DZ_OPERATE)
	{
		m_Txd.Buf[m_Txd.WritePtr++]=0x15;
		m_Txd.Buf[m_Txd.WritePtr++]=0x81;
		m_Txd.Buf[m_Txd.WritePtr++]=0x2a;
		m_Txd.Buf[m_Txd.WritePtr++]=0x01;
		m_Txd.Buf[m_Txd.WritePtr++]=0xfe;
		m_Txd.Buf[m_Txd.WritePtr++]=0xf1;
		m_Txd.Buf[m_Txd.WritePtr++]=0;
		m_Txd.Buf[m_Txd.WritePtr++]=0x01;
		m_Txd.Buf[m_Txd.WritePtr++]=0x04;
		m_Txd.Buf[m_Txd.WritePtr++]=0;
		m_Txd.Buf[m_Txd.WritePtr++]=1;
	}

	Txd68_Tail(DevNo);

	return true;
}

//��ֵ�޸Ĳ���
void Txd_Current_quhao(int DevNo)
{
	Txd68_Head(DevNo, FC_M_DATA,FCV_VALID);
	
	m_Txd.Buf[m_Txd.WritePtr++]=0x0a;
	m_Txd.Buf[m_Txd.WritePtr++]=0x81;
	m_Txd.Buf[m_Txd.WritePtr++]=0x28;
	m_Txd.Buf[m_Txd.WritePtr++]=1;
	m_Txd.Buf[m_Txd.WritePtr++]=0xFE;
	m_Txd.Buf[m_Txd.WritePtr++]=0xFA;
	m_Txd.Buf[m_Txd.WritePtr++]=0;
	m_Txd.Buf[m_Txd.WritePtr++]=1;
	m_Txd.Buf[m_Txd.WritePtr++]=5;
	m_Txd.Buf[m_Txd.WritePtr++]=1;
	m_Txd.Buf[m_Txd.WritePtr++]=1;
	m_Txd.Buf[m_Txd.WritePtr++]=1;
	m_Txd.Buf[m_Txd.WritePtr++]=1;
	m_Txd.Buf[m_Txd.WritePtr++]=1;
	m_Txd.Buf[m_Txd.WritePtr++]=0;
	Txd68_Tail(DevNo);
}

void Txd_ChgDZ_Select(int DevNo)
{
        //lgh--
//        TDZ* pDZ;
		
//	pDZ = pGetDZ();
        //lgh
        
	Txd68_Head(DevNo, FC_M_DATA,FCV_VALID);
	
	m_Txd.Buf[m_Txd.WritePtr++]=0x0a;
	m_Txd.Buf[m_Txd.WritePtr++]=0x81;
	m_Txd.Buf[m_Txd.WritePtr++]=0x28;
	m_Txd.Buf[m_Txd.WritePtr++]=1;
	m_Txd.Buf[m_Txd.WritePtr++]=0xFE;
	m_Txd.Buf[m_Txd.WritePtr++]=0xF9;
	m_Txd.Buf[m_Txd.WritePtr++]=0;
	m_Txd.Buf[m_Txd.WritePtr++]=1;
	m_Txd.Buf[m_Txd.WritePtr++]=4;
//	m_Txd.Buf[m_Txd.WritePtr++]=pDZ->dwNo+1;//lgh
	m_Txd.Buf[m_Txd.WritePtr++]=1;
	m_Txd.Buf[m_Txd.WritePtr++]=3;
	m_Txd.Buf[m_Txd.WritePtr++]=2;
	m_Txd.Buf[m_Txd.WritePtr++]=1;
//	m_Txd.Buf[m_Txd.WritePtr++]=LOBYTE(pDZ->wDzValue);//lgh
//	m_Txd.Buf[m_Txd.WritePtr++]=HIBYTE(pDZ->wDzValue);//lgh
	Txd68_Tail(DevNo);
}

void Txd_ChgDZ_Operate(int DevNo)
{
         //lgh--
//        TDZ* pDZ;
		
//	pDZ = pGetDZ();
        //lgh

	Txd68_Head(DevNo, FC_M_DATA,FCV_VALID);
	
	m_Txd.Buf[m_Txd.WritePtr++]=0x0a;
	m_Txd.Buf[m_Txd.WritePtr++]=0x81;
	m_Txd.Buf[m_Txd.WritePtr++]=0x28;
	m_Txd.Buf[m_Txd.WritePtr++]=1;
	m_Txd.Buf[m_Txd.WritePtr++]=0xFE;
	m_Txd.Buf[m_Txd.WritePtr++]=0xFA;
	m_Txd.Buf[m_Txd.WritePtr++]=0;
	m_Txd.Buf[m_Txd.WritePtr++]=1;
	m_Txd.Buf[m_Txd.WritePtr++]=4;
//	m_Txd.Buf[m_Txd.WritePtr++]=pDZ->dwNo+1;//lgh
	m_Txd.Buf[m_Txd.WritePtr++]=1;
	m_Txd.Buf[m_Txd.WritePtr++]=3;
	m_Txd.Buf[m_Txd.WritePtr++]=2;
	m_Txd.Buf[m_Txd.WritePtr++]=1;
//	m_Txd.Buf[m_Txd.WritePtr++]=LOBYTE(pDZ->wDzValue);//lgh
//	m_Txd.Buf[m_Txd.WritePtr++]=HIBYTE(pDZ->wDzValue);//lgh
	Txd68_Tail(DevNo);
}




