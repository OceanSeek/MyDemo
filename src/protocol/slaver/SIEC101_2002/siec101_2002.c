//#ifdef IEC1012002

#include "sys.h"


//=========================================================================
// ����: S101.cpp
// ����: ����S101��Լ�����̿���
//=========================================================================

#define TESTMODE 1

extern TVars     gVars;


static TCommIO m_Txd;				  //���ͻ������Ĵ���ṹ

static TIEC101Cfg *m_pCfg;
static WORD m_wMinFrmLen;
static BYTE m_bSendMirrorFlag;
static BYTE m_bMirrorBuf[300];
static WORD m_wSelfInit;
static TIEC101Cfg *m_pCfg;
static WORD m_wDelayTime;
static WORD m_wFromTime;
TSIEC101_2002RxdFm *pRxdFm; //����֡ͷָ��
TSIEC101_2002TxdFm *pTxdFm;//����֡ͷָ��
static uint8_t	GX101Slaver_ReSendbuf[255];


void CheckCfgFile(int DevID);
void DefaultCfgFile();
void ReadCfgFile(int DevID);
BOOL TxdYkExeInfo(BYTE bReason); 
BOOL TxdCallGroupCIData(int DevID, WORD wGroup);
BOOL CheckAndRetry(int DevID, BYTE bControl);
BOOL CancelSelectFile(WORD wFileName);
void SetALLDataFlag(int DevID);
BOOL ChkVarControl(int DevID, BYTE bCode);
BOOL TxdCallDBIData(int DevID);
BOOL TxdCallBIData();
static BOOL ResetProcess(int DevID, uint8_t *pBuf);
static BOOL Txd_ChangeAI(int DevID);
static BOOL Txd_ChangeAI13(int DevID);
static BOOL Txd_ChangeAI34(int DevID);
static BOOL Txd_ChangeAI36(int DevID);
static BOOL TxdClockSyn();
static BOOL TxdMirror(int DevID);
static void MakeMirror(BYTE bReason);
static BOOL TxdTimeDelay();
static BYTE GetCtrCode(int DevID, BYTE bPRM, BYTE bCode);
static BOOL Txd_AllStop(int DevID);
static BOOL RxdDelay(int DevID, uint8_t *pBuf);
static BOOL RxdClockSyn(int DevID, uint8_t *pBuf);//ʱ��ͬ��
static BOOL RxdCallAll(int DevID);//���ٻ�/�ٻ�ĳһ������
static int RxdMonitor(int DevID, uint8_t *buf, uint16_t len);
BOOL RxdReqLinkStatus(int DevID);
static int16_t ReadAI32(uint32_t DevID,uint32_t dwAINo, IEC10X_DETECT_F_T* pValue);
BOOL RxdNoAsdu();
BOOL RxdWriteFileData(int DevID);//�����ļ�����дӦ��
BOOL RxdReadRequestFile(int DevID);
BOOL RxdCallDirFile(int DevID, uint8_t *pBuf);
BOOL RxdReadFileData(int DevID);
BOOL RxdWriteRequestFile(int DevID);
BOOL RxdSetPara3(int DevID, uint8_t *pBuf);//���ò��� �̸�����
BOOL RxdSetPara2(int DevID, uint8_t *pBuf);//���ò��� scale
BOOL RxdSetPara(int DevID, uint8_t *pBuf);//���ò���normalize value
BOOL RxdTestLink(int DevID);//������·
BOOL RxdCallCI(int DevID, uint8_t *pBuf);
BOOL RxdDOCmd(int DevID, uint8_t *pBuf);
BOOL RxdYtCmd(int DevID, uint8_t *pBuf);
BOOL RxdYkCmd(int DevID, uint8_t *pBuf);//ң�ش���
BOOL RxdVarFrame(int DevID, uint8_t *buf);
BOOL RxdFixFrame(int DevID, BYTE* pBuf);
BOOL Txd_Protect40(int DevID, TSysTimeMr* pSysMr);
BOOL Txd_Protect38(int DevID, TSysTimeMr* pSysMr);
BOOL Txd_Protect39(int DevID, TSysTimeMr* pSysMr);
BOOL Txd_YcSoe34(int DevID);
BOOL Txd_YcSoe36(int DevID);
BOOL TxdScanAiData13(int DevID);
BOOL Txd_CP56TDSoe(int DevID);//����˫��SOE
BOOL Txd_CP24TDSoe(int DevID);//����˫��SOE
BOOL Txd_CP24TSoe(int DevID); //����SOE����
BOOL TxdCICP56Time2a(int DevID);
BOOL Txd_CP56TSoe(int DevID); //����SOE����
BOOL TxdCICP24Time2a(int DevID);
BOOL TxdCINoTimeData(int DevID);
BOOL TxdCallAIData13(int DevID);
BOOL TxdScanAiData(int DevID);
BOOL TxdFreezeCIData(int DevID);
BOOL TxdChangeSpiData(int DevID);
BOOL Txd_ProtectMr(int DevID);
BOOL Txd_YcSoe(int DevID);
BOOL Txd_Class1(int DevID); //����1������
BOOL Txd_Class2(int DevID); //����2������
BOOL Txd_Soe(int DevID);
BOOL Txd_DSoe(int DevID);
BOOL Txd_BurstDBI(int DevID);//����˫��仯ң��
BOOL ScanDirFile();


//=========================================================================
// ��������: ʵ��S101��Լ��Ӧ�ò���ȹ������� 
//=========================================================================
void SIEC101_2002_Task()
{
//	CSIEC101_2002 *pS101=new CSIEC101_2002();
//	
//	if(pS101 ==NULL)
//		Exit();
//
//	if(pS101->Init()!=TRUE)
//            {
//	       SetLight(0, 1);	
//               Exit("IEXS101 Init Error");
//            }
//    pS101->Run();
}

void CSIEC101_2002()
{
	m_bSendMirrorFlag = FALSE;
	m_wSelfInit = 0x55aa;
	m_wDelayTime = 0;
	m_wMinFrmLen = 5;
}

static uint8_t CK_send(int DevID,char *buf, int len){
    if(-1 == write(gpDevice[DevID].fd,buf,len)){
        perror("Send error \n");
		log("devid(%d)  fd(%d)\n",DevID, gpDevice[DevID].fd);
        return RET_ERROR;
    }

	if(buf[0] == 0x68){//��ر���
		gpDevice[DevID].ReSendOldTime = gpDevice[DevID].TimeCnt;
		gpDevice[DevID].ReSendDataLen = len;
		gpDevice[DevID].ReSendFlag = RESEND_YES;
	
		memcpy(GX101Slaver_ReSendbuf, buf, len);
	}
	MonitorTx(monitorData._TX_ID, DevID, monitorData._fd, buf, len);
	
    return RET_SUCESS;
}


//=========================================================================
// ��������: CS101::Init 
// ��������: ʵ��S101��Լ�ĳ�ʼ�� 
//=========================================================================
BOOL Init_siec101_2002(int DevID)
{

	
//	gpDevice[DevID].STATE_FLAG_INIT = GX101Slaver_FLAG_LINK_CLOSED;
//	gpDevice[DevID].STATE_FLAG_CALLALL = GX101Slaver_FLAG_LINK_CLOSED;
//	gpDevice[DevID].STATE_FLAG_GROUP = GX101Slaver_FLAG_LINK_CLOSED;
//	gpDevice[DevID].STATE_FLAG_DELAY = GX101Slaver_FLAG_LINK_CLOSED;
//	gpDevice[DevID].STATE_FLAG_YK = GX101Slaver_FLAG_LINK_CLOSED;

	gpDevice[DevID].Receive = RxdMonitor;
//	gpDevice[DevID].Task = GX101Slaver_Task;
//	gpDevice[DevID].OnTimeOut = GX101Slaver_On_Time_Out;

	TPMI PMI;

	m_pCfg = malloc(sizeof(TIEC101Cfg));
//	GetPMI(&PMI);//��ȡ��վ���Լ��Ϣ��
	
	PMI.dwGBAddress = 0xFF; //�㲥���ĵĵ�ַ

	PMI.dwMaxErrCount = 15;

	
	PMI.dwMaxAINum = 128*32; //ÿһģ�����AI����
	PMI.dwMaxBINum = 128*32;	//ÿһģ�����BI����
	PMI.dwMaxCINum = 511;	//ÿһģ�����CI����
	PMI.dwMaxBONum = 256;	//ÿһģ�����BO����
	PMI.dwMinLogicDevNum = 1; //����ģ����

	PMI.dwTxdBufSize = MAX_TXDFM_SIZE;           //���ͻ�������С
	PMI.dwRxdBufSize = MAX_RXDFM_SIZE;           //���ջ�������С

//	SetPMI(&PMI);//��ȡ��վ���Լ��Ϣ��

//	if(C_Slave::Init() != TRUE)
//		return FALSE;

	gVars.dwAddress=gpDevice[DevID].Address;		//��ַ
	

//	for(int dwDevNo=0; dwDevNo<m_dwDevNum; dwDevNo++)
//	{
//		SwitchToDevNo(dwDevNo);
//		SetAINum(DevID,PMI.dwMaxAINum);
//		SetBINum(DevID,PMI.dwMaxBINum);
//		SetCINum(DevID,PMI.dwMaxCINum);
//		SetBONum(DevID,PMI.dwMaxBONum);
//      	SetSQZ(DevID,10);	
//	}
	
	

//1. =========  ���øù�Լ����Ҫ����Ϣ���͡�=========
//	StartCommEvent(MCC_ReadComm, 5);  //��Ҫͨ������Ϣ
//	StartCommEvent(MCC_CommIdle,2*1000*10); //����CommIdle��Ϣ
//	StartCommEvent(MCC_CommIdle,2*1000); //����CommIdle��Ϣ
//	StartCommEvent(MCC_CommTxdIdle, 3|PM_BYTE); //����CommIdle��Ϣ

//	m_pRxdBuf = NULL;
//	m_pTxdBuf = NULL;

	pTxdFm = (TSIEC101_2002TxdFm *)m_Txd.Buf; 
	

	ReadCfgFile(DevID);

    m_pCfg->yc.wStartAddr=0x4001;      
    m_pCfg->spi.wStartAddr=0x01;
	m_pCfg->yk.wStartAddr =0x6001;
	m_pCfg->kwh.wStartAddr = 0x6401;

//	SetBIFlag(DevID, FALSE);
//	SetCIFlag(DevID, FALSE);
//	SetDBIFlag(DevID, FALSE);
//	SetAIFlag(DevID, FALSE);
//	SetBcdFlag(DevID, FALSE);

	ScanDirFile();

//	StartDbaseEvent(MCC_SOE);//ע����Ҫң�ű�λ��Ϣ
//	StartDbaseEvent(MCC_BurstBI);//ע����Ҫң�ű�λ��Ϣ

	m_Txd.Buf = malloc(256);

	gpDevice[DevID].ReadAI32 = ReadAI32;


	return TRUE;
}


static int16_t ReadAI32(uint32_t DevID,uint32_t dwAINo, IEC10X_DETECT_F_T* pValue)
{
	uint32_t RealDevID,RealPoint;
	if(gpDevice[DevID].pLogicBase == NULL)
		return -1;
	if(gpDevice[DevID].pLogicBase->pLogicAI == NULL)
		return -1;
	RealDevID = gpDevice[DevID].pLogicBase->pLogicAI[dwAINo].wRealID;
	RealPoint = gpDevice[DevID].pLogicBase->pLogicAI[dwAINo].wOffset;
	*pValue = gpDevice[RealDevID].pBurstAI[RealPoint].detect32_F;
}


static BYTE GetCtrCode(int DevID, BYTE bPRM, BYTE bCode)
{
//	BYTE bCodeTmp = 0x80;

	BYTE bCodeTmp = 0;
	
	bCodeTmp += bCode;
	
	if(bPRM)
		bCodeTmp|=0x40;

// 	if(SearchClass1(DevID))
// 		bCodeTmp |= 0x20;//��λң��Ϊ1������

	return bCodeTmp;
}



void ReadCfgFile(int DevID)
{
	char lzName[20];
	DWORD dwLen;
	int i;
	
//	::GetDevParaName(DevID, lzName);
//	strcat(lzName, ".cfg");

//	dwLen = File_Read(lzName, (BYTE*)m_pCfg, 0, sizeof(TIEC101Cfg));
	
//	i = sizeof(TIEC101Cfg);

//	if(dwLen != i)//��������
//	{
//		DefaultCfgFile();
//		return;
//	}
	
//	if(m_pCfg->wCheckSum != CheckSum(m_pCfg, sizeof(TIEC101Cfg)-2))
//	{
//		DefaultCfgFile();
//		return;
//	}

//	if(m_pCfg->wProtocolID != 169)
//	{
//		DefaultCfgFile();
//		return;
//	}
	
	DefaultCfgFile();
	CheckCfgFile(DevID);
}

void DefaultCfgFile()
{
	memset((BYTE*)m_pCfg, 0, sizeof(TIEC101Cfg));

	m_pCfg->bLinkAddr = 1;
	m_pCfg->bAsduAddr = 1;
	m_pCfg->bInfoAddr = 2;
	m_pCfg->bCauseTrans = 1;
	
	m_pCfg->bTestMode = 0;
	m_pCfg->bCounterMode = 0;
	m_pCfg->wCylicDataNum = 0;
	m_pCfg->wScanTime = 0;
	m_pCfg->wMaxFrm = 255;

	m_pCfg->spi.bStatus1 = 1;//����������
	m_pCfg->spi.bStatus2 = 30;
	m_pCfg->spi.bClass = 1;
	
	m_pCfg->dpi.bStatus1 = 3;
	m_pCfg->dpi.bStatus2 = 31;
	m_pCfg->dpi.bClass = 1;

	m_pCfg->step.bStatus1 = 5;
	m_pCfg->step.bStatus2 = 5;
	m_pCfg->step.bClass = 2;

	m_pCfg->bcd.bStatus1 = 7;
	m_pCfg->bcd.bStatus2 = 7;
	m_pCfg->bcd.bClass = 2;

	m_pCfg->yc.bStatus1 = 9;
	m_pCfg->yc.bStatus2 = 34;
	m_pCfg->yc.bClass = 2;

	m_pCfg->kwh.bStatus1 = 37;//���Ǳ���
	m_pCfg->kwh.bClass = 2;

	m_pCfg->yk.bClass = 1;
	m_pCfg->yt.bClass = 1;
	m_pCfg->bdo.bClass = 1;
	m_pCfg->tq.bClass = 1;

	m_pCfg->mr.bClass = 1;
	m_pCfg->file.bClass = 1;
	
}

void CheckCfgFile(int DevID)
{
	m_pCfg->bLinkAddr += 1;//��·��ַ 1 or 2
	m_pCfg->bAsduAddr += 1;//��ͬ��ַ1 or 2
	m_pCfg->bInfoAddr += 1;//��Ϣ���ַ 1 or 2 or 3
	m_pCfg->bCauseTrans += 1;//����ԭ��1 or 2
	
	if(m_pCfg->bLinkAddr == 2)//��·��ַ2
		m_wMinFrmLen = 6;
	else//��·��ַ 1
		m_wMinFrmLen = 5;

	if(m_pCfg->bLinkAddr > 2)
		m_pCfg->bLinkAddr = 1;
	if(m_pCfg->bAsduAddr > 2)
		m_pCfg->bAsduAddr = 1;
	if(m_pCfg->bInfoAddr > 3)
		m_pCfg->bInfoAddr = 2;
	if(m_pCfg->bCauseTrans > 2)
		m_pCfg->bCauseTrans = 1;

	if(m_pCfg->bTestMode != 0 && m_pCfg->bTestMode != 1)
		m_pCfg->bTestMode = 0;
	
	if(m_pCfg->bTestMode == 1)
		m_pCfg->bTestMode = 0x80;

	if(m_pCfg->bCounterMode >= 3)
		m_pCfg->bCounterMode = 0;//modeA
	
	if(m_pCfg->wScanTime == 0)
		m_pCfg->wCylicDataNum = 0;
	if(m_pCfg->wCylicDataNum == 0)
		m_pCfg->wScanTime = 0;
	
	if(m_pCfg->wMaxFrm > 255 || m_pCfg->wMaxFrm < 50)
		m_pCfg->wMaxFrm = 255;

	if(m_pCfg->wCylicDataNum > GetAINum(DevID))
		m_pCfg->wCylicDataNum = GetAINum(DevID);

	m_pCfg->spi.bStatus1 = 1;//����������
	
	switch(m_pCfg->spi.bStatus2)
	{
	case 0:
		m_pCfg->spi.bStatus2 = 2;
		break;
	case 1:
		m_pCfg->spi.bStatus2 = 30;
		break;
	default:
		m_pCfg->spi.bStatus2 = 30;//Ĭ�ϳ��Ǳ�
		break;
	}

	if(m_pCfg->spi.bClass == 0)//class 1
		m_pCfg->spi.bClass = 1;
	else
		m_pCfg->spi.bClass = 2;

	
	m_pCfg->dpi.bStatus1 = 3;
	
	switch(m_pCfg->dpi.bStatus2)
	{
	case 0:
		m_pCfg->dpi.bStatus2 = 4;
		break;
	case 1:
		m_pCfg->dpi.bStatus2 = 31;
		break;
	default:
		m_pCfg->dpi.bStatus2 = 31;
		break;
	}
	if(m_pCfg->dpi.bClass == 0)//class 1
		m_pCfg->dpi.bClass = 1;
	else
		m_pCfg->dpi.bClass = 2;

	switch(m_pCfg->step.bStatus1)
	{
	case 0:
		m_pCfg->step.bStatus1 = 5;
		break;
	case 1:
		m_pCfg->step.bStatus1 = 32;
		break;
	default:
		m_pCfg->step.bStatus1 = 5;
		break;
	}

	switch(m_pCfg->step.bStatus2)
	{
	case 0:
		m_pCfg->step.bStatus2 = 5;
		break;
	case 1:
		m_pCfg->step.bStatus2 = 6;
		break;
	case 2:
		m_pCfg->step.bStatus2 = 32;
		break;
	default:
		m_pCfg->step.bStatus2 = 5;
		break;
	}

	if(m_pCfg->step.bClass == 0)//class 1
		m_pCfg->step.bClass = 1;
	else
		m_pCfg->step.bClass = 2;

	switch(m_pCfg->bcd.bStatus1)
	{
	case 0:
		m_pCfg->bcd.bStatus1 = 7;
		break;
	case 1:
		m_pCfg->bcd.bStatus1 = 33;
		break;
	default:
		m_pCfg->bcd.bStatus1 = 7;
		break;
	}
	
	switch(m_pCfg->bcd.bStatus2)
	{
	case 0:
		m_pCfg->bcd.bStatus2 = 7;
		break;
	case 1:
		m_pCfg->bcd.bStatus2 = 8;
		break;
	case 2:
		m_pCfg->bcd.bStatus2 = 33;
		break;
	default:
		m_pCfg->bcd.bStatus2 = 7;
		break;
	}

	if(m_pCfg->bcd.bClass == 0)//class 1
		m_pCfg->bcd.bClass = 1;
	else
		m_pCfg->bcd.bClass = 2;


	switch(m_pCfg->yc.bStatus1)
	{
	case 0:
		m_pCfg->yc.bStatus1 = 9;
		break;
	case 1:
		m_pCfg->yc.bStatus1 = 11;
		break;
	case 2:
		m_pCfg->yc.bStatus1 = 13;
		break;
	case 3:
		m_pCfg->yc.bStatus1 = 21;
		break;
	case 4:
		m_pCfg->yc.bStatus1 = 34;
		break;
	case 5:
		m_pCfg->yc.bStatus1 = 36;
		break;
	default:
		m_pCfg->yc.bStatus1 = 9;
		break;
	}

	switch(m_pCfg->yc.bStatus2)
	{
	case 0:
		m_pCfg->yc.bStatus2 = 34;
		break;
	case 1:
		m_pCfg->yc.bStatus2 = 36;
		break;
	default:
		m_pCfg->yc.bStatus2 = 34;
		break;
	}

	if(m_pCfg->yc.bClass == 0)//class 1
		m_pCfg->yc.bClass = 1;
	else
		m_pCfg->yc.bClass = 2;


	switch(m_pCfg->kwh.bStatus1)
	{
	case 0:
		m_pCfg->kwh.bStatus1 = 15;
		break;
	case 1:
		m_pCfg->kwh.bStatus1 = 16;
		break;
	case 2:
		m_pCfg->kwh.bStatus1 = 37;
		break;
	default:
		m_pCfg->kwh.bStatus1 = 37;
		break;
	}

	if(m_pCfg->kwh.bClass == 0)//class 1
		m_pCfg->kwh.bClass = 1;
	else
		m_pCfg->kwh.bClass = 2;

	if(m_pCfg->yk.bClass == 0)//class 1
		m_pCfg->yk.bClass = 1;
	else
		m_pCfg->yk.bClass = 2;

	if(m_pCfg->yt.bClass == 0)//class 1
		m_pCfg->yt.bClass = 1;
	else
		m_pCfg->yt.bClass = 2;

	if(m_pCfg->bdo.bClass == 0)//class 1
		m_pCfg->bdo.bClass = 1;
	else
		m_pCfg->bdo.bClass = 2;

	if(m_pCfg->tq.bClass == 0)//class 1
		m_pCfg->tq.bClass = 1;
	else
		m_pCfg->tq.bClass = 2;

	m_pCfg->mr.bClass ++;
	if(m_pCfg->mr.bClass > 2)
		m_pCfg->mr.bClass = 1;

	m_pCfg->file.bClass = 1;//�ļ������ȼ���1
}


DWORD Standard_C_Float_To_Dword(DWORD C_Float,int DecimalPoint)
{
	DWORD TempA,Exp,MinusSign,i;
	double TempB;
	MinusSign=0;
	if(C_Float&0x80000000) MinusSign=1;//Ϊ����
	Exp=C_Float&0x7F800000;
	Exp>>=23;	//����E
	if(Exp<100)
	return 0;
	if(Exp>200)
	return 0x7FFFFFFF;
	C_Float&=0x7FFFFF;	//��ʱIU.UIΪβ��M
	C_Float+=0x800000;	//2��23�η�
	TempA=TempB=1;
	if(DecimalPoint>0)
	{
		for(i=0;i<DecimalPoint;i++)TempA*=10;
		if(Exp<150) 
		{
			for(i=0;i<(150-Exp);i++)TempB*=2; 
			C_Float=(double)C_Float*(double)TempA/TempB;
		}
		else
		{
			for(i=0;i<(Exp-150);i++)TempB*=2; 
			C_Float=(double)C_Float*(double)TempA*TempB;
		}
	}
	else
	{
		DecimalPoint=DecimalPoint^0xFFFFFFFF;
		DecimalPoint++;
		for(i=0;i<DecimalPoint;i++)TempA*=10;
		if(Exp<150) 
		{
			for(i=0;i<(150-Exp);i++)TempB*=2; 
			C_Float=(double)C_Float/(double)TempA/TempB;
		}
		else
		{
			for(i=0;i<(Exp-150);i++)TempB*=2; 
			C_Float=(double)C_Float*TempB/(double)TempA;
		}
	}
	if(MinusSign) //Ϊ����
	{
		C_Float^=0xFFFFFFFF;
		C_Float++;
	}
	return C_Float;
}

void ScanAiData(int DevID)
{
	int i;
	WORD wNum;
	int nStart;
	
	if(GetDevFlag(DevID, SF_HaveScanAiData))//ɨ�������û�з�����
		return;
	
	if(m_pCfg->wScanTime == 0)
		return ;
	if(m_pCfg->wCylicDataNum == 0)
		return;
	
	wNum = GetAINum(DevID);
	
	if(wNum < m_pCfg->wCylicDataNum)
		m_pCfg->wCylicDataNum = wNum;
	
	nStart = wNum - m_pCfg->wCylicDataNum;

	for(i=nStart; i<wNum; i++)
		SetAIFlag(DevID, i, 1);

	SetDevFlag(DevID, SF_HaveScanAiData);
}

