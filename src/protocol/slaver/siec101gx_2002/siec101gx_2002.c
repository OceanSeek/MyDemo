#ifdef SIE101GX_2002
#include "sys.h"
#include "siec101gx_2002.h"

//外部变量声明区
extern TBurstYXRecord YXBrustRecord[MAXYXRECORD];
extern TBurstYXRecord YXSoeRecord[MAXYXRECORD];
extern TBurstYCRecord YCBrustRecord[MAXYCRECORD];
extern TransTable_T *TransYKTable;


//本地变量
uint8_t	gx101_Sendbuf[GX101Slaver_MAX_BUF_LEN];

uint8_t COICALL;
uint8_t SF_CallDZData = 0;
uint8_t SF_CallDZGroupData = 0;
uint8_t SF_CallDZCmdFlag = 0;
static uint8_t m_dwDevID = 0;
WORD flag_dz_return = 0;
static uint32_t Now_Time_cnt = 0;
static BYTE CKDevID = 8;
	

//局部函数
int RxdVarFrame(int fd, int DevID, uint8_t *buf, uint16_t len);
BOOL RxdCallDZ(int fd, int DevID, uint8_t *pBuf);//召唤参数
BOOL ACK(int fd);
BOOL RxdFixFrame(int fd, int DevID, uint8_t *buf, uint16_t len);
BOOL Txd_OldCP56TSoe(int fd);
uint8_t ResConfirm(int fd);
static BOOL RxdCallAll(int fd, uint8_t *pbuf);//总召唤
BOOL RxdReqLinkStatus(int fd);
BOOL TxdFixFrame(int fd, BYTE bPRM, BYTE bCode);
BOOL RxdResetLink(int fd, uint8_t *pBuf);
BOOL Txd_Head101(BYTE bType, BYTE bReason);
BOOL Txd_Tail101(int fd, BYTE bPRM, BYTE bCode, BYTE bNum);
void SetDZDataFlag();
BOOL TxdCallDZALLAck(int fd, BYTE COI);
BOOL RxdDZYZCmd(int fd, uint8_t *pBuf);
BOOL TxdDzYkReturn(int fd);
static BOOL Txd_AllStop(int fd, BYTE data);//发送总召唤结束帧
static void MakeMirror(int fd, BYTE bReason, uint8_t *pbuf);
static BOOL RxdClockSyn(int fd, uint8_t *pBuf);//时钟同步
int r_call_Dz(DWORD DirAddr,BYTE comman);
BOOL TxdCallDZGROUP(int fd);
void SetALLDZFlag(DWORD dwDevID, WORD wFlag);
BOOL Txd_AllDZStop(int fd, BYTE COI);//发送定值召唤结束帧
void upDateDZData(BYTE common);
int siec101gx_Task(int DevID);
int siec101gx_OnTimeOut(int DevID);
static BOOL RxdDelay(int fd, uint8_t *pBuf);


BYTE DZReturnReanson101;
BYTE FlagGourpCall = 0;//组召唤标志

BYTE FlagIntFinsh = 0;//初始化结束

BYTE DZ_Comman = 0;//1:定值预置 2:定值激活
BYTE DZ_JIHUO_ADDR = 0; 
DZRETURNADDR_T DZReturnAddr;
BYTE DZGroupID;//定值组召唤ID
BYTE DZGroupDataNum;//定值组召唤数量
bool YCsendEnable_Flag;//允许发突发遥测标志位true允许   false不允许 
BYTE bStartFlag_s101;			//允许数据发送标志

bool Flag_Call_ALLDZData = FALSE;
bool Flag_Call_GROUPDZData = FALSE;

static TIEC101Cfg m_pCfg;


int dwSoeDataNum = 0;
int SOESendNum = 0;


int Flag_SoeSendAll = 0;
int FlagSendOldSoe = 0;
int FlagCallOldSoe = 0;

WORD SoedwReadPtr = 0;
int GroupCallCOI = 0;
DWORD info_num_satic;
DWORD info_SQ;
DWORD info_addr;
DWORD dir_DzAddr;//目标定值地址
static WORD m_wDelayTime;
static WORD m_wFromTime;
static uint8_t m_bMirrorBuf[255];

static TCommIO m_Txd;				  //发送缓冲区的处理结构
S101DZDATA_T *S101DzValue = NULL;//定值存储区
S101DZDATA_T *S101DzValueBackUp = NULL;//定值待处理区
TSysTimeSoe *tmpSysTimeSoe = NULL;


static int RxdMonitor(int DevID, uint8_t *buf, uint16_t len);


int Init_siec101gx(int DevID)
{
	gpDevice[DevID].Receive = RxdMonitor;
	gpDevice[DevID].Task = siec101gx_Task;
	gpDevice[DevID].OnTimeOut = siec101gx_OnTimeOut;
	int i;
	m_dwDevID = DevID;
//	m_pCfg.bAsduAddr = 2;
//	m_pCfg.bInfoAddr = 2;
//	m_pCfg.bLinkAddr = 2;
//	m_pCfg.bCauseTrans = 1;
//	m_pCfg.bTestMode = 0;
//	m_pCfg.wMaxFrm = 255;
	YCsendEnable_Flag = false;
	
	S101DzValue = (S101DZDATA_T *)malloc(342*sizeof(S101DZDATA_T));
	memset(S101DzValue,0,342*sizeof(S101DZDATA_T));
	S101DzValueBackUp = (S101DZDATA_T *)malloc(342*sizeof(S101DZDATA_T));
	memset(S101DzValueBackUp,0,342*sizeof(S101DZDATA_T));

	tmpSysTimeSoe = (TSysTimeSoe *)malloc(300*sizeof(TSysTimeSoe));
	memset(tmpSysTimeSoe, 0, 300*sizeof(TSysTimeSoe));

	//初始化转发表
	for(i=0;i<gVars.TransYCTableNum;i++){
		gpDevice[DevID].pLogicBase->pLogicAI[i].wRealID = TransYCTable[i].wRealID;
		gpDevice[DevID].pLogicBase->pLogicAI[i].wOffset = TransYCTable[i].nPoint;
	}



	if(gpDevice[Dz_DevID_3].pDZ == NULL){
		perror("dz is null");
		gpDevice[Dz_DevID_3].DZNum = 255;
		gpDevice[Dz_DevID_3].pDZ = (uint32_t*)malloc(gpDevice[Dz_DevID_3].DZNum * sizeof(uint32_t));
		memset(gpDevice[Dz_DevID_3].pDZ, 0, gpDevice[Dz_DevID_3].DZNum * sizeof(uint32_t));
		return FALSE;
	}
	
}
//==============================================
//  函数功能: 接收处理
//==============================================
static int RxdMonitor(int DevID, uint8_t *buf, uint16_t len)
{
    uint8_t* pBuf;
	int fd = gpDevice[DevID].fd;
	m_dwDevID = DevID;
    // 处理接收
	pBuf = buf;

	if(pBuf[0] == 0x10)
		return RxdFixFrame(fd, DevID, buf, len);
	if(pBuf[0] == 0x68)
		return RxdVarFrame(fd, DevID, buf, len);
	

	return 0;
}
BOOL RxdFixFrame(int fd, int DevID, uint8_t *buf, uint16_t len)
{
	BYTE* pBuf;
	BYTE bControl;
	pBuf = buf;
	bControl = pBuf[1];
	
	switch(bControl)
	{
	case 0xC0: //0x00 复位远方链路（复位从站）
		RxdResetLink(fd, buf);  
		FlagIntFinsh = 1;
		bStartFlag_s101 = 1;
		break; 
	case 0x80:
//		if(FlagSendOldSoe == 1){
//			PRINT_FUNLINE;
//			Txd_OldCP56TSoe(fd);
//		}
//		if(Flag_SoeSendAll == 1){
//			Flag_SoeSendAll = 0;
//			PRINT_FUNLINE;
//			Txd_AllStop(fd, 0x24);
//		}
//		if(Flag_Call_GROUPDZData == TRUE){
//			PRINT_FUNLINE;
//			TxdCallDZGROUP(fd);
//		}
//		
//		
//		if(Flag_Call_ALLDZData == TRUE){
//			PRINT_FUNLINE;
//			TxdCallDZAll(fd);
//		}
		if(FlagIntFinsh){
			Txd_InitFinish(fd);
			FlagIntFinsh = 0;
		}

        	
		if(FlagGourpCall == 1){
//			Txd_CallGroupData(GroupCallCOI);
			FlagGourpCall = 0;
		}
		
		break;
	case 0xC9: //0x09 请求远方链路状态
//		YCsendEnable_Flag=false;
		PRINT_FUNLINE;
		RxdReqLinkStatus(fd); 
		PRINT_FUNLINE;
		break; 
// 	case 0x0A: //召唤一级用户数据
// 		RxdClass1Data(); 
// 		break; 
	case 0xF2: //远方链路状态完好或召唤二级用户数据
	case 0xD2: //远方链路状态完好或召唤二级用户数据
// 		RxdClass1Data(); 
// 		RxdClass2Data(); 
		ACK(fd);
		break; 
	case 0x8B: //复位远方链路（复位主站）
		TxdFixFrame(fd, 1, 0);//
		break; 
	default:
//		ACK();
		break;
	}
	return TRUE;
}