BOOL ScanDirFile()
{
/*	TFDRTA* pFDRTA;
	TFDRTA* pFirst;
	FILE* fileID;
	TSysTime sysTime;
	POSITION1 pos;
	int i;
	WORD wFileName;
	char lzBuf[100];

	remove_f(pIEC101DirName);

	if(open_dir(pDataPath, &xdir))
		return FALSE;
		
	pFDRTA = new TFDRTA;
	pFirst = pFDRTA;

	memset(pFDRTA, 0, sizeof(TFDRTA));

	ReadSysTime(&sysTime);

	sprintf(pFDRTA->lzName, "iec101.txt");
	pFDRTA->wFileName = 1;
	pFDRTA->bSOF = 0;
	
	pFDRTA->wMSecond = (WORD)sysTime.Second*1000 + sysTime.MSecond;
	pFDRTA->bMinute = sysTime.Minute;
	pFDRTA->bHour = sysTime.Hour;
	pFDRTA->bDay = sysTime.Day;
	pFDRTA->bMonth = sysTime.Month;
	pFDRTA->wYear = sysTime.Year;

	m_dirList.AddTail(pFDRTA);
	
	wFileName = 2;
////62*n+57
	while(1)
	{
		if(read_dir(&xdir,&direntry))
			break;
		
		if(strcmp(direntry.d_name,".") == 0)
			continue;
		if(strcmp(direntry.d_name,"..") == 0)
			continue;

		strcpy(lzBuf, pDataPath);
		strcat(lzBuf, direntry.d_name);

		stat_f(lzBuf, &filestat);

		if((filestat.st_mode&_IFMT) == _IFDIR)
			continue;
		
		pFDRTA = new TFDRTA;
		
		m_dirList.AddTail(pFDRTA);		
		
		memset(pFDRTA->lzName, 0, sizeof(pFDRTA->lzName));

		for(i=0; i<strlen(direntry.d_name) && i<12; i++)
		{
			pFDRTA->lzName[i] = direntry.d_name[i];
		}
		
		pFDRTA->dwLen = filestat.st_size;
		pFDRTA->wFileName = wFileName;
		Unixtodos(filestat.st_mtime, &sysTime);
		
		pFDRTA->wMSecond = (WORD)sysTime.Second*1000 + sysTime.MSecond;
		pFDRTA->bMinute = sysTime.Minute;
		pFDRTA->bHour = sysTime.Hour;
		pFDRTA->bDay = sysTime.Day;
		pFDRTA->bMonth = sysTime.Month;
		pFDRTA->wYear = sysTime.Year;

		wFileName ++;
	}

	close_dir(&xdir);

	pFirst->dwLen = 62*m_dirList.GetCount() + 57;

	fileID = fopen(pIEC101DirName, "w+");
	
	if(fileID == NULL)
		return 0;
	
	pos = m_dirList.GetHeadPosition();
	
	sprintf(lzBuf, "FileID  ");//8
	fwrite(lzBuf, strlen(lzBuf), 1, fileID);
	
	sprintf(lzBuf, "   FileName    ");//15
	fwrite(lzBuf, strlen(lzBuf), 1, fileID);

	sprintf(lzBuf, "   FileLen     ");//15
	fwrite(lzBuf, strlen(lzBuf), 1, fileID);

	sprintf(lzBuf, "      Time     \r\n\r\n");//15
	fwrite(lzBuf, strlen(lzBuf), 1, fileID);

	while(pos)
	{
		pFDRTA = (TFDRTA*)m_dirList.GetNext(pos);
		WriteInfoToFile(fileID, pFDRTA);
	}

	fclose(fileID);
*/
	return TRUE;
}

void WriteInfoToFile(FILE* pFileID, TFDRTA* pTFDRTA)
{
/*	char lzBuf[50];
	char lzTmp[50];
	int i;

	if(pFileID == NULL || pTFDRTA == NULL)
		return;
	
	sprintf(lzBuf, "%d", pTFDRTA->wFileName);
	sprintf(lzTmp, "%s", "                ");
	strcat(lzBuf, lzTmp);
	fwrite(lzBuf, 8, 1, pFileID);

	strcpy(lzBuf, pTFDRTA->lzName);
	strcat(lzBuf, lzTmp);
	fwrite(lzBuf, 15, 1, pFileID);
	
	memset(lzBuf, 0, sizeof(lzBuf));
	memset(lzTmp, 0, sizeof(lzTmp));

	SizeToStr(pTFDRTA->dwLen, lzTmp);
	sprintf(lzBuf, "%20s", " ");
	
	i = 0;

	while(lzTmp[i] != 0)
	{
		lzBuf[15-strlen(lzTmp)+i] = lzTmp[i];
		i ++;
	}
//	strcat(lzBuf, lzTmp);
	fwrite(lzBuf, 20, 1, pFileID);

	sprintf(lzBuf, "%02d-%02d-%04d  %02d:%02d\r\n", 
		pTFDRTA->bMonth, pTFDRTA->bDay, pTFDRTA->wYear, pTFDRTA->bHour, pTFDRTA->bMinute);
	fwrite(lzBuf, strlen(lzBuf), 1, pFileID);
*/
}


void SizeToStr(DWORD dwSize, char* lzBuf)
{
	int i,k;
	DWORD j;
	char lzTmp[100];

	if(lzBuf == NULL)
		return;

	i = 0;
	memset(lzTmp, 0, sizeof(lzTmp));

	if(dwSize == 0)
		lzTmp[0]='0';

	for(k=0; dwSize > 0; )
	{
		j = dwSize%10;
		j += '0';
		dwSize/=10;

		lzTmp[i++]=(char)(j&0xFF);
		k++;
		if(k%3==0 && dwSize)
			lzTmp[i++]=',';
	}
	
	k = strlen(lzTmp);

	for(i = 0; i<k; i++)
		lzBuf[i] = lzTmp[k-i-1];

}

static bool CheckClearDevFlag(int DevID, DWORD dwFlagNo)//���___���
{
	if(!GetDevFlag(DevID, dwFlagNo))
		return FALSE;

	ClearDevFlag(DevID, dwFlagNo);
	return TRUE;
}


/*
const WORD Mr38Info[] = //�����¼�
{
	16,0
};
*/

const WORD Mr39Info[] = //�����¼�general,l1,l2,l3,n, 
{
	84,64,65,66,67,0
};

const WORD Mr40Info[] = //�����¼�GerneralTrip, TripL1, TripL2, TripL3
{
	68,69,70,71,0
};
//�������ٻ�

BOOL SearchClass1(int DevID) //�������1������
{
	if(GetDevFlag(DevID, SF_SendInitFlag))
		return TRUE;

	if(GetDevFlag(DevID, SF_CALLDIR_FLAG))
		return TRUE;
	if(GetDevFlag(DevID, SF_FILE_SELECT))
		return TRUE;

	if(GetDevFlag(DevID, SF_FILE_FLAG))//�ļ�״̬���ͱ��
		return TRUE;

	if(GetDevFlag(DevID, SF_HaveBOReturn)) //��ң����Ϣ
		return TRUE;
	if(GetDevFlag(DevID, SF_HaveAOReturn)) //��ң����Ϣ
		return TRUE;
	if(GetDevFlag(DevID, SF_HaveTQReturn)) //��ͬ����Ϣ
		return TRUE;
	if(GetDevFlag(DevID, SF_HaveDOReturn))//�������������־
		return TRUE;

//	if(GetDevFlag(DevID, SF_YkRtnInfoFlag))
//		return TRUE;
	if(GetDevFlag(DevID, SF_YkExitFlag))
		return TRUE;
	
//	if(GetDevFlag(DevID, SF_YtRtnInfoFlag))
//		return TRUE;
	if(GetDevFlag(DevID, SF_YtExitFlag))
		return TRUE;

	if(GetDevFlag(DevID, SF_MirrorFlag))//�о���
		return TRUE;

	if(GetDevFlag(DevID, SF_CallDelayFlag))
		return TRUE;
	if(GetDevFlag(DevID, SF_SetParaFlag))
		return TRUE;
	if(GetDevFlag(DevID, SF_ClockSynFlag))
		return TRUE;


	if(GetDevFlag(DevID, SF_CallCmdFlag))
		return TRUE;
	if(GetDevFlag(DevID, SF_ReadCIFlag))
		return TRUE;

	if(GetDevFlag(DevID, SF_CallData))
		return TRUE;
	
	if(GetDevFlag(DevID, SF_CALLCIFLAG0))
		return TRUE;
	if(GetDevFlag(DevID, SF_CALLCIFLAG0+1))
		return TRUE;
	if(GetDevFlag(DevID, SF_CALLCIFLAG0+2))
		return TRUE;
	if(GetDevFlag(DevID, SF_CALLCIFLAG0+3))
		return TRUE;
	
	if(m_pCfg->spi.bClass == 1)
	{
//		if(GetBurstBINum(DevID))
//			return TRUE;
//		if(GetSoeNum(DevID))
//			return TRUE;
	}

	if(m_pCfg->dpi.bClass == 1)
	{
//		if(GetBurstDBINum(DevID))
//			return TRUE;
//		if(GetDBISoeNum(DevID))
//			return TRUE;
	}


	if(m_pCfg->yc.bClass == 1)
	{
		if(GetBurstAINum(DevID))
			return TRUE;
	}

	if(m_pCfg->step.bClass == 1)
	{
//		if(GetBurstBcdNum(DevID))
//			return TRUE;
	}

	if(m_pCfg->yc.bClass == 1)
	{
//		if(GetAitNum(DevID))
//			return TRUE;
	}

	if(m_pCfg->kwh.bClass == 1)
	{
//		if(GetFreezeCINum(DevID))
//			return TRUE;
	}
	
	if(m_pCfg->mr.bClass == 1)
	{
//		if(GetMrNum(DevID))
//			return TRUE;
	}

	return FALSE;
}



//=======================================================================================
// ��������: �����ĵ�ͷ��
//=======================================================================================
static BOOL Txd_Head(int DevID, BYTE bType, BYTE bReason)
{
	BYTE* pBuf;
	WORD wAddr;
	BYTE bLen;

	pBuf = m_Txd.Buf;

	m_Txd.ReadPtr = 0;
	m_Txd.WritePtr = 0;

	bLen = 0;
	
	wAddr = gpDevice[DevID].Address;

	pBuf[0] = pBuf[3] = 0x68;

	bLen = 5;

	pBuf[bLen++] = LOBYTE(wAddr);

	if(m_pCfg->bLinkAddr == 2)
		pBuf[bLen++] = HIBYTE(wAddr);
	
	pBuf[bLen++] = bType;

	bLen ++;//bDefinitive
	
	pBuf[bLen++] = bReason|m_pCfg->bTestMode;

	if(m_pCfg->bCauseTrans == 2)
		pBuf[bLen++] = 0;
	
	pBuf[bLen++] = LOBYTE(wAddr);
	
	if(m_pCfg->bAsduAddr == 2)
		pBuf[bLen++] = HIBYTE(wAddr);
	
	m_Txd.WritePtr = bLen;

	return TRUE;
}


static BOOL Txd_Tail(int DevID, BYTE bPRM, BYTE bCode, BYTE bNum)
{
	BYTE bChkSum;
	BYTE* pBuf;
	
	pBuf = m_Txd.Buf;

	pBuf[1] = pBuf[2] = m_Txd.WritePtr-4;
	pBuf[4] = GetCtrCode(DevID, bPRM, bCode);

	if(m_pCfg->bLinkAddr == 2)
		pBuf[8] = bNum;
	else
		pBuf[7] = bNum;

	bChkSum = ChkSum(pBuf+4, pBuf[1]);

	pBuf[m_Txd.WritePtr++] = bChkSum;
	pBuf[m_Txd.WritePtr++] = 0x16;

//	WriteToComm(pBuf[5], 0);
	
//	pTxdFm = (TSIEC101_2002TxdFm *)m_Txd.Buf;

//	pTxdFm->varFm.bLength1 = pTxdFm->varFm.bLength2 = m_Txd.WritePtr-4;
//	pTxdFm->varFm.bControl = GetCtrCode(DevID, bPRM, bCode);
//	pTxdFm->varFm.bDefinitive = bNum;
	
//	bChkSum = ChkSum((BYTE*)&pTxdFm->varFm.bControl, pTxdFm->varFm.bLength1);

//	m_Txd.Buf[m_Txd.WritePtr++] = bChkSum;
//	m_Txd.Buf[m_Txd.WritePtr++] = 0x16;
	
//	WriteToComm(pTxdFm->varFm.bAddress1, 0);
	CK_send(DevID, m_Txd.Buf, m_Txd.WritePtr);
	return TRUE;
}


static BOOL TxdMirror(int DevID)
{
	BYTE* pBuf;
	BYTE bLen;
	BYTE bChkSum;
	
	ClearDevFlag(DevID, SF_MirrorFlag);

	bLen = m_bMirrorBuf[1] + 6;

	pBuf = m_Txd.Buf;
	
	memcpy(pBuf, m_bMirrorBuf, bLen);

	m_Txd.WritePtr = bLen;
	m_Txd.ReadPtr = 0;

//	bChkSum = ChkSum(pBuf+4, pBuf[1]);

	pBuf[pBuf[1]+4] = bChkSum;

//	WriteToComm(pBuf[5], 0);
	
	if(CheckClearDevFlag(DevID, SF_RESET_PROCESS))//resetrtu
	{
//		Sleep(5000);
//		SystemReset(0);
	}


	return TRUE;
}

BOOL TxdCIReadTermination(int DevID)
{
	BYTE bStyle;
	BYTE bReason = 10;
	BYTE bPRM = 0, bCode = 8;
	BYTE* pBuf;
	WORD wNo;
	
	bStyle = 0x65;
	pBuf = m_Txd.Buf;
	
	Txd_Head(DevID, bStyle, bReason);
	
	wNo = 0;

	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}

	pBuf[m_Txd.WritePtr++] = 0;//qcc
	
	Txd_Tail(DevID, bPRM, bCode, 1);
	
	return TRUE;
}


BOOL TxdCallDBIData(int DevID)
{
	BYTE bStyle = 3;
	BYTE bReason = 20;
	BYTE bPRM = 0, bCode = 8;
	WORD wBINum, wNo;
	int i;
	BYTE* pBuf;
	WORD wStatus;
	BYTE bStatus;
	WORD wSendFlag;
	BYTE bNum;
	int nStart;

	pBuf = m_Txd.Buf;

	wBINum = GetDBINum(DevID);
	
	for(i=0; i<wBINum; i++)
	{
		wSendFlag = GetDBIFlag(DevID, i);
		wSendFlag = (wSendFlag>>8);
		if(wSendFlag == 0xFF)//���ٻ����
			break;
	}
	
	if(i >= wBINum)
		return FALSE;
	
	nStart = i;
	wNo = nStart + m_pCfg->dpi.wStartAddr;
	
	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}

	bNum = 0;

	for(i = nStart; i<wBINum; i++)
	{
		wSendFlag = GetDBIFlag(DevID, i);
		wSendFlag = wSendFlag>>8;
		
		if(wSendFlag != 0xFF)
			break;
		
		SetDBIFlag(DevID, i, 0);

		bStatus = ReadDBI(DevID, i, &wStatus);
		bStatus &= 0x03;
		
		bStatus |= (wStatus&0xF0);

		pBuf[ m_Txd.WritePtr++ ] = bStatus;

		bNum ++;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN))
			break;
	}
	
	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum|0x80);

	return TRUE;
}


BOOL TxdCallBIData(int DevID)
{
	BYTE bStyle = 1;
	BYTE bReason = 20;
	BYTE bPRM = 0, bCode = 8;
	WORD wBINum, wNo;
	int i;
	BYTE* pBuf;
	WORD wStatus;
	BYTE bStatus;
	BYTE bTmp;
	WORD wSendFlag;
	BYTE bNum;
	int nStart;

	pBuf = m_Txd.Buf;

	wBINum = GetBINum(DevID);
	
	for(i=0; i<wBINum; i++)
	{
		wSendFlag = GetBIFlag(DevID, i);
		wSendFlag = wSendFlag>>8;
		if(wSendFlag == 0xFF)
			break;
	}
	
	if(i >= wBINum)
		return FALSE;
	
	nStart = i;
	wNo = nStart + m_pCfg->spi.wStartAddr;


	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}
	
	bNum = 0;

	for(i = nStart; i<wBINum; i++)
	{
		wSendFlag = GetBIFlag(DevID, i);
		wSendFlag = wSendFlag>>8;

		if(wSendFlag != 0xFF)
			break;;
		
		SetBIFlag(DevID, i, 0);

		bTmp = ReadBI(DevID, i, &wStatus);

		bStatus = (wStatus&0xFF);
		bStatus &= 0xF0;

		if(bTmp&PBI_ON)
			bStatus |= 1;

		pBuf[ m_Txd.WritePtr++ ] = bStatus;

		bNum ++;

//		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN))
      	if(m_Txd.WritePtr >= 139)    //127+12,cl20091230
			break;

	}
	
	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum|0x80);

	return TRUE;
}

BOOL TxdCallAIData(int DevID)
{
	BYTE bStyle;
	BYTE bReason = 20;
	BYTE bPRM = 0, bCode = 8;
	WORD wAINum, wNo;
	short sValue;
	int i;
	BYTE* pBuf;
	WORD wStatus;
	BYTE bNum;
	WORD wSendFlag;
	int nStart;
	
	bStyle = m_pCfg->yc.bStatus1;//9,11,13,21,34,36

	if(bStyle == 34)//nomalised with full time
		bStyle = 9;
	else if(bStyle == 36)//short float with full time 
		bStyle = 13;

	if(bStyle == 13)//float value
		return TxdCallAIData13(DevID);

	pBuf = m_Txd.Buf;

	wAINum = GetAINum(DevID);
		
	for(i=0; i<wAINum; i++)
	{
		wSendFlag = GetAIFlag(DevID, i);
		if((wSendFlag&0xFF00) == 0xFF00)
			break;
	}
	
	if(i>=wAINum)
		return FALSE;
	
	nStart = i;
//        m_pCfg->yc.wStartAddr=0x4001;   //���ɹ������ų�101�ϵ��ȵ�ң����Ϣ����ʼ��ַΪ0x4001������2002�汾��101��Լ
	wNo = i+m_pCfg->yc.wStartAddr;

	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}

	bNum = 0;
		
	for(i = nStart; i<wAINum; i++)
	{
		wSendFlag = GetAIFlag(DevID, i);

		if((wSendFlag&0xFF00) != 0xFF00)//���ٻ���־
			break;
		
		SetAIFlag(DevID, i, 0);//�뷢�ͱ�־
		sValue = ReadAI(DevID, i, &wStatus);

		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(sValue); //ң��ֵ Lo
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(sValue); //ң��ֵ Hi
		
		if(bStyle == 9 || bStyle == 11)
			pBuf[m_Txd.WritePtr++] = (wStatus&0xFF);
	
		bNum ++;

		if(bStyle == 9 || bStyle == 11)
		{
			if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-2))
				break;
		}
		else
		{
			if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-1))
				break;
		}
	}
	
	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum|0x80);
	
	return TRUE;
}


BOOL TxdCallAIData13(int DevID)
{
	BYTE bStyle;
	BYTE bReason = 20;
	BYTE bPRM = 0, bCode = 8;
	WORD wAINum, wNo;
	short sValue;
	int i;
	BYTE* pBuf;
	WORD wStatus;
	float* pFloat;
	DWORD dwValue;
	WORD wSendFlag;
	BYTE bNum;
	int nStart;

	bStyle = 13;//m_pCfg->yc.bStatus1; float value
	pBuf = m_Txd.Buf;

	wAINum = GetAINum(DevID);
	
	for(i=0; i<wAINum; i++)
	{
		wSendFlag = GetAIFlag(DevID, i);
		wSendFlag = wSendFlag>>8;

		if(wSendFlag == 0xFF)
			break;
	}
	
	if(i >= wAINum)
		return FALSE;
	
	nStart = i;
	wNo = i+m_pCfg->yc.wStartAddr;
	
	pFloat = (float*)&dwValue;

	Txd_Head(DevID, bStyle, bReason);
	
	bNum = 0;
//    m_pCfg->yc.wStartAddr=0x4001;   //���ɹ������ų�101�ϵ��ȵ�ң����Ϣ����ʼ��ַΪ0x4000������97�汾��101��Լ

	wNo = i+m_pCfg->yc.wStartAddr;

	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}

	for(i = nStart;  i<wAINum; i++)
	{
		wSendFlag = GetAIFlag(DevID, i);
		wSendFlag = wSendFlag>>8;

		if(wSendFlag != 0xFF)//���ٻ���־
			break;
		
		SetAIFlag(DevID, i, 0);//�뷢�ͱ��

		sValue = ReadAI(DevID, i, &wStatus);
		*pFloat = (float)sValue;
		
		pBuf[m_Txd.WritePtr++] = LLBYTE(dwValue); //ң��ֵ Lo
		pBuf[m_Txd.WritePtr++] = LHBYTE(dwValue); //ң��ֵ Hi
		pBuf[m_Txd.WritePtr++] = HLBYTE(dwValue); //ң��ֵ Lo
		pBuf[m_Txd.WritePtr++] = HHBYTE(dwValue); //ң��ֵ Hi
		
		pBuf[m_Txd.WritePtr++] = (wStatus&0xFF);

		bNum ++;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-4))
			break;
	}
	
	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum|0x80);
	
	return TRUE;
}

BOOL TxdCallCIData(int DevID)
{
	BYTE bStyle=15;
	BYTE bPRM = 0;
	DWORD dwValue;
	WORD wKwhNum;
	int i;
	BYTE bReason;
	WORD wNo;
	BYTE* pBuf;
	BYTE bCA;
	BYTE bNum;
	TFreezeCI freezeKwh;
	WORD wSendFlag;
	int nStart;

	bReason = 20;
	bNum = 0;

	wKwhNum = GetCINum(DevID);

	for(i=0; i<wKwhNum; i++)
	{
		wSendFlag = GetCIFlag(DevID, i);
		wSendFlag = wSendFlag>>8;

		if(wSendFlag == 0xFF)
			break;
	}

	if(i>=wKwhNum)
		return FALSE;
	
	nStart = i;

	pBuf = m_Txd.Buf;
	
	Txd_Head(DevID, bStyle, bReason);

	bNum = 0;

	for(i = nStart; i<wKwhNum && bNum<GRP_CINUM; i++)
	{
		wSendFlag = GetCIFlag(DevID, i);

		if((wSendFlag&0xFF00) != 0xFF00)
			continue;
		
		SetCIFlag(DevID, i, wSendFlag&0xFF);

		if(!ReadFreezeCI(DevID, i, &freezeKwh))
				continue;

		wNo = i + m_pCfg->kwh.wStartAddr;

//		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wNo); //��Ϣ���ַLo
//		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wNo); //��Ϣ���ַHi
		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
		
		if(GetFreezeType(DevID, i))
			bCA = 0x40;
		else
			bCA = 0;
		
		dwValue = freezeKwh.dwValue;
		
		
		pBuf[ m_Txd.WritePtr++ ] = LLBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = LHBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = HLBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = HHBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = ((i%GRP_CINUM)|bCA); //˳���

		bNum ++;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-4))
			break;
	}
	
	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, 8, bNum);//bcode = 8;

	return TRUE;

}

BOOL TxdCallGroupCIData(int DevID, WORD wGroup)
{
	BYTE bStyle=15;
	BYTE bPRM = 0;
	DWORD dwValue;
	WORD wKwhNum;
	int i;
	BYTE bReason;
	WORD wNo;
	BYTE* pBuf;
	BYTE bCA;
	BYTE bNum;
	TFreezeCI freezeKwh;
	int nStart;

	bReason = 37+wGroup;
	bNum = 0;

	wKwhNum = GetCINum(DevID);
	
	nStart = (wGroup-1)*GRP_CINUM;

	pBuf = m_Txd.Buf;
	
	Txd_Head(DevID, bStyle, bReason);

	bNum = 0;

	for(i = nStart; i<wKwhNum && bNum<GRP_CINUM; i++)
	{
		if(!ReadFreezeCI(DevID, i, &freezeKwh))
				continue;
		wNo = m_pCfg->kwh.wStartAddr+i;
		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
		
		if(GetFreezeType(DevID, i))
			bCA = 0x40;
		else
			bCA = 0;
		
		dwValue = freezeKwh.dwValue;
		
//		dwValue = ReadCI(DevID, wOffset+i);
		
		pBuf[ m_Txd.WritePtr++ ] = LLBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = LHBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = HLBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = HHBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = ((i%GRP_CINUM)|bCA); //˳���

		bNum ++;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-4))
			break;

	}
	
	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, 8, bNum);//bcode = 8;

	return TRUE;

}


BOOL TxdCallStepData(int DevID)
{
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	int i;
	WORD wNo;
	WORD wBcdNum;
	BYTE* pBuf;
	DWORD dwValue;
	WORD wStatus;
	WORD wSendFlag;
	int nStart;
	BYTE bNum;

	pBuf = m_Txd.Buf;
	
	wBcdNum = GetBCDNum(DevID);
	
	for(i=0; i<wBcdNum; i++)
	{
		wSendFlag = GetBcdFlag(DevID, i);
		if((wSendFlag&0xFF00) == 0xFF00)
			break;
	}
	
	if(i >= wBcdNum)
		return FALSE;

	nStart = i;
	wNo = nStart + m_pCfg->step.wStartAddr;

	Txd_Head(DevID, SPI_CMD_CODE, bReason);


	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}

	bNum = 0;	

	for(i = nStart; i<wBcdNum; i++)
	{
		wSendFlag = GetBcdFlag(DevID, i);
		if((wSendFlag&0xFF00) != 0xFF00)
			break;
		
		SetBcdFlag(DevID, i, 0);

		dwValue = ReadBCD(DevID, i, &wStatus);

		wStatus &= 0xF0;

		if(dwValue > 128)//���
			wStatus |= 0x01;


		pBuf[m_Txd.WritePtr++] = (dwValue&0x7F); 
		pBuf[m_Txd.WritePtr++] = (wStatus&0xFF);

		bNum ++;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-1))
			break;
	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum|0x80);
	
	return TRUE;
}


BOOL TxdCallBcdData(int DevID)
{
	BYTE bReason = 20;
	BYTE bPRM = 0;
	DWORD dwValue;
	WORD wBcdNum;
	int i;
	WORD wNo;
	BYTE* pBuf;
	WORD wStatus;
	WORD wSendFlag;
	BYTE bNum;
	int nStart;

	wBcdNum = GetBCDNum(DevID);
	
	for(i=0; i<wBcdNum; i++)
	{
		wSendFlag = GetBcdFlag(DevID, i);
		if((wSendFlag&0xFF00) == 0xFF00)
			break;
	}

	if(i>= wBcdNum)
		return FALSE;

	nStart = i;
	wNo = nStart + m_pCfg->bcd.wStartAddr;

	
	pBuf = m_Txd.Buf;

	Txd_Head(DevID, BCD_CMD_CODE, bReason);

	if(m_pCfg->bInfoAddr == 1)
		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[ m_Txd.WritePtr++ ] = 0;
	}
	else
	{
		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wNo); //��Ϣ���ַHi
	}

	bNum = 0;
	
	for(i = 0; i<wBcdNum; i++)
	{
		wSendFlag = GetBcdFlag(DevID, i);
		
		if((wSendFlag&0xFF00) != 0xFF00)
			break;

		dwValue = ReadBCD(DevID, i, &wStatus);
		
		pBuf[m_Txd.WritePtr++] = LLBYTE(dwValue); //
		pBuf[m_Txd.WritePtr++] = LHBYTE(dwValue); //
		pBuf[m_Txd.WritePtr++] = HLBYTE(dwValue); //
		pBuf[m_Txd.WritePtr++] = HHBYTE(dwValue); //
		pBuf[m_Txd.WritePtr++] = (wStatus&0xFF);

		bNum ++;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-4))
			break;

	}
	
	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, 8, bNum|0x80);

	return TRUE;
}

////�����Ǳ仯���ݵķ���

BOOL TxdCallAll(int DevID)
{
	if(GetDevFlag(DevID, SF_CallData) == FALSE)
		return FALSE;

	if(TxdCallBIData(DevID))
		return TRUE;

	if(TxdCallDBIData(DevID))
		return TRUE;

	if(TxdCallAIData(DevID))
		return TRUE;

	if(TxdCallCIData(DevID))
		return TRUE;

	if(TxdCallStepData(DevID))
		return TRUE;
	
	ClearDevFlag(DevID, SF_CallData);

	Txd_AllStop(DevID);

	return TRUE;
}


BOOL TxdFreezeCIData(int DevID)
{
	if(m_pCfg->kwh.bStatus1 == 15)
		return TxdCINoTimeData(DevID);
	else if(m_pCfg->kwh.bStatus1 == 16)
		return TxdCICP24Time2a(DevID);

	return TxdCICP56Time2a(DevID);
}


BOOL TxdCICP56Time2a(int DevID)
{
	BYTE bStyle=37;
	BYTE bPRM = 0;
	DWORD dwValue;
	WORD wKwhNum;
	int i;
	BYTE bReason;
	WORD wNo;
	BYTE* pBuf;
	TFreezeCI freezeKwh;
	TSysTime stTime;
	WORD wMSecond;
	BYTE bCA;
	BYTE bNum;
	
	wKwhNum = GetCINum(DevID);
	pBuf = m_Txd.Buf;
	
	if(m_pCfg->bCounterMode == 0)
		bReason = 3;
	else
		bReason = 7;

	bNum = 0;

	Txd_Head(DevID, bStyle, bReason);

	for(i = 0; i<wKwhNum && bNum<GRP_CINUM; i++)
	{
		if(!ReadFreezeCI(DevID, i, &freezeKwh))
				continue;
		
		wNo = i + m_pCfg->kwh.wStartAddr;

		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}

		if(GetFreezeType(DevID, i))
			bCA = 0x40;
		else
			bCA = 0;
		
		dwValue = freezeKwh.dwValue;
		stTime = freezeKwh.stTime;
		
		wMSecond = stTime.MSecond;

		pBuf[ m_Txd.WritePtr++ ] = LLBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = LHBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = HLBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = HHBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = ((i%GRP_CINUM)|bCA); //˳���

		pBuf[m_Txd.WritePtr++] = wMSecond;
		pBuf[m_Txd.WritePtr++] = (wMSecond>>8);
		pBuf[m_Txd.WritePtr++] = stTime.Minute;
		pBuf[m_Txd.WritePtr++] = stTime.Hour;
		pBuf[m_Txd.WritePtr++] = stTime.Day;
		pBuf[m_Txd.WritePtr++] = stTime.Month;
		pBuf[ m_Txd.WritePtr++ ] = stTime.Year%100;  //MSEC H

		bNum ++;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-11))
			break;

	}
	

	if(bNum == 0)
		return FALSE;
	
	Txd_Tail(DevID, bPRM, 8, bNum);//bCode=8;

	return TRUE;
}


BOOL TxdCICP24Time2a(int DevID)
{
	BYTE bStyle=16;
	BYTE bPRM = 0;
	DWORD dwValue;
	WORD wKwhNum;
	int i;
	BYTE bReason;
	WORD wNo;
	BYTE* pBuf;
	TFreezeCI freezeKwh;
	TSysTime stTime;
	WORD wMSecond;
	BYTE bCA;
	BYTE bNum;
	
	wKwhNum = GetCINum(DevID);
	pBuf = m_Txd.Buf;
	
	if(m_pCfg->bCounterMode == 0)
		bReason = 3;
	else
		bReason = 7;

	bNum = 0;

	Txd_Head(DevID, bStyle, bReason);

	for(i = 0; i<wKwhNum; i++)
	{
		if(!ReadFreezeCI(DevID, i, &freezeKwh))
				continue;
		

		wNo = i + m_pCfg->kwh.wStartAddr;

		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}

		if(GetFreezeType(DevID, i))
			bCA = 0x40;
		else
			bCA = 0;
		
		dwValue = freezeKwh.dwValue;
		stTime = freezeKwh.stTime;
		
		wMSecond = stTime.MSecond;

		pBuf[ m_Txd.WritePtr++ ] = LLBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = LHBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = HLBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = HHBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = ((i%GRP_CINUM)|bCA); //˳���

		pBuf[m_Txd.WritePtr++] = wMSecond;
		pBuf[m_Txd.WritePtr++] = (wMSecond>>8);
		pBuf[m_Txd.WritePtr++] = stTime.Minute;

		bNum ++;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-7))
			break;
	}
	

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, 8, bNum);//bCode=8;

	return TRUE;
}


BOOL TxdCINoTimeData(int DevID)
{
	BYTE bStyle=15;
	BYTE bPRM = 0;
	DWORD dwValue;
	WORD wKwhNum;
	int i;
	BYTE bReason;
	WORD wNo;
	BYTE* pBuf;
	BYTE bCA;
	BYTE bNum;
	TFreezeCI freezeKwh;

	wKwhNum = GetCINum(DevID);
	pBuf = m_Txd.Buf;

	
	if(m_pCfg->bCounterMode == 0)
		bReason = 3;
	else
		bReason = 7;

	bNum = 0;

	Txd_Head(DevID, bStyle, bReason);

	for(i = 0; bNum<GRP_CINUM && i<wKwhNum; i++)
	{
		if(GetCIFlag(DevID, i))
			continue;
		
		if(!ReadFreezeCI(DevID, i, &freezeKwh))
				continue;
		

		wNo = i + m_pCfg->kwh.wStartAddr;

		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
		
		if(GetFreezeType(DevID, i))
			bCA = 0x40;
		else
			bCA = 0;
		
		dwValue = freezeKwh.dwValue;
		
		
		pBuf[ m_Txd.WritePtr++ ] = LLBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = LHBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = HLBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = HHBYTE(dwValue); //
		pBuf[ m_Txd.WritePtr++ ] = ((i%GRP_CINUM)|bCA); //˳���

		bNum ++;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-4))
			break;
	}
	

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, 8, bNum);//bcode = 8;

	return TRUE;
}

/////////�����Ǳ仯����
BOOL Txd_Soe(int DevID)
{
	if(m_pCfg->spi.bStatus2 == 2)//short time
	{
		if(Txd_CP24TSoe(DevID))
			return TRUE;
	}
	else
	{
		if(Txd_CP56TSoe(DevID))
			return TRUE;
	}

	return FALSE;
}

BOOL Txd_DSoe(int DevID)
{
	if(m_pCfg->dpi.bStatus2 == 4)//short time
	{
		if(Txd_CP24TDSoe(DevID))
			return TRUE;
	}
	else
	{
		if(Txd_CP56TDSoe(DevID))
			return TRUE;
	}

	return FALSE;
}

BOOL Txd_BurstDBI(int DevID)//����˫��仯ң��
{
	BYTE bStyle = 3, bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum, bStatus;
	WORD wNo;
	TBurstDBI burstBI;
	BYTE* pBuf;
	int i;

	pBuf = m_Txd.Buf;

	Txd_Head(DevID, bStyle, bReason);
	bNum = 0;

	for(i=0; GetBurstDBINum(DevID); i++)
	{
		if( ReadBurstDBI(DevID, &burstBI) == FALSE)
			break;
		
		IncBurstDBIReadPtr(DevID);
		
		wNo = burstBI.nNo;
		bStatus = burstBI.bStatus;
		
		wNo += m_pCfg->dpi.wStartAddr;

		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
		
		pBuf[ m_Txd.WritePtr++ ] = bStatus;   //����ֵ Lo

		bNum ++;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr))
			break;
	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}

BOOL Txd_BurstBI(int DevID) //���ͱ仯ң��
{
	BYTE bStyle = 1, bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum, bStatus;
	WORD wNo;
	TBurstBI burstBI;
	BYTE* pBuf;
	int i;

	pBuf = m_Txd.Buf;

	Txd_Head(DevID, bStyle, bReason);
	
	bNum = 0;

	for(i = 0; GetBurstBINum(DevID); i++)
	{
		if( ReadBurstBI(DevID, &burstBI) == FALSE)
			break;
		
		IncBurstBIReadPtr(DevID);
		wNo = burstBI.nNo;
		bStatus = burstBI.bStatus;
		wNo += m_pCfg->spi.wStartAddr;


		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}

		pBuf[ m_Txd.WritePtr++ ] = bStatus?1:0;   //����ֵ Lo

		bNum++;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr))
			break;
	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}

BOOL TxdChangeSpiData32(int DevID)
{
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	WORD wNo;
	BYTE* pBuf;
	DWORD dwValue;
	BYTE bStatus;
	WORD wBurstBcdNum;
	TBurstBcd burstBcd;
	int i;
	WORD wMSecond;

	pBuf = m_Txd.Buf;
	
	wBurstBcdNum = GetBurstBcdNum(DevID);

	if(wBurstBcdNum == 0)
		return FALSE;
	
	bNum = 0;

	Txd_Head(DevID, 32, bReason);

	for(i = 0; i<wBurstBcdNum; i++)
	{
		if(!ReadBurstBcd(DevID, &burstBcd))
			continue;
		IncBCDReadPtr(DevID);
		
		bNum++;

		dwValue = burstBcd.dwValue;
		wNo = burstBcd.dwNo + m_pCfg->step.wStartAddr;
		bStatus = (BYTE)burstBcd.wStatus;
		bStatus &= 0xF0;

		if(dwValue > 128)//���
			bStatus |= 0x01;

		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}

		pBuf[m_Txd.WritePtr++] = (dwValue&0x7F); 
		pBuf[m_Txd.WritePtr++] = bStatus;


		wMSecond = burstBcd.stTime.MSecond;

		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wMSecond);  //MSEC L
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wMSecond);  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = burstBcd.stTime.Minute;  //MSEC H
		
//long time for DF8002		
		pBuf[ m_Txd.WritePtr++ ] = burstBcd.stTime.Hour;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = burstBcd.stTime.Day;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = burstBcd.stTime.Month;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = burstBcd.stTime.Year%100;  //MSEC H
		
		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-8))
			break;

	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);
	
	return TRUE;
}

BOOL TxdChangeSpiData(int DevID)
{
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	WORD wNo;
	BYTE* pBuf;
	DWORD dwValue;
	BYTE bStatus;
	WORD wBurstBcdNum;
	TBurstBcd burstBcd;
	int i;

	if(m_pCfg->step.bStatus1 == 32)
		return TxdChangeSpiData32(DevID);
	
	pBuf = m_Txd.Buf;
	
	wBurstBcdNum = GetBurstBcdNum(DevID);

	if(wBurstBcdNum == 0)
		return FALSE;
	
	bNum = 0;

	Txd_Head(DevID, SPI_CMD_CODE, bReason);

	for(i = 0; i<wBurstBcdNum; i++)
	{
		if(!ReadBurstBcd(DevID, &burstBcd))
			continue;
		IncBCDReadPtr(DevID);
		bNum++;

		dwValue = burstBcd.dwValue;
		wNo = burstBcd.dwNo + m_pCfg->step.wStartAddr;
		bStatus = (BYTE)burstBcd.wStatus;
		bStatus &= 0xF0;

		if(dwValue > 128)//���
			bStatus |= 0x01;

		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}

		pBuf[m_Txd.WritePtr++] = (dwValue&0x7F); 
		pBuf[m_Txd.WritePtr++] = bStatus;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-1))
			break;

	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);
	
	return TRUE;
}

BOOL TxdChangeBcdData(int DevID)
{
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	WORD wNo;
	WORD wBcdNum;
	BYTE* pBuf;
	DWORD dwValue;
	WORD wStatus;

	pBuf = m_Txd.Buf;
	
	wBcdNum = GetBCDNum(DevID);

	Txd_Head(DevID, BCD_CMD_CODE, bReason);

	for(bNum = 0; bNum<wBcdNum; bNum++) 
	{
//		dwValue = ReadSaveBcd(DevID, bNum, &wStatus);
	
		wNo = bNum + m_pCfg->bcd.wStartAddr;

		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}

		pBuf[m_Txd.WritePtr++] = dwValue; 
		pBuf[m_Txd.WritePtr++] = (dwValue>>8);
		pBuf[m_Txd.WritePtr++] = (dwValue>>16);
		pBuf[m_Txd.WritePtr++] = (dwValue>>24);
		pBuf[m_Txd.WritePtr++] = (wStatus&0xFF);
	
		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-4))
			break;

	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);
	
	return TRUE;
}

BOOL Txd_YcSoe(int DevID)
{
	if(m_pCfg->yc.bStatus2 == 34)
		return Txd_YcSoe34(DevID);
	if(m_pCfg->yc.bStatus2 == 36)
		return Txd_YcSoe36(DevID);

	return FALSE;
}

BOOL Txd_YcSoe34(int DevID)
{
	BYTE bStyle = 34;
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	TSysTimeAit sysSoe; 
	WORD wNo;
	WORD wMSecond;
	BYTE* pBuf;
	short sValue;
	WORD wStatus;
	int i;

	pBuf = m_Txd.Buf;
	
	Txd_Head(DevID, bStyle, bReason);
	
	bNum = 0;

	for(i = 0; GetAitNum(DevID); i++)
	{
		if(ReadSysTimeAit(DevID, &sysSoe) == FALSE)
			break;
		
		IncAitReadPtr(DevID);

//		if(sysSoe.bStatus != 1 && sysSoe.bStatus != 2)
//			continue;
		
		bNum ++;	
//        m_pCfg->yc.wStartAddr=0x4001;   //���ɹ������ų�101�ϵ��ȵ�ң����Ϣ����ʼ��ַΪ0x4000������97�汾��101��Լ

		wNo = sysSoe.nNo + m_pCfg->yc.wStartAddr;

//		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wNo);  //���L
//		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wNo);  //���H
		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
		
		sValue = LogicAIToScale(DevID, sysSoe.nNo, sysSoe.sValue, &wStatus);
		pBuf[ m_Txd.WritePtr++ ] = (sValue&0xFF);  //wValue
		pBuf[ m_Txd.WritePtr++ ] = (sValue>>8);
		

		pBuf[ m_Txd.WritePtr++ ] = (wStatus&0xFF);
		
		wMSecond = sysSoe.stTime.MSecond;

		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wMSecond);  //MSEC L
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wMSecond);  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Minute;  //MSEC H
		
//long time for DF8002		
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Hour;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Day;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Month;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Year%100;  //MSEC H
		
		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-9))
			break;

	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}

BOOL Txd_YcSoe36(int DevID)
{
	BYTE bStyle = 36;
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	TSysTimeAit sysSoe; 
	WORD wNo;
	WORD wMSecond;
	BYTE* pBuf;
	DWORD dwValue;
	float* pFloat;
	short sValue;
	WORD wStatus;
	int i;

	pBuf = m_Txd.Buf;
	
	Txd_Head(DevID, bStyle, bReason);
	
	bNum = 0;
	pFloat = (float*)&dwValue;

	for(i = 0; GetAitNum(DevID); i++)
	{
		if(ReadSysTimeAit(DevID, &sysSoe) == FALSE)
			break;
		
		IncAitReadPtr(DevID);

		
		bNum ++;
		sValue = LogicAIToScale(DevID, sysSoe.nNo, sysSoe.sValue, &wStatus);
		*pFloat = (float)sValue;
//        m_pCfg->yc.wStartAddr=0x4001;   //���ɹ������ų�101�ϵ��ȵ�ң����Ϣ����ʼ��ַΪ0x4000������97�汾��101��Լ
		
		wNo = sysSoe.nNo + m_pCfg->yc.wStartAddr;

		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
		
		pBuf[ m_Txd.WritePtr++ ] = LLBYTE(dwValue);  //wValue
		pBuf[ m_Txd.WritePtr++ ] = LHBYTE(dwValue);
		pBuf[ m_Txd.WritePtr++ ] = HLBYTE(dwValue);  //wValue
		pBuf[ m_Txd.WritePtr++ ] = HHBYTE(dwValue);
		

		pBuf[ m_Txd.WritePtr++ ] = wStatus&0xFF;//sysSoe.bStatus;
		
		wMSecond = sysSoe.stTime.MSecond;

		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wMSecond);  //MSEC L
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wMSecond);  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Minute;  //MSEC H
		
//long time for DF8002		
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Hour;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Day;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Month;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Year%100;  //MSEC H
		
		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-11))
			break;
	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}

BOOL Txd_CP24TDSoe(int DevID)//����˫��SOE
{
	BYTE bStyle = 4;
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	TSysTimeDBISoe sysSoe; 
	WORD wNo;
	WORD wMSecond;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;
	
	Txd_Head(DevID, bStyle, bReason);

	for(bNum = 0; GetDBISoeNum(DevID); bNum++)
	{
		if(ReadSysTimeDBISoe(DevID, &sysSoe) == FALSE)
			break;
		
		IncDBISoeReadPtr(DevID);
		
		wNo = sysSoe.nNo + m_pCfg->dpi.wStartAddr;

//		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wNo);  //���L
//		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wNo);  //���H
		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
		
		pBuf[ m_Txd.WritePtr++ ] = (sysSoe.bStatus&0x3);//(sysSoe.bStatus & PBI_ON) ? 1:0;  //״̬
		
		wMSecond = sysSoe.stTime._milliseconds;

		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wMSecond);  //MSEC L
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wMSecond);  //MSEC H
		
//		if(sysSoe.bStatus & PBI_TIV)
//			pBuf[ m_Txd.WritePtr++ ] = (sysSoe.stTime.Minute | 0x80);
//		else
			pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._min._minutes;
		
//		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Hour;  
//		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Day;  
//		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Month;
//		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime.Year%100;
		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-3))
			break;

	}
	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}

BOOL Txd_CP56TDSoe(int DevID)//����˫��SOE
{
	BYTE bStyle = 31;
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	TSysTimeDBISoe sysSoe; 
	WORD wNo;
	WORD wMSecond;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;
	
	Txd_Head(DevID, bStyle, bReason);

	for(bNum = 0; GetDBISoeNum(DevID); bNum++)
	{
		if(ReadSysTimeDBISoe(DevID, &sysSoe) == FALSE)
			break;
		
		IncDBISoeReadPtr(DevID);
		
		wNo = sysSoe.nNo + m_pCfg->dpi.wStartAddr;

//		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wNo);  //���L
//		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wNo);  //���H
		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
		
		pBuf[ m_Txd.WritePtr++ ] = (sysSoe.bStatus&0x3);//(sysSoe.bStatus & PBI_ON) ? 1:0;  //״̬
		
		wMSecond = sysSoe.stTime._milliseconds;

		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wMSecond);  //MSEC L
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wMSecond);  //MSEC H
		
//		if(sysSoe.bStatus & PBI_TIV)
//			pBuf[ m_Txd.WritePtr++ ] = (sysSoe.stTime.Minute | 0x80);
//		else
			pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._min._minutes;
		
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._hour._hours;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._day._dayofmonth;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._month._month;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._year._year%100;  //MSEC H

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-7))
			break;

	}
	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}

BOOL Txd_CP24TSoe(int DevID) //����SOE����
{
	BYTE bStyle = 2;
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	TSysTimeSoe sysSoe; 
	WORD wNo;
	WORD wMSecond;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;
	
	Txd_Head(DevID, bStyle, bReason);

	for(bNum = 0; GetSoeNum(DevID); bNum++)
	{
		if(ReadSysTimeSoe(DevID, &sysSoe) == FALSE)
			break;
		
		IncSoeReadPtr(DevID);
//		m_pCfg->spi.wStartAddr=0x01;
		wNo = sysSoe.nNo + m_pCfg->spi.wStartAddr;

		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
	
		pBuf[ m_Txd.WritePtr++ ] = (sysSoe.bStatus & PBI_ON) ? 1:0;  //״̬
		
		wMSecond = sysSoe.stTime._milliseconds;

		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wMSecond);  //MSEC L
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wMSecond);  //MSEC H
		
//		if(sysSoe.bStatus & PBI_TIV)//ʱ�䲻׼���
//			pBuf[ m_Txd.WritePtr++ ] = (sysSoe.stTime.Minute | 0x80);
//		else
			pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._min._minutes;
		
		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-3))
			break;

	}
	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}


BOOL Txd_CP56TSoe(int DevID) //����SOE����
{
	BYTE bStyle = 30;
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	TSysTimeSoe sysSoe; 
	WORD wNo;
	WORD wMSecond;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;
	
	Txd_Head(DevID, bStyle, bReason);

	for(bNum = 0; GetSoeNum(DevID); bNum++)
	{
		if(ReadSysTimeSoe(DevID, &sysSoe) == FALSE)
			break;
		
		IncSoeReadPtr(DevID);
//		m_pCfg->spi.wStartAddr=0x01;
		wNo = sysSoe.nNo + m_pCfg->spi.wStartAddr;

//		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wNo);  //���L
//		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wNo);  //���H
		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
	
		pBuf[ m_Txd.WritePtr++ ] = (sysSoe.bStatus & PBI_ON) ? 1:0;  //״̬
		
		wMSecond = sysSoe.stTime._milliseconds;

		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wMSecond);  //MSEC L
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wMSecond);  //MSEC H
		
//		if(sysSoe.bStatus & PBI_TIV)//ʱ�䲻׼���
//			pBuf[ m_Txd.WritePtr++ ] = (sysSoe.stTime.Minute | 0x80);
//		else
			pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._min._minutes;
		
//long time for DF8002		
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._hour._hours;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._day._dayofmonth;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._month._month;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._year._year%100;  //MSEC H
		
		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-7))
			break;

	}
	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}