int RxdVarFrame(int fd, int DevID, uint8_t *buf, uint16_t len)
{
	uint8_t *pBuf;
	uint8_t bControl;
	uint8_t bType;
	uint16_t wAddress;
	uint8_t* pData;

	pBuf = buf;
	bControl = pBuf[4];
	pData = pBuf+8;
	bType = pBuf[7];
	
	
	wAddress = pData[0];//公共地址
//	log("bType(%d)\n",bType);
	switch(bType)
	{
	case 45:
	case 46:  //遥控
	case 47:  
//		RxdYkCmd();
		break;
	case 48:
	case 49:
	case 50:
//		RxdYtCmd();
		break;
	case 0x64:  //总召唤/召唤某一组数据
		YCsendEnable_Flag=false;
		RxdCallAll(fd, buf);
		break;
	case 0x67:  //时钟同步
		RxdClockSyn(fd, buf); 
		break;
	case 0x69://复位RTU
		break;
	case 0x6A://复位RTU
		RxdDelay(fd, buf);
		break;
	case 0x6c://参数召唤
		RxdCallDZ(fd, DevID, buf); 
		break;
	case 0x70://定值预置
		ACK(fd);
		RxdDZYZCmd(fd ,buf);
		break;
	case 0x71://定值激活
		ACK(fd);
		if(DZReturnAddr.Dev1 == 1){
			r_call_Dz(1,TP_DZ_CALL_EXE);
			DZReturnAddr.Dev1 = 0;
		}
		if(DZReturnAddr.Dev2 == 1){
			r_call_Dz(2,TP_DZ_CALL_EXE);
			DZReturnAddr.Dev2 = 0;
		}
		if(DZReturnAddr.Dev3 == 1){
			r_call_Dz(3,TP_DZ_CALL_EXE);
			DZReturnAddr.Dev3 = 0;
		}
		
		TxdDzYkReturn(fd);
		upDateDZData(DZ_JI_HUO);
		break;

	default:
		break;
	}
	return 1;
}
void SetALLDZFlag101(BYTE wFlag)
{
	int i;
	for (i = 0; i < 342; i++)
	{
		S101DzValue[i].Flag = wFlag;
	}
	
	Flag_Call_ALLDZData = TRUE;
}
void SetGROUPDZFlag101(BYTE GroupID,WORD wFlag)
{
	int i;
	if(GroupID>2){
		return;
	}
	for (i = 0; i < 114; i++)
	{
		S101DzValue[i + GroupID*114].Flag = wFlag;
	}
	Flag_Call_GROUPDZData = TRUE;

}
static bool Real_ReadSysTimeSoe(DWORD dwDevID,WORD *pdwReadPtr,TBurstYXRecord *pSysTimeSoe)
{
	DWORD SoeNum;

	
//	SoeNum = gpDevice[dwDevID].pSoeWrap->Ptr.dwWritePtr-*pdwReadPtr;
//	
//	if(SoeNum>GetSoeMaxBuffer(dwDevID)){
//		*pdwReadPtr = gpDevice[dwDevID].pSoeWrap->Ptr.dwWritePtr-GetSoeMaxBuffer(dwDevID);
//	}
	
//	*pSysTimeSoe = gpDevice[dwDevID].pSysTimeSoe[ (*pdwReadPtr)%GetSoeMaxBuffer(dwDevID) ];
	*pSysTimeSoe = YXBrustRecord[(*pdwReadPtr)%MAXYXRECORD];
	(*pdwReadPtr)++;
    return TRUE;
}

static bool ReadSysTimeSoe(DWORD dwDevID,WORD *pdwReadPtr, TBurstYXRecord *pSysTimeSoe)
{
	//防错
	if(dwDevID >= gVars.dwDevNum){
		return FALSE; 
	}
//	if(!gpDevice[dwDevID].pSoeWrap){
//		return FALSE;
//	}
	
	if(*pdwReadPtr == gVars.YxWritePtr){
		return FALSE;
	}
	
	
	return Real_ReadSysTimeSoe(dwDevID,pdwReadPtr,pSysTimeSoe);	
}



BOOL Txd_OldCP56TSoe(int fd) //发送旧SOE数据
{
	BYTE bStyle = 0x1e;
	BYTE bReason = 0x24;
	BYTE bPRM = 1, bCode = 3;
	BYTE bNum;
	TBurstYXRecord sysSoe; 
	WORD wNo;
	WORD wMSecond;
	BYTE* pBuf;
	DWORD TempDevID = 0;
	DWORD devadd1,devadd2,devadd3;
	int i;
	
	
	GetDevFromAddr(7, &TempDevID);



	

	pBuf = m_Txd.Buf;
	
	Txd_Head101(bStyle, bReason);

	for(bNum = 0; bNum < 20 ; bNum++)
	{
		if(SOESendNum >= dwSoeDataNum){
			FlagSendOldSoe = 0;//判断发完所有soe后标志位清零
			SOESendNum = 0;//soe已发送数清零
			Flag_SoeSendAll = 1;
			break;
		}
	
		if(ReadSysTimeSoe(TempDevID,&SoedwReadPtr,&sysSoe) == FALSE){
			break;
		}
//		IncSoeReadPtr(m_dwDevID);
//		m_pCfg->spi.wStartAddr=0x01;
		wNo = sysSoe.point + 1;

//		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wNo);  //点号L
//		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wNo);  //点号H
		if(m_pCfg.bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
		else if(m_pCfg.bInfoAddr == 3)
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
	
		pBuf[ m_Txd.WritePtr++ ] = (sysSoe.bStatus & PBI_ON) ? 1:0;  //状态
		
		wMSecond = sysSoe.stTime._milliseconds;
		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wMSecond);  //MSEC L
		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wMSecond);  //MSEC H
		
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._min._minutes;
		
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._hour._hours;	//MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._day._dayofmonth;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._month._month;  //MSEC H
		pBuf[ m_Txd.WritePtr++ ] = sysSoe.stTime._year._year%100;	//MSEC H
		
		SoedwReadPtr++;
		SOESendNum++;


		
		if(m_Txd.WritePtr >= (m_pCfg.wMaxFrm-TAILLEN-m_pCfg.bInfoAddr-7))
			break;
		

	}
	
	if(bNum == 0)
		return FALSE;

	
	Txd_Tail101(fd, bPRM, bCode, bNum);



	return TRUE;
}