static BOOL Txd_ChangeAI(int DevID) //���ͱ仯ң������
{
	BYTE bStyle;
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	short sValue;
	TBurstAI burstAI;
	DWORD dwBustAINum;
	int i;
	WORD wNo;
//	float fValue;
	BYTE* pBuf;
	WORD wAINum;
	
	bStyle = m_pCfg->yc.bStatus1;

	if(bStyle == 13)//���͸�����
		return Txd_ChangeAI13(DevID);
	if(bStyle == 34)//full time nomal value
		return Txd_ChangeAI34(DevID);
	if(bStyle == 36)
		return Txd_ChangeAI36(DevID);

	pBuf = m_Txd.Buf;

	dwBustAINum = GetBurstAINum(DevID);
	wAINum = GetAINum(DevID);

	if(m_pCfg->wCylicDataNum >= wAINum)
		return FALSE;

	wAINum -= m_pCfg->wCylicDataNum;

	if(dwBustAINum == 0 || wAINum == 0)
		return FALSE;
	
	Txd_Head(DevID, bStyle, bReason);

	bNum = 0;

	for(i = 0; i < dwBustAINum; i++)
	{
		if(ReadBurstAI(DevID,&burstAI) != TRUE)
			break;
		
		if(burstAI.dwNo >= wAINum)
			continue;

		bNum ++;
//        m_pCfg->yc.wStartAddr=0x4001;   //���ɹ������ų�101�ϵ��ȵ�ң����Ϣ����ʼ��ַΪ0x4000������97�汾��101��Լ

		wNo = burstAI.dwNo + m_pCfg->yc.wStartAddr;
		sValue = burstAI.sValue;

		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
		
		pBuf[m_Txd.WritePtr++] = LOBYTE(sValue); //ң��ֵ Lo
		pBuf[m_Txd.WritePtr++] = HIBYTE(sValue); //ң��ֵ Hi
		
		if(bStyle == 11 || bStyle == 9)
			pBuf[m_Txd.WritePtr++] = (burstAI.wStatus&0xFF);

		if(bStyle == 11 || bStyle == 9)
		{
			if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-2))
				break;
		}
		else
		{
			if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-1))
				break;
		}
	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);
	
	return TRUE;
}

static BOOL Txd_ChangeAI34(int DevID)//���ͱ仯ң�ⳤʱ��nomalise����
{
	BYTE bStyle;
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	TBurstAI burstAI;
	DWORD dwBustAINum;
	int i;
	WORD wNo;
	BYTE* pBuf;
	WORD wAINum;
	WORD wMSecond;
	
	bStyle = 34;//short float
	pBuf = m_Txd.Buf;

	dwBustAINum = GetBurstAINum(DevID);
	wAINum = GetAINum(DevID);

	if(m_pCfg->wCylicDataNum >= wAINum)
		return FALSE;

	wAINum -= m_pCfg->wCylicDataNum;

	if(dwBustAINum == 0 || wAINum == 0)
		return FALSE;
	
	Txd_Head(DevID, bStyle, bReason);

	bNum = 0;

	for(i = 0; i < dwBustAINum; i++)
	{
		if(ReadBurstAI(DevID,&burstAI) != TRUE)
			break;
		
		if(burstAI.dwNo >= wAINum)
			continue;

		bNum ++;
//        m_pCfg->yc.wStartAddr=0x4001;   //���ɹ������ų�101�ϵ��ȵ�ң����Ϣ����ʼ��ַΪ0x4000������97�汾��101��Լ
		
		wNo = burstAI.dwNo + m_pCfg->yc.wStartAddr;

		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
		
		pBuf[m_Txd.WritePtr++] = burstAI.sValue&0xFF; //ң��ֵ Lo
		pBuf[m_Txd.WritePtr++] = burstAI.sValue>>8;//ң��ֵ Hi
		
		pBuf[m_Txd.WritePtr++] = (burstAI.wStatus&0xFF);

		wMSecond = burstAI.stTime.MSecond;

		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wMSecond);  //MSEC L
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wMSecond);  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Minute;  //MSEC H
		
		pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Hour;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Day;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Month;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Year%100;  //MSEC H

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-9))
			break;

	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);
	
	return TRUE;
}

static BOOL Txd_ChangeAI36(int DevID)//���ͱ仯ң�ⳤʱ�긡������
{
	BYTE bStyle;
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	TBurstAI burstAI;
	DWORD dwBustAINum;
	int i;
	WORD wNo;
	BYTE* pBuf;
	DWORD dwValue;
	WORD wAINum;
	WORD wMSecond;

	float* pFloat;
	
	bStyle = 36;//short float
	pBuf = m_Txd.Buf;

	dwBustAINum = GetBurstAINum(DevID);
	wAINum = GetAINum(DevID);

	if(m_pCfg->wCylicDataNum >= wAINum)
		return FALSE;

	wAINum -= m_pCfg->wCylicDataNum;

	if(dwBustAINum == 0 || wAINum == 0)
		return FALSE;
	
	Txd_Head(DevID, bStyle, bReason);

	bNum = 0;

	pFloat = (float*)&dwValue;

	for(i = 0; i < dwBustAINum; i++)
	{
		if(ReadBurstAI(DevID,&burstAI) != TRUE)
			break;
		
		if(burstAI.dwNo >= wAINum)
			continue;

		bNum ++;
//        m_pCfg->yc.wStartAddr=0x4001;   //���ɹ������ų�101�ϵ��ȵ�ң����Ϣ����ʼ��ַΪ0x4000������97�汾��101��Լ
		
		wNo = burstAI.dwNo + m_pCfg->yc.wStartAddr;
		*pFloat = (float)burstAI.sValue;

		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
		
		pBuf[m_Txd.WritePtr++] = LLBYTE(dwValue); //ң��ֵ Lo
		pBuf[m_Txd.WritePtr++] = LHBYTE(dwValue); //ң��ֵ Hi
		pBuf[m_Txd.WritePtr++] = HLBYTE(dwValue); //ң��ֵ Lo
		pBuf[m_Txd.WritePtr++] = HHBYTE(dwValue); //ң��ֵ Hi
		
		pBuf[m_Txd.WritePtr++] = (burstAI.wStatus&0xFF);

		wMSecond = burstAI.stTime.MSecond;

		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wMSecond);  //MSEC L
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wMSecond);  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Minute;  //MSEC H
		
		pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Hour;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Day;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Month;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Year%100;  //MSEC H


		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-11))
			break;

	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);
	
	return TRUE;
}

BOOL Txd_ChangeAI14(int DevID)//���ͱ仯ң���ʱ�긡������
{
	BYTE bStyle;
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	TBurstAI burstAI;
	DWORD dwBustAINum;
	int i;
	WORD wNo;
	BYTE* pBuf;
	DWORD dwValue;
	WORD wAINum;

	float* pFloat;
	
	bStyle = 13;//short float
	pBuf = m_Txd.Buf;

	dwBustAINum = GetBurstAINum(DevID);
	wAINum = GetAINum(DevID);

	if(m_pCfg->wCylicDataNum >= wAINum)
		return FALSE;

	wAINum -= m_pCfg->wCylicDataNum;

	if(dwBustAINum == 0 || wAINum == 0)
		return FALSE;
	
	Txd_Head(DevID, bStyle, bReason);

	bNum = 0;

	pFloat = (float*)&dwValue;

	for(i = 0; i < dwBustAINum; i++)
	{
		if(ReadBurstAI(DevID,&burstAI) != TRUE)
			break;
		
		if(burstAI.dwNo >= wAINum)
			continue;

		bNum ++;
//        m_pCfg->yc.wStartAddr=0x4001;   //���ɹ������ų�101�ϵ��ȵ�ң����Ϣ����ʼ��ַΪ0x4000������97�汾��101��Լ
		
		wNo = burstAI.dwNo + m_pCfg->yc.wStartAddr;
		*pFloat = (float)burstAI.sValue;

//		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wNo); //��Ϣ���ַLo
//		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wNo); //��Ϣ���ַHi
		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
		
		pBuf[m_Txd.WritePtr++] = LLBYTE(dwValue); //ң��ֵ Lo
		pBuf[m_Txd.WritePtr++] = LHBYTE(dwValue); //ң��ֵ Hi
		pBuf[m_Txd.WritePtr++] = HLBYTE(dwValue); //ң��ֵ Lo
		pBuf[m_Txd.WritePtr++] = HHBYTE(dwValue); //ң��ֵ Hi
		
		pBuf[m_Txd.WritePtr++] = (burstAI.wStatus&0xFF);

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-4))
			break;

	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);
	
	return TRUE;
}

static BOOL Txd_ChangeAI13(int DevID) //���ͱ仯ң�⸡������
{
	BYTE bStyle;
	BYTE bReason = 3;
	BYTE bPRM = 0, bCode = 8;
	BYTE bNum;
	TBurstAI burstAI;
	DWORD dwBustAINum;
	int i;
	WORD wNo;
	BYTE* pBuf;
	DWORD dwValue;
	WORD wAINum;
	float* pFloat;
	
	bStyle = 13;//short float
	pBuf = m_Txd.Buf;

	dwBustAINum = GetBurstAINum(DevID);
	wAINum = GetAINum(DevID);

	if(m_pCfg->wCylicDataNum >= wAINum)
		return FALSE;

	wAINum -= m_pCfg->wCylicDataNum;

	if(dwBustAINum == 0 || wAINum == 0)
		return FALSE;
	
	Txd_Head(DevID, bStyle, bReason);

	bNum = 0;

	pFloat = (float*)&dwValue;

	for(i = 0; i < dwBustAINum; i++)
	{
		if(ReadBurstAI(DevID,&burstAI) != TRUE)
			break;
		
		if(burstAI.dwNo >= wAINum)
			continue;

		bNum ++;
//        m_pCfg->yc.wStartAddr=0x4001;   //���ɹ������ų�101�ϵ��ȵ�ң����Ϣ����ʼ��ַΪ0x4000������97�汾��101��Լ
		
		wNo = burstAI.dwNo + m_pCfg->yc.wStartAddr;
		*pFloat = (float)burstAI.sValue;

//		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wNo); //��Ϣ���ַLo
//		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wNo); //��Ϣ���ַHi
		if(m_pCfg->bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		else if(m_pCfg->bInfoAddr == 3)
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
			pBuf[m_Txd.WritePtr++] = 0;
		}
		else
		{
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		}
		
		pBuf[m_Txd.WritePtr++] = LLBYTE(dwValue); //ң��ֵ Lo
		pBuf[m_Txd.WritePtr++] = LHBYTE(dwValue); //ң��ֵ Hi
		pBuf[m_Txd.WritePtr++] = HLBYTE(dwValue); //ң��ֵ Lo
		pBuf[m_Txd.WritePtr++] = HHBYTE(dwValue); //ң��ֵ Hi
		
		pBuf[m_Txd.WritePtr++] = (burstAI.wStatus&0xFF);

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-4))
			break;
	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail(DevID, bPRM, bCode, bNum);
	
	return TRUE;
}


BOOL TxdTestLink(int DevID)//������·
{
	BYTE bStyle = 0x68;
	BYTE bReason = 0x87;
	BYTE bPRM = 0, bCode = 8, bNum = 1;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;

	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
		pBuf[ m_Txd.WritePtr++ ] = 0; //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[ m_Txd.WritePtr++ ] = 0; //��Ϣ���ַLo
		pBuf[ m_Txd.WritePtr++ ] = 0; //��Ϣ���ַHi
		pBuf[ m_Txd.WritePtr++ ] = 0;
	}
	else
	{
		pBuf[ m_Txd.WritePtr++ ] = 0; //��Ϣ���ַLo
		pBuf[ m_Txd.WritePtr++ ] = 0; //��Ϣ���ַHi
	}
	
	pBuf[ m_Txd.WritePtr++ ] = 0xAA; 
	pBuf[ m_Txd.WritePtr++ ] = 0x55; 

	Txd_Tail(DevID, bPRM, bCode, bNum);
	
	return TRUE;
}

BOOL TxdSetPara(int DevID)//���ò���
{
	BYTE bStyle = 0x6E, bReason = 7;
	BYTE bPRM = 0, bCode = 8, bNum = 1;
	BYTE* pData;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;
//	pData = (m_bTmpBuf+1);
//	
//	bStyle = m_bTmpBuf[0];

	Txd_Head(DevID, bStyle, bReason);
	
//	pBuf[ m_Txd.WritePtr++ ] = pData[0]; //��Ϣ���ַLo
//	pBuf[ m_Txd.WritePtr++ ] = pData[1]; //��Ϣ���ַHi
	
	if(m_pCfg->bInfoAddr == 1)
	{
		pBuf[m_Txd.WritePtr++] = pData[0]; //��Ϣ���ַLo
		pData += 1;
	}
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = pData[0]; //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = pData[1]; //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = pData[2];

		pData += 3;
	}
	else
	{
		pBuf[ m_Txd.WritePtr++ ] = pData[0]; //��Ϣ���ַLo
		pBuf[ m_Txd.WritePtr++ ] = pData[1]; //��Ϣ���ַHi

		pData += 2;
	}


	pBuf[ m_Txd.WritePtr++ ] = pData[0]; //QPM
	pBuf[ m_Txd.WritePtr++ ] = pData[1]; //QPM
	pBuf[ m_Txd.WritePtr++ ] = pData[2]; //QPM

	if(bStyle == 112)
	{
		pBuf[ m_Txd.WritePtr++ ] = pData[3]; //QPM
		pBuf[ m_Txd.WritePtr++ ] = pData[4]; //QPM
	}

	
	Txd_Tail(DevID, bPRM, bCode, bNum);
	
	return TRUE;
}


static BOOL TxdTimeDelay(int DevID)
{
	BYTE bStyle = 0x6a, bReason = 7;
	BYTE bPRM = 0, bCode = 8, bNum = 1;
	BYTE* pBuf;
	TAbsTime absTime;
	WORD wDelayTime;

	pBuf = m_Txd.Buf;

	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
	{
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
	}
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;

	}
	else
	{
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַHi
	}

//	ReadAbsTime(&absTime);
//
//	if(absTime.Lo >= m_absTime1.Lo)
//		wDelayTime = LOWORD(absTime.Lo - m_absTime1.Lo);
//	else
//		wDelayTime = LOWORD(0xffffffff - m_absTime1.Lo + absTime.Lo + 1);
//
//	wDelayTime = m_wFromTime + wDelayTime;

	pBuf[m_Txd.WritePtr++] = LOBYTE(wDelayTime);
	pBuf[m_Txd.WritePtr++] = HIBYTE(wDelayTime);
	
	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}

static BOOL TxdClockSyn(int DevID)//���ӵ�ȷ��
{
	BYTE bStyle = 0x67, bReason = 7;
	BYTE bPRM = 0, bCode = 8, bNum = 1;
	BYTE* pData;
	BYTE* pBuf;
	int i;
  
	pBuf = m_Txd.Buf;
//	pData = m_RxdFrame.Buf + 10;

	if(m_pCfg->bLinkAddr == 2)
		pData += 1;
	if(m_pCfg->bAsduAddr == 2)
		pData += 1; 
	if(m_pCfg->bCauseTrans == 2)
		pData += 1;

	Txd_Head(DevID, bStyle, bReason);
	
//	m_Txd.Buf[ m_Txd.WritePtr++ ] = 00; //��Ϣ���ַLo
//	m_Txd.Buf[ m_Txd.WritePtr++ ] = 00; //��Ϣ���ַHi
	if(m_pCfg->bInfoAddr == 1)
	{
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
		pData += 1;
	}
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;

		pData += 3;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַHi

		pData += 2;
	}
	
	for(i=0;i<7;i++)
		m_Txd.Buf[ m_Txd.WritePtr++ ] = pData[i]; // 
	
	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}



static BOOL Txd_AllStop(int DevID)//�������ٻ�����֡
{
	BYTE bStyle = 0x64;
	BYTE bReason = 0x0A;
	BYTE bPRM = 0, bCode = 8, bNum = 1;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;

	Txd_Head(DevID, bStyle, bReason);

	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַHi
	}

	pBuf[ m_Txd.WritePtr++ ] = 0x14; //COI

	Txd_Tail(DevID, bPRM, bCode, bNum);
	
	return TRUE;
}

BOOL TxdCallAllAck(int DevID)//�������ٻ���������
{
	BYTE bStyle = 0x64, bReason = 7;
	BYTE bPRM = 0, bCode = 8, bNum = 1;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;

	Txd_Head(DevID, bStyle, bReason);

//	m_Txd.Buf[ m_Txd.WritePtr++ ] = 00; //��Ϣ���ַLo
//	m_Txd.Buf[ m_Txd.WritePtr++ ] = 00; //��Ϣ���ַHi
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַHi
	}

	m_Txd.Buf[ m_Txd.WritePtr++ ] = 0x14; //COI
	
	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}

//��վ��ʼ�����
BOOL TxdInitFinish(int DevID)
{
	BYTE bStyle = 0x46;
	BYTE bReason = 4;
	BYTE bPRM = 0, bCode = 8, bNum = 1;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;

	Txd_Head(DevID, bStyle, bReason);
	
//	pBuf[ m_Txd.WritePtr++ ] = 00; //��Ϣ���ַLo
//	pBuf[ m_Txd.WritePtr++ ] = 00; //��Ϣ���ַHi
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = 0; //��Ϣ���ַHi
	}

	pBuf[ m_Txd.WritePtr++ ] = 0; //COI
	
	Txd_Tail(DevID, bPRM, bCode, bNum);
	
	return TRUE;
}


BOOL TxdFileStatus(int DevID)//�����ļ�״̬
{
	char lzName[16];
	BYTE bPRM = 0, bCode = 8, bNum = 1;
	BYTE* pBuf;
	
	pBuf = m_Txd.Buf;

//	::GetDevParaName(DevID, lzName);
//	strcat(lzName, ".dal");
//
//	if(!GetFileStatus(lzName, &fileStatus))
//		return FALSE;
//	
//	Txd_Head(DevID, FILE_DIR_SEND, FILE_SEND_REASON);
//
//	pBuf[ m_Txd.WritePtr++ ] = 00; //��Ϣ���ַLo
//	pBuf[ m_Txd.WritePtr++ ] = 0xF0; //��Ϣ���ַHi
//	pBuf[ m_Txd.WritePtr++ ] = 00; //Ŀ¼�����޶���
//
//	pBuf[m_Txd.WritePtr++] = fileStatus.dwStatus&0xFF;
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwStatus>>8)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwStatus>>16)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwStatus>>24)&0xFF);
//
//	pBuf[m_Txd.WritePtr++] = (fileStatus.dwLen&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwLen>>8)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwLen>>16)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwLen>>24)&0xFF);
//
//	pBuf[m_Txd.WritePtr++] = (fileStatus.dwTime&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwTime>>8)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwTime>>16)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwTime>>24)&0xFF);
//
//	
//	pBuf[m_Txd.WritePtr++] = (fileStatus.wCrc16&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.wCrc16>>8)&0xFF);
//
//	pBuf[m_Txd.WritePtr++] = strlen(lzName);
//	
//	int i;
//	for(i=0; i<strlen(lzName); i++)
//		pBuf[m_Txd.WritePtr++] = lzName[i];

	
	Txd_Tail(DevID, bPRM, bCode, bNum);


	return TRUE;
}

BOOL TxdYtExeInfo(BYTE bReason)
{
//	BYTE bStyle;
//	BYTE bPRM = 0, bCode = 8, bNum = 1;
//	TAO  *pAO;
//	BYTE* pBuf;
//	
//	pAO = pGetAO();
//	
//	if(pAO == NULL)
//		return FALSE;
//		
//	pBuf = m_Txd.Buf;
//
//	bStyle = pAO->Info[0];
//	
//	Txd_Head(DevID, bStyle, bReason);
//
//	if(m_pCfg->bInfoAddr == 1)
//	{
//		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[1]; //��Ϣ���ַLo
//
//		if((bStyle==48)||(bStyle==49))
//		{
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[2]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[3]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //DCO
//		}
//		if(bStyle==50)
//		{
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[2]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[3]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[5]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[6]; //DCO
//		}
//	}
//	else if(m_pCfg->bInfoAddr == 3)
//	{
//		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[1]; //��Ϣ���ַLo
//		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[2]; //��Ϣ���ַHi
//		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[3]; //��Ϣ���ַHi
//
//		if((bStyle==48)||(bStyle==49))
//		{
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[5]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[6]; //DCO
//		}
//		if(bStyle==50)
//		{
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[5]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[6]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[7]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[8]; //DCO
//		}
//	}
//	else
//	{
//		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[1]; //��Ϣ���ַLo
//		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[2]; //��Ϣ���ַHi
//		
//		if((bStyle==48)||(bStyle==49))
//		{
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[3]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[5]; //DCO
//		}
//		if(bStyle==50)
//		{
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[3]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[5]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[6]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[7]; //DCO
//		}
//	}
//
//	Txd_Tail(DevID, bPRM, bCode, bNum);  

	return TRUE;
}
	
BOOL TxdYtReturn()
{
//	BYTE bStyle, bReason;
//	BYTE bPRM = 0, bCode = 8, bNum = 1;
//	TAO  *pAO;
//	BYTE* pBuf;
//	
//	pAO = pGetAO();
//	
//	if(pAO == NULL)
//		return FALSE;
//		
//	pBuf = m_Txd.Buf;
//
//	bStyle = pAO->Info[0];
//	
//	if(pAO->dwCommand == TP_AOCANCEL)
//		bReason = 9;
//	else
//	{
//		bReason = 7;
//	//	if(pAO->dwCommand == TP_AOSELECT)
////		bReason = 7;
//		if((pAO->dwCommand == TP_AOOPERATE) || (pAO->dwCommand == TP_AODIRECT))
//		{
//			if(pAO->dwResult == PAORET_SUCCESS)
//			{
////				SetDevFlag(SF_YtRtnInfoFlag);
//				SetDevFlag(SF_YtExitFlag);
//			}
//		}
//	}
//
////	if(pAO->dwCommand == TP_AOCANCEL)
////		bReason = 9;
////	else
////		bReason = 7;
//	
//	if(pAO->dwResult != PAORET_SUCCESS)
//		bReason |= 0x40;
//	
//	Txd_Head(DevID, bStyle, bReason);
//
//	if(m_pCfg->bInfoAddr == 1)
//	{
//		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[1]; //��Ϣ���ַLo
//
//		if((bStyle==48)||(bStyle==49))
//		{
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[2]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[3]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //DCO
//		}
//		if(bStyle==50)
//		{
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[2]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[3]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[5]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[6]; //DCO
//		}
////		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[2]; //value Lo
////		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[3]; //value Hi
////		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //DCO
//	}
//	else if(m_pCfg->bInfoAddr == 3)
//	{
//		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[1]; //��Ϣ���ַLo
//		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[2]; //��Ϣ���ַHi
//		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[3]; //��Ϣ���ַHi
//
//		if((bStyle==48)||(bStyle==49))
//		{
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[5]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[6]; //DCO
//		}
//		if(bStyle==50)
//		{
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[5]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[6]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[7]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[8]; //DCO
//		}
//
////		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //value Lo
////		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[5]; //value Hi
////		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[6]; //DCO
//	}
//	else
//	{
//		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[1]; //��Ϣ���ַLo
//		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[2]; //��Ϣ���ַHi
//		
//		if((bStyle==48)||(bStyle==49))
//		{
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[3]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[5]; //DCO
//		}
//		if(bStyle==50)
//		{
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[3]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[5]; //value Lo
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[6]; //value Hi
//			pBuf[ m_Txd.WritePtr++ ] = pAO->Info[7]; //DCO
//		}
////		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[3]; //value Lo
////		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[4]; //value Hi
////		pBuf[ m_Txd.WritePtr++ ] = pAO->Info[5]; //DCO
//	}
//
//	Txd_Tail(DevID, bPRM, bCode, bNum);  

	return TRUE;
}

BOOL TxdDOReturn()
{
//	BYTE bStyle=51, bReason = 7;
//	BYTE bPRM = 0, bCode = 8, bNum = 1;
//	TDO  *pDO;
//	BYTE* pBuf;
//	
//	pDO = pGetDO();
//
//	if(pDO == NULL)
//		return FALSE;
//
//	pBuf = m_Txd.Buf;
//	
//	
//	if(pDO->dwResult != PDORET_SUCCESS)
//		bReason |= 0x40;
//	
//	Txd_Head(DevID, bStyle, bReason);
//	
//	if(m_pCfg->bInfoAddr == 1)
//	{
//		pBuf[m_Txd.WritePtr++] = pDO->Info[0];//addr
//		pBuf[m_Txd.WritePtr++] = pDO->Info[1];
//		pBuf[m_Txd.WritePtr++] = pDO->Info[2];
//		pBuf[m_Txd.WritePtr++] = pDO->Info[3];
//		pBuf[m_Txd.WritePtr++] = pDO->Info[4];
//	}
//	else if(m_pCfg->bInfoAddr == 3)
//	{
//		pBuf[m_Txd.WritePtr++] = pDO->Info[0];//addr
//		pBuf[m_Txd.WritePtr++] = pDO->Info[1];//addr
//		pBuf[m_Txd.WritePtr++] = pDO->Info[2];
//		pBuf[m_Txd.WritePtr++] = pDO->Info[3];
//		pBuf[m_Txd.WritePtr++] = pDO->Info[4];
//		pBuf[m_Txd.WritePtr++] = pDO->Info[5];
//		pBuf[m_Txd.WritePtr++] = pDO->Info[6];
//	}
//	else
//	{
//		pBuf[m_Txd.WritePtr++] = pDO->Info[0];//addr
//		pBuf[m_Txd.WritePtr++] = pDO->Info[1];//addr
//		pBuf[m_Txd.WritePtr++] = pDO->Info[2];
//		pBuf[m_Txd.WritePtr++] = pDO->Info[3];
//		pBuf[m_Txd.WritePtr++] = pDO->Info[4];
//		pBuf[m_Txd.WritePtr++] = pDO->Info[5];
//	}
//        //pBuf[8] =0x07;
//	Txd_Tail(DevID, bPRM, bCode, bNum);  

	return TRUE;
}

BOOL TxdTqReturn()
{
/*	BYTE bStyle=60, bReason = 7;
	BYTE bPRM = 0, bCode = 8, bNum = 1;
	BYTE* pBuf;
	TTQ* pTQ;
	
	pTQ = pGetTQ();
	
	if(pTQ == NULL)
		return FALSE;

	pBuf = m_Txd.Buf;
	
	if(pTQ->dwCommand == TP_TQCANCEL)
		bReason = 9;
	else
		bReason = 7;
	
	if(pTQ->dwResult != PTQRET_SUCCESS)
		bReason |= 0x40;
	
	Txd_Tail(DevID, bStyle, bReason);

	pBuf[ m_Txd.WritePtr++ ] = LOBYTE(m_pCfg->tq.wStartAddr + pTQ->wNo); //��Ϣ���ַLo
	pBuf[ m_Txd.WritePtr++ ] = LOBYTE(m_pCfg->tq.wStartAddr+pTQ->wNo); //��Ϣ���ַHi
	pBuf[ m_Txd.WritePtr++ ] = pTQ->bInfo[2]; //bTQS

	Txd_Tail(DevID, bPRM, bCode, bNum);  
*/
	return TRUE;
}

BOOL TxdYkExeInfo(BYTE bReason) //����return info
{
//	BYTE bStyle;
//	BYTE bPRM = 0, bCode = 8, bNum = 1;
//	TBO  *pBO;
//	BYTE* pBuf;
//	
//	pBO = pGetBO();
//	
//	if(pBO == NULL)
//		return FALSE;
//		
//	pBuf = m_Txd.Buf;
//
//	bStyle = pBO->Info[0];
//	
//	Txd_Head(DevID, bStyle, bReason);
//
//	if(m_pCfg->bInfoAddr == 1)
//	{
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[1]; //��Ϣ���ַLo
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[2]; //DCO
//	}
//	else if(m_pCfg->bInfoAddr == 3)
//	{
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[1]; //��Ϣ���ַLo
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[2]; //��Ϣ���ַHi
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[3]; //��Ϣ���ַHi
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[4]; //DCO
//	}
//	else
//	{
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[1]; //��Ϣ���ַLo
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[2]; //��Ϣ���ַHi
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[3]; //DCO
//	}
//       
//
//	Txd_Tail(DevID, bPRM, bCode, bNum);  

	return TRUE;
}


BOOL TxdYkReturn() //����ң��Ԥ��/ִ��/ȡ�����ؽ��	
{
//	BYTE bStyle, bReason = 7;
//	BYTE bPRM = 0, bCode = 8, bNum = 1;
//	TBO  *pBO;
//	BYTE* pBuf;
//	
//	pBO = pGetBO();
//	
//	if(pBO == NULL)
//		return FALSE;
//		
//	pBuf = m_Txd.Buf;
//
//	bStyle = pBO->Info[0];
//	
//	if(pBO->dwCommand == TP_YKCANCEL)
//	{
//		bReason = 9;
//	}
//	else
//	{
//		bReason = 7;
//		
//		if((pBO->dwCommand == TP_YKOPERATE) || (pBO->dwCommand == TP_YKDIRECT))
//		{
//			if(pBO->dwResult == PBORET_SUCCESS)			
//			{
////				SetDevFlag(SF_YkRtnInfoFlag);
//				SetDevFlag(SF_YkExitFlag);
//			}
//		}
//	}
//
//	if(pBO->dwResult != PBORET_SUCCESS)
//		bReason |= 0x40;
//	
//	Txd_Head(DevID, bStyle, bReason);
//
//	if(m_pCfg->bInfoAddr == 1)
//	{
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[1]; //��Ϣ���ַLo
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[2]; //DCO
//	}
//	else if(m_pCfg->bInfoAddr == 3)
//	{
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[1]; //��Ϣ���ַLo
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[2]; //��Ϣ���ַHi
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[3]; //��Ϣ���ַHi
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[4]; //DCO
//	}
//	else
//	{
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[1]; //��Ϣ���ַLo
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[2]; //��Ϣ���ַHi
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[3]; //DCO
//	}
//
//	Txd_Tail(DevID, bPRM, bCode, bNum);  

	return TRUE;
}

BOOL TxdFixFrame(int DevID, BYTE bPRM, BYTE bCode)
{
	BYTE bChkSum; 
	BYTE *pBuf;
	int len = 0;

	pBuf = m_Txd.Buf;
	m_Txd.ReadPtr = 0;
	m_Txd.WritePtr = 0;
 
	pBuf[m_Txd.WritePtr++] = 0x10;//�����ַ�
	pBuf[m_Txd.WritePtr++] = GetCtrCode(DevID, bPRM, bCode);//������
	pBuf[m_Txd.WritePtr++] = 1;//��·��ַ����վվַ��
	pBuf[m_Txd.WritePtr++] = 0;//��·��ַ����վվַ��
	bChkSum = (BYTE)ChkSum(pBuf+1, 3);


	pBuf[m_Txd.WritePtr++] = bChkSum;
	pBuf[m_Txd.WritePtr++] = 0x16;
	len = m_Txd.WritePtr;
	CK_send(DevID,m_Txd.Buf,len);
		

	return TRUE;
}

BOOL Txd_NoData(int DevID)
{
	 return TxdFixFrame(DevID, 0, 0x09);
}; //������������֡


//=======================================================================================
// ��������: �����ĵ�β��
//=======================================================================================


BOOL ACK(int DevID)
{
	BYTE bRxdCode;
	BYTE bTxdCode = 0;
	
	bRxdCode = GetDevFlagValue(DevID, SF_RXDCONTROL)& BIT_CODE;

	if(bRxdCode == 4)//no answer
		return TRUE;

	switch(bRxdCode)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			bTxdCode = 0;
			break;
		case 4:
			return TRUE;
		case 8:
		case 9:
			bTxdCode = 11;
			break;
		default:
			bTxdCode = 15;//��·δ���
			break;
	}
	TxdFixFrame(DevID, 0, bTxdCode);

	return TRUE;
}

BOOL NACK(int DevID)
{
	//bPRM = 0, bCode = 1;
	TxdFixFrame(DevID, 0, 1);

	return TRUE;
}

BOOL GetFileStatus(char* pName, TFileStatus* pFileStatus)
{
//	TFileHeader fileHead;
//
//	if(pName == NULL || pFileStatus == NULL)
//		return FALSE;
//
//	if(!GetFileLength(pName, &pFileStatus->dwLen))
//		return FALSE;
//	
//	ReadFile(pName, &fileHead, sizeof(fileHead), 0);
//
//	pFileStatus->wCrc16 = GetFileCrc16(pName);
//
//	pFileStatus->dwTime = fileHead.dwTime;
//
//	return TRUE;
}

WORD GetFileCrc16(char* pFileName)
{
//	FILE* fileID;
//	BYTE bData;
//	WORD wCrc;
//	char lzName[40];
//
//	if(pFileName == NULL)
//		return 0;
//
//	ToFullName(pFileName, lzName);
//
//	fileID = fopen(lzName, "rb");
//	wCrc = 0;
//
//	if(fileID == NULL)
//		return 0;

//	while(fread(&bData, 1, 1, fileID))
//		wCrc = Crc16(wCrc, bData);	
	
//	fclose(fileID);

//	return wCrc;
}

WORD Crc16(WORD wCrc, BYTE bData)
{
	WORD wTmp;
	int i;

	wTmp = bData;
	wTmp <<= 8;

	wCrc = wCrc ^ wTmp;

	for (i = 0; i < 8; ++i)//ѭ����������8
	{
		if (wCrc & 0x8000)
			wCrc = ((wCrc << 1) ^ 0x1021);
		else
		wCrc = wCrc << 1;
	}

	return wCrc;
}

BOOL Txd_NoData_E5(int DevID)
{
    m_Txd.ReadPtr = 0;
    m_Txd.Buf[0] = 0xE5;
    m_Txd.WritePtr = 1;
	TxdFixFrame(DevID, 0, 0x09);
	return TRUE;
}

static void MakeMirror(BYTE bReason)
{
//	memcpy(m_bMirrorBuf, m_RxdFrame.Buf, m_RxdFrame.Buf[1]+6);
//	
//	//m_bMirrorBuf[4] &= (~BIT_PRM);
//	m_bMirrorBuf[4] = 0x08;
//	
//	if(bReason != 0xFF)
//	{
//		if(m_pCfg->bLinkAddr == 2)
//			m_bMirrorBuf[9] = bReason;
//		else
//			m_bMirrorBuf[8] = bReason;
//	}
//	
////	SetDevFlag(SF_MirrorFlag);
}

BOOL TxdScanAiData(int DevID)
{
	BYTE bStyle;
	BYTE bReason = 1;
	BYTE bPRM = 0, bCode = 8;
	WORD wAINum, wNo;
	short sValue;
	int i;
	BYTE* pBuf;
	WORD wStatus;
	BYTE bNum;
	WORD wSendFlag;
	int nStart;
	
	bStyle = m_pCfg->yc.bStatus1;//9,11,21

	if(bStyle == 13)//float value
		return TxdScanAiData13(DevID);
	
	if(bStyle == 34)//nomalised with full time
		bStyle = 9;
	else if(bStyle == 36)//short float with full time 
		bStyle = 13;

	pBuf = m_Txd.Buf;

	wAINum = GetAINum(DevID);
		
	for(i=0; i<wAINum; i++)
	{
		wSendFlag = GetAIFlag(DevID, i);
		if(wSendFlag == 1)
			break;
	}
	
	if(i>=wAINum)
	{
		ClearDevFlag(DevID, SF_HaveScanAiData);
		return FALSE;
	}
	
	nStart = i;
//    m_pCfg->yc.wStartAddr=0x4001;   //���ɹ������ų�101�ϵ��ȵ�ң����Ϣ����ʼ��ַΪ0x4000������97�汾��101��Լ

	wNo = i+m_pCfg->yc.wStartAddr;

	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}

	bNum = 0;
		
	for(i = nStart; i<wAINum; i++)
	{
		wSendFlag = GetAIFlag(DevID, i);

		if(wSendFlag != 1)//ɨ���־
			break;
		
		SetAIFlag(DevID, i, 0);//�뷢�ͱ�־
		sValue = ReadAI(DevID, i, &wStatus);

		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(sValue); //ң��ֵ Lo
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(sValue); //ң��ֵ Hi
		
		if(bStyle == 9 || bStyle == 11)
			pBuf[m_Txd.WritePtr++] = (wStatus&0xFF);

		bNum ++;

		if(bStyle == 9 || bStyle == 11)
		{
			if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-2))
				break;
		}
		else
		{
			if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-1))
				break;
		}
	}
	
	if(bNum == 0)
	{
		ClearDevFlag(DevID, SF_HaveScanAiData);
		return FALSE;
	}

	Txd_Tail(DevID, bPRM, bCode, bNum|0x80);
	
	return TRUE;
}


BOOL TxdScanAiData13(int DevID)
{
	BYTE bStyle;
	BYTE bReason = 1;
	BYTE bPRM = 0, bCode = 8;
	WORD wAINum, wNo;
	short sValue;
	int i;
	BYTE* pBuf;
	WORD wStatus;
	float* pFloat;
	DWORD dwValue;
	WORD wSendFlag;
	BYTE bNum;
	int nStart;

	bStyle = 13;//m_pCfg->yc.bStatus1; float value
	pBuf = m_Txd.Buf;

	wAINum = GetAINum(DevID);
	
	for(i=0; i<wAINum; i++)
	{
		wSendFlag = GetAIFlag(DevID, i);

		if(wSendFlag == 1)
			break;
	}
	
	if(i >= wAINum)
	{
		ClearDevFlag(DevID, SF_HaveScanAiData);
		return FALSE;
	}
	
	nStart = i;
//    m_pCfg->yc.wStartAddr=0x4001;   //���ɹ������ų�101�ϵ��ȵ�ң����Ϣ����ʼ��ַΪ0x4000������97�汾��101��Լ

	wNo = i+m_pCfg->yc.wStartAddr;
	
	pFloat = (float*)&dwValue;

	Txd_Head(DevID, bStyle, bReason);
	
	bNum = 0;
	wNo = i+m_pCfg->yc.wStartAddr;

	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}

	for(i = nStart;  i<wAINum; i++)
	{
		wSendFlag = GetAIFlag(DevID, i);

		if(wSendFlag != 0x01)//ɨ���־
			break;
		
		SetAIFlag(DevID, i, 0);//�뷢�ͱ��

		sValue = ReadAI(DevID, i, &wStatus);
		*pFloat = (float)sValue;
		
		pBuf[m_Txd.WritePtr++] = LLBYTE(dwValue); //ң��ֵ Lo
		pBuf[m_Txd.WritePtr++] = LHBYTE(dwValue); //ң��ֵ Hi
		pBuf[m_Txd.WritePtr++] = HLBYTE(dwValue); //ң��ֵ Lo
		pBuf[m_Txd.WritePtr++] = HHBYTE(dwValue); //ң��ֵ Hi
		
		pBuf[m_Txd.WritePtr++] = (wStatus&0xFF);

		bNum ++;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-4))
			break;

	}
	
	if(bNum == 0)
	{
		ClearDevFlag(DevID, SF_HaveScanAiData);
		return FALSE;
	}

	Txd_Tail(DevID, bPRM, bCode, bNum|0x80);
	
	return TRUE;
}


BOOL Txd_ProtectMr(int DevID)
{
	TSysTimeMr sysMr;
	WORD wInfo;
	WORD wNum;
	int i;

	if(!GetMrNum(DevID))
		return FALSE;
	
	if(!ReadSysTimeMr(DevID, &sysMr))
		return FALSE;
	
	IncMrReadPtr(DevID);

	wInfo = (sysMr.dwTripType & 0xFFFF);
	wNum = sizeof(Mr39Info)/sizeof(Mr39Info[0]);
	
	for(i=0; i<wNum; i++)
	{
		if(Mr39Info[i] == wInfo)
			break;
	}

	if(i < wNum)
		return Txd_Protect39(DevID, &sysMr);

	wNum = sizeof(Mr40Info)/sizeof(Mr40Info[0]);
	
	for(i=0; i<wNum; i++)
	{
		if(Mr40Info[i] == wInfo)
			break;
	}

	if(i < wNum)
		return Txd_Protect40(DevID, &sysMr);

	return Txd_Protect38(DevID, &sysMr);
}

BOOL Txd_Protect38(int DevID, TSysTimeMr* pSysMr)
{
	BYTE bReason;
	BYTE bStyle;
	BYTE bPRM;
	BYTE bCode;
	WORD wInfo;
	BYTE bQCC;
	WORD wNo;
	BYTE* pBuf;

	bReason = 3;
	bStyle = 38;
	bPRM = 0;
	bCode = 8;
	
	pBuf = m_Txd.Buf;

	if(pSysMr == NULL)
		return FALSE;
	
	wInfo = (pSysMr->dwTripType&0xFFFF);

	bQCC = (pSysMr->dwData[0]&0xFF);

	wNo = m_pCfg->mr.wStartAddr + wInfo;

	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}
	
	pBuf[m_Txd.WritePtr++] = bQCC;

	pBuf[m_Txd.WritePtr++] = (pSysMr->dwTripTime&0xFF);
	pBuf[m_Txd.WritePtr++] = (pSysMr->dwTripTime>>8);

	pBuf[m_Txd.WritePtr++] = (pSysMr->stTime.MSecond&0xFF);
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.MSecond>>8;
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.Minute;
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.Hour;
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.Day;
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.Month;
	pBuf[m_Txd.WritePtr++ ] = pSysMr->stTime.Year%100;  //MSEC H

	Txd_Tail(DevID, bPRM, bCode, 1);

	return TRUE;
}

BOOL Txd_Protect39(int DevID, TSysTimeMr* pSysMr)
{
	BYTE bReason;
	BYTE bStyle;
	BYTE bPRM;
	BYTE bCode;
	WORD wInfo;
	WORD wNum;
	BYTE bQCC;
	WORD wNo;
	BYTE* pBuf;
	int i;

	bReason = 3;
	bStyle = 39;
	bPRM = 0;
	bCode = 8;
	
	pBuf = m_Txd.Buf;

	if(pSysMr == NULL)
		return FALSE;
	
	wInfo = (pSysMr->dwTripType&0xFFFF);

	wNum = sizeof(Mr39Info)/sizeof(Mr39Info[0]);
	
	for(i=0; i<wNum; i++)
	{
		if(Mr39Info[i] == wInfo)
			break;
	}

	if(i >= wNum)
		return FALSE;

	bQCC = (1<<i);

	wNo = m_pCfg->mr.wStartAddr + wInfo;

	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}
	
	pBuf[m_Txd.WritePtr++] = bQCC;
	pBuf[m_Txd.WritePtr++] = 0;

	pBuf[m_Txd.WritePtr++] = (pSysMr->dwTripTime&0xFF);
	pBuf[m_Txd.WritePtr++] = (pSysMr->dwTripTime>>8);

	pBuf[m_Txd.WritePtr++] = (pSysMr->stTime.MSecond&0xFF);
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.MSecond>>8;
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.Minute;
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.Hour;
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.Day;
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.Month;
	pBuf[m_Txd.WritePtr++ ] = pSysMr->stTime.Year%100;  //MSEC H

	Txd_Tail(DevID, bPRM, bCode, 1);

	return TRUE;
}

BOOL Txd_Protect40(int DevID, TSysTimeMr* pSysMr)
{
	BYTE bReason;
	BYTE bStyle;
	BYTE bPRM;
	BYTE bCode;
	BYTE bQCC;
	WORD wInfo;
	WORD wNum;
	WORD wNo;
	BYTE* pBuf;
	int i;

	bReason = 3;
	bStyle = 40;
	bPRM = 0;
	bCode = 8;
	
	pBuf = m_Txd.Buf;

	if(pSysMr == NULL)
		return FALSE;
	
	wInfo = (pSysMr->dwTripType&0xFFFF);

	wNum = sizeof(Mr40Info)/sizeof(Mr40Info[0]);
	
	for(i=0; i<wNum; i++)
	{
		if(Mr40Info[i] == wInfo)
			break;
	}

	if(i >= wNum)
		return FALSE;

	bQCC = (1<<i);

	wNo = m_pCfg->mr.wStartAddr + wInfo;

	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}
	
	pBuf[m_Txd.WritePtr++] = bQCC;
	pBuf[m_Txd.WritePtr++] = 0;

	pBuf[m_Txd.WritePtr++] = (pSysMr->dwTripTime&0xFF);
	pBuf[m_Txd.WritePtr++] = (pSysMr->dwTripTime>>8);

	pBuf[m_Txd.WritePtr++] = (pSysMr->stTime.MSecond&0xFF);
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.MSecond>>8;
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.Minute;
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.Hour;
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.Day;
	pBuf[m_Txd.WritePtr++] = pSysMr->stTime.Month;
	pBuf[m_Txd.WritePtr++ ] = pSysMr->stTime.Year%100;  //MSEC H

	Txd_Tail(DevID, bPRM, bCode, 1);

	return TRUE;
}

BOOL TxdCallDir()
{
/*	BYTE bReason, bStyle, bPRM, bCode;
	WORD wNum;
	WORD wNo;
	BYTE* pBuf;
	WORD wOffset;
	TFDRTA* pFDRTA;
	POSITION1 pos;
	WORD wFileName;

	bReason = 13;
	bStyle = 126;
	bPRM = 0;
	bCode = 8;
	
	pBuf = m_Txd.Buf;

	wNo = m_pCfg->file.wStartAddr + 1;

	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}
	
	wNum = 0;
	wOffset = GetDevFlagValue(SF_FILE_OFFSET);
	wFileName = wOffset;

	while(1)
	{
		pos = m_dirList.FindIndex(wFileName);
		
		wFileName += 1;
		if(!pos)
			break;
		
		pFDRTA = (TFDRTA*)m_dirList.GetAt(pos);
		
		if(pFDRTA == NULL)
			break;

		pBuf[m_Txd.WritePtr++] = wFileName&0xFF;//file wname
		pBuf[m_Txd.WritePtr++] = (wFileName>>8);
		
		pBuf[m_Txd.WritePtr++] = (pFDRTA->dwLen&0xFF);//file len
		pBuf[m_Txd.WritePtr++] = (pFDRTA->dwLen>>8);
		pBuf[m_Txd.WritePtr++] = (pFDRTA->dwLen>>16);

		if(wFileName == m_dirList.GetCount())
			pBuf[m_Txd.WritePtr++] = pFDRTA->bSOF|0x20;
		else
			pBuf[m_Txd.WritePtr++] = pFDRTA->bSOF;
 
		pBuf[m_Txd.WritePtr++] = pFDRTA->wMSecond&0xFF;
		pBuf[m_Txd.WritePtr++] = pFDRTA->wMSecond>>8;
		pBuf[m_Txd.WritePtr++] = pFDRTA->bMinute;
		pBuf[m_Txd.WritePtr++] = pFDRTA->bHour;
		pBuf[m_Txd.WritePtr++] = pFDRTA->bDay;
		pBuf[m_Txd.WritePtr++] = pFDRTA->bMonth;
		pBuf[m_Txd.WritePtr++] = pFDRTA->wYear%100;  //MSEC H
		
		wNum ++;

		if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-12))
			break;
	}
	
	SetDevFlagValue(SF_FILE_OFFSET, wOffset+wNum);

	if(wNum == 0)
	{
		ClearDevFlag(DevID, SF_CALLDIR_FLAG);
		return FALSE;
	}

	Txd_Tail(DevID, bPRM, bCode, wNum|0x80);
*/
	return TRUE;
}

BOOL TxdFileSelect()
{
/*	BYTE bReason, bStyle, bPRM, bCode;
	BYTE* pBuf;
	TFDRTA* pFDRTA;
	POSITION1 pos;
	WORD wFileName;
	WORD wNo;
	char lzName[100];
	BYTE bSRQ;
	DWORD dwLen;

	bReason = 13;
	bStyle = 120;
	bPRM = 0;
	bCode = 8;
	

	pBuf = m_Txd.Buf;
	
	wNo = GetDevFlagValue(SF_FILEOBJ_ADDR);
	wFileName = GetDevFlagValue(SF_FILE_NAME);

	pFDRTA = NULL;


	pFDRTA = NULL;
	dwLen = 0;
	bSRQ = 0x80;//not ready;

	if(wFileName && wFileName <= m_dirList.GetCount())
	{
		pos = m_dirList.FindIndex(wFileName-1);	
		pFDRTA = (TFDRTA*)m_dirList.GetAt(pos);
		
	}

	if(pFDRTA)
	{
		strcpy(lzName, pDataPath);
		strcat(lzName, pFDRTA->lzName);

		if(stat_f(lzName, &filestat) == 0)
		{
			pFDRTA->dwLen = filestat.st_size;
			dwLen = filestat.st_size;
			bSRQ = 0;
		}
		else
			bSRQ = 0x80;
	}

	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}
	
	pBuf[m_Txd.WritePtr++] = wFileName&0xFF;//fileName;
	pBuf[m_Txd.WritePtr++] = wFileName>>8; //filename
	
	pBuf[m_Txd.WritePtr++] = dwLen&0xFF;
	pBuf[m_Txd.WritePtr++] = dwLen>>8;
	pBuf[m_Txd.WritePtr++] = dwLen>>16;
		
	pBuf[m_Txd.WritePtr++] = 0;//file ok;

	if(bSRQ == 0)
	{
		pFDRTA->bSOF = FILE_SELECT_OK;//���ڼ���״̬
		pFDRTA->dwLen = dwLen;
	}


	Txd_Tail(DevID, bPRM, bCode, 1);
*/
	return TRUE;
}