BOOL TxdCallDZGroupAck(int fd, BYTE COI)//组召唤激活确认
{
	BYTE bStyle = 0x6c, bReason = 7;
	BYTE bPRM = 1, bCode = 3, bNum = 1;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;

	Txd_Head101(bStyle, bReason);

//	m_Txd.Buf[ m_Txd.WritePtr++ ] = 00; //信息体地址Lo
//	m_Txd.Buf[ m_Txd.WritePtr++ ] = 00; //信息体地址Hi
	if(m_pCfg.bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
	else if(m_pCfg.bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
	}

	m_Txd.Buf[ m_Txd.WritePtr++ ] = COI; //COI
	Txd_Tail101(fd, bPRM, bCode, bNum);

	return TRUE;
}
BOOL TxdCallDZGROUPData(int fd)
{
		BYTE TI;
		BYTE bReason;
		BYTE bPRM = 1, bCode = 3;
		WORD wDZNum, wNo;
		int i = 0;
		BYTE* pBuf;
		WORD dwStatus;
		DWORD dwValue;
		WORD wSendFlag;
		BYTE bNum;
		int nStart;
		TI = 112;
		pBuf = m_Txd.Buf;
	
		wDZNum = 114;
		bReason = DZGroupID + 0x15;
		
		nStart = DZGroupDataNum;
		
	
		Txd_Head101(TI, bReason);
		
		bNum = 0;
		
//		wNo = i+m_pCfg->yc.wStartAddr;
		wNo = DZGroupDataNum + 0x5001 + DZGroupID*114;
	
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //信息体地址Hi
//		pBuf[m_Txd.WritePtr++] = 0;
		for(i = nStart;  i<wDZNum; i++)
		{
			dwValue = S101DzValue[DZGroupID*114 + i].DZValue;
	
			
			pBuf[m_Txd.WritePtr++] = LLBYTE(dwValue); //遥测值 Lo
			pBuf[m_Txd.WritePtr++] = LHBYTE(dwValue); //遥测值 Hi
			pBuf[m_Txd.WritePtr++] = HLBYTE(dwValue); //遥测值 Lo
			pBuf[m_Txd.WritePtr++] = HHBYTE(dwValue); //遥测值 Hi
			
			bNum ++;
			DZGroupDataNum++;
			if(DZGroupDataNum >= 114)
				break;
			if(m_Txd.WritePtr >= (m_pCfg.wMaxFrm-TAILLEN-4))
				break;
		}
		
		if(bNum == 0)
			return FALSE;
		pBuf[m_Txd.WritePtr++] = 6; //QPM
		Txd_Tail101(fd, bPRM, bCode, bNum|0x80);
		
		return TRUE;


}

BOOL TxdCallDZGROUP(int fd)
{
	if(Flag_Call_GROUPDZData == FALSE)
		return FALSE;



	PRINT_FUNLINE;
 	if(TxdCallDZGROUPData(fd))
 		return TRUE;
	DZGroupDataNum = 0;
	
	Flag_Call_GROUPDZData = FALSE;

	Txd_AllDZStop(fd, COICALL);
	YCsendEnable_Flag = true;
	return TRUE;
}

BOOL RxdCallDZ(int fd, int DevID, uint8_t *pBuf)//召唤参数
{

	
	COICALL = pBuf[14];
	ACK(fd);
	if(COICALL == 0x14){
		SetALLDZFlag101(DZ_CALL_ALL);
		TxdCallDZALLAck(fd, COICALL);
	}else{
		DZGroupID = COICALL - 0x15;
		SetGROUPDZFlag101(DZGroupID, DZ_CALL_GROUP);
		SF_CallDZGroupData = TRUE;//置定值组召唤标记
		TxdCallDZGroupAck(fd, COICALL);
	}
	

	return 1;
}

BOOL ACK(int fd)
{
//	BYTE bRxdCode;
//	BYTE bTxdCode = 0;
//	
//	bRxdCode = GetDevFlagValue(SF_RXDCONTROL)& BIT_CODE;
//
//	if(bRxdCode == 4)//no answer
//		return TRUE;
//
//	switch(bRxdCode)
//	{
//		case 0:
//		case 1:
//		case 2:
//		case 3:
//			bTxdCode = 0;
//			break;
//		case 4:
//			return TRUE;
//		case 8:
//		case 9:
//			bTxdCode = 11;
//			break;
//		default:
//			bTxdCode = 15;//链路未完成
//			break;
//	}
	ResConfirm(fd);

	return 1;
}
uint8_t ResConfirm(int fd)
{

    uint16_t len;

    PGX101Slaver_10_T GX101Slaver = (PGX101Slaver_10_T)&gx101_Sendbuf;

    len = GX101Slaver_STABLE_LEN;

    GX101Slaver->_begin = GX101Slaver_STABLE_BEGING;

    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_SLAVE;
    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_NONE_DATA;
    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CAN_REC;

    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_RES_CONFIRM;

    GX101Slaver->_addr = Slaver_Sta_Addr;
    GX101Slaver->_cs = gx101_Sendbuf[1]+gx101_Sendbuf[2]+gx101_Sendbuf[3];
    GX101Slaver->_end = GX101Slaver_STABLE_END;

	
	CK_send(fd,gx101_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(gx101_Sendbuf, len, Prio, NULL,NULL);

    return 0;
}


/*以下三个函数是给，发送数据时做标记用的*/
void SetALLDZFlag(DWORD dwDevID, WORD wFlag)
{
	TLogicDZ* pLogicDZ;
	int i;
	DWORD dwNum;

	if(dwDevID>=gVars.dwDevNum)
		return;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return;

	dwNum = gpDevice[dwDevID].DZNum;
	pLogicDZ = gpDevice[dwDevID].pLogicBase->pLogicDZ;

	if(pLogicDZ == NULL || dwNum == 0)
		return;

	for(i=0; i<dwNum; i++)
		pLogicDZ[i].wFlag = wFlag;
}

WORD GetDZFlag(DWORD dwDevID, DWORD dwNo)
{
	TLogicDZ* pLogicDZ;
	DWORD dwNum;

	if(dwDevID >= gVars.dwDevNum)
		return 0;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return 0;

	dwNum = gpDevice[dwDevID].DZNum;
	pLogicDZ = gpDevice[dwDevID].pLogicBase->pLogicDZ;

	if(pLogicDZ == NULL || dwNum == 0)
		return 0;

	if(dwNo >= dwNum)
		return 0;
	
	return pLogicDZ[dwNo].wFlag;
}


void SetDZDataFlag()
{
	
	
	SetALLDZFlag(m_dwDevID, 0xFF00);

	SF_CallDZData = TRUE;//置定值召唤总标记
	SF_CallDZCmdFlag = TRUE;//响应定值召唤标记
}
BOOL TxdCallBack(int fd, BYTE TI, BYTE bReason, BYTE COI)
{
	BYTE bPRM = 1, bCode = 3, bNum = 1;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;

	Txd_Head101(TI, bReason);

//	m_Txd.Buf[ m_Txd.WritePtr++ ] = 00; //信息体地址Lo
//	m_Txd.Buf[ m_Txd.WritePtr++ ] = 00; //信息体地址Hi
	if(m_pCfg.bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
	else if(m_pCfg.bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
	}

	m_Txd.Buf[ m_Txd.WritePtr++ ] = COI; //COI
	Txd_Tail101(fd, bPRM, bCode, bNum);

	return TRUE;
}

static BOOL RxdCallAll(int fd, uint8_t *pbuf)//总召唤
{
	BYTE TI = 0x64, bReason = 7, COI;
	DWORD TempDevID;
	COI = pbuf[14];
	GroupCallCOI = pbuf[14];
	
	if(COI == 0x14){
//		SetALLDataFlag();
		ACK(fd);
		Txd_AllStop(fd, 0x14);

//		TxdCallAllAck();
	}else if(COI == 0x24){//第16组召唤

		ACK(fd);

		FlagCallOldSoe = 1;
		Now_Time_cnt = 0;
		GetDevFromAddr(5,&TempDevID);
		gpDevice[TempDevID].pDZ[6]|=0x1;
		gpDevice[TempDevID].pDZ[6]|=0x2;
		gpDevice[TempDevID].pDZ[6]|=0x4;
		log("gpDevice[TempDevID].pDZ[6](%x)\n",gpDevice[TempDevID].pDZ[6]);
		PRINT_FUNLINE;
		SoedwReadPtr = 0;//历史soe第0个开始发起
		SOESendNum = 0;
		
//		if(gpDevice[m_dwDevID].SoeSendNum == 0){//无soe则发送结束
//				TxdCallBack(fd, TI, 0x0a, COI);
//				return FALSE;
//		}
//		FlagSendOldSoe = 1;
//		SOESendNum = 0;
//		
//		
//		
//		if(gpDevice[m_dwDevID].SoeSendNum >= 100){
//			SoedwReadPtr = gpDevice[m_dwDevID].SoeSendNum - 100;//历史soe大于100，则从倒数第100个开始发起
//			dwSoeDataNum = 100;
//		}else {
//			SoedwReadPtr = 0;//历史soe不大于100，则从倒数第0个开始发起
//			dwSoeDataNum = gpDevice[m_dwDevID].SoeSendNum;
//			
//		}
//		
//		TxdCallBack(fd, TI, bReason, COI);
		

	}else if(COI > 0x14 && COI < 0x24){
//		if((COI - 0x14) > bGroupALL){//组召唤大于总分组数，则否定激活
//			ACK();
//			TxdCallBack(TI, 0x47, COI);
//			return FALSE;
//		}
//		FlagGourpCall = 1;
//		ACK();
//		TxdCallBack(TI, bReason, COI);
	}


	return TRUE;
}
static BYTE GetCtrCode(BYTE bPRM, BYTE bCode)
{
//	BYTE bCodeTmp = 0x80;

	BYTE bCodeTmp = 0;
	
	bCodeTmp += bCode;
	
	if(bPRM)
		bCodeTmp|=0x40;

// 	if(SearchClass1())
// 		bCodeTmp |= 0x20;//变位遥信为1级数据

	return bCodeTmp;
}


BOOL TxdFixFrame(int fd, BYTE bPRM, BYTE bCode)
{
	BYTE bChkSum; 
	BYTE *pBuf;
	int len = 0;

	pBuf = m_Txd.Buf;
	m_Txd.ReadPtr = 0;
	m_Txd.WritePtr = 0;
 
	
	pBuf[m_Txd.WritePtr++] = 0x10;//启动字符
	pBuf[m_Txd.WritePtr++] = GetCtrCode(bPRM, bCode);//控制域

	pBuf[m_Txd.WritePtr++] = 1;//链路地址域（子站站址）
	pBuf[m_Txd.WritePtr++] = 0;//链路地址域（子站站址）
	
	bChkSum = (BYTE)ChkSum(pBuf+1, 3);


	pBuf[m_Txd.WritePtr++] = bChkSum;
	pBuf[m_Txd.WritePtr++] = 0x16;
	len = m_Txd.WritePtr;
	log("len (%d)\n",len);
	CK_send(fd,m_Txd.Buf,len);
		
	return TRUE;
}
BOOL RxdReqLinkStatus(int fd)
{
	TxdFixFrame(fd, 0, 0x0B);

	return TRUE;
}
BOOL RxdResetLink(int fd, uint8_t *pBuf)
{
	BYTE bControl;

	bControl = pBuf[1];

	if(bControl & BIT_PRM)
	{
//		SetDevFlag(SF_RtuInitOk);   //置RTU初始化结束状态

//		if(m_wSelfInit == 0x55aa)
//		{
//			SetDevFlag(SF_SendInitFlag);//置位需要发送初始化结束帧
//			m_wSelfInit = 0xaa55;
//		}
	}
	
	ACK(fd);
	TxdFixFrame(fd, 1, 9);//增加第五步
	return TRUE;
}

BOOL TxdCallDZALLAck(int fd, BYTE COI)//定值总召唤激活确认
{
	BYTE bStyle = 0x6c, bReason = 7;
	BYTE bPRM = 1, bCode = 3, bNum = 1;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;

	Txd_Head101(bStyle, bReason);

//	m_Txd.Buf[ m_Txd.WritePtr++ ] = 00; //信息体地址Lo
//	m_Txd.Buf[ m_Txd.WritePtr++ ] = 00; //信息体地址Hi
	if(m_pCfg.bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
	else if(m_pCfg.bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
	}

	m_Txd.Buf[ m_Txd.WritePtr++ ] = COI; //COI
	Txd_Tail101(fd, bPRM, bCode, bNum);

	return TRUE;
}

//=======================================================================================
// 函数功能: 处理报文的头部
//=======================================================================================
BOOL Txd_Head101(BYTE bType, BYTE bReason)
{
	BYTE* pBuf;
	WORD wAddr;
	BYTE bLen;

	pBuf = m_Txd.Buf;

	m_Txd.ReadPtr = 0;
	m_Txd.WritePtr = 0;

	bLen = 0;
	
	wAddr = 0x01;

	pBuf[0] = pBuf[3] = 0x68;

	bLen = 5;

	pBuf[bLen++] = LOBYTE(wAddr);

	if(m_pCfg.bLinkAddr == 2)
		pBuf[bLen++] = HIBYTE(wAddr);
	
	pBuf[bLen++] = bType;

	bLen ++;//bDefinitive
	
	pBuf[bLen++] = bReason|m_pCfg.bTestMode;

	if(m_pCfg.bCauseTrans == 2)
		pBuf[bLen++] = 0;
	
	pBuf[bLen++] = LOBYTE(wAddr);
	
	if(m_pCfg.bAsduAddr == 2)
		pBuf[bLen++] = HIBYTE(wAddr);
	
	m_Txd.WritePtr = bLen;

//	pTxdFm = (TSIEC101gx_2002TxdFm *)m_Txd.Buf;

//	pTxdFm->varFm.bStart1 = pTxdFm->varFm.bStart2 = 0x68;
//	pTxdFm->varFm.bAddress1 = pTxdFm->varFm.bAddress2 = GetAddress();
//	pTxdFm->varFm.bType		= bType;
//	pTxdFm->varFm.bReason	= bReason;
	
//	m_Txd.WritePtr = (BYTE*)&pTxdFm->varFm.bData - (BYTE*)&pTxdFm->varFm.bStart1;
	
	return TRUE;
}

//=======================================================================================
// 函数功能: 处理报文的尾部
//=======================================================================================
BOOL Txd_Tail101(int fd, BYTE bPRM, BYTE bCode, BYTE bNum)
{
	BYTE bChkSum;
	BYTE* pBuf;
	uint8_t len = 0;
	pBuf = m_Txd.Buf;

	pBuf[1] = pBuf[2] = m_Txd.WritePtr-4;
	pBuf[4] = GetCtrCode(bPRM, bCode);
	pBuf[4] |=0x10;

	if(m_pCfg.bLinkAddr == 2)
		pBuf[8] = bNum;
	else
		pBuf[7] = bNum;

	bChkSum = ChkSum(pBuf+4, pBuf[1]);

	pBuf[m_Txd.WritePtr++] = bChkSum;
	pBuf[m_Txd.WritePtr++] = 0x16;
	len = m_Txd.WritePtr;
	
	CK_send(fd,m_Txd.Buf,len);


	return TRUE;
}
BOOL Txd_AllDZStop(int fd, BYTE COI)//发送定值召唤结束帧
{
	BYTE bStyle = 108;
	BYTE bReason = 0x0A;
	BYTE bPRM = 1, bCode = 3, bNum = 1;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;

	Txd_Head101(bStyle, bReason);

	if(m_pCfg.bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
	else if(m_pCfg.bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
	}

	pBuf[ m_Txd.WritePtr++ ] = COI; //COI

	Txd_Tail101(fd, bPRM, bCode, bNum);
	log("Dz cal over\n");
	return TRUE;
}

BOOL TxdCallDZData(int fd)
{
	BYTE TI;
	BYTE bReason = 20;
	BYTE bPRM = 1, bCode = 3;
	WORD wDZNum, wNo;
	int i;
	BYTE* pBuf;
	WORD dwStatus;
	DWORD dwValue;
	WORD wSendFlag;
	BYTE bNum;
	int nStart;
	Data_Value_T DataValue;
	
	TI = 112;
	pBuf = m_Txd.Buf;

	wDZNum = 342;
	log("dznum is (%d)\n",wDZNum);
	for(i=0; i<wDZNum; i++)
	{
		wSendFlag = S101DzValue[i].Flag;
		if(wSendFlag == DZ_CALL_ALL)
			break;
	}
	
	if(i >= wDZNum)
		return FALSE;
	
	nStart = i;
	
	Txd_Head101(TI, bReason);
	
	bNum = 0;
	
//		wNo = i+m_pCfg->yc.wStartAddr;
	wNo = i+0x5001;

	pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
	pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //信息体地址Hi
//		pBuf[m_Txd.WritePtr++] = 0;
	for(i = nStart;  i<wDZNum; i++)
	{
		if(i>=wDZNum){
			perror("i>=wDZNum");
			return FALSE;
		}
	
		wSendFlag = S101DzValue[i].Flag;

		if(wSendFlag != DZ_CALL_ALL){//总召唤标志
			break;
		}
		
		S101DzValue[i].Flag = 0;//请发送标记

		
		dwValue = S101DzValue[i].DZValue;
		DataValue._int32 = dwValue;
		if(dwValue)
			log("devid(%d)addr(%d)value(%x)value(%f)\n",i/114+1, i%114+1, dwValue, DataValue._float);
		
		pBuf[m_Txd.WritePtr++] = LLBYTE(dwValue); 
		pBuf[m_Txd.WritePtr++] = LHBYTE(dwValue); 
		pBuf[m_Txd.WritePtr++] = HLBYTE(dwValue); 
		pBuf[m_Txd.WritePtr++] = HHBYTE(dwValue);
		
		bNum ++;

		if(m_Txd.WritePtr >= (m_pCfg.wMaxFrm-TAILLEN-4)){
			break;
		}
	}
	
	if(bNum == 0){
		return FALSE;
	}
	pBuf[m_Txd.WritePtr++] = 6; //QPM
	Txd_Tail101(fd, bPRM, bCode, bNum|0x80);
	
	return TRUE;


}
BOOL TxdCallDZReturn(int fd)
{
	BYTE TI;
	BYTE bReason = 7;
	BYTE bPRM = 1, bCode = 3;
	WORD wDZNum, wNo;
	int i;
	BYTE* pBuf;
	WORD dwStatus;
	DWORD dwValue;
	WORD wSendFlag;
	BYTE bNum;
	int nStart;
	DWORD value;
	BYTE Flag_sq = 0;
	BYTE DZReturnNum = 0;
	BYTE bufftmp[100];
	
	TI = 0x70;
	pBuf = m_Txd.Buf;

	wDZNum = gpDevice[m_dwDevID].DZNum;

	for(i=0;i<(wDZNum-1);i++){
		if(S101DzValueBackUp[i].Flag == DZ_YZ_CONFIRM){
			if(DZReturnNum >= 100)
				break;
			bufftmp[DZReturnNum] = i;
			DZReturnNum ++;
		}

	}
	log("DZReturnNum is (%d)\n",DZReturnNum);
	
	if(DZReturnNum == 0)
		return FALSE;
	
	for(i=0;i<(DZReturnNum-1);i++){
		log("i is (%d)(%d)\n",bufftmp[i],bufftmp[i+1]);
		if((bufftmp[i] + 1) == bufftmp[i + 1])
			Flag_sq = 0x80;
	}
	log("Flag_sq is (%d)\n",Flag_sq);
	Txd_Head101(TI, DZReturnReanson101);
	m_Txd.WritePtr = 12;
	if(Flag_sq){//连续
		wNo = bufftmp[0] + 0x5001;
		log("wNo is (%d)\n",wNo);
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //信息体地址Hi
		for(i=0;i<DZReturnNum;i++){
			value = S101DzValueBackUp[bufftmp[i]].DZValue;
			pBuf[m_Txd.WritePtr++] = LLBYTE(value);
			pBuf[m_Txd.WritePtr++] = LHBYTE(value); 
			pBuf[m_Txd.WritePtr++] = HLBYTE(value); 
			pBuf[m_Txd.WritePtr++] = HHBYTE(value);
			bNum++;
			if(m_Txd.WritePtr >= (m_pCfg.wMaxFrm-TAILLEN-4)){
				break;
			}
			
		}
		pBuf[m_Txd.WritePtr++] = 9; //QPM
		Txd_Tail101(fd, bPRM, bCode, bNum|0x80);
		return TRUE;

	}
//	log("DZReturnNum is (%d)\n",DZReturnNum);
	//不连续
	for(i=0;i<DZReturnNum;i++){
		wNo = bufftmp[i] + 0x5001;
//		log("wNo is (%d)\n",wNo);
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //信息体地址Hi
		value = S101DzValueBackUp[bufftmp[i]].DZValue;
		pBuf[m_Txd.WritePtr++] = LLBYTE(value);
		pBuf[m_Txd.WritePtr++] = LHBYTE(value); 
		pBuf[m_Txd.WritePtr++] = HLBYTE(value); 
		pBuf[m_Txd.WritePtr++] = HHBYTE(value);
		
		if(m_Txd.WritePtr >= (m_pCfg.wMaxFrm-TAILLEN-4)){
			break;
		}
	}
	pBuf[m_Txd.WritePtr++] = 9; //QPM
	Txd_Tail101(fd, bPRM, bCode, bNum);
	return TRUE;


}

BOOL WriteDZReturn(int fd, BYTE *buf)
{
	WORD addr,infoaddr;//公共地址
	
	WORD i;
	WORD asdu_num;
	APDU104_T *gx104buf = NULL;
	DWORD value;
	WORD offset;
	BYTE *Data;
	gx104buf = (APDU104_T *)buf;
	addr = MAKEWORD(gx104buf->Address_L, gx104buf->Address_H);
	offset = (addr -1)*114; 
	Data = &gx104buf->Data1;
	
	asdu_num = gx104buf->Definitive;
	if ( asdu_num&0x80 ){//连续
		asdu_num ^= 0x80;

		infoaddr = MAKEWORD(gx104buf->AddInfom1, gx104buf->AddInfom2) - 0x5001;
		for(i=0;i<asdu_num;i++){
			value = MAKEDWORD(Data[0], Data[1], Data[2], Data[3]);
			S101DzValueBackUp[offset + infoaddr +i].DZValue = value;
			S101DzValueBackUp[offset + infoaddr +i].Flag = DZ_YZ_CONFIRM;
			Data += 4;
		}
		return TRUE;
	}
	//不连续
	Data = &gx104buf->AddInfom1;
	for(i=0;i<asdu_num;i++){
		infoaddr = MAKEWORD(Data[0], Data[1]) - 0x5001;
		value = MAKEDWORD(Data[3], Data[4], Data[5], Data[6]);
		S101DzValueBackUp[offset + infoaddr].DZValue = value;
		S101DzValueBackUp[offset + infoaddr].Flag = DZ_YZ_CONFIRM;
		Data += 7;
	}
	
	return TRUE;


}



BOOL TxdCallDZAll(int fd)
{
	if(Flag_Call_ALLDZData == FALSE)
		return FALSE;




 	if(TxdCallDZData(fd) == TRUE)
 		return TRUE;

	Flag_Call_ALLDZData = FALSE;

	Txd_AllDZStop(fd, COICALL);
	YCsendEnable_Flag = true;
	return TRUE;
}

BOOL RxdDZYZCmd(int fd, uint8_t *pBuf)//定值预置
{
	BYTE bStyle = 0x70;
	BYTE bReason = 0x07;
	BYTE bPRM = 1, bCode = 3, bNum = info_num_satic;
	char lzBuf[100];
	DWORD info_addr_star = 12;
	DWORD info_data_star = 15;
	DWORD COT_start = 9;
	BYTE COT;
	WORD sq_start = 8;
	WORD rec_len;
	BYTE *dzbuf;
	int i;
	BYTE *pData;
	Data_Value_T DataValue;
	WORD addr;
	rec_len = pBuf[1];
	COT = pBuf[COT_start];
	info_SQ = pBuf[8];
	info_num_satic = pBuf[8]&0x7f;
	info_addr = MAKEWORD(pBuf[info_addr_star], pBuf[info_addr_star+1]);
	dir_DzAddr = (info_addr - 0x5001)/114 +1;

	if(COT == 8){
		bReason = 9;
		MakeMirror(fd, bReason, pBuf);
		upDateDZData(DZ_STOP);
		return TRUE;
	}

	for(i=0;i<342;i++){
		S101DzValueBackUp[i].Flag = DZ_Clear;
	}

	
	if(info_SQ & 0x80){//连续
		pData = &pBuf[info_addr_star + 2];
		addr = MAKEWORD(pBuf[info_addr_star], pBuf[info_addr_star+1]) - 0x5001;
		for(i=0;i<info_num_satic;i++){
			S101DzValueBackUp[addr + i].DZValue = MAKEDWORD(pData[0], pData[1], pData[2], pData[3]);
			S101DzValueBackUp[addr + i].Flag = DZ_YU_ZHI;
			pData+=4;
			
			DataValue._int32 = S101DzValueBackUp[addr + i].DZValue;
			if(S101DzValueBackUp[addr + i].DZValue)
				log("addr(%x)value(%f)\n",addr + i, DataValue._float);
			
		}
		
	}else{//不连续
		pData = &pBuf[info_addr_star];
		for(i=0;i<info_num_satic;i++){
			addr = MAKEWORD(pData[0], pData[1]) - 0x5001;
			S101DzValueBackUp[addr].DZValue = MAKEDWORD(pData[2], pData[3], pData[4], pData[5]);
			S101DzValueBackUp[addr].Flag = DZ_YU_ZHI;
			pData+=6;
			
			DataValue._int32 = S101DzValueBackUp[addr].DZValue;
//			if(S101DzValueBackUp[addr].DZValue)
				log("addr(%d)value(%f)\n", addr+1, DataValue._float);
		}
	
	}
	DZ_Comman = DZ_YU_ZHI;
	return TRUE;

}
int r_call_Dz(DWORD DirAddr,BYTE comman)
{
	int i;
	uint32_t m_TxdDevID;
	
	GetDevFromAddr(DirAddr,&m_TxdDevID);
	gpDevice[m_TxdDevID].Flag_Set_DZ = comman;
	log("dir_DevAddr(%d),devid(%d)\n",DirAddr, m_TxdDevID);
//	if(::GetDevFromAddr(dir_DzAddr,&m_TxdDevID)){//找到模块地址是m_dwDzID的设备
//		::SetDz(m_TxdDevID,  TP_DZ_CALL_SEL, 0, 0);//发送信息给对应的装置
//	}
	
	
	return 0;
}

BOOL TxdDzYkReturn(int fd) //发送遥控预置/执行/取消返回结果	
{
	BYTE bStyle, bReason = 7;
	BYTE bPRM = 1, bCode = 3, bNum = 1;
//	TBO  *pBO;
	BYTE* pBuf;
	
//	pBO = pGetBO();
	
//	if(pBO == NULL)
//		return FALSE;
	
// 	if(CheckClearDevFlag(SF_YkExitFlag))//执行完毕
// 		return TxdYkExeInfo(10);
		
	pBuf = m_Txd.Buf;

	bStyle = 0x71;
	
	Txd_Head101(bStyle, bReason);

	if(m_pCfg.bInfoAddr == 1)
	{
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[1]; //信息体地址Lo
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[2]; //DCO
	}
	else if(m_pCfg.bInfoAddr == 3)
	{
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[1]; //信息体地址Lo
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[2]; //信息体地址Hi
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[3]; //信息体地址Hi
//		pBuf[ m_Txd.WritePtr++ ] = pBO->Info[4]; //DCO
	}
	else
	{
		pBuf[ m_Txd.WritePtr++ ] = 1; //信息体地址Lo
		pBuf[ m_Txd.WritePtr++ ] = 2; //信息体地址Hi
		pBuf[ m_Txd.WritePtr++ ] = 3; //DCO
	}

	Txd_Tail101(fd, bPRM, bCode, bNum);  

	return TRUE;
}

static BOOL Txd_AllStop(int fd, BYTE data)//发送总召唤结束帧
{
	BYTE bStyle = 0x64;
	BYTE bReason = 0x0A;
	BYTE bPRM = 1, bCode = 3, bNum = 1;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;

	Txd_Head101(bStyle, bReason);

	if(m_pCfg.bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
	else if(m_pCfg.bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
	}

	pBuf[ m_Txd.WritePtr++ ] = data; //COI

	Txd_Tail101(fd, bPRM, bCode, bNum);
	
	return TRUE;
}
static BOOL TxdTimeDelay(int fd)
{
	BYTE bStyle = 0x6a, bReason = 7;
	BYTE bPRM = 1, bCode = 3, bNum = 1;
	BYTE* pBuf;
//	TAbsTime absTime;
	WORD wDelayTime;

	pBuf = m_Txd.Buf;

	Txd_Head101(bStyle, bReason);
	
	if(m_pCfg.bInfoAddr == 1)
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
	}
	else if(m_pCfg.bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
		pBuf[m_Txd.WritePtr++] = 0;

	}
	else
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
	}

//	ReadAbsTime(&absTime);

//	if(absTime.Lo >= m_absTime1.Lo)
//		wDelayTime = LOWORD(absTime.Lo - m_absTime1.Lo);
//	else
//		wDelayTime = LOWORD(0xffffffff - m_absTime1.Lo + absTime.Lo + 1);

	wDelayTime = m_wFromTime + wDelayTime;

	pBuf[m_Txd.WritePtr++] = LOBYTE(wDelayTime);
	pBuf[m_Txd.WritePtr++] = HIBYTE(wDelayTime);
	
	Txd_Tail101(fd, bPRM, bCode, bNum);

	return TRUE;
}

static BOOL RxdDelay(int fd, uint8_t *pBuf)
{
	WORD wReason;
	BYTE *pData;
	
	pData = pBuf + 8;

	if(m_pCfg.bLinkAddr == 2)
		pData += 1;
	
	if(m_pCfg.bCauseTrans == 2)
		wReason = MAKEWORD(pData[0],pData[1]);
	else
		wReason = pData[0];
	
	pData += m_pCfg.bCauseTrans;
	pData += m_pCfg.bAsduAddr;
	pData += m_pCfg.bInfoAddr;
	
	if(wReason == 6)
	{
//		ReadAbsTime(&m_absTime1);
		m_wFromTime = MAKEWORD(pData[0],pData[1]);
// 		SetDevFlag(SF_CallDelayFlag);
 		TxdTimeDelay(fd);
// 		TXD_RxdDelay();
	}
	else if(wReason == 3)
	{
		m_wDelayTime = MAKEWORD(pData[0],pData[1]);
	}
	else
	{
		MakeMirror(fd, 45|0x40, pBuf);
		ACK(fd);
		return FALSE;
	}
	ACK(fd);
	return TRUE;
}
static BOOL TxdMirror(int fd)
{
	BYTE* pBuf;
	BYTE bLen;
	BYTE bChkSum;
	

	bLen = m_bMirrorBuf[1] + 6;

	pBuf = m_bMirrorBuf;
	
	memcpy(pBuf, m_bMirrorBuf, bLen);


	bChkSum = ChkSum(pBuf+4, pBuf[1]);

	pBuf[pBuf[1]+4] = bChkSum;
	

	
	CK_send(fd,pBuf,bLen);
	return TRUE;
}

static void MakeMirror(int fd, BYTE bReason, uint8_t *pbuf)
{
	memcpy(m_bMirrorBuf, pbuf, pbuf[1]+6);
	
	m_bMirrorBuf[4] = 0x53;
	
	if(bReason != 0xFF)
	{
		if(m_pCfg.bLinkAddr == 2)
			m_bMirrorBuf[9] = bReason;
		else
			m_bMirrorBuf[8] = bReason;
	}
	TxdMirror(fd);
}
static BOOL TxdClockSyn(int fd, uint8_t *rbuf)//对钟的确认
{
	BYTE bStyle = 0x67, bReason = 7;
	BYTE bPRM = 1, bCode = 3, bNum = 1;
	BYTE* pData;
	BYTE* pBuf;
  
	pBuf = m_Txd.Buf;
	pData = rbuf + 10;

	if(m_pCfg.bLinkAddr == 2)
		pData += 1;
	if(m_pCfg.bAsduAddr == 2)
		pData += 1; 
	if(m_pCfg.bCauseTrans == 2)
		pData += 1;

	Txd_Head101(bStyle, bReason);
	
//	m_Txd.Buf[ m_Txd.WritePtr++ ] = 00; //信息体地址Lo
//	m_Txd.Buf[ m_Txd.WritePtr++ ] = 00; //信息体地址Hi
	if(m_pCfg.bInfoAddr == 1)
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pData += 1;
	}
	else if(m_pCfg.bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
		pBuf[m_Txd.WritePtr++] = 0;

		pData += 3;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi

		pData += 2;
	}
	WORD i;
	for(i=0;i<7;i++)
		m_Txd.Buf[ m_Txd.WritePtr++ ] = pData[i]; // 
	
	Txd_Tail101(fd, bPRM, bCode, bNum);

	return TRUE;
}

static BOOL RxdClockSyn(int fd, uint8_t *pBuf)//时钟同步
{
	BYTE* pData;
	TSysTime sysTime;	
	WORD wMSecond;
//	TAbsTime absTime;
//	TAbsTime beforSynTime;
	BYTE bReason;
	int nDiffer;
	DWORD dwControl = 0;
	
	//zym add 判断是否接收该端口的对时命令
//	dwControl = gpDevice[m_dwDevID].dwControl;
//	dwControl &= 0x700;
//	if(dwControl==0x100)    //禁止对钟
//		return;
//	//end

	pData = pBuf + 10;

	if(m_pCfg.bLinkAddr == 2)
		bReason = pBuf[9];
	else
		bReason = pBuf[8];
	
	if(bReason != 6)
	{
		MakeMirror(fd, 45|0x40, pBuf);
		ACK(fd);
		return FALSE;
	}

	if(m_pCfg.bLinkAddr == 2)
		pData += 1;
	if(m_pCfg.bAsduAddr == 2)
		pData += 1;
	if(m_pCfg.bCauseTrans == 2)
		pData += 1;
	
	if(m_pCfg.bInfoAddr == 1)
		pData += 1;
	if(m_pCfg.bInfoAddr == 2)
		pData += 2;
	if(m_pCfg.bInfoAddr == 3)
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

//	nDiffer = absTime.Lo - beforSynTime.Lo;
//
//	if(abs(nDiffer) > 3000)
//		SetDevFlag(SF_ClockDifferFlag);


	ACK(fd);

	TxdClockSyn(fd, pBuf);//对钟的确认
	
	return TRUE;
}

bool Edit_BrustDz(int fd, BYTE *buff,BYTE len, BYTE DzAddr)
{
	DWORD info_addr;
	BYTE info_addr_start = 12,info_data_start101 = 14,info_data_start104 = 15;
	BYTE *bufftmp;
	BYTE bPRM = 1, bCode = 3, bNum, info_num;
	int i;
	DWORD dwValue;
	Data_Value_T DataValue;
	
	bNum = buff[7];
	bufftmp = m_Txd.Buf;
	memcpy(bufftmp, buff, len);
	Txd_Head101(0x70, 3);

	if(bNum & 0x80){//连续
		info_num = bNum&0x7f;
		info_addr = MAKEDWORD(buff[info_addr_start], buff[info_addr_start+1], 0, 0);
		info_addr = info_addr + (DzAddr-1)*114;
		m_Txd.WritePtr = 12;
		bufftmp[m_Txd.WritePtr++] = LOBYTE(info_addr);	 //控制字节1，发送序列号高
		bufftmp[m_Txd.WritePtr++] = HIBYTE(info_addr); 	//控制字节2
		for(i=0;i<info_num;i++){
			dwValue = MAKEDWORD(buff[info_data_start104 + i*4 +1], buff[info_data_start104 + i*4 + 2], \
				buff[info_data_start104 + i*4 + 3], buff[info_data_start104 + i*4 + 4]) ;
				
			bufftmp[m_Txd.WritePtr++] = LLBYTE(dwValue);
			bufftmp[m_Txd.WritePtr++] = LHBYTE(dwValue); 
			bufftmp[m_Txd.WritePtr++] = HLBYTE(dwValue);
			bufftmp[m_Txd.WritePtr++] = HHBYTE(dwValue);
			DataValue._int32 = dwValue;
			if(dwValue)
				log("(line%d)devid(%d)addr(%d)value(%x)value(%f)\n",__LINE__, i/114+1, i%114, dwValue, DataValue._float);
			
		}
		
	}else{
		info_num = bNum;
		m_Txd.WritePtr = 12;
		for(i=0;i<info_num;i++){
			info_addr = MAKEDWORD(buff[info_addr_start+7*i], buff[info_addr_start+7*i+1], 0, 0);
			info_addr = info_addr + (DzAddr-1)*114;
			bufftmp[info_addr_start+6*i] = LOBYTE(info_addr);	 //控制字节1，发送序列号高
			bufftmp[info_addr_start+6*i+1] = HIBYTE(info_addr); 	//控制字节2
			dwValue = MAKEDWORD(buff[info_addr_start + i*7 + 3], buff[info_addr_start + i*7 + 4], \
				buff[info_addr_start + i*7 + 5], buff[info_addr_start + i*7 + 6]) ;
			bufftmp[info_addr_start + i*6 + 2] = LLBYTE(dwValue);
			bufftmp[info_addr_start + i*6 + 3] = LHBYTE(dwValue); 
			bufftmp[info_addr_start + i*6 + 4] = HLBYTE(dwValue);
			bufftmp[info_addr_start + i*6 + 5] = HHBYTE(dwValue);
			m_Txd.WritePtr += 6;
			DataValue._int32 = dwValue;
			if(dwValue)
				log("devid(%d)addr(%x)value(%x)value(%f)\n", i/114+1, info_addr, dwValue, DataValue._float);
		}
	}
	bufftmp[m_Txd.WritePtr++]=6;//PQM
	Txd_Tail101(fd, bPRM, bCode, bNum);
	
}
/*******************************************************************  
*名称：      		InitTransTab  
*功能：			定值预置防止在地址4对应的ID缓存  
*	
*入口参数：         
*出口参数：正确返回为0，错误返回为-1 
*******************************************************************/ 
int Edit_DZYZ()
{
	int i,j;
	int GroupNum = 3;
	int GroupID = 0;
	int addr = 0;
	int SendNum = 0;
	DWORD tmpDevID;
	BYTE *buf;
	S101DZVALUE_T *DZvalue = NULL;
	
	GetDevFromAddr(4,&tmpDevID);//地址4设备用作定值预置确认缓冲区给101用
	gpDevice[tmpDevID].pDZ[5] = Type_101;//PDZ[5] == 1为101，PDZ[1] == 0为104
	
	buf = (BYTE *)&gpDevice[tmpDevID].pDZ[10];
	
	for(j=0;j<GroupNum;j++){
		for(i=0;i<114;i++){
			addr = j*114 +i;
			if(S101DzValueBackUp[addr].Flag == DZ_YU_ZHI){
				S101DzValueBackUp[addr].Flag = DZ_Clear;
				
				GroupID = j+1;
				log("addr(%d)value(%x)\n",addr +1 , S101DzValueBackUp[addr].DZValue);
				DZvalue = (S101DZVALUE_T *)buf;
				DZvalue->addr = addr;
				DZvalue->DZValue = S101DzValueBackUp[addr].DZValue;
				SendNum++;
				buf += 6;
				if(i == 113){
					gpDevice[tmpDevID].pDZ[0] = SendNum;
					DZ_Comman = DZ_YU_ZHI;
					log("GroupID(%d)\n",GroupID);
					return r_call_Dz(GroupID, TP_DZ_CALL_SEL);
				}
				if(SendNum > 30){
					gpDevice[tmpDevID].pDZ[0] = SendNum;
					DZ_Comman = DZ_YU_ZHI;
					return r_call_Dz(GroupID, TP_DZ_CALL_SEL);
				}
			}

		}
	}
	if(SendNum == 0)
		return 0;
	gpDevice[tmpDevID].pDZ[0] = SendNum;
//	log("SendNum(%d)j+1(%d) sq(%d)\n",SendNum, GroupID,gpDevice[tmpDevID].pDZ[0]);
	return r_call_Dz(GroupID, TP_DZ_CALL_SEL);


}
BOOL Txd_InitFinish(int fd)//发送初始化结束帧
{
	BYTE bStyle = 0x46;
	BYTE bReason = 0x04;
	BYTE bPRM = 1, bCode = 3, bNum = 1;
	BYTE* pBuf;

	pBuf = m_Txd.Buf;

	Txd_Head101(bStyle, bReason);

	if(m_pCfg.bInfoAddr == 1)
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
	else if(m_pCfg.bInfoAddr == 3)
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
		pBuf[m_Txd.WritePtr++] = 0;
	}
	else
	{
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
	}

	pBuf[ m_Txd.WritePtr++ ] = 0; //COI

	Txd_Tail101(fd, bPRM, bCode, bNum);
	
	return TRUE;
}
/*****************************************************************
函数名称: ComPareTime
函数功能: 时间比较函数，time1>time2则返回ture,否则返回false
输入参数: 
	time1:
	time2:
返回值：  
修改日期：2020-3-31

*****************************************************************/
static BOOL ComPareTime(TSysTimeSoe time1, TSysTimeSoe time2) 
{
	if(time1.stTime.Year > time2.stTime.Year){
		return FALSE;
	}else if(time1.stTime.Year < time2.stTime.Year){
		return TRUE;
	}
	if(time1.stTime.Month > time2.stTime.Month){
		return FALSE;
	}else if(time1.stTime.Month < time2.stTime.Month){
		return TRUE;
	}
	
	if(time1.stTime.Day > time2.stTime.Day){
		return FALSE;
	}else if(time1.stTime.Day < time2.stTime.Day){
		return TRUE;
	}

	if(time1.stTime.Hour > time2.stTime.Hour){
		return FALSE;
	}else if(time1.stTime.Hour < time2.stTime.Hour){
		return TRUE;
	}

	if(time1.stTime.Minute > time2.stTime.Minute){
		return FALSE;
	}else if(time1.stTime.Minute < time2.stTime.Minute){
		return TRUE;
	}

	if(time1.stTime.MSecond > time2.stTime.MSecond){
		return FALSE;
	}else if(time1.stTime.MSecond < time2.stTime.MSecond){
		return TRUE;
	}
	return FALSE;


}

/*****************************************************************
函数名称: PaiXu_OldSoe
函数功能: 排序函数，最新时间放在最前
输入参数: 
返回值：历史SOE个数
修改日期：2020-3-31

*****************************************************************/
static DWORD PaiXu_OldSoe() 
{
	DWORD TempDev_1, TempDev_2, TempDev_3, TempDev_all;
	DWORD Dev1_SoeNum = 0, Dev2_SoeNum = 0, Dev3_SoeNum = 0, SoeAllNum = 0;
	int i,j;
    TSysTimeSoe 	tmpSysTimeSoe;	
    TSysTimeSoe 	testsoe;	
    TSysTimeSoe 	testsoe1;	
	BOOL ret;
	DWORD WritePtrNum;
	
	GetDevFromAddr(4,&TempDev_1);
	Dev1_SoeNum = gpDevice[TempDev_1].pSoeWrap->Ptr.dwWritePtr; //已经发出的SOE数
	GetDevFromAddr(5,&TempDev_2);
	Dev2_SoeNum = gpDevice[TempDev_2].pSoeWrap->Ptr.dwWritePtr; //已经发出的SOE数
	GetDevFromAddr(6,&TempDev_3);
	Dev3_SoeNum = gpDevice[TempDev_3].pSoeWrap->Ptr.dwWritePtr; //已经发出的SOE数
	GetDevFromAddr(7,&TempDev_all);
	memset(gpDevice[TempDev_all].pSysTimeSoe,0,400*sizeof(TSysTimeSoe));
	memset(&gpDevice[TempDev_all].pSoeWrap->Ptr,0,sizeof(TPtr));
	
	for(i=0;i<Dev1_SoeNum;i++){
		WritePtrNum = gpDevice[TempDev_all].pSoeWrap->Ptr.dwWritePtr;
		gpDevice[TempDev_all].pSysTimeSoe[WritePtrNum] = gpDevice[TempDev_1].pSysTimeSoe[i];
		gpDevice[TempDev_all].pSoeWrap->Ptr.dwWritePtr++;
		SoeAllNum++;
	}
	for(i=0;i<Dev2_SoeNum;i++){
		WritePtrNum = gpDevice[TempDev_all].pSoeWrap->Ptr.dwWritePtr;
		gpDevice[TempDev_all].pSysTimeSoe[WritePtrNum] = gpDevice[TempDev_2].pSysTimeSoe[i];
		gpDevice[TempDev_all].pSysTimeSoe[WritePtrNum].nNo += 100;
		gpDevice[TempDev_all].pSoeWrap->Ptr.dwWritePtr++;
		SoeAllNum++;
	}
	for(i=0;i<Dev3_SoeNum;i++){
		WritePtrNum = gpDevice[TempDev_all].pSoeWrap->Ptr.dwWritePtr;
		gpDevice[TempDev_all].pSysTimeSoe[WritePtrNum] = gpDevice[TempDev_3].pSysTimeSoe[i];
		gpDevice[TempDev_all].pSysTimeSoe[WritePtrNum].nNo += 200;
		gpDevice[TempDev_all].pSoeWrap->Ptr.dwWritePtr++;
		SoeAllNum++;
	}
	if(SoeAllNum == 0)
		return FALSE;
	//排序
	for(i=0;i<(SoeAllNum - 1);i++){
		for(j=0;j<((SoeAllNum - 1)-i);j++){
			ret = ComPareTime(gpDevice[TempDev_all].pSysTimeSoe[j], gpDevice[TempDev_all].pSysTimeSoe[j+1]);
			if(ret){
				tmpSysTimeSoe = gpDevice[TempDev_all].pSysTimeSoe[j];
				gpDevice[TempDev_all].pSysTimeSoe[j] = gpDevice[TempDev_all].pSysTimeSoe[j+1];
				gpDevice[TempDev_all].pSysTimeSoe[j+1] = tmpSysTimeSoe;
			}

		}

	}

	return SoeAllNum;
}

static BOOL Txd_ChangeAI13(int fd) //发送变化遥测浮点数据
{
	BYTE bStyle;
	BYTE bReason = 3;
	BYTE bPRM = 1, bCode = 3;
	BYTE bNum;
	TBurstAI burstAI;
	DWORD dwBustAINum;
	int i;
	WORD wNo;
	BYTE* pBuf;
	DWORD dwValue;
	WORD wAINum;
	float* pFloat;
	TLogicAI* logicAI = gpDevice[m_dwDevID].pLogicBase->pLogicAI;
	
	
	dwBustAINum = GetBurstAINum(m_dwDevID);
//	log("dwBustAINum is(%d)\n",dwBustAINum);
	if( dwBustAINum==0 ) //无遥测变化
		return FALSE;
	
	bStyle = 13;//short float
	pBuf = m_Txd.Buf;
	
	wAINum = GetAINum(m_dwDevID);

	if(m_pCfg.wCylicDataNum >= wAINum)
		return FALSE;

	wAINum -= m_pCfg.wCylicDataNum;

	if(dwBustAINum == 0 || wAINum == 0)
		return FALSE;
	
	Txd_Head101(bStyle, bReason);

	bNum = 0;

	pFloat = (float*)&dwValue;

	for(i = 0; i < dwBustAINum; i++)
	{
		if(ReadBurstAI(m_dwDevID,&burstAI) != TRUE)
			break;
		
		if(burstAI.dwNo >= wAINum)
			continue;

		bNum ++;
//        m_pCfg->yc.wStartAddr=0x4001;   //内蒙古伊东古城101上调度的遥测信息体起始地址为0x4000，这是97版本的101规约
		
		wNo = burstAI.dwNo + 0x4001;
		*pFloat = (float)burstAI.sValue;
		dwValue = burstAI.sValue;

//		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wNo); //信息体地址Lo
//		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wNo); //信息体地址Hi
		if(m_pCfg.bInfoAddr == 1)
			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
		else if(m_pCfg.bInfoAddr == 3)
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
			
	      
		float a,b,c;//遥测系数
		a =1;
		b =1;
		c =1;
		float val =(*pFloat*b)/(a*c);
        *pFloat= val;


		pBuf[m_Txd.WritePtr++] = LLBYTE(dwValue); //遥测值 Lo
		pBuf[m_Txd.WritePtr++] = LHBYTE(dwValue); //遥测值 Hi
		pBuf[m_Txd.WritePtr++] = HLBYTE(dwValue); //遥测值 Lo
		pBuf[m_Txd.WritePtr++] = HHBYTE(dwValue); //遥测值 Hi
		
		pBuf[m_Txd.WritePtr++] = (burstAI.wStatus&0xFF);//QOS

		if(m_Txd.WritePtr >= (m_pCfg.wMaxFrm-TAILLEN-m_pCfg.bInfoAddr-4))
			break;
	}

	if(bNum == 0)
		return FALSE;

	Txd_Tail101(fd, bPRM, bCode, bNum);
	
	return TRUE;
}

static BOOL Txd_ChangeAI(int fd) //发送变化遥测数据
{
	BYTE bStyle;
	BYTE bReason = 3;
	BYTE bPRM = 1, bCode = 3;
	BYTE bNum;
	short sValue;
	TBurstAI burstAI;
	DWORD dwBustAINum;
	int i;
	WORD wNo;
//	float fValue;
	BYTE* pBuf;
	WORD wAINum;
	
	bStyle = 13;
	if(bStyle == 13)//发送浮点数
		return Txd_ChangeAI13(fd);
//	if(bStyle == 34)//full time nomal value
//		return Txd_ChangeAI34();
//	if(bStyle == 36)
//		return Txd_ChangeAI36();
//
//	pBuf = m_Txd.Buf;
//
//	dwBustAINum = GetBurstAINum(m_dwDevID);
//	wAINum = GetAINum(m_dwDevID);
//
//	if(m_pCfg->wCylicDataNum >= wAINum)
//		return FALSE;
//
//	wAINum -= m_pCfg->wCylicDataNum;
//
//	if(dwBustAINum == 0 || wAINum == 0)
//		return FALSE;
//	
//	Txd_Head(bStyle, bReason);
//
//	bNum = 0;
//
//	for(i = 0; i < dwBustAINum; i++)
//	{
//		if(ReadBurstAI(m_dwDevID,&burstAI) != TRUE)
//			break;
//		
//		if(burstAI.dwNo >= wAINum)
//			continue;
//
//		bNum ++;
////        m_pCfg->yc.wStartAddr=0x4001;   //内蒙古伊东古城101上调度的遥测信息体起始地址为0x4000，这是97版本的101规约
//
//		wNo = burstAI.dwNo + m_pCfg->yc.wStartAddr;
//		sValue = burstAI.sValue;
//
////		pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wNo); //信息体地址Lo
////		pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wNo); //信息体地址Hi
//		if(m_pCfg->bInfoAddr == 1)
//			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
//		else if(m_pCfg->bInfoAddr == 3)
//		{
//			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
//			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //信息体地址Hi
//			pBuf[m_Txd.WritePtr++] = 0;
//		}
//		else
//		{
//			pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
//			pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //信息体地址Hi
//		}
//		
//		pBuf[m_Txd.WritePtr++] = LOBYTE(sValue); //遥测值 Lo
//		pBuf[m_Txd.WritePtr++] = HIBYTE(sValue); //遥测值 Hi
//		
//		if(bStyle == 11 || bStyle == 9)
//			pBuf[m_Txd.WritePtr++] = (burstAI.wStatus&0xFF);
//
//		if(bStyle == 11 || bStyle == 9)
//		{
//			if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-2))
//				break;
//		}
//		else
//		{
//			if(m_Txd.WritePtr >= (m_pCfg->wMaxFrm-TAILLEN-m_pCfg->bInfoAddr-1))
//				break;
//		}
//	}
//
//	if(bNum == 0)
//		return FALSE;
//	
//	Txd_Tail(bPRM, bCode, bNum);
	
	return TRUE;
}


static BOOL TimerOn(DWORD TimeOut) 
{ 
	return (Now_Time_cnt%TimeOut)==0; 
}; //检查是否到了TimeOut间隔定时
void S101OnTimeOut(int fd, int DevID)
{
	DWORD TempDevID;
	DWORD RerurnDevID;
	int i;
	Now_Time_cnt++;

	if(TimerOn(3)){
		if(FlagCallOldSoe){
			FlagCallOldSoe = 0;
			TxdCallBack(fd, 0x64, 0x07, 0x24);
			dwSoeDataNum = PaiXu_OldSoe();
			if(dwSoeDataNum >= 100)
				dwSoeDataNum = 100;
			
			if(dwSoeDataNum){
				FlagSendOldSoe = 1;
			}else{
				Flag_SoeSendAll = 1;
			}
		
		}
	}

	
	
	if(FlagSendOldSoe == 1){
		Txd_OldCP56TSoe(fd);
		return;
	}
	if(Flag_SoeSendAll == 1){
		Flag_SoeSendAll = 0;
		Txd_AllStop(fd, 0x24);
		return;
	}
	
	if(Flag_Call_ALLDZData == TRUE){
		TxdCallDZAll(fd);
		return;
	}
	if(Flag_Call_GROUPDZData == TRUE){
		TxdCallDZGROUP(fd);
		return;
	}
//		if(YXsendEnable_Flag == TRUE){
//			if(GetBurstBINum(m_dwDevID)) //发送变化遥信
//			{
//				if(Txd_BurstBI())
//					return;
//			}
//		
//			if(GetSoeNum(m_dwDevID))
//			{
//				if(Txd_Soe())
//					return;
//			}
//		}



	if(DZ_Comman == DZ_YU_ZHI){
		DZ_Comman = DZ_Clear;
		Edit_DZYZ();
		return;
	}
	if(Now_Time_cnt%6 == 0){
		for(i=0;i<342;i++){
			S101DzValue[i].DZValue = gpDevice[CKDevID].pDZ[i];
		}
	
	}

	
	if(TimerOn(3))// 6秒检查一次，总召结束才发突发遥测
	{
		Txd_ChangeAI(fd);
	}

	BYTE dzBrustAddr;//定值突发的地址号
	BYTE *buff;
	BYTE len;
	GetDevFromAddr(5,&TempDevID);//定值突发缓冲区
	if(gpDevice[TempDevID].pDZ[3] == DZ_TU_FA){//定值突发
		dzBrustAddr = gpDevice[TempDevID].pDZ[2];
		gpDevice[TempDevID].pDZ[3] = DZ_Clear;
		buff = (BYTE *)&gpDevice[TempDevID].pDZ[10];
		len = buff[1];
		Edit_BrustDz(fd, buff,len+2,dzBrustAddr);
		return;
	}
	BYTE *bufftmp;
	GetDevFromAddr(4,&RerurnDevID);//定值预置确认缓冲区
	if(gpDevice[RerurnDevID].pDZ[1] == 1){//定值预置确认
		gpDevice[RerurnDevID].pDZ[1] = 0;
		if(gpDevice[RerurnDevID].pDZ[0] == 0x47){
			DZReturnReanson101 = 0x47;
		}else{
			DZReturnReanson101 = 0x7;
		}
		DZReturnAddr.Dev1 = 1;
		bufftmp = (BYTE *)&gpDevice[RerurnDevID].pDZ[Dev1_DZ_Start];
		WriteDZReturn(fd, bufftmp);
		flag_dz_return = 1;
		return;
	}
	if(gpDevice[RerurnDevID].pDZ[2] == 1){//定值预置确认
		gpDevice[RerurnDevID].pDZ[2]=0;
		if(gpDevice[RerurnDevID].pDZ[0] == 0x47){
			DZReturnReanson101 = 0x47;
		}else{
			DZReturnReanson101 = 0x7;
		}
		DZReturnAddr.Dev2 = 1;
		bufftmp = (BYTE *)&gpDevice[RerurnDevID].pDZ[Dev2_DZ_Start];
		WriteDZReturn(fd, bufftmp);
		flag_dz_return = 1;
		return;
	}
	if(gpDevice[RerurnDevID].pDZ[3] == 1){//定值预置确认
		gpDevice[RerurnDevID].pDZ[3] = 0;
		if(gpDevice[RerurnDevID].pDZ[0] == 0x47){
			DZReturnReanson101 = 0x47;
		}else{
			DZReturnReanson101 = 0x7;
		}
		DZReturnAddr.Dev3 = 1;
		bufftmp = (BYTE *)&gpDevice[RerurnDevID].pDZ[Dev3_DZ_Start];
		WriteDZReturn(fd, bufftmp);
		flag_dz_return = 1;
		return;
	}

	if(TimerOn(2)){
		if(flag_dz_return == 1){
			flag_dz_return = 0;
			TxdCallDZReturn(fd);
			return;
		}
	}

 		


}
void upDateDZData(BYTE common)
{
	int dznum = 342;
	int i;
	if(common == DZ_JI_HUO){
		for(i=0;i<dznum;i++){
			if(S101DzValueBackUp[i].Flag == DZ_YZ_CONFIRM){
				S101DzValueBackUp[i].Flag = DZ_Clear;
				S101DzValue[i].DZValue = S101DzValueBackUp[i].DZValue;
			}
		}
	}else if(common == DZ_STOP){
		for(i=0;i<dznum;i++){
			if(S101DzValueBackUp[i].Flag == DZ_YZ_CONFIRM){
				S101DzValueBackUp[i].Flag = DZ_Clear;
			}
		}
	}
	
}

int siec101gx_Task(int DevID)
{
	return 0;
}


int siec101gx_OnTimeOut(int DevID)
{
	return 0;
}
#endif