BOOL TxdFileRequest()
{
/*	BYTE bReason, bStyle, bPRM, bCode;
	BYTE* pBuf;
	TFDRTA* pFDRTA;
	POSITION1 pos;
	WORD wFileName;
	WORD wNo;
	char lzName[100];
	BYTE bSRQ;
	DWORD dwLen;

	bReason = 13;
	bStyle = 121;
	bPRM = 0;
	bCode = 8;
	

	pBuf = m_Txd.Buf;
	
	wNo = GetDevFlagValue(SF_FILEOBJ_ADDR);
	wFileName = GetDevFlagValue(SF_FILE_NAME);

	pFDRTA = NULL;
	dwLen = 0;
	bSRQ = 0x80;//not ready;

	if(wFileName && wFileName <= m_dirList.GetCount())
	{
		pos = m_dirList.FindIndex(wFileName-1);	
		pFDRTA = (TFDRTA*)m_dirList.GetAt(pos);
		
	}

	if(pFDRTA && (pFDRTA->bSOF&FILE_SELECT_OK))
	{
		strcpy(lzName, pDataPath);
		strcat(lzName, pFDRTA->lzName);

		if(stat_f(lzName, &filestat) == 0)
		{
			pFDRTA->dwLen = filestat.st_size;
			dwLen = filestat.st_size;
			bSRQ = 0;
		}
		else
			bSRQ = 0x80;
	}

	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}
	
	pBuf[m_Txd.WritePtr++] = wFileName&0xFF;//fileName;
	pBuf[m_Txd.WritePtr++] = wFileName>>8; //filename
	
	pBuf[m_Txd.WritePtr++] = 1;//section name : only one

	pBuf[m_Txd.WritePtr++] = (dwLen&0xFF);
	pBuf[m_Txd.WritePtr++] = dwLen>>8;
	pBuf[m_Txd.WritePtr++] = dwLen>>16;
		
	pBuf[m_Txd.WritePtr++] = bSRQ;//frq

	if(bSRQ == 0)
		pFDRTA->bSOF |= FILE_SECTION_OK;//section ���ڼ���״̬

	Txd_Tail(DevID, bPRM, bCode, 1);
*/
	return TRUE;
}

BOOL TxdFileSending() 
{
/*
	BYTE bReason, bStyle, bPRM, bCode;
	BYTE* pBuf;
	TFDRTA* pFDRTA;
	POSITION1 pos;
	WORD wFileName;
	WORD wNo;
	char lzName[100];
	BYTE bSRQ;
	DWORD dwLen;
	DWORD dwOffset;
	BYTE bMaxLen;
	WORD wSendLen = 0;

	bReason = 13;
	bStyle = 125;
	bPRM = 0;
	bCode = 8;
	
	pBuf = m_Txd.Buf;
	
	wNo = GetDevFlagValue(SF_FILEOBJ_ADDR);
	wFileName = GetDevFlagValue(SF_FILE_NAME);
	dwOffset = GetDevFlagValue(SF_FILE_OFFSET);

	pFDRTA = NULL;
	dwLen = 0;
	bSRQ = 0x80;//not ready;

	if(wFileName && wFileName <= m_dirList.GetCount())
	{
		pos = m_dirList.FindIndex(wFileName-1);	
		pFDRTA = (TFDRTA*)m_dirList.GetAt(pos);
		
	}

	if(pFDRTA)
	{
		strcpy(lzName, pDataPath);
		strcat(lzName, pFDRTA->lzName);

		if(stat_f(lzName, &filestat) == 0)
		{
			pFDRTA->dwLen = filestat.st_size;
			dwLen = filestat.st_size;
			bSRQ = 0;
		}
		else
			bSRQ = 0x80;
	}

	if(bSRQ != 0)
		return FALSE;

	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}
	
	pBuf[m_Txd.WritePtr++] = wFileName&0xFF;//fileName;
	pBuf[m_Txd.WritePtr++] = wFileName>>8; //filename
	
	pBuf[m_Txd.WritePtr++] = 1;//section name : only one

	bMaxLen = m_pCfg->wMaxFrm - m_Txd.WritePtr - TAILLEN;
//	bSendLen = ReadFile(lzName, dwOffset, bMaxLen);

	fReadFile(lzName, dwOffset, bMaxLen, pBuf+m_Txd.WritePtr+1, &wSendLen, FILE_SYNC);
	
	pBuf[m_Txd.WritePtr++] = wSendLen;

	m_Txd.WritePtr += wSendLen;

	SetDevFlagValue(SF_FILE_OFFSET, dwOffset+wSendLen);

	if((wSendLen < bMaxLen) || (dwOffset+wSendLen) == dwLen)
	{
		ClearDevFlag(DevID, SF_FILE_SENDING);
		SetDevFlag(SF_LAST_SEGMENT);
	}

	Txd_Tail(DevID, bPRM, bCode, 1);
*/
	return TRUE;
}

BOOL TxdLastSegment()
{
/*
	BYTE bReason, bStyle, bPRM, bCode;
	BYTE* pBuf;
	TFDRTA* pFDRTA;
	POSITION1 pos;
	WORD wFileName;
	WORD wNo;
	char lzName[100];
	BYTE bSRQ;
	BYTE bChkSum;
	DWORD dwOffset;
	WORD wReadLen;
	DWORD dwRtn;
	BYTE bReadChar;

	bReason = 13;
	bStyle = 123;
	bPRM = 0;
	bCode = 8;
	dwOffset = 0;
	
	pBuf = m_Txd.Buf;
	
	wNo = GetDevFlagValue(SF_FILEOBJ_ADDR);
	wFileName = GetDevFlagValue(SF_FILE_NAME);

	pFDRTA = NULL;

	if(wFileName && wFileName <= m_dirList.GetCount())
	{
		pos = m_dirList.FindIndex(wFileName-1);	
		pFDRTA = (TFDRTA*)m_dirList.GetAt(pos);
		
	}

	if(pFDRTA)
	{
		strcpy(lzName, pDataPath);
		strcat(lzName, pFDRTA->lzName);
	}
	else
		return 0;
		
	bChkSum = 0;

	while(1)
	{
		dwRtn = fReadFile(lzName, dwOffset, 1, &bReadChar, &wReadLen, FILE_SYNC);
		if(dwRtn == FILE_ERROR)
			break;
		bChkSum += bReadChar;
		dwOffset ++;
	}

	Txd_Head(DevID, bStyle, bReason);
	
	if(m_pCfg->bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
	else if(m_pCfg->bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //��Ϣ���ַLo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //��Ϣ���ַHi
	}
	
	pBuf[m_Txd.WritePtr++] = wFileName&0xFF;//fileName;
	pBuf[m_Txd.WritePtr++] = wFileName>>8; //filename
	
	pBuf[m_Txd.WritePtr++] = 1;//section name : only one
	pBuf[m_Txd.WritePtr++] = bChkSum;

	Txd_Tail(DevID, bPRM, bCode, 1);
*/
	return TRUE;
}


//==============================================
//  ��������: ���մ���
//==============================================
static int RxdMonitor(int DevID, uint8_t *buf, uint16_t len)
{


	uint8_t *BufTemp = NULL;
	int16_t LenRemain,LenTmp;
	int LenHeadAndTail = 6;//����ͷ����4�ֽںͱ���β2�ֽ�
	
	if(buf == NULL){
		LOG("-%s-,buffer (null)",__FUNCTION__);
		return RET_ERROR;
	}
	if(len <= 0 || len>256){
		LOG("-%s-,buffer len error(%d) \n",__FUNCTION__,len);
		log("devid(%d)	fd(%d)\n",DevID, gpDevice[DevID].fd);
		return RET_ERROR;
	}

	BufTemp = buf;
	LenRemain = len;
	while(BufTemp<buf+len){
//		LOG("-%s-,LINE %s\n",__FUNCTION__,__LINE__);
		if(BufTemp[0] == 0x10){
			LenTmp = 6;
			if(LenRemain < LenHeadAndTail){
				perror("_%s_,len error(%d) \n",__FUNCTION__,len);
				return RET_ERROR;
			}
			RxdFixFrame(DevID, buf);
		}else if(BufTemp[0] == 0x68){
			LenTmp = BufTemp[1] + LenHeadAndTail;
			if(BufTemp[1]!=BufTemp[2]){
				perror("_%s_,len error(%d)(%d) \n",__FUNCTION__,BufTemp[1],BufTemp[2]);
				return RET_ERROR;
			}
			RxdVarFrame(DevID, buf);
		}else{
			return RET_ERROR;
		}
		BufTemp+=LenTmp;
		LenRemain-=LenTmp;
	}
	return RET_SUCESS;

}


//=========================================================================
// ��������: ���Buf���Ƿ���һ����Ч����
// �������: Buf:Ҫ���ı�����ʼ��ַ
//           Len:Buf����Ч����
// ����ֵ: 
//          FM_OK +ʵ�ʱ��ĳ��� //��⵽һ��������֡
//          FM_ERR +�Ѵ����ֽ��� //��⵽һ��У������֡
//          FM_LESS+�Ѵ����ֽ��� //��⵽һ����������֡����δ���룩
// ע:�Ѵ����ֽ�����Ϊ0���´���������һ��ַ��ʼ,�����´������������Ѵ����
//      ���ֽ�,�Ӷ��ӿ������ٶ�
//=========================================================================
static DWORD SearchOneFrame(BYTE* pBuf, short nLen)
{
    WORD wFrameLen;
	BYTE bChkSum;
	BYTE bChkLen;
	WORD wAddr;

	if(nLen < m_wMinFrmLen)
		return FM_LESS;
	
	if(pBuf == NULL)
		return (FM_ERR|1);

	switch(pBuf[0])
	{
	case 0x10:

		if(m_pCfg->bLinkAddr == 2)//��·���ַ����2
		{
			bChkLen = 3;//��·���ַ����2
			wFrameLen = 6;
	
			if(pBuf[5] != 0x16)
				return FM_ERR|1;	 
			
			if(wFrameLen > nLen)
				return FM_LESS;
			
			wAddr = MAKEWORD(pBuf[2], pBuf[3]);

			bChkSum = ChkSum(pBuf+1, bChkLen);
		
			if(pBuf[4] != bChkSum)
				return (FM_ERR|1);	 

//			if(SwitchToAddress(wAddr) != TRUE)
//				return (FM_ERR|wFrameLen);
		
			return (FM_OK|wFrameLen);
		}
		else//��·���ַ����1
		{
			bChkLen = 2;//��·���ַ����1
			wFrameLen = 5;
	
			if(pBuf[4] != 0x16)
				return (FM_ERR|1);	 
			
			if(wFrameLen > nLen)
				return FM_LESS;
			
			wAddr = pBuf[2];

			bChkSum = ChkSum(pBuf+1, bChkLen);
		
			if(pBuf[3] != bChkSum)
				return (FM_ERR|1);	 

//			if(SwitchToAddress(wAddr) != TRUE)
//				return (FM_ERR|wFrameLen);
		
			return (FM_OK|wFrameLen);
		}
	case 0x68:
		 if(pBuf[1] != pBuf[2])
			 return (FM_ERR|1);
		 if(pBuf[3] != 0x68)
			 return (FM_ERR|1);
		 
		 wFrameLen = pBuf[1] + 6;
		 		 		
		 if(wFrameLen > nLen)
			 return FM_LESS;

		 if(pBuf[wFrameLen-1] != 0x16)
			 return (FM_ERR|1);
		 
		 bChkSum = ChkSum(pBuf+4, pBuf[1]);
		 
		 if(pBuf[wFrameLen-2] != bChkSum)
			 return (FM_ERR|1);
		 
		 if(m_pCfg->bLinkAddr == 2)//��·���ַ����2
			 wAddr = MAKEWORD(pBuf[5], pBuf[6]);
		 else
			 wAddr = pBuf[5];

//		 if(SwitchToAddress(wAddr) != TRUE)
//			 return (FM_ERR|wFrameLen);
	 
		 return (FM_OK|wFrameLen);

	default:
		return (FM_ERR|1);
	}
}



BOOL ChkVarControl(int DevID, BYTE bCode)
{
	BYTE bCode1;

	bCode1 = (bCode&0x0F);
	
	if(bCode1 == 4)//no answer
		return FALSE;
	if(bCode1 == 3)
		return TRUE;

	TxdFixFrame(DevID, 0, 15);//����δ������

	return FALSE;
}


BOOL RxdResetLink(int DevID, uint8_t *pBuf)//0x00 ��λԶ����·
{
	BYTE bControl;

	bControl = pBuf[1];

	if(bControl & BIT_PRM)
	{
		SetDevFlag(DevID, SF_RtuInitOk);   //��RTU��ʼ������״̬

		if(m_wSelfInit == 0x55aa)
		{
			SetDevFlag(DevID, SF_SendInitFlag);//��λ��Ҫ���ͳ�ʼ������֡
			m_wSelfInit = 0xaa55;
		}
	}
	
	ACK(DevID);

	return TRUE;
}

BOOL RxdReqLinkStatus(int DevID)//0x09 ����Զ����·״̬
{
	TxdFixFrame(DevID, 0, 0x0B);

	return TRUE;
}

BOOL RxdClass1Data(int DevID)//�ٻ�һ���û�����
{
	if(Txd_Class1(DevID))
		return TRUE;
		
	Txd_NoData(DevID);
//	Txd_NoData_E5(DevID);

	return TRUE;
}

BOOL RxdClass2Data(int DevID)//Զ����·״̬��û��ٻ������û�����
{
	if(Txd_Class2(DevID))
		return TRUE;
	
	if(Txd_Class1(DevID))
		return TRUE;

	Txd_NoData(DevID);
//	Txd_NoData_E5(DevID);

	return TRUE;
}


BOOL RxdTestLink(int DevID)//������·
{
	MakeMirror(7);//����ԭ�򲻱�

	ACK(DevID);
	
	return TRUE;
}

static BOOL ResetProcess(int DevID, uint8_t *pBuf)
{
	WORD wReason;
	BYTE *pData;
	BYTE bGrp;
	TSysTimeDBISoe sysDSoe; 
	TSysTimeSoe sysSoe; 
	TSysTimeMr sysMr;
	TSysTimeAit sysAiSoe;
	WORD wAddr;

	pData = pBuf + 8;

	if(m_pCfg->bLinkAddr == 2)
		pData += 1;
	
	if(m_pCfg->bCauseTrans == 2)
		wReason = MAKEWORD(pData[0],pData[1]);
	else
		wReason = pData[0];
	
	pData += m_pCfg->bCauseTrans;
	pData += m_pCfg->bAsduAddr;

	if(m_pCfg->bInfoAddr == 1)
		wAddr = pData[0];
	else
		wAddr = MAKEWORD(pData[0], pData[1]);

	pData += m_pCfg->bInfoAddr;
	
	if(wReason != 6)//δ֪�Ĵ���ԭ��
	{
		MakeMirror(45|0x40);
		ACK(DevID);
		return FALSE;
	}
	if(wAddr != 0)
	{
		MakeMirror(47|0x40);
		ACK(DevID);
		return FALSE;
	}

	bGrp = pData[0];
	
	MakeMirror(7);

	if(bGrp == 2)//���¼�������
	{
//		while(ReadSysTimeSoe(DevID, &sysSoe))//��SOE������
//			IncSoeReadPtr(DevID);
//
//		while(ReadSysTimeDBISoe(DevID, &sysDSoe))//��˫��SOE������
//			IncDBISoeReadPtr(DevID);
//
//		while(ReadSysTimeMr(DevID, &sysMr))//�屣����Ϣ������
//			IncMrReadPtr(DevID);
//
//		while(ReadSysTimeAit(DevID, &sysAiSoe))
//			IncAitReadPtr(DevID);
//
	}
	else if(bGrp == 1)//reset rtu
	{
		SetDevFlag(DevID, SF_RESET_PROCESS);
	}

	ACK(DevID);
	
	return TRUE;
}


static BOOL RxdDelay(int DevID, uint8_t *pBuf)
{
	WORD wReason;
	BYTE *pData;
	
	pData = pBuf + 8;

	if(m_pCfg->bLinkAddr == 2)
		pData += 1;
	
	if(m_pCfg->bCauseTrans == 2)
		wReason = MAKEWORD(pData[0],pData[1]);
	else
		wReason = pData[0];
	
	pData += m_pCfg->bCauseTrans;
	pData += m_pCfg->bAsduAddr;
	pData += m_pCfg->bInfoAddr;
	
	if(wReason == 6)
	{
//		ReadAbsTime(&m_absTime1);
//		m_wFromTime = MAKEWORD(pData[0],pData[1]);
		SetDevFlag(DevID, SF_CallDelayFlag);	
	}
	else if(wReason == 3)
	{
//		m_wDelayTime = MAKEWORD(pData[0],pData[1]);
	}
	else
	{
		MakeMirror(45|0x40);
		ACK(DevID);
		return FALSE;
	}
	ACK(DevID);
	
	return TRUE;
}


BOOL RxdSetPara3(int DevID, uint8_t *pBuf)//���ò��� �̸�����
{
	BYTE* pData;
	WORD wNo;
	short sValue;
	DWORD dwValue;
	BYTE bStatus;
	BYTE bType;
	BYTE bReason;
	BYTE bParaErr;
	float* pFloat;
	
	bParaErr = 0;
	pFloat = (float*)&dwValue;

	
	if(m_pCfg->bLinkAddr == 2)
	{
		bType = pBuf[7];
		bReason = pBuf[9];
	}
	else
	{
		bType = pBuf[6];
		bReason = pBuf[8];
	}

	if(bReason != 6)
	{
		MakeMirror(45|0x40);
		ACK(DevID);

		return FALSE;
	}

	pData = pBuf + 10;
	
	if(m_pCfg->bLinkAddr == 2)
		pData += 1;
	if(m_pCfg->bAsduAddr == 2)
		pData += 1;
	if(m_pCfg->bCauseTrans == 2)
		pData += 1;
	
//	m_bTmpBuf[0] = bType;
//	m_bTmpBuf[1] = pData[0];
//	m_bTmpBuf[2] = pData[1];
//	m_bTmpBuf[3] = pData[2];
//	m_bTmpBuf[4] = pData[3];
//	m_bTmpBuf[5] = pData[4];
//	m_bTmpBuf[6] = pData[5];
//	m_bTmpBuf[7] = pData[6];
//	m_bTmpBuf[8] = pData[7];
	
	if(m_pCfg->bInfoAddr == 1)
	{
		wNo = pData[0];
		bStatus = (pData[5] & 0x3F);
		pData += 1;
	}
	if(m_pCfg->bInfoAddr == 2)
	{
		wNo = MAKEWORD(pData[0],pData[1]);
		bStatus = (pData[6] & 0x3F);
		pData += 2;
	}
	if(m_pCfg->bInfoAddr == 3)
	{
		wNo = MAKEWORD(pData[0],pData[1]);
		bStatus = (pData[7] & 0x3F);
		pData += 3;
	}

	if(wNo < m_pCfg->yc.wStartAddr)
	{
		MakeMirror(47|0x40);
		ACK(DevID);
		return FALSE;
	}
	
	wNo -= m_pCfg->yc.wStartAddr;

	if(wNo >= GetAINum(DevID))
	{
		MakeMirror(47|0x40);
		ACK(DevID);
		return FALSE;
	}
	
	dwValue = MAKEDWORD(pData[0], pData[1], pData[2], pData[3]);
	sValue = (short)(*pFloat);


	if(bStatus == 1)//�������ż�ֵ
	{
//		if(*pFloat > 2000 || *pFloat<0)		
//			bParaErr = 1;
//		else
//			SetSQZ(DevID, wNo, sValue);
//		SetSQZToFile(DevID,wNo,LOWORD(dwValue));
	}
	else if(bStatus == 2)//�˲�ϵ��
	{
//		if(*pFloat > 1 || *pFloat < 0)
//			bParaErr = 1;
//		else
//			SetCOE(DevID, bStatus, wNo, (DWORD)(*pFloat*100));
	}
	else
		bParaErr = 1;

	if(bParaErr)
		MakeMirror(7|0x40);
	else
		MakeMirror(7);
	
//	SetDevFlag(DevID, SF_SetParaFlag);
	ACK(DevID);
	
	//TxdSetPara();
	return TRUE;
}


BOOL RxdSetPara2(int DevID, uint8_t *pBuf)//���ò��� scale
{
	BYTE* pData;
	WORD wNo;
	short sValue;
	DWORD dwValue;
	BYTE bStatus;
	BYTE bType;
	BYTE bReason;
	BYTE bParaErr;

	bParaErr = 0;


	pData = pBuf + 10;

	if(m_pCfg->bLinkAddr == 2)
	{
		bType = pBuf[7];
		bReason = pBuf[9];
	}
	else
	{
		bType = pBuf[6];
		bReason = pBuf[8];
	}
	
	if(bReason != 6)
	{
		MakeMirror(45|0x40);
		ACK(DevID);
		return FALSE;
	}

	if(m_pCfg->bLinkAddr == 2)
		pData += 1;
	if(m_pCfg->bAsduAddr == 2)
		pData += 1;
	if(m_pCfg->bCauseTrans == 2)
		pData += 1;
	
	if(m_pCfg->bInfoAddr == 1)
	{
		wNo = pData[0];
		sValue = MAKEWORD(pData[1], pData[2]);
		bStatus = (pData[3] & 0x3F);
	}
	if(m_pCfg->bInfoAddr == 2)
	{
		wNo = MAKEWORD(pData[0],pData[1]);
		sValue = MAKEWORD(pData[2], pData[3]);
		bStatus = (pData[4] & 0x3F);
	}
	if(m_pCfg->bInfoAddr == 3)
	{
		wNo = MAKEWORD(pData[0],pData[1]);
		sValue = MAKEWORD(pData[3], pData[4]);
		bStatus = (pData[5] & 0x3F);
	}
	
	if(wNo < m_pCfg->yc.wStartAddr)
	{
		MakeMirror(47|0x40);
		ACK(DevID);
		return FALSE;
	}
	
	wNo -= m_pCfg->yc.wStartAddr;

	if(wNo >= GetAINum(DevID))
	{
		MakeMirror(47|0x40);
		ACK(DevID);
		return FALSE;
	}
//	pData = &pRxdFm->varFm.bData;
//	wNo = MAKEWORD(pData[0],pData[1]) - m_pCfg->yc.wStartAddr;
//	sValue = MAKEWORD(pData[2], pData[3]);
//	bStatus = (pData[4] & 0x3F);

	if(bStatus == 1)//�������ż�ֵ
	{
//		dwValue = abs(sValue);
		
//		if(dwValue >= 2000)
//			bParaErr = 1;
//		else
//			SetSQZ(DevID, wNo, dwValue);
	}
	else if(bStatus == 2)//�˲�ϵ��
	{
//		dwValue = abs(sValue);
//		
//		if(dwValue > 100)
//			bParaErr = 1;
//		else
//			SetCOE(DevID, 2, wNo, dwValue);
	}
	else
		bParaErr = 1;
	
	ACK(DevID);
	
	if(bParaErr)
		MakeMirror(7|0x40);
	else
		MakeMirror(7);

//	SetDevFlag(DevID, SF_SetParaFlag);
	//TxdSetPara();
	
//	m_bTmpBuf[0] = bType;
//	m_bTmpBuf[1] = pData[0];
//	m_bTmpBuf[2] = pData[1];
//	m_bTmpBuf[3] = pData[2];
//	m_bTmpBuf[4] = pData[3];
//	m_bTmpBuf[5] = pData[4];
//	m_bTmpBuf[6] = pData[5];

	return TRUE;
}


BOOL RxdSetPara(int DevID, uint8_t *pBuf)//���ò���normalize value
{
	BYTE* pData;
	WORD wNo;
	short sValue;
	DWORD dwValue;
	BYTE bStatus;
	BYTE bType;
	BYTE bReason;
	BOOL bParaError;

	bParaError = 0;


	pData = pBuf + 10;

	if(m_pCfg->bLinkAddr == 2)
	{
		bType = pBuf[7];
		bReason = pBuf[9];
	}
	else
	{
		bType = pBuf[6];
		bReason = pBuf[8];
	}

	if(bReason != 6)
	{
		MakeMirror(45|0x40);
		ACK(DevID);
		return FALSE;
	}

	if(m_pCfg->bLinkAddr == 2)
		pData += 1;
	if(m_pCfg->bAsduAddr == 2)
		pData += 1;
	if(m_pCfg->bCauseTrans == 2)
		pData += 1;
	
	if(m_pCfg->bInfoAddr == 1)
	{
		wNo = pData[0];
		sValue = MAKEWORD(pData[1], pData[2]);
		bStatus = (pData[3] & 0x3F);
	}
	if(m_pCfg->bInfoAddr == 2)
	{
		wNo = MAKEWORD(pData[0],pData[1]);
		sValue = MAKEWORD(pData[2], pData[3]);
		bStatus = (pData[4] & 0x3F);
	}
	if(m_pCfg->bInfoAddr == 3)
	{
		wNo = MAKEWORD(pData[0],pData[1]);
		sValue = MAKEWORD(pData[3], pData[4]);
		bStatus = (pData[5] & 0x3F);
	}
	
	if(wNo < m_pCfg->yc.wStartAddr)
	{
		MakeMirror(47|0x40);
		ACK(DevID);
		return FALSE;
	}
	
	wNo -= m_pCfg->yc.wStartAddr;

	if(wNo >= GetAINum(DevID))
	{
		MakeMirror(47|0x40);
		ACK(DevID);
		return FALSE;
	}
//	pData = &pRxdFm->varFm.bData;
//	wNo = MAKEWORD(pData[0],pData[1]) - m_pCfg->yc.wStartAddr;
//	sValue = MAKEWORD(pData[2], pData[3]);
//	bStatus = (pData[4] & 0x3F);

	if(bStatus == 1)//�������ż�ֵ
	{
//		dwValue = (DWORD)abs(sValue)*1000/0x8000;

//		if(dwValue >= 500)
//			bParaError = 1;
//		else
//			SetSQZ(DevID, wNo, dwValue|0x8000);
	}
	else if(bStatus == 2)//�˲�ϵ��
	{
//		dwValue = (DWORD)abs(sValue)*100/0x8000;
//		SetCOE(DevID, 2, wNo, dwValue);
	}
	else
		bParaError = 1;

	ACK(DevID);
	
	if(bParaError)
		MakeMirror(7|0x40);//��������
	else
		MakeMirror(7);
//	SetDevFlag(DevID, SF_SetParaFlag);
//	m_bTmpBuf[0] = bType;
//	m_bTmpBuf[1] = pData[0];
//	m_bTmpBuf[2] = pData[1];
//	m_bTmpBuf[3] = pData[2];
//	m_bTmpBuf[4] = pData[3];
//	m_bTmpBuf[5] = pData[4];
//	m_bTmpBuf[6] = pData[5];
	
	return TRUE;
}

static BOOL RxdClockSyn(int DevID, uint8_t *pBuf)//ʱ��ͬ��
{
	BYTE* pData;
	TSysTime sysTime;	
	WORD wMSecond;
	TAbsTime absTime;
	TAbsTime beforSynTime;
	BYTE bReason;
	int nDiffer;
	DWORD dwControl = 0;
	
	//zym add �ж��Ƿ���ոö˿ڵĶ�ʱ����
	dwControl = gpDevice[DevID].dwControl;
	dwControl &= 0x700;
	if(dwControl==0x100)    //��ֹ����
		return FALSE;
	//end

	pData = pBuf + 10;

	if(m_pCfg->bLinkAddr == 2)
		bReason = pBuf[9];
	else
		bReason = pBuf[8];
	
	if(bReason != 6)
	{
		MakeMirror(45|0x40);
		ACK(DevID);
		return FALSE;
	}

	if(m_pCfg->bLinkAddr == 2)
		pData += 1;
	if(m_pCfg->bAsduAddr == 2)
		pData += 1;
	if(m_pCfg->bCauseTrans == 2)
		pData += 1;
	
	if(m_pCfg->bInfoAddr == 1)
		pData += 1;
	if(m_pCfg->bInfoAddr == 2)
		pData += 2;
	if(m_pCfg->bInfoAddr == 3)
		pData += 3;

//	ReadAbsTime(&beforSynTime);

	wMSecond = MAKEWORD(pData[0], pData[1]);
	sysTime.MSecond = wMSecond;
	sysTime.Minute = pData[2];
	sysTime.Hour = pData[3];
	sysTime.Day = (pData[4]&0x1F);
	sysTime.Month = (pData[5]&0x0F);
	sysTime.Year = (pData[6]&0x7F)+2000;
	
//	ToAbsTime(&sysTime, &absTime);

//	absTime.Lo += m_wDelayTime;

//	if(absTime.Lo<(DWORD)m_wDelayTime)
//		absTime.Hi += 1;

//	WriteAbsTime(&absTime);

	nDiffer = absTime.Lo - beforSynTime.Lo;

//	if(abs(nDiffer) > 3000)
//		SetDevFlag(DevID, SF_ClockDifferFlag);

	SetDevFlag(DevID, SF_ClockSynFlag);

	ACK(DevID);

//	TxdClockSyn();//���ӵ�ȷ��
	
	return TRUE;
}

static BOOL RxdCallAll(int DevID)//���ٻ�/�ٻ�ĳһ������
{
	SetALLDataFlag(DevID);
	
	ACK(DevID);
	

	return TRUE;
}

BOOL RxdCallCI(int DevID, uint8_t *pBuf)
{
	BYTE bQcc;
	BYTE bCmd;
	BYTE bGroup;
	BYTE *pData;
	WORD wReason;
	int i;
	

//	memcpy(m_bMirrorBuf,m_RxdFrame.Buf,(m_RxdFrame.Buf[1]+6));
//	m_bSendMirrorFlag = true;
	
	pData = pBuf+8;

	if(m_pCfg->bLinkAddr == 2)
		pData ++;
	
	if(m_pCfg->bCauseTrans == 2)
	{
		wReason = MAKEWORD(pData[0], pData[1]);
	}
	else
		wReason = pData[0];
	
	if(wReason != 6)
	{
		MakeMirror(45|0x40);
		ACK(DevID);
		return FALSE;
	}
	
	pData += m_pCfg->bCauseTrans;
	pData += m_pCfg->bAsduAddr;
	pData += m_pCfg->bInfoAddr;
	
	bQcc = pData[0];
	bCmd = bQcc>>6;
	bGroup = bQcc&0x3F;
	
	if(bCmd == 1)//�ۼ�ֵ
		FreezeCI(DevID, TRUE);
	if(bCmd == 2)//����
		FreezeCI(DevID, FALSE);
	
	if(bCmd == 0)//��
	{
		if(bGroup == 1)
			SetDevFlag(DevID, SF_CALLCIFLAG0);
		if(bGroup == 2)
			SetDevFlag(DevID, SF_CALLCIFLAG0+1);
		if(bGroup == 3)
			SetDevFlag(DevID, SF_CALLCIFLAG0+2);
		if(bGroup == 4)
			SetDevFlag(DevID, SF_CALLCIFLAG0+3);
		if(bGroup == 5)
		{
			for(i=0; i<GRPNUMCI; i++)
				SetDevFlag(DevID, SF_CALLCIFLAG0 + i);
		}
		SetDevFlag(DevID, SF_ReadCIFlag);
	}
	
	MakeMirror(7);//��Ӧ�ٻ���ȱ�־
	
//	TxdMirror();

	ACK(DevID);

	return TRUE;
}


BOOL RxdDOCmd(int DevID, uint8_t *pBuf)
{
	WORD wCmd = 0;
	WORD wNo;
	BYTE* pData;
	TDO* pDO;
	DWORD dwValue;
	BYTE bReason;


//	pDO = pGetDO();
	
	ClearDevFlag(DevID, SF_HaveDOReturn);

	if(pDO == NULL)
	{
		ACK(DevID);
//		SetDOReturn(PAORET_FAIL);
		return FALSE;	
	}

	if(m_pCfg->bLinkAddr == 2)
		bReason = pBuf[9];
	else
		bReason = pBuf[8];
	
	if(bReason != 6 && bReason != 8)
	{
		MakeMirror(45|0x40);
		ACK(DevID);
		return FALSE;
	}

	
	pData = pBuf + 10;

	if(m_pCfg->bLinkAddr == 2)
		pData += 1;
	if(m_pCfg->bAsduAddr == 2)
		pData += 1;
	if(m_pCfg->bCauseTrans == 2)
		pData += 1;
	


	if(m_pCfg->bInfoAddr == 1)
	{
		wNo = pData[0] - m_pCfg->bdo.wStartAddr;
		dwValue = MAKEDWORD(pData[1], pData[2], pData[3], pData[4]);
	}

	if(m_pCfg->bInfoAddr == 2)
	{
		wNo = MAKEWORD(pData[0],pData[1]) - m_pCfg->bdo.wStartAddr;
		dwValue = MAKEDWORD(pData[2], pData[3], pData[4], pData[5]);
	}
	if(m_pCfg->bInfoAddr == 3)
	{
		wNo = MAKEWORD(pData[0],pData[1]) - m_pCfg->bdo.wStartAddr;
		dwValue = MAKEDWORD(pData[3], pData[4], pData[5], pData[6]);
	}
	
//	wCmd = TP_DODIRECT;	


	pDO->Info[0] = pData[0];
	pDO->Info[1] = pData[1];
	pDO->Info[2] = pData[2];
	pDO->Info[3] = pData[3];
	pDO->Info[4] = pData[4];
	pDO->Info[5] = pData[5];
	pDO->Info[6] = pData[6];

//	SetDO(DevID, wCmd, wNo, dwValue, 0, 0, 0);
	
	ACK(DevID);

	return TRUE;
}

BOOL RxdYtCmd(int DevID, uint8_t *pBuf)
{
	WORD wCmd = 0;
	WORD wAONo;
	BYTE bType;
	BYTE bReason;
	BYTE* pData;
	TAO* pAO;
	short sValue;
	BYTE bStatus;
	TRealAO* pRealAO;
	DWORD dwValue;
	float* pFloat;
	BYTE bParaErr;
		
//	pAO = pGetAO();
	bParaErr = 0;
	pFloat = (float*)&dwValue;
	
	ClearDevFlag(DevID, SF_HaveAOReturn);

	if(pAO == NULL)
	{
//		SetAOReturn(PAORET_FAIL);
		return FALSE;	
	}
	
	if(m_pCfg->bLinkAddr == 2)
	{
		bType = pBuf[7];
		bReason = pBuf[9];
	}
	else
	{
		bType = pBuf[6];
		bReason = pBuf[8];
	}
	
	if((bReason!=6)&&(bReason!=8))
	{
		MakeMirror(45|0x40);
		ACK(DevID);
		return FALSE;
	}


	pData = pBuf + 10;
	
	if(m_pCfg->bLinkAddr == 2)
		pData += 1;
	if(m_pCfg->bAsduAddr == 2)
		pData += 1;
	if(m_pCfg->bCauseTrans == 2)
		pData += 1;
	
	if(m_pCfg->bInfoAddr == 1)
	{
		wAONo = pData[0];
		
		if(bType == 48 || bType == 49)
		{
			sValue = MAKEWORD(pData[1], pData[2]);

			if(sValue > 4095 || sValue<0)
				bParaErr = 1;

			bStatus = pData[3];
		}
		
		if(bType == 50)//
		{
			dwValue = MAKEDWORD(pData[1], pData[2], pData[3], pData[4]);

			if(*pFloat<0 || *pFloat>4095)
				bParaErr = 1;

			sValue = (short)*pFloat;
			bStatus = pData[5];

		}
	}
	else if(m_pCfg->bInfoAddr == 3)
	{
		wAONo = MAKEWORD(pData[0],pData[1]);
		
		if(bType == 48 || bType == 49)
		{
			sValue = MAKEWORD(pData[3], pData[4]);

			if(sValue > 4095 || sValue < 0)
				bParaErr = 1;

			bStatus = pData[5];
		}
		if(bType == 50)
		{
			dwValue = MAKEDWORD(pData[3], pData[4], pData[5], pData[6]);

			if(*pFloat<0 || *pFloat>4095)
				bParaErr = 1;

			sValue = (short)*pFloat;
			bStatus = pData[7];
		}
	}
	else//(m_pCfg->bInfoAddr == 2)
	{
		wAONo = MAKEWORD(pData[0],pData[1]);
		
		if(bType == 48 || bType == 49)
		{
			sValue = MAKEWORD(pData[2], pData[3]);
			
			if(sValue > 4095 || sValue<0)
				bParaErr = 1;

			bStatus = pData[4];
		}
		if(bType == 50)
		{
			dwValue = MAKEDWORD(pData[2], pData[3], pData[4], pData[5]);
		
			if(*pFloat<0 || *pFloat>4095)
				bParaErr = 1;
			
			sValue = (short)*pFloat;
			bStatus = pData[6];
		}
	}
	
	if(wAONo < m_pCfg->yt.wStartAddr)
	{
		MakeMirror(47|0x40);
		ACK(DevID);
		return FALSE;
	}
	
	wAONo -= m_pCfg->yt.wStartAddr;

	if(bParaErr)
	{
		if(bReason == 6)
			MakeMirror(7|0x40);
		if(bReason == 8)
			MakeMirror(9|0x40);
		ACK(DevID);

		return FALSE;
	}

	pAO->Info[0] = bType;
	pAO->Info[1] = pData[0];
	pAO->Info[2] = pData[1];
	pAO->Info[3] = pData[2];
	pAO->Info[4] = pData[3];
	pAO->Info[5] = pData[4];
	pAO->Info[6] = pData[5];
	pAO->Info[7] = pData[6];
	pAO->Info[8] = pData[7];

	if(bType == 48 || bType == 49 || bType == 50)//
	{
		if(bReason == 8)
			wCmd = TP_AOCANCEL;
		if(bReason == 6)
		{
			if(bStatus&0x80)
				wCmd = TP_AOSELECT;
			else 
				wCmd = TP_AOOPERATE;
		}
	}
	
//	pRealAO = GetYtPara(DevID, wAONo);
	
	if(pRealAO == NULL)
	{
//		SetAOReturn(PAORET_FAIL);
		ACK(DevID);
		return FALSE;
	}
	
	if(pRealAO->bResetTime == 0)//����ң��
	{
		if(wCmd != TP_AOOPERATE)
		{
//			SetAOReturn(PAORET_FAIL);
			ACK(DevID);
			return FALSE;
		}
		wCmd = TP_AODIRECT;
	}

//	SetAO(DevID, wCmd, wAONo, sValue);
	
	ACK(DevID);

	return TRUE;
}

BOOL RxdTqCmd(int DevID)
{
/*	WORD wTQCmd = 0;
	BYTE bTQS;
	WORD wControl;
	WORD wTQNo;
	BYTE bReason;
	BYTE* pData;
	TTQ* pTQ;
	
	pTQ = pGetTQ();
	
	if(pTQ == NULL)
	{
		NACK(DevID);
		return TRUE;
	}
	
	bReason = pRxdFm->varFm.bReason;
			
	pData = &pRxdFm->varFm.bData;

	wTQNo = MAKEWORD(pData[0],pData[1]) - m_pCfg->tq.wStartAddr;
	bTQS = pData[2];

	switch(bTQS&0x03)
	{
	case 0://ͬ��
		wControl = 0;
		break; 
	case 1://��ѹ
		wControl = DB_TQWY;
		break;
	case 2://����
		wControl = DB_TQFG;
		break;
	default:
		wControl = DB_TQFG;
		break;
	}

	if(bReason == 8)
		wTQCmd = TP_TQCANCEL;
	if((bReason == 6) && (bTQS & 0x80))
		wTQCmd = TP_TQSELECT;
	else 
		wTQCmd = TP_TQOPERATE;

	pTQ->bInfo[0] = pData[0];
	pTQ->bInfo[1] = pData[1];
	pTQ->bInfo[2] = pData[2];

	if(!SetTQ(DevID, wTQCmd, wControl, wTQNo))
		SetTQReturn(wTQNo, PTQRET_FAIL);
*/	
	ACK(DevID);

	return TRUE;
}

BOOL RxdYkCmd(int DevID, uint8_t *pBuf)//ң�ش���
{
	WORD wYkCmd = 0;
	BYTE bDCO;
	WORD wAttrib;
	WORD wBONo;
	BYTE bType;
	BYTE bReason;
	BYTE* pData;
	TBO* pBO;
	TRealBO* pYkPara;
	BYTE bQU;
	DWORD dwDegree;


//	pBO = pGetBO();
	wAttrib = TP_NULL;
	
	ClearDevFlag(DevID, SF_HaveBOReturn);

//	m_pCfg->bLinkAddr += 1;//��·��ַ 1 or 2
//	m_pCfg->bAsduAddr += 1;//��ͬ��ַ1 or 2
//	m_pCfg->bInfoAddr += 1;//��Ϣ���ַ 1 or 2 or 3
//	m_pCfg->bCauseTrans += 1;//����ԭ��1 or 2
	
	if(m_pCfg->bLinkAddr == 2)
	{
		bType = pBuf[7];
		bReason = pBuf[9];
	}
	else
	{
		bType = pBuf[6];
		bReason = pBuf[8];
	}
	 
	if(bReason != 6 && bReason != 8)
	{
		MakeMirror(45|0x40);
		ACK(DevID);
		return FALSE;
	}

	pData = pBuf + 10;
	
	if(m_pCfg->bLinkAddr == 2)
		pData += 1;
	if(m_pCfg->bAsduAddr == 2)
		pData += 1;
	if(m_pCfg->bCauseTrans == 2)
		pData += 1;
	
	if(m_pCfg->bInfoAddr == 1)
	{
		wBONo = pData[0];
		bDCO = pData[1];
		bQU = (pData[1]>>2)&0x1F;
	}
	else if(m_pCfg->bInfoAddr == 3)
	{
		wBONo = MAKEWORD(pData[0],pData[1]); 
		bDCO = pData[3];
		bQU = (pData[3]>>2)&0x1F;
	}	
	else//(m_pCfg->bInfoAddr == 2)
	{
		wBONo = MAKEWORD(pData[0],pData[1]);
		bDCO = pData[2];
		bQU = (pData[2]>>2)&0x1F;
	}




	if(wBONo < 0x6001)//m_pCfg->yk.wStartAddr
	{
		MakeMirror(47|0x40);
		ACK(DevID);
		return FALSE;
	}
	
	wBONo -= 0x6001;//m_pCfg->yk.wStartAddr;

	pBO->Info[0] = bType;
	pBO->Info[1] = pData[0];
	pBO->Info[2] = pData[1];
	pBO->Info[3] = pData[2];
	pBO->Info[4] = pData[3];


//	wBONo = MAKEWORD(pData[0],pData[1]) - m_pCfg->yk.wStartAddr;
//	bDCO = pData[2];
//	bQU = (pData[2]>>2)&0x1F;

	if(bType == 45)
	{
		if(bDCO&0x01)
			wAttrib = TP_CLOSE;
		else
			wAttrib = TP_OPEN;

		if(bReason == 8)
			wYkCmd = TP_YKCANCEL;
		else if(bReason == 6)
		{
			if(bDCO&0x80)
				wYkCmd = TP_YKSELECT;
			else 
				wYkCmd = TP_YKOPERATE;
		}
	}

	if(bType == 46 || bType == 47) //˫��͵�������
	{
		switch(bDCO&0x03)
		{
		case 1://��
			wAttrib = TP_OPEN;
			break; 
		case 2://��
			wAttrib = TP_CLOSE;
			break;
		default:
			wAttrib = TP_NULL;
			break;
		}

		if(bReason == 8)
			wYkCmd = TP_YKCANCEL;
		else if(bReason == 6)
		{
			if(bDCO&0x80)
				wYkCmd = TP_YKSELECT;
			else 
				wYkCmd = TP_YKOPERATE;
		}
	}
	

/*	if(bType == 47)//����ִ��
	{
		if(bDCO&0x01)
			wAttrib = TP_OPEN;
		else
			wAttrib = TP_CLOSE;

		wYkCmd = TP_YKDIRECT;	
	}
*/
//	pYkPara = GetYkPara(DevID, wBONo, wAttrib);//��ң�صĲ���

	if(pYkPara == NULL)
	{
//		SetBOReturn(wBONo, PBORET_FAIL);
		MakeMirror(47|0x40);
		ACK(DevID);

		return FALSE;
	}

	if(pYkPara->ucAutoResetTime == 0)//����ң��
	{
		if(wYkCmd != TP_YKOPERATE)
		{
			MakeMirror(44|0x40);//invalid type;
			ACK(DevID);
//			SetBOReturn(wBONo, PBORET_FAIL);
			return FALSE;
		}
		wYkCmd = TP_YKDIRECT;//���ó�����ң������
	}


	dwDegree = pYkPara->ucAutoResetTime;
	dwDegree <<= 16;
	dwDegree |= pYkPara->ucCount;

//	if(bQU == 2)//������ʱ��
//		SetYK(DevID, wYkCmd, wBONo, wAttrib, dwDegree, pYkPara->wOffTime, pYkPara->wLastTime, 1);
//	else//����ΪĬ��ֵ
//		SetYK(DevID, wYkCmd, wBONo, wAttrib, dwDegree, pYkPara->wLastTime, pYkPara->wOffTime, 1);

	ACK(DevID);

	return TRUE;
}


void SetALLDataFlag(int DevID)
{
	WORD wNum;
	int i;
	int nCallAINum;
	
	wNum = GetAINum(DevID);

	nCallAINum = wNum;//���ٻ���ң��ĸ���
	
	if(m_pCfg->wCylicDataNum)
	{
		if(wNum <= m_pCfg->wCylicDataNum)
			nCallAINum = 0;
		else
			nCallAINum = wNum - m_pCfg->wCylicDataNum;
	}

	for(i=0; i<nCallAINum; i++)
		SetAIFlag(DevID, i, 0xFF00);
	for(i=0; i<gpDevice[DevID].BINum; i++)
		SetBIFlag(DevID, i, 0xFF00);
	for(i=0; i<gpDevice[DevID].DBINum; i++)
		SetDBIFlag(DevID, i, 0xFF00);
	for(i=0; i<gpDevice[DevID].dwCINum; i++)
		SetCIFlag(DevID, i, 0xFF00);
	for(i=0; i<gpDevice[DevID].dwBCDNum; i++)
		SetBcdFlag(DevID, i, 0xFF00);
		

	SetDevFlag(DevID, SF_CallData);//�����ٻ��ܱ��
	SetDevFlag(DevID, SF_CallCmdFlag);//��Ӧ���ٻ����
}

BOOL RxdReadRequestFile(int DevID)
{
	char lzName[16];
	BYTE bPRM = 0, bCode = 0, bNum = 1;
	BYTE bType;
	BYTE *pBuf;

	pBuf = m_Txd.Buf;

//	::GetDevParaName(DevID, lzName);
//	strcat(lzName, ".dal");
	bType = 1;//������ȷ

//	if(!GetFileStatus(lzName, &fileStatus))
//		return FALSE;
	
	Txd_Head(DevID, FILE_READ_SELECT_RESPOND, FILE_SEND_REASON);

	pBuf[ m_Txd.WritePtr++ ] = 00; //��Ϣ���ַLo
	pBuf[ m_Txd.WritePtr++ ] = 0xF0; //��Ϣ���ַHi
	pBuf[ m_Txd.WritePtr++ ] = bType; //�ļ������޶���

	pBuf[ m_Txd.WritePtr++ ] = 1;//fileid = 1
	pBuf[ m_Txd.WritePtr++ ] = 0;
	pBuf[ m_Txd.WritePtr++ ] = 0;
	pBuf[ m_Txd.WritePtr++ ] = 0;


//	pBuf[m_Txd.WritePtr++] = (fileStatus.dwStatus&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwStatus>>8)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwStatus>>16)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwStatus>>24)&0xFF);

//	pBuf[m_Txd.WritePtr++] = (fileStatus.dwLen&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwLen>>8)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwLen>>16)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwLen>>24)&0xFF);
//
//	pBuf[m_Txd.WritePtr++] = (fileStatus.dwTime&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwTime>>8)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwTime>>16)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwTime>>24)&0xFF);
//
//	
//	pBuf[m_Txd.WritePtr++] = (fileStatus.wCrc16&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.wCrc16>>8)&0xFF);
//
//	pBuf[m_Txd.WritePtr++] = strlen(lzName);
	
//	for(int i=0; i<strlen(lzName); i++)
//		pBuf[m_Txd.WritePtr++] = lzName[i];

	
	Txd_Tail(DevID, bPRM, bCode, bNum);


	return TRUE;
}

BOOL RxdWriteRequestFile(int DevID)
{
	char lzName[16];
	BYTE bPRM = 0, bCode = 0, bNum = 1;
	BYTE* pBuf;
	BYTE bType;
	BYTE* pData;

//	pFileStatus = NULL;

	pBuf = m_Txd.Buf;
//	pData = &pRxdFm->varFm.bData;
	pData += 3;
//	pFileStatus = (TFileStatus*)pData;

//	SetDevFlagValue(DevID ,SF_FILE_CRC, pFileStatus->wCrc16);
 
//	::GetDevParaName(DevID, lzName);
//	strcat(lzName, ".dal");
	bType = 1;//������ȷ

//	if(!GetFileStatus(lzName, &fileStatus))
//		return FALSE;
	

	Txd_Head(DevID, FILE_WRITE_SELECT_RESPOND, FILE_SEND_REASON);

	pBuf[ m_Txd.WritePtr++ ] = 00; //��Ϣ���ַLo
	pBuf[ m_Txd.WritePtr++ ] = 0xF0; //��Ϣ���ַHi
	pBuf[ m_Txd.WritePtr++ ] = bType; //�ļ������޶���

	pBuf[ m_Txd.WritePtr++ ] = 1;//fileid = 1
	pBuf[ m_Txd.WritePtr++ ] = 0;
	pBuf[ m_Txd.WritePtr++ ] = 0;
	pBuf[ m_Txd.WritePtr++ ] = 0;


//	pBuf[m_Txd.WritePtr++] = (fileStatus.dwStatus&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwStatus>>8)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwStatus>>16)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwStatus>>24)&0xFF);
//
//	pBuf[m_Txd.WritePtr++] = (fileStatus.dwLen&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwLen>>8)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwLen>>16)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwLen>>24)&0xFF);
//
//	pBuf[m_Txd.WritePtr++] = (fileStatus.dwTime&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwTime>>8)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwTime>>16)&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.dwTime>>24)&0xFF);
//
//	
//	pBuf[m_Txd.WritePtr++] = (fileStatus.wCrc16&0xFF);
//	pBuf[m_Txd.WritePtr++] = ((fileStatus.wCrc16>>8)&0xFF);
//
//	pBuf[m_Txd.WritePtr++] = strlen(lzName);
//	
//	for(int i=0; i<strlen(lzName); i++)
//		pBuf[m_Txd.WritePtr++] = lzName[i];

	
	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}

BOOL RxdReadFileData(int DevID)
{
	char lzName[16];
	BYTE* pData;
	TSendDataHead* pHead;
	TSendDataHead* pSHead;
	DWORD dwLen;

	BYTE bPRM = 0, bCode = 0, bNum = 1;
	BYTE* pBuf;
	BYTE bType;
	DWORD dwRtn;

//	pData = &pRxdFm->varFm.bData;
	pHead = (TSendDataHead*)pData;

//	::GetDevParaName(DevID, lzName);
//	strcat(lzName, ".dal");
	
	pBuf = m_Txd.Buf;
	bType = 1;//������ȷ
	
	Txd_Head(DevID, FILE_DATA_SEND, FILE_SEND_REASON);

	pTxdFm = (TSIEC101_2002TxdFm *)m_Txd.Buf;
	pSHead = (TSendDataHead*)&pTxdFm->varFm.bData;
 
	dwLen = pHead->bPacketLen + 1;

//	dwRtn = fReadFile(lzName, pHead->dwOffset, pHead->bPacketLen, &pSHead->bData, &dwLen);
	
//	if(dwRtn == FILE_ERROR)
//		bType = 3;
//	else if(dwRtn == FILE_END)//���һ��
//		bType = 1 + (2<<4);
//	else 
//		bType = 1 + (1<<4);//


	pBuf[ m_Txd.WritePtr++ ] = 00; //��Ϣ���ַLo
	pBuf[ m_Txd.WritePtr++ ] = 0xF0; //��Ϣ���ַHi
	pBuf[ m_Txd.WritePtr++ ] = bType; //�ļ������޶���

	pBuf[ m_Txd.WritePtr++ ] = 1;//fileid = 1
	pBuf[ m_Txd.WritePtr++ ] = 0;
	pBuf[ m_Txd.WritePtr++ ] = 0;
	pBuf[ m_Txd.WritePtr++ ] = 0;

	pBuf[ m_Txd.WritePtr++ ] = (pHead->dwOffset&0xFF);
	pBuf[ m_Txd.WritePtr++ ] = ((pHead->dwOffset>>8)&0xFFFF);
	pBuf[ m_Txd.WritePtr++ ] = ((pHead->dwOffset>>16)&0xFFFF);
	pBuf[ m_Txd.WritePtr++ ] = ((pHead->dwOffset>>24)&0xFFFF);

	pBuf[ m_Txd.WritePtr++ ] = dwLen;//����

	m_Txd.WritePtr += dwLen;

	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}

BOOL RxdWriteFileData(int DevID)//�����ļ�����дӦ��
{
	char lzName[16];
	BYTE* pData;
	TSendDataHead* pHead;

	BYTE bPRM = 0, bCode = 0, bNum = 1;
	BYTE* pBuf;
	BYTE bType;
	DWORD dwRtn;

//	pData = &pRxdFm->varFm.bData;
	pHead = (TSendDataHead*)pData;

//	::GetDevParaName(DevID, lzName);
//	strcat(lzName, ".dal");
//
//	sprintf(lzName, "tmp02.dat");
	
//	if( (pHead->bSFQ>>4) == 2)//���һ��
//		bType = FILE_END;
//	else
//		bType = FILE_OK;
//
//	dwRtn = ::WriteFile(lzName, pHead->dwOffset, pHead->bPacketLen, &pHead->bData, bType);
//	
//	if(dwRtn == FILE_ERROR)
//		bType = 2;
//	else
//		bType = 1;
//
//	bType |= (pHead->bSFQ&0xF0);
//
//	if( (pHead->bSFQ>>4) == 2)//���һ��
//	{
//		dwRtn = GetFileCrc16(lzName);
//		
//		if(dwRtn == GetDevFlagValue(SF_FILE_CRC))
//			bType = 1;
//		else//file crc error, ���´� offset = 0;
//		{
//			bType = 2;
//			pHead->dwOffset = 0;
//		}
//		bType |= (pHead->bSFQ&0xF0);
//
//	}
//
//	pBuf = m_Txd.Buf;
	
	Txd_Head(DevID, FILE_FILE_ACK, FILE_SEND_REASON);


	pBuf[ m_Txd.WritePtr++ ] = 00; //��Ϣ���ַLo
	pBuf[ m_Txd.WritePtr++ ] = 0xF0; //��Ϣ���ַHi
	pBuf[ m_Txd.WritePtr++ ] = bType; //�ļ������޶���

	pBuf[ m_Txd.WritePtr++ ] = (pHead->dwFileID&0xFF);
	pBuf[ m_Txd.WritePtr++ ] = ((pHead->dwFileID>>8)&0xFF);
	pBuf[ m_Txd.WritePtr++ ] = ((pHead->dwFileID>>16)&0xFF);
	pBuf[ m_Txd.WritePtr++ ] = ((pHead->dwFileID>>24)&0xFF);

	pBuf[ m_Txd.WritePtr++ ] = (pHead->dwOffset&0xFF);
	pBuf[ m_Txd.WritePtr++ ] = ((pHead->dwOffset>>8)&0xFFFF);
	pBuf[ m_Txd.WritePtr++ ] = ((pHead->dwOffset>>16)&0xFFFF);
	pBuf[ m_Txd.WritePtr++ ] = ((pHead->dwOffset>>24)&0xFFFF);

	pBuf[ m_Txd.WritePtr++ ] = pHead->bPacketLen;//����

	Txd_Tail(DevID, bPRM, bCode, bNum);

	return TRUE;
}

BOOL RxdNoAsdu()
{
//	BYTE bRxdCode;
//	
//	bRxdCode = GetDevFlagValue(SF_RXDCONTROL)& BIT_CODE;
//	
//	switch(bRxdCode)
//	{
//		case 3:
//			MakeMirror(44|0x40);//noasdu;
//			ACK(DevID);
//			break;
//		
//		case 8:
//		case 9:
//			ACK(DevID);
//			break;
//		default:
//			break;
//	}

	return TRUE;
}


BOOL RxdCallDirFile(int DevID, uint8_t *pBuf)
{
	BYTE* pData;
	BYTE bReason;
	WORD wFileName;
	WORD wObjAddr;
	BYTE bSectionName;
	BYTE bSCQ;

	pData = pBuf + 10;

	if(m_pCfg->bLinkAddr == 2)
		bReason = pBuf[9];
	else
		bReason = pBuf[8];
	
	if(m_pCfg->bLinkAddr == 2)
		pData += 1;
	if(m_pCfg->bAsduAddr == 2)
		pData += 1;
	if(m_pCfg->bCauseTrans == 2)
		pData += 1;
	
	if(m_pCfg->bInfoAddr == 1)
	{
		wObjAddr = pData[0];
		pData += 1;
	}
	if(m_pCfg->bInfoAddr == 2)
	{
		wObjAddr = MAKEWORD(pData[0], pData[1]);
		pData += 2;
	}
	if(m_pCfg->bInfoAddr == 3)
	{
		wObjAddr = MAKEWORD(pData[0], pData[1]);
		pData += 3;
	}

//	SetDevFlagValue(DevID ,SF_FILEOBJ_ADDR, wObjAddr);	

	wFileName = MAKEWORD(pData[0], pData[1]);
	bSectionName = pData[2];
	bSCQ = pData[3];

	if(bReason != 5 && bReason != 13)//δ֪�Ĵ���ԭ��
	{
		MakeMirror(45|0x40);
		ACK(DevID);
		return FALSE;
	}

	if(wObjAddr < m_pCfg->file.wStartAddr)
	{
		MakeMirror(47|0x40);
		ACK(DevID);
		return FALSE;
	}

	if(bReason == 5)//Ŀ¼�ٻ�
	{
		SetDevFlagValue(DevID ,SF_FILE_NAME, wFileName);
		SetDevFlagValue(DevID ,SF_SECTION_NAME, bSectionName);
		SetDevFlagValue(DevID ,SF_FILE_OFFSET, 0);
		SetDevFlag(DevID, SF_CALLDIR_FLAG);

//		TxdCallDir();
		ACK(DevID);

		return TRUE;
	}

	switch(bSCQ&0x0F)
	{
	case 1://select file
		SetDevFlag(DevID, SF_FILE_SELECT);
		SetDevFlagValue(DevID ,SF_FILE_NAME, wFileName);
		break;
	case 2://request file
		SetDevFlag(DevID, SF_FILE_REQUEST);
		SetDevFlagValue(DevID ,SF_FILE_NAME, wFileName);
		break;
	case 3://cancel select file
		CancelSelectFile(wFileName);
		break;
	case 4://del file
		break;
	case 5://ѡ���
		break;
	case 6://requrest section
//		SetDevFlag(DevID, SF_FILE_SENDING, wFileName);
		SetDevFlagValue(DevID ,SF_FILE_OFFSET, 0);
		break;
	case 7://cancel select section
		break;
	default:
		break;
	}

	ACK(DevID);

	return TRUE;
}


BOOL CancelSelectFile(WORD wFileName)
{
/*
	TFDRTA* pFDRTA;
	POSITION1 pos;

	if(wFileName && wFileName <= m_dirList.GetCount())
	{
		pos = m_dirList.FindIndex(wFileName-1);	
		pFDRTA = (TFDRTA*)m_dirList.GetAt(pos);
		
		pFDRTA->bSOF = 0;

		return TRUE;
	}	
*/
	return FALSE;
}

BOOL CheckAndRetry(int DevID, BYTE bControl)
{
//	return FALSE;

	BYTE bCode;
	BYTE bNextFcbFcv;
	BYTE bRxdFcbFcv;

	bCode = bControl&BIT_CODE;
	bNextFcbFcv = GetDevFlagValue(DevID, SF_NEXT_FCBFCV) & BIT_FCBFCV;
	bRxdFcbFcv = bControl&BIT_FCBFCV;

	if(bCode == 0 || bCode == 1)//��λ֡����,�ڴ���һ����FCB=1, FCV=1
	{
		SetDevFlagValue(DevID ,SF_NEXT_FCBFCV, BIT_FCB|BIT_FCV);
		return FALSE;
	}
	
	if(!(bRxdFcbFcv&BIT_FCV))//FCV��Ч,����Ҫ�ط�����
	{
		SetDevFlagValue(DevID ,SF_NEXT_FCBFCV, 0);
		return FALSE;
	}
	
	if(bNextFcbFcv == 0)//��һ֡�ǲ���Ҫ�ط���֡
	{
		SetDevFlagValue(DevID ,SF_NEXT_FCBFCV, (~bRxdFcbFcv&BIT_FCB)|BIT_FCV);
		return FALSE;
	}

	if(bNextFcbFcv != bRxdFcbFcv)
	{
		if(m_Txd.WritePtr > 4)
		{
//			TxdRetry();
			return TRUE;
		}
	}

	SetDevFlagValue(DevID ,SF_NEXT_FCBFCV, (~bRxdFcbFcv&BIT_FCB)|BIT_FCV);

	return FALSE;
}

//=============================================================================
//  ��������: ��ʱ����Ĵ���
//=============================================================================
static void OnTimeOut(int DevID)
{
	gpDevice[DevID].TimeCnt++;



	if(gpDevice[DevID].TimeCnt%60 == 0)
	{
		if((m_pCfg->bCounterMode == 0) || (m_pCfg->bCounterMode == 1))//mode A and mode b	
			ScanFreeze();
	}

	if(gpDevice[DevID].TimeCnt%10 == 0)
		ScanAiData(DevID);
}

void OnBOReturn(DWORD dwDevID,DWORD dwCommand,DWORD dwBONo,DWORD dwResult)//ң�ط�У
{
//	OnBOReturn(dwDevID,dwCommand,dwBONo,dwResult);

//	if(CheckClearDevFlag(SF_HaveBOReturn))
//		TxdYkReturn();
}

void OnAOReturn(DWORD dwDevID,DWORD dwCommand,DWORD dwAONo,short AOValue,DWORD dwResult)//ң����У
{
//	OnAOReturn(dwDevID,dwCommand,dwAONo,AOValue,dwResult);//ң����У

//	if(CheckClearDevFlag(SF_HaveAOReturn))
//		TxdYtReturn();
}

void OnBurstBI()
{
}

void OnSoe()
{
}

BOOL RxdFixFrame(int DevID, BYTE* pBuf)
{
	BYTE bControl;

	bControl = pBuf[1];

	SetDevFlagValue(DevID ,SF_RXDCONTROL, bControl);

	if(m_pCfg->bTestMode == 0)
	{
		if(GetDevFlag(DevID, SF_RtuInitOk) == FALSE)
		{
			switch(bControl & BIT_CODE)
			{
				case 0x00://��λԶ����·
				case 0x09://����Զ����·״̬
					break;
				default:
					return FALSE;
			}
		}

		if(CheckAndRetry(DevID, bControl))
			return TRUE;
	}
	
	switch(bControl & BIT_CODE)
	{
	case 0x00: //0x00 ��λԶ����·
		RxdResetLink(DevID, pBuf);  
		break; 
	case 0x08://��Ӧ,��·״̬
	case 0x09: //0x09 ����Զ����·״̬
		RxdReqLinkStatus(DevID); 
		break; 
	case 0x0A: //�ٻ�һ���û�����
		RxdClass1Data(DevID); 
		break; 
	case 0x0B: //Զ����·״̬��û��ٻ������û�����
		RxdClass2Data(DevID); 
		break; 
	default:
		ACK(DevID);
		break;
	}
	return TRUE;
}

BOOL RxdVarFrame(int DevID, uint8_t *buf)
{
	BYTE* pBuf;
	BYTE bControl;
	BYTE bType;
	WORD wAddress;
	BYTE* pData;

//	pBuf = m_RxdFrame.Buf;
	bControl = pBuf[4];

//	SetDevFlagValue(DevID ,SF_RXDCONTROL, bControl);

	if(m_pCfg->bTestMode == 0)
	{
		if(GetDevFlag(DevID, SF_RtuInitOk) == FALSE) //û�г�ʼ����·
			return FALSE;
	}

	if(!ChkVarControl(DevID, bControl&0x0F))
		return FALSE;

	if(m_pCfg->bTestMode == 0)
	{
	//	if(CheckAndRetry(bControl))//�ط�����
	//		return FALSE;
	}
	
	pData = pBuf+9;
	
	if(m_pCfg->bLinkAddr == 2)
	{
		bType = pBuf[7];
		pData++;
	}
	else
		bType = pBuf[6];
	
	if(m_pCfg->bCauseTrans == 2)
		pData ++;
	if(m_pCfg->bAsduAddr == 2)
		wAddress = MAKEWORD(pData[0],pData[1]);
	else
		wAddress = pData[0];
	
//	if(SwitchToAddress(wAddress) != TRUE)//��Ч��ַ
//	{
//		MakeMirror(46|0x40);//46, invalid addr
//		ACK(DevID, );
//		return FALSE;
//	}

	switch(bType)
	{
	case 45:
	case 46:  //ң��
	case 47:  
		RxdYkCmd(DevID, buf);
		break;
	case 48:
	case 49:
	case 50:
		RxdYtCmd(DevID, buf);
		break;
	case 51:
		RxdDOCmd(DevID, buf);
		break;
//	case 60://ͬ��
//		RxdTqCmd(DevID);
//		break;
	case 0x64:  //���ٻ�/�ٻ�ĳһ������
		RxdCallAll(DevID); 
		break;
	case 0x65:
		RxdCallCI(DevID, buf);//�ٻ�������
		break;
	case 0x67:  //ʱ��ͬ��
		RxdClockSyn(DevID, buf); 
		break;
	case 0x68:  //������·
		RxdTestLink(DevID);
		break;
	case 0x69://��λRTU
		//SystemReset(0);
		ResetProcess(DevID, buf);
		break;
	case 0x6a://��ʱ���
		RxdDelay(DevID, buf);
		break;
	case 110://���ò���  normalize
		RxdSetPara(DevID,buf); 
		break;
	case 111:////���ò���Scale
		RxdSetPara2(DevID,buf); 
		break;  
	case 112://���ò����̸�����
		RxdSetPara3(DevID,buf);
		break;
	case 131://���ļ�����
		RxdReadRequestFile(DevID);
		break;
	case 132://д�ļ�����
		RxdWriteRequestFile(DevID);
		break;
	case 135://���ļ�
		RxdReadFileData(DevID);
		break;
	case 136://д�ļ�
		RxdWriteFileData(DevID);
		break;

	case 122://�ٻ�Ŀ¼,ѡ���ļ�
		RxdCallDirFile(DevID, buf);
		break;
	default:
		RxdNoAsdu();
		break;
	}
	return TRUE;
}
BOOL Txd_Class1(int DevID) //����1������
{
	if(CheckClearDevFlag(DevID, SF_SendInitFlag))
		return TxdInitFinish(DevID);
	
	if(GetDevFlag(DevID, SF_CALLDIR_FLAG))//�ڷ��ͺ�����ͷ���־��Ŀ¼�п���һ֡���Ͳ���
	{
		return TxdCallDir();		
	}
	if(CheckClearDevFlag(DevID, SF_FILE_SELECT))
		return TxdFileSelect();
	if(CheckClearDevFlag(DevID, SF_FILE_REQUEST))
		return TxdFileRequest();
	if(GetDevFlag(DevID, SF_FILE_SENDING))
		return TxdFileSending();
	if(CheckClearDevFlag(DevID, SF_LAST_SEGMENT))
		return TxdLastSegment();

	if(CheckClearDevFlag(DevID, SF_MirrorFlag))
		return TxdMirror(DevID);

	if(CheckClearDevFlag(DevID, SF_ClockSynFlag))
		return TxdClockSyn(DevID);
	if(CheckClearDevFlag(DevID, SF_CallDelayFlag))
		return TxdTimeDelay(DevID);
	
//	if(CheckClearDevFlag(DevID, SF_SetParaFlag))
//		return TxdSetPara();

	if(CheckClearDevFlag(DevID, SF_CallCmdFlag))
		return TxdCallAllAck(DevID);
	
	if(CheckClearDevFlag(DevID, SF_HaveBOReturn))
		return TxdYkReturn();
	if(CheckClearDevFlag(DevID, SF_HaveAOReturn))
		return TxdYtReturn();
	if(CheckClearDevFlag(DevID, SF_HaveTQReturn))
		return TxdTqReturn();

	if(CheckClearDevFlag(DevID, SF_HaveDOReturn))
		return TxdDOReturn();
	
//	if(CheckClearDevFlag(DevID, SF_YkRtnInfoFlag))
//		return TxdYkExeInfo(11);//Զ��������Ϣ
	if(CheckClearDevFlag(DevID, SF_YkExitFlag))//ִ�����
		return TxdYkExeInfo(10);

//	if(CheckClearDevFlag(DevID, SF_YtRtnInfoFlag))
//		return TxdYtExeInfo(11);
//	if(CheckClearDevFlag(DevID, SF_YtExitFlag))
//		return TxdYtExeInfo(10);

	if(CheckClearDevFlag(DevID, SF_FILE_FLAG))
		return TxdFileStatus(DevID);
	
	if(TxdCallAll(DevID))
		return TRUE;
	int i;
	for(i=0 ;i<4; i++)
	{
		if(CheckClearDevFlag(DevID, SF_CALLCIFLAG0+i))
		{
			if(TxdCallGroupCIData(DevID, i+1))
				return TRUE;
		}
	}

	if(CheckClearDevFlag(DevID, SF_ReadCIFlag))//����ٻ��������
	{
		return TxdCIReadTermination(DevID);
	}

	if(m_pCfg->spi.bClass == 1)//�仯ң��һ������
	{
		if(GetBurstBINum(DevID)) //���ͱ仯ң��
		{
			if(Txd_BurstBI(DevID))
				return TRUE;
		}
	}
	
	if(m_pCfg->dpi.bClass == 1)//�仯˫��ң��һ������
	{
		if(GetBurstDBINum(DevID))
		{
			if(Txd_BurstDBI(DevID))
				return TRUE;
		}
	}

	if(m_pCfg->spi.bClass == 1)
	{
	    if(GetSoeNum(DevID))
		{
			if(Txd_Soe(DevID))
				return TRUE;
		}
	}

	if(m_pCfg->dpi.bClass == 1)
	{
	    if(GetDBISoeNum(DevID))
		{
			if(Txd_DSoe(DevID))
				return TRUE;
		}
	}

	if(m_pCfg->mr.bClass == 1)
	{
		if(GetMrNum(DevID))
		{
			if(Txd_ProtectMr(DevID))
				return TRUE;
		}
	}

	if(m_pCfg->yc.bClass == 1)
	{
		if(Txd_ChangeAI(DevID))
			return TRUE;
	}
	
	if(m_pCfg->yc.bClass == 1)
	{
		if(!GetAitNum(DevID))
			return FALSE;
		if(Txd_YcSoe(DevID))
			return TRUE;
	}

	if(m_pCfg->step.bClass == 1)
	{
		if(GetBurstBcdNum(DevID))
		{
			if(TxdChangeSpiData(DevID))
				return TRUE;
		}
	}
	
	if(m_pCfg->kwh.bClass == 1)
	{
		if(m_pCfg->bCounterMode == 0)//mode A
		{
			if(GetFreezeCINum(DevID))
				return TxdFreezeCIData(DevID);
		}
	}

	return FALSE;
}

BOOL Txd_Class2(int DevID) //����2������
{
	if(m_pCfg->spi.bClass == 2)//�仯ң��һ������
	{
		if(GetBurstBINum(DevID)) //���ͱ仯ң��
		{
			if(Txd_BurstBI(DevID))
				return TRUE;
		}
	}
	
	if(m_pCfg->dpi.bClass == 2)//�仯˫��ң��һ������
	{
		if(GetBurstDBINum(DevID))
		{
			if(Txd_BurstDBI(DevID))
				return TRUE;
		}
	}
	
	if(m_pCfg->spi.bClass == 2)
	{
	    if(GetSoeNum(DevID))
		{
			if(Txd_Soe(DevID))
				return TRUE;
		}
	}

	if(m_pCfg->dpi.bClass == 2)
	{
	    if(GetDBISoeNum(DevID))
		{
			if(Txd_DSoe(DevID))
				return TRUE;
		}
	}

	if(m_pCfg->mr.bClass == 2)
	{
		if(GetMrNum(DevID))
		{
			if(Txd_ProtectMr(DevID))
				return TRUE;
		}
	}

	if(m_pCfg->yc.bClass == 2)
	{
		if(Txd_ChangeAI(DevID))
			return TRUE;
	}

	if(m_pCfg->yc.bClass == 2)
	{
		if(!GetAitNum(DevID))
			return FALSE;
		if(Txd_YcSoe(DevID))
			return TRUE;
	}

	if(m_pCfg->step.bClass == 2)
	{
		if(GetBurstBcdNum(DevID))
		{
			if(TxdChangeSpiData(DevID))
				return TRUE;
		}
	}
	
	if(m_pCfg->kwh.bClass == 2)
	{
		if(m_pCfg->bCounterMode == 0)//mode A
		{
			if(GetFreezeCINum(DevID))
				return TxdFreezeCIData(DevID);
		}
	}
	
	if(GetDevFlag(DevID, SF_HaveScanAiData))
		return TxdScanAiData(DevID);
	//�����ݣ���E5�ش�
//	Txd_NoData();
	
	return FALSE;
}


//#endif

