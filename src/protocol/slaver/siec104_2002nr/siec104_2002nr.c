#include "sys.h"

static TS104APDU *pTxdFm;
static TS104APDU *pRxdFm;
BYTE bStartFlag;			//允许数据发送标志
BYTE InitFinishFlag = 0;
WORD wS104_2002nr_TxCount;			//发送帧记数
WORD wR104_2002nr_RxCount;			//接收帧记数		
BYTE Flag_SendOneYX = 0;
static TCommIO m_Txd;				  //发送缓冲区的处理结构
int localfd_s104;

WORD SendNum_S104 = 0;
WORD RecNum_S104 = 0;	
int DzSendFinishFlag;
int DzSendNum;
BYTE CKLogicDevID = 8;
S104DZDATA_T *S104DzValue = NULL;//定值存储区
S104DZDATA_T *S104DzValueBackUp = NULL;//定值待处理区
int DzGroupSendNum;
uint32_t TimecntS104;
BYTE DZ_Comman104 = 0;//1:定值预置 2:定值激活
DZRETURNADDR_T DZReturnAddr104;
WORD flag_dz_return104 = 0;
BYTE DZReturnReanson;
BYTE bCall_Group;			//总召唤分批组数
BYTE flag_call_16 = 0;//16组召唤标志位
BYTE bGroupYx;				//遥信分组数
BYTE bGroupSYx; 			//遥信分组数
BYTE bGroupDYx; 			//遥信分组数
static BYTE bGroupYc;				//遥测分组数
BYTE bGroupMc;				//脉冲分组数	
int INCLUDE_ACTVALUE;
static uint8_t m_dwDevID = 0;


void Edit_InfoAddr_s104(BYTE *pBuf,WORD wNo); 
void Txd_Tail_s104(BYTE bReason, BYTE bNum, BYTE EmpFlat);
void Txd_Head_s104(BYTE bType, BYTE bdata);
void r_Reqe_Erro(BYTE bStyle,BYTE bReason,BYTE bLength);
int RxdAPCIInfo(int DevID, uint8_t *buf, uint16_t len);
int S104OnTimeOut(int DevID);
int siec104Task(int DevID);

int Init_siec104_2002nr(int DevID)
{
	gpDevice[DevID].Receive = RxdAPCIInfo;
	gpDevice[DevID].OnTimeOut = S104OnTimeOut;
	gpDevice[DevID].Task = siec104Task;
	int i;
	m_dwDevID = DevID;
	S104DzValue = (S104DZDATA_T *)malloc(342*sizeof(S104DZDATA_T));
	memset(S104DzValue,0,342*sizeof(S104DZDATA_T));
	S104DzValueBackUp = (S104DZDATA_T *)malloc(342*sizeof(S104DZDATA_T));
	memset(S104DzValueBackUp,0,342*sizeof(S104DZDATA_T));

//	//初始化转发表
//	for(i=0;i<gVars.TransYCTableNum;i++){
//		gpDevice[DevID].pLogicBase->pLogicAI[i].wRealID = TransYCTable[i].nFlag;
//		gpDevice[DevID].pLogicBase->pLogicAI[i].wOffset = TransYCTable[i].nPoint;
//	}
	
	if(gpDevice[Dz_DevID_3].pDZ == NULL){
		perror("dz is null");
		gpDevice[Dz_DevID_3].DZNum = 255;
		gpDevice[Dz_DevID_3].pDZ = (uint32_t*)malloc(gpDevice[Dz_DevID_3].DZNum * sizeof(uint32_t));
		memset(gpDevice[Dz_DevID_3].pDZ, 0, gpDevice[Dz_DevID_3].DZNum * sizeof(uint32_t));
		return FALSE;
	}

	
}

//===================================================================
//  函数名称: CS104_2002nr::Edit_DZ_DATA         
//  函数功能: 编辑总召唤信息量的发送
//  输入参数:                       
//  返回值：无                        
//===================================================================
void Edit_DZ_DATA()
{
	BYTE* pBuf;	
	BYTE bStyle;
	BYTE bNum = 0;
	BYTE bReson = 0x14;	
	WORD wNo;		
	WORD wStatus;
	short sValue;
	DWORD dwValue;
	DWORD i;	
	float* pFloat;
 	char ss[256];
	BYTE DZLogicDevID;
	Data_Value_T DataValue;
	
	DZLogicDevID = CKLogicDevID;


	

	pBuf = m_Txd.Buf;
	

  	bStyle = 0x70;		
 	

  		
  	Txd_Head_s104(bStyle, 0);	
  	wNo = DzSendNum +  STARTDZ104_2002nr;	
	Edit_InfoAddr_s104(pBuf,wNo);
  	
	for(i=0;i<40;i++ )
	{					
		
		dwValue = S104DzValue[DzSendNum].DZValue;
		pBuf[m_Txd.WritePtr++] = LLBYTE(dwValue); //遥测值 Lo
		pBuf[m_Txd.WritePtr++] = LHBYTE(dwValue); //遥测值 Hi
		pBuf[m_Txd.WritePtr++] = HLBYTE(dwValue); //遥测值 Lo
		pBuf[m_Txd.WritePtr++] = HHBYTE(dwValue); //遥测值 Hi
		
		DataValue._int32 = dwValue;
		if(DataValue._int32)
			log("DevID(%d)addr(%d)value(%f)\n", DzSendNum/114+1, DzSendNum%114 + 1, DataValue._float);


		
		bNum ++;
		DzSendNum++;
		if(DzSendNum >= gpDevice[DZLogicDevID].DZNum){
			log("gpDevice[DZLogicDevID].DZNum(%d)\n",gpDevice[DZLogicDevID].DZNum);
			DzSendFinishFlag = 1;
			break;
		}
		
	}	
	pBuf[m_Txd.WritePtr++] = 6; //QPM
	bReson = 0x14;
	Txd_Tail_s104(bReson, (bNum|0x80), 1);
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

//===================================================================
//  函数名称: CS104_2002nr::Edit_DZ_Group_DATA         
//  函数功能: 编辑定值组召唤信息量的发送
//  输入参数:                       
//  返回值：无                        
//===================================================================
void Edit_DZ_Group_DATA(BYTE GroupID)
{
	BYTE* pBuf;	
	BYTE bStyle;
	BYTE bNum = 0;
	BYTE bReson;	
	WORD wNo;		
	WORD wStatus;
	short sValue;
	DWORD dwValue;
	DWORD i;	
	float* pFloat;
 	char ss[256];
	WORD DzGroupDevId;
	BYTE DZLogicDevID;
	DZLogicDevID = CKLogicDevID;
	Data_Value_T DataValue;
	pBuf = m_Txd.Buf;
	

  	bStyle = 0x70;		
 	

  		
  	Txd_Head_s104(bStyle, 0);	
  	wNo = (GroupID-1)*114 + DzGroupSendNum +  STARTDZ104_2002nr;	
	Edit_InfoAddr(pBuf,wNo);
  	
	for(i=0;i<40;i++ )
	{					
		
		dwValue = S104DzValue[(GroupID-1)*114 + DzGroupSendNum].DZValue;
		pBuf[m_Txd.WritePtr++] = LLBYTE(dwValue); //遥测值 Lo
		pBuf[m_Txd.WritePtr++] = LHBYTE(dwValue); //遥测值 Hi
		pBuf[m_Txd.WritePtr++] = HLBYTE(dwValue); //遥测值 Lo
		pBuf[m_Txd.WritePtr++] = HHBYTE(dwValue); //遥测值 Hi

		DataValue._int32 = dwValue;
		if(DataValue._int32)
			log("DevID(%d)addr(%d)value(%f)\n", GroupID, DzGroupSendNum + 1, DataValue._float);
		
		bNum ++;
		DzGroupSendNum++;
		if(DzGroupSendNum >= 114){
			log("DzGroupSendNum(%d)\n",DzGroupSendNum);
			DzSendFinishFlag = 1;
			break;
		}
		
	}	
	pBuf[m_Txd.WritePtr++] = 6; //QPM
	bReson = 0x14 + GroupID;
	Txd_Tail_s104(bReson, (bNum|0x80), 1);
	return;	 
}

//===================================================================
//  函数名称: CS104_2002nr::Edit_DZ_All         
//  函数功能: 编辑总召唤信息量的发送
//  输入参数:                         
//  返回值：无                        
//===================================================================
void Edit_DZ_All_s104()
{
	BYTE *pData;
	pData = ( BYTE* )pRxdFm;
	BYTE qoi;
	BYTE DzGroupID;	//组号从1开始
	
	qoi = pData[15];
	DzSendFinishFlag = 0;
	
	if(qoi == 0x14){
		DzSendNum = 0;
		while(DzSendFinishFlag !=1){
			Edit_DZ_DATA();
		}
		return; 
	}else if(qoi > 0x14){
		DzGroupID = qoi - 0x14;
		if(DzGroupID >=6)
			return;
		DzGroupSendNum = 0;
		while(DzSendFinishFlag !=1){
			Edit_DZ_Group_DATA(DzGroupID);
		}
		
	}

	
}

void Edit_InfoAddr_s104(BYTE *pBuf,WORD wNo) 
{                       
	m_Txd.WritePtr = DEALSTATRT;
	
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

void Txd_Head_s104(BYTE bType, BYTE bdata)
{
	BYTE* pBuf;
	
	pBuf = m_Txd.Buf;
	m_Txd.ReadPtr = 0;
	
	pBuf[0] = STARTHEAD;	//报文头
  										//报文长度，先不处理
  	pBuf[2] = LOBYTE(wS104_2002nr_TxCount);    //控制字节1，发送序列号高
  	pBuf[3] = HIBYTE(wS104_2002nr_TxCount);  	//控制字节2
  	pBuf[4] = LOBYTE(wR104_2002nr_RxCount);    //控制字节3，接收序列号高
  	pBuf[5] = HIBYTE(wR104_2002nr_RxCount);  	//控制字节4
  	pBuf[6] = bType;					//类型表标识
										//传送数目，先不处理
										//传送原因，先不处理
  	pBuf[9] = 0x00;
  	pBuf[10] = 1;					//公共地址，虚拟RTU
  	pBuf[11] = 0x00;					//公共地址改为1个字节
	
	wS104_2002nr_TxCount += 2;				//发送序列号加2
	m_Txd.WritePtr = 12;				//加上信息体地址前的12个字节
	return;
}
void Txd_Tail_s104(BYTE bReason, BYTE bNum, BYTE EmpFlat)
{
	BYTE* pBuf;	

	pBuf = m_Txd.Buf;
	pBuf[1] = m_Txd.WritePtr-2;		//发送报文长度
	pBuf[7] = bNum;					//传送个数
	pBuf[8] = bReason;				//传送原因
//	log("m104send:");
//	DumpHEX(m_Txd.Buf, m_Txd.WritePtr);
	IEC10X->Send(localfd_s104, m_Txd.Buf, m_Txd.WritePtr);
	return;
}

//===================================================================
//  函数名称: CS104_2002nr::Edit_InitFinsh         
//  函数功能: 编辑初始化结束帧的发送
//  输入参数:                       
//  返回值：无                        
//===================================================================
void Edit_InitFinsh(int fd)
{
	BYTE* pBuf;	
	BYTE bTI;
	BYTE bNum = 1;
	BYTE bCOT;	
	WORD wNo;		
	DWORD dwValue;
	DWORD i;	
	float* pFloat;
 	char ss[256];
	WORD DzGroupDevId;
	BYTE DZLogicDevID;
	pBuf = m_Txd.Buf;
	

  	bTI = 0x46;		
  	Txd_Head_s104(bTI, 0);	
  	wNo = 0;	
	Edit_InfoAddr_s104(pBuf,wNo);
  	
	bCOT = 4;
	pBuf[m_Txd.WritePtr++] = 00;//COI
	Txd_Tail_s104(bCOT, bNum, 1);
	return;	 

}
//================================================================
//  函数名称: CS104_2002nr::r_Reqe_Erro
//  函数功能: 子站对主站报文的肯定或否定确认
//  输入参数: bStyle类型标识，bReason传送原因，bLength传送长度。
//  返回值：  无
//================================================================
void r_Reqe_Erro_s104(BYTE bStyle,BYTE bReason,BYTE bLength)
{
	BYTE i = 0;
	BYTE* pBuf;
	
	if( bLength<10 )
		return;
		
	pBuf = (BYTE *)pRxdFm;
	Txd_Head_s104(bStyle,0);
	
	for( i=0;i<bLength-10;i++ )
	{
		m_Txd.Buf[DEALSTATRT_2002nr+i]=pBuf[DEALSTATRT_2002nr+i];
	}
	
	m_Txd.WritePtr += bLength-10;
	Txd_Tail_s104(bReason, 1,1);
}
void upDateDZData104(BYTE common)
{
	int dznum = 342;
	int i;
	if(common == DZ_JI_HUO){
		for(i=0;i<dznum;i++){
			if(S104DzValueBackUp[i].Flag == DZ_YZ_CONFIRM){
				S104DzValueBackUp[i].Flag = DZ_Clear;
				S104DzValue[i].DZValue = S104DzValueBackUp[i].DZValue;
			}
		}
	}else if(common == DZ_STOP){
		for(i=0;i<dznum;i++){
			if(S104DzValueBackUp[i].Flag == DZ_YZ_CONFIRM){
				S104DzValueBackUp[i].Flag = DZ_Clear;
			}
		}

	}
	

	
}

//===================================================================
//  函数名称: CS104_2002nr::r_call_DZ_YZ         
//  函数功能: 
//  输入参数:  bLength 帧长度（不含包头和帧长度）              
//  返回值：无                        
//===================================================================
BOOL r_call_DZ_YZ(BYTE bLength)
{
	BYTE *pData;
	BYTE *pbuf;
	BYTE reason;
	BYTE bStyle;
	int i;
	BYTE info_sq;
	BYTE *dzbuf;
	WORD sq_start = 7;
	DWORD info_addr_star = 12;
	DWORD info_addr;
	DWORD addr;
	Data_Value_T DataValue;
	DWORD infonum;
	
	pbuf = ( BYTE* )pRxdFm;
	bStyle = pbuf[6];
	info_sq = pbuf[7];
	reason = pbuf[8];

	infonum = pRxdFm->Definitive;

	if(reason == 8){
		reason = 9;
		memcpy(m_Txd.Buf,pRxdFm,bLength + 2);//从info_sq开始缓存数据
		Txd_Head_s104(bStyle,0);
		m_Txd.WritePtr += bLength-10;
		Txd_Tail_s104(reason, info_sq,1);
		upDateDZData104(DZ_STOP);
		return TRUE;
	}
	for(i=0;i<342;i++){
		S104DzValueBackUp[i].Flag = DZ_Clear;
	}
	if(infonum & 0x80){//连续
		infonum ^= 0x80;
		pData = &pRxdFm->Data1;
		addr = MAKEWORD(pRxdFm->AddInfom1, pRxdFm->AddInfom2) - 0x5001;
		for(i=0;i<infonum;i++){
			S104DzValueBackUp[addr + i].DZValue = MAKEDWORD(pData[0], pData[1], pData[2], pData[3]);
			S104DzValueBackUp[addr + i].Flag = DZ_YU_ZHI;
			pData+=4;
			
			DataValue._int32 = S104DzValueBackUp[addr + i].DZValue;
			if(S104DzValueBackUp[addr + i].DZValue)
				log("addr(%d)value(%f)\n",addr + i, DataValue._float);
			
		}
		
	}else{//不连续
		pData = &pRxdFm->AddInfom1;
		for(i=0;i<infonum;i++){
			addr = MAKEWORD(pData[0], pData[1]) - 0x5001;
			S104DzValueBackUp[addr].DZValue = MAKEDWORD(pData[3], pData[4], pData[5], pData[6]);
			S104DzValueBackUp[addr].Flag = DZ_YU_ZHI;
			pData+=7;
			
			DataValue._int32 = S104DzValueBackUp[addr].DZValue;
			if(S104DzValueBackUp[addr].DZValue)
				log("addr(%d)value(%f)\n", addr, DataValue._float);
		}
	
	}
	DZ_Comman104 = DZ_YU_ZHI;
	return TRUE;
}

int r_call_Dz104(DWORD DirAddr,BYTE comman)
{
	int i;
	uint32_t m_TxdDevID;
	
	GetDevFromAddr(DirAddr,&m_TxdDevID);
	gpDevice[m_TxdDevID].Flag_Set_DZ = comman;
	log("dir_DzAddr(%d),devid(%d)\n",DirAddr, m_TxdDevID);
//	if(::GetDevFromAddr(dir_DzAddr,&m_TxdDevID)){//找到模块地址是m_dwDzID的设备
//		::SetDz(m_TxdDevID,  TP_DZ_CALL_SEL, 0, 0);//发送信息给对应的装置
//	}
	
	
	return 0;
}

static void Edit_call6465(int DevID, BYTE Style, BYTE Reason, BYTE QOI)
{
	
	Txd_Head_s104(Style,0);
	Edit_InfoAddr(m_Txd.Buf,0);
    m_Txd.Buf[ m_Txd.WritePtr++ ] = QOI;
	Txd_Tail_s104(Reason, 1, 1);
	return;
}


/*******************************************************************  
*名称：      		InitTransTab  
*功能：			定值预置防止在地址4对应的ID缓存  
*	
*入口参数：         
*出口参数：正确返回为0，错误返回为-1 
*******************************************************************/ 
int Edit_DZYZ104()
{
	int i,j;
	int GroupNum = 3;
	int GroupID = 0;
	int addr = 0;
	int SendNum = 0;
	DWORD tmpDevID;
	DWORD addr_4;
	BYTE *buf;
	S104DZVALUE_T *DZvalue = NULL;
	
	GetDevFromAddr(4,&addr_4);//地址6设备用作定值预置确认缓冲区给104用
	gpDevice[addr_4].pDZ[5] = Type_104;//PDZ[5] == 1为101，PDZ[1] == 0为104
	
	GetDevFromAddr(6,&tmpDevID);//地址6设备用作定值预置确认缓冲区给104用
	log("dir_DzAddr(%d),devid(%d)\n",6, tmpDevID);
	
	buf = (BYTE *)&gpDevice[tmpDevID].pDZ[10];
	
	for(j=0;j<GroupNum;j++){
		for(i=0;i<114;i++){
			addr = j*114 +i;
			if(S104DzValueBackUp[addr].Flag == DZ_YU_ZHI){
				S104DzValueBackUp[addr].Flag = DZ_Clear;
				

			GroupID = j+1;
			buf[0] = LOBYTE(addr);
			buf[1] = HIBYTE(addr);
			buf[2] = LLBYTE(S104DzValueBackUp[addr].DZValue);
			buf[3] = LHBYTE(S104DzValueBackUp[addr].DZValue);
			buf[4] = HLBYTE(S104DzValueBackUp[addr].DZValue);
			buf[5] = HHBYTE(S104DzValueBackUp[addr].DZValue);
			SendNum++;
			buf += 6;




				
				if(i == 113){
					gpDevice[tmpDevID].pDZ[0] = SendNum;
					DZ_Comman104 = DZ_YU_ZHI;
					log("GroupID(%d)\n",GroupID);
					return r_call_Dz104(GroupID, TP_DZ_CALL_SEL);
				}
				if(SendNum > 30){
					gpDevice[tmpDevID].pDZ[0] = SendNum;
					DZ_Comman104 = DZ_YU_ZHI;
					return r_call_Dz104(GroupID, TP_DZ_CALL_SEL);
				}
			}

		}
	}
	if(SendNum == 0)
		return 0;
	gpDevice[tmpDevID].pDZ[0] = SendNum;
	return r_call_Dz104(GroupID, TP_DZ_CALL_SEL);


}

bool Edit_BrustDz104(int fd, BYTE *buff,BYTE len, BYTE DzAddr)
{
	DWORD info_addr;
	BYTE info_addr_start = 12,info_data_start101 = 14,info_data_start104 = 15;
	BYTE *bufftmp;
	BYTE bPRM = 1, bCode = 3, bNum, info_num;
	APDU104_T *buf104;
	BYTE *pData;
	int i;
	DWORD dwValue;
	Data_Value_T DataValue;

	buf104 = (APDU104_T *)buff;
	bNum = buff[7];
	bufftmp = m_Txd.Buf;
	memcpy(bufftmp, buff, len);
	Txd_Head_s104(0x70, 3);

	if(bNum & 0x80){//连续
		info_num = bNum&0x7f;
		info_addr = MAKEWORD(buf104->AddInfom1, buf104->AddInfom2);
		info_addr = info_addr + (DzAddr-1)*114;
		m_Txd.WritePtr = 12;
		bufftmp[m_Txd.WritePtr++] = LOBYTE(info_addr);	 //信息地址
		bufftmp[m_Txd.WritePtr++] = HIBYTE(info_addr); 	//信息地址
		bufftmp[m_Txd.WritePtr++] = 0; 	//信息地址
		pData = &bufftmp[info_data_start104];
		for(i=0;i<info_num;i++){
			dwValue = MAKEDWORD(pData[0], pData[1], pData[2], pData[3]) ;
			bufftmp[m_Txd.WritePtr++] = LLBYTE(dwValue);
			bufftmp[m_Txd.WritePtr++] = LHBYTE(dwValue); 
			bufftmp[m_Txd.WritePtr++] = HLBYTE(dwValue);
			bufftmp[m_Txd.WritePtr++] = HHBYTE(dwValue);
			pData += 4;
			DataValue._int32 = dwValue;
			if(dwValue)
				log("devid(%d)addr(%d)value(%x)value(%f)\n",(info_addr-0x5001)/114+1, (info_addr-0x5001)%114, dwValue, DataValue._float);
			
		}
		
	}else{
		info_num = bNum;
		m_Txd.WritePtr = 12;
		
		pData = &buf104->AddInfom1;
		for(i=0;i<info_num;i++){
			info_addr = MAKEWORD(pData[0], pData[1]);
			info_addr = info_addr + (DzAddr-1)*114;
			bufftmp[m_Txd.WritePtr++] = LOBYTE(info_addr);	 //信息地址
			bufftmp[m_Txd.WritePtr++] = HIBYTE(info_addr); 	//信息地址
			bufftmp[m_Txd.WritePtr++] = 0; 	//信息地址
			
			dwValue = MAKEDWORD(pData[3], pData[4], pData[5], pData[6]) ;
			bufftmp[m_Txd.WritePtr++] = LLBYTE(dwValue);
			bufftmp[m_Txd.WritePtr++] = LHBYTE(dwValue); 
			bufftmp[m_Txd.WritePtr++] = HLBYTE(dwValue);
			bufftmp[m_Txd.WritePtr++] = HHBYTE(dwValue);
			pData += 7;
			DataValue._int32 = dwValue;
			if(dwValue)
				log("devid(%d)addr(%x)value(%x)value(%f)\n",(info_addr-0x5001)/114+1, info_addr, dwValue, DataValue._float);
		}
	}
	bufftmp[m_Txd.WritePtr++]=6;//PQM
	Txd_Tail_s104(bPRM, bCode, bNum);
	
}

//===================================================================
//  函数名称: CS104_2002nr::Edit_ChgYc         
//  函数功能: 编辑发送越限遥测
//  输入参数:                      
//  返回值：无                        
//===================================================================
bool Edit_ChgYc()
{
	TBurstAI burstAI;
	BYTE* pBuf;
	BYTE bNum = 0;	
	WORD wNo;
	WORD wBustAINum;
	WORD wMSecond;
	short sValue;
	DWORD dwValue;
	DWORD dwDelYc;
	float* pFloat;
	BYTE bTxChYcType;			//变化遥测发送类型标识
	DWORD wAllYcNum;
//	TLogicAI* logicAI = gpDevice[m_dwDevID].pLogicBase->pLogicAI;
	
	wAllYcNum = gVars.TransYCTableNum;
  	wBustAINum = GetBurstAINum(m_dwDevID);
//	log("wBustAINum is(%d)\n",wBustAINum);
	if( wBustAINum==0 ) //无遥测变化
		return FALSE;
	
//   	if( wBustAINum==0 )	//无遥测变化
// 	{
// 		if( bStarTFlag_s104&&(!Find_ChgYx())&&(!Find_SOE()) )
// 		{	
// 			if(sendcount <30)
// 			{
// 				sendcount ++;
// 				return 0;
// 			}
// 			sendcount =0;
//             for(int BNum=0;BNum<bGroupYc;BNum++ )//遥测
// 			{
// 				Edit_AllYc(BNum,BNum+1, 1);	
// 			
// 			}
// 				char lzbuf[32];
// 				sprintf(lzbuf, "主动发送遥测:%d 组", 1);				
// 	            LogError(MLE_NORMAL, lzbuf);
// 		}
// 		return 0;
// 	}	
  	pBuf = m_Txd.Buf;
  	pFloat = (float*)&dwValue;

	bTxChYcType = 0x0d;
	Txd_Head_s104(bTxChYcType,0);	
	
	if( (bTxChYcType == 0x15) || (bTxChYcType == 0x09) || (bTxChYcType == 0x0b)
		|| (bTxChYcType == 0x22) || (bTxChYcType == 0x23) )	//非短浮点,这里用或比较好
	{
		PRINT_FUNLINE;
		for(dwDelYc = 0; dwDelYc < wBustAINum; dwDelYc++)
		{
			if(ReadBurstAI(m_dwDevID,&burstAI) != TRUE)
				break;
			
			if(burstAI.dwNo >= wAllYcNum)
				continue;
	
			bNum ++;			
			wNo = burstAI.dwNo +  STARTYC104_2002nr;
			sValue = burstAI.sValue;
	
			Edit_InfoAddr(pBuf,wNo);
			
			pBuf[m_Txd.WritePtr++] = LOBYTE(sValue); //遥测值 Lo
			pBuf[m_Txd.WritePtr++] = HIBYTE(sValue); //遥测值 Hi
			
			if(bTxChYcType != 0x15)
			{
				pBuf[m_Txd.WritePtr++] = (burstAI.wStatus&0xFF);
				if( bTxChYcType == 0x22 || bTxChYcType == 0x23 )	//带时标
				{
					wMSecond = burstAI.stTime.MSecond;
	
					pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wMSecond);  		//MSEC L
					pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wMSecond);		//MSEC H
					pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Minute;  	//Minute	
					pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Hour;  	//Hour
					pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Day;  	//Day
					pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Month;  	//Month
					pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Year%100;	//Year
					
					if(bNum>16)
						break;
				}
			}
			else if(bNum>32)	
				break;
		}
	}
	else	//短浮点
	{
		for(dwDelYc = 0; dwDelYc < wBustAINum; dwDelYc++)
		{
			if(ReadBurstAI(m_dwDevID,&burstAI) != TRUE)
				break;
			
			if(burstAI.dwNo >= wAllYcNum)
				continue;
	
			bNum ++;			
			wNo = burstAI.dwNo +  STARTYC104_2002nr;
			*pFloat = (float)burstAI.sValue;

			float a,b,c;
			a =1;
			b =1;
			c =1;
			float val =(*pFloat*b)/(a*c);
            *pFloat= val;
			
			dwValue = burstAI.sValue;
//			char ss[64];
			
//            sprintf(ss,"发送 点=%d,值=%f ",burstAI.dwNo,val);
		//	LogError(MLE_NORMAL, ss);
	
			Edit_InfoAddr(pBuf,wNo);
			
			pBuf[m_Txd.WritePtr++] = LLBYTE(dwValue); //遥测值 Lo
			pBuf[m_Txd.WritePtr++] = LHBYTE(dwValue); //遥测值 Hi
			pBuf[m_Txd.WritePtr++] = HLBYTE(dwValue); //遥测值 Lo
			pBuf[m_Txd.WritePtr++] = HHBYTE(dwValue); //遥测值 Hi	
			pBuf[m_Txd.WritePtr++] = (burstAI.wStatus&0xFF);
			if(bNum>GENGRP_YC_2002nr)	
				break;
			
		/*	if( bTxChYcType == 0x24 )	//短浮点带时标
			{
				wMSecond = (WORD)burstAI.stTime.Second*1000 + burstAI.stTime.MSecond;

				pBuf[ m_Txd.WritePtr++ ] = LOBYTE(wMSecond);  		//MSEC L
				pBuf[ m_Txd.WritePtr++ ] = HIBYTE(wMSecond); 		//MSEC H				
				pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Minute;  	//Minute					
				pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Hour;  	//Hour
				pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Day;  	//Day
				pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Month;  	//Month
				pBuf[ m_Txd.WritePtr++ ] = burstAI.stTime.Year%100;	//Year
				
				if(bNum>16)
					break;
			}
			else if(bNum>32)	
				break;*/
		}
	}
	Txd_Tail_s104(0x03, bNum, bNum);
	return 1;
}

//获取SOE区大小,与遥信大小相同
DWORD GetSoeMaxBuffer(DWORD dwDevID)
{
	if(dwDevID>=gVars.dwDevNum)
		return 1;
	if(gpDevice[dwDevID].pSoeWrap==0)
		return 1;

	if(gpDevice[dwDevID].BINum)
		return gpDevice[dwDevID].BINum;
	else
		return 128;
}


static BOOL WriteDZReturn104(int fd, BYTE *buf)
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
			S104DzValueBackUp[offset + infoaddr +i].DZValue = value;
			S104DzValueBackUp[offset + infoaddr +i].Flag = DZ_YZ_CONFIRM;
			Data += 4;
		}
		return TRUE;
	}
	//不连续
	Data = &gx104buf->AddInfom1;
	for(i=0;i<asdu_num;i++){
		infoaddr = MAKEWORD(Data[0], Data[1]) - 0x5001;
		value = MAKEDWORD(Data[3], Data[4], Data[5], Data[6]);
		S104DzValueBackUp[offset + infoaddr].DZValue = value;
		S104DzValueBackUp[offset + infoaddr].Flag = DZ_YZ_CONFIRM;
		Data += 7;
	}
	
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
	if(time1.stTime._year._year > time2.stTime._year._year ){
		return FALSE;
	}else if(time1.stTime._year._year  < time2.stTime._year._year ){
		return TRUE;
	}
	if(time1.stTime._month._month > time2.stTime._month._month){
		return FALSE;
	}else if(time1.stTime._month._month < time2.stTime._month._month){
		return TRUE;
	}
	
	if(time1.stTime._day._dayofmonth > time2.stTime._day._dayofmonth){
		return FALSE;
	}else if(time1.stTime._day._dayofmonth < time2.stTime._day._dayofmonth){
		return TRUE;
	}

	if(time1.stTime._hour._hours > time2.stTime._hour._hours){
		return FALSE;
	}else if(time1.stTime._hour._hours < time2.stTime._hour._hours){
		return TRUE;
	}

	if(time1.stTime._min._minutes > time2.stTime._min._minutes){
		return FALSE;
	}else if(time1.stTime._min._minutes < time2.stTime._min._minutes){
		return TRUE;
	}

	if(time1.stTime._milliseconds > time2.stTime._milliseconds){
		return FALSE;
	}else if(time1.stTime._milliseconds < time2.stTime._milliseconds){
		return TRUE;
	}
	return FALSE;


}

/*****************************************************************
函数名称: PaiXu_OldSoe
函数功能: 排序函数，最新时间放在最前
输入参数: 
返回值：  
修改日期：2020-3-31

*****************************************************************/

static BOOL PaiXu_OldSoe() 
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
	memset(gpDevice[TempDev_all].pSysTimeSoe,0,300*sizeof(TSysTimeSoe));
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

	
}

//===================================================================
//  函数名称: CS104_2002nr::Edit_OldSoe         
//  函数功能: 编辑发送历史SOE
//  输入参数:                      
//  返回值：无                        
//===================================================================
BOOL Edit_OldSoe() 
{
	TSysTimeSoe SysTimeSoe; 	
	
	BYTE RealBINo;
	DWORD dwControl=0;
//	DWORD dwDevType;
	BYTE bNum;
	BYTE bOnceDel;
	BYTE bDelTrsOnce;	
	WORD wNo;
	WORD wTrsNum;
	WORD wDelTrsNum;
	DWORD MSecond;	
	DWORD dwSoeAllNum = 0;
	DWORD dwReadPtr = 0;
	DWORD TempDevID = 0;

	GetDevFromAddr(7,&TempDevID);
	dwSoeAllNum = gpDevice[TempDevID].pSoeWrap->Ptr.dwWritePtr; //已经发出的SOE数
	if(dwSoeAllNum == 0){
		return FALSE;
	}
	if(dwSoeAllNum >= 100){
		dwSoeAllNum = 100;
	}
	
	if(dwSoeAllNum%20 == 0){
		wTrsNum = dwSoeAllNum/20;
	}else{
		wTrsNum = dwSoeAllNum/20+1;
	}
	log("dwSoeAllNum(%d),wTrsNum(%d)\n",dwSoeAllNum,wTrsNum);
	for( wDelTrsNum=0;wDelTrsNum<wTrsNum;wDelTrsNum++ )
	{
		bOnceDel = (wDelTrsNum+1<wTrsNum)? 20:(dwSoeAllNum%20);
		if(dwSoeAllNum%20 == 0)
			bOnceDel = 20;
		
		Txd_Head_s104(0x1e, 0);
		bNum = 0;

		for(bDelTrsOnce=0;bDelTrsOnce<bOnceDel; bDelTrsOnce++)
		{
			if( ReadSysTimeSoe(TempDevID, &SysTimeSoe)==false ){
				break;
			}
			IncSoeReadPtr(TempDevID);
			
			wNo=SysTimeSoe.nNo+STARTSYX104_2002nr;
			
			Edit_InfoAddr(m_Txd.Buf,wNo);

			m_Txd.Buf[ m_Txd.WritePtr++ ] = (SysTimeSoe.bStatus & PBI_ON) ? 1:0;

			MSecond=SysTimeSoe.stTime._milliseconds;

			m_Txd.Buf[ m_Txd.WritePtr++ ] = LOBYTE(MSecond);  		//MSEC L
			m_Txd.Buf[ m_Txd.WritePtr++ ] = HIBYTE(MSecond);  		//MSEC H
			m_Txd.Buf[ m_Txd.WritePtr++ ] = SysTimeSoe.stTime._min._minutes;  	//Minute					
			m_Txd.Buf[ m_Txd.WritePtr++ ] = SysTimeSoe.stTime._hour._hours;  	//Hour
			m_Txd.Buf[ m_Txd.WritePtr++ ] = SysTimeSoe.stTime._day._dayofmonth;  	//Day
			m_Txd.Buf[ m_Txd.WritePtr++ ] = SysTimeSoe.stTime._month._month;  	//Month
			m_Txd.Buf[ m_Txd.WritePtr++ ] = SysTimeSoe.stTime._year._year%100;	//Year

			if(INCLUDE_ACTVALUE==1)	
			{
//				m_Txd.Buf[ m_Txd.WritePtr++ ] = SysTimeSoe.bType;
//				m_Txd.Buf[ m_Txd.WritePtr++ ] = SysTimeSoe.bValue0;
//				m_Txd.Buf[ m_Txd.WritePtr++ ] = SysTimeSoe.bValue1;
//				m_Txd.Buf[ m_Txd.WritePtr++ ] = SysTimeSoe.bValue2;
//				m_Txd.Buf[ m_Txd.WritePtr++ ] = SysTimeSoe.bValue3;

			//	dwDevType = gpDevice[pLBI->wRealID].dwType;

			/*	if(dwDevType==0x6621 || dwDevType==0x6631 || dwDevType==0x6651 || dwDevType==0x6661
				|| dwDevType==0x6681 || dwDevType==0x6682 || dwDevType==0x6683 || dwDevType==0x6688)
				{
					RealBINo = gpDevice[pLBI->wRealID].pRealBase->pRealBI[pLBI->wOffset].wDINo;	
					if(RealBINo>14 && RealBINo<50)
						WriteBI(pLBI->wRealID,RealBINo,PBI_OFF,0,0,0,0,0);
				}*/
			}
//			pLBI = &gpDevice[TempDevID].pLogicBase->pLogicBI[wNo-STARTSYX104_2002nr];
//			RealBINo = gpDevice[pLBI->wRealID].pRealBase->pRealBI[pLBI->wOffset].wDINo;
//			dwControl = gpDevice[pLBI->wRealID].pRealBase->pRealBI[pLBI->wOffset].dwControl;

			if(dwControl&BI_REVERT)				//信号复归
			{
//********************//加入实际库遥信编号与实际排序是否一致的判别chchenli200809117**********************			
//                                           
//                           if(RealBINo!=pLBI->wOffset)    
//			      RealBINo=SearchWriteBINo(RealBINo);
//**************************************************************************************	
                             
//                             WriteBI(pLBI->wRealID,RealBINo,PBI_OFF,0,0,0,0,0);
			}

			bNum++;		
		}

		Txd_Tail_s104(0x24,bNum,bNum);
	}
		
	return TRUE;
}

BOOL TxdCallDZReturn104(int fd)
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

	wDZNum = gpDevice[CKLogicDevID].DZNum;

	for(i=0;i<(wDZNum-1);i++){
		if(S104DzValueBackUp[i].Flag == DZ_YZ_CONFIRM){
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
		if((bufftmp[i] + 1) == bufftmp[i + 1])
			Flag_sq = 0x80;
	}
	Txd_Head_s104(TI, DZReturnReanson);
	m_Txd.WritePtr = 12;
	if(Flag_sq){//连续
		wNo = bufftmp[0] + 0x5001;
		log("wNo is (%x)\n",wNo);
	
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //信息体地址Hi
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址Hi
		for(i=0;i<DZReturnNum;i++){
			value = S104DzValueBackUp[bufftmp[i]].DZValue;
			pBuf[m_Txd.WritePtr++] = LLBYTE(value);
			pBuf[m_Txd.WritePtr++] = LHBYTE(value); 
			pBuf[m_Txd.WritePtr++] = HLBYTE(value); 
			pBuf[m_Txd.WritePtr++] = HHBYTE(value);
			bNum++;
			
		}
		pBuf[m_Txd.WritePtr++] = 9; //QPM
		Txd_Tail_s104(DZReturnReanson, bNum|0x80, 1);
		return TRUE;

	}
	log("DZReturnNum is (%d)\n",DZReturnNum);
	//不连续
	for(i=0;i<DZReturnNum;i++){
		wNo = bufftmp[i] + 0x5001;
		log("wNo is (%d)\n",wNo);
		pBuf[m_Txd.WritePtr++] = LOBYTE(wNo); //信息体地址Lo
		pBuf[m_Txd.WritePtr++] = HIBYTE(wNo); //信息体地址Hi
		pBuf[m_Txd.WritePtr++] = 0; //信息体地址
		value = S104DzValueBackUp[bufftmp[i]].DZValue;
		pBuf[m_Txd.WritePtr++] = LLBYTE(value);
		pBuf[m_Txd.WritePtr++] = LHBYTE(value); 
		pBuf[m_Txd.WritePtr++] = HLBYTE(value); 
		pBuf[m_Txd.WritePtr++] = HHBYTE(value);
		
	}
	pBuf[m_Txd.WritePtr++] = 9; //QPM
	Txd_Tail_s104(DZReturnReanson, bNum, 1);
	return TRUE;


}
static BOOL TimerOn(DWORD TimeOut) 
{ 
	return (TimecntS104%TimeOut)==0; 
}; //检查是否到了TimeOut间隔定时



//===================================================================
//  函数名称: CS104_2002nr::Edit_64_All         
//  函数功能: 编辑总召唤信息量的发送
//  输入参数:                         
//  返回值：无                        
//===================================================================
void Edit_64_All()
{
//	BYTE BNum = 0;
//
//	if( bWaitSFlag == 0 )
//	{
//		for( BNum=0;BNum<bGroupYx;BNum++ )//遥信
//		{
//			Compe_SDYx(BNum,BNum+1,1);
//		}
//
//		for( BNum=0;BNum<bGroupYc;BNum++ )//遥测
//		{
//			Edit_AllYc(BNum,BNum+1, 1);	
//		}		
//		
//		r_Reqe_Erro(0x64,0x0a,pRxdFm->Length);
//	}
//	else
//	{
//		bCall_Group|=0x80;//置标志
//	}	
//	r_Reqe_Erro_s104(0x64,0x07,pRxdFm->Length);
//	r_Reqe_Erro_s104(0x64,0x0a,pRxdFm->Length);

	
	return;
}

//===================================================================
//  函数名称: CS104_2002nr::r_call_All         
//  函数功能: 处理总召唤
//  输入参数:                         
//  返回值：无    
//	修改日期：2020-01-13
//	1、单独增加第16组召唤
//	2、第16组召唤历史100条SOE事件
//===================================================================
void r_call_All()
{
	BYTE *pData;
	BYTE BCallGroup = 0;
	DWORD TempDevID;

	pData = ( BYTE* )pRxdFm;
	bCall_Group=0;
	
	if( (pRxdFm->Reason_L!=0x06)&&(pRxdFm->Reason_L!=0x08) )
	{
		r_Reqe_Erro_s104(0x64,0x47,pRxdFm->Length);
		return;
	}
	
	if( pRxdFm->Reason_L==0x08 )	//撤销
	{
		r_Reqe_Erro_s104(0x64,0x09,pRxdFm->Length);
		return;
	}
	
	if(pData[DEALSTATRT_2002nr+BINFOADDR_2002]==0x14)
	{
		r_Reqe_Erro_s104(0x64,0x07,pRxdFm->Length);
		Edit_64_All();
		r_Reqe_Erro_s104(0x64,0x0a,pRxdFm->Length);
		return;
	}
	else if( pData[DEALSTATRT_2002nr+BINFOADDR_2002]>0x14&&pData[DEALSTATRT_2002nr+BINFOADDR_2002]<0x25 )
	{
		BCallGroup = pData[DEALSTATRT_2002nr+BINFOADDR_2002]-0x14;

//		if( BCallGroup > (bGroupYx+bGroupYc) )
//		{
//			r_Reqe_Erro(0x64,0x47,pRxdFm->Length);//错误
//			return;
//		}
//		else if( pData[DEALSTATRT_2002nr+BINFOADDR_2002] == 0x24 )//第16组  
//		{
//			r_Reqe_Erro(0x64,0x07,pRxdFm->Length);
//			Edit_OldSoe();
//			return;
//		}
		if( pData[DEALSTATRT_2002nr+BINFOADDR_2002] == 0x24 )//第16组  
		{
//			r_Reqe_Erro(0x64,0x07,pRxdFm->Length);
//			Edit_OldSoe();
//			r_Reqe_Erro(0x64,0x0a,pRxdFm->Length);
			flag_call_16 = 1;
			TimecntS104 = 0;

			GetDevFromAddr(5,&TempDevID);
			gpDevice[TempDevID].pDZ[6]|=0x1;
			gpDevice[TempDevID].pDZ[6]|=0x2;
			gpDevice[TempDevID].pDZ[6]|=0x4;

			return;
		}
		else if( BCallGroup > (bGroupYx+bGroupYc) )
		{
			r_Reqe_Erro_s104(0x64,0x47,pRxdFm->Length);//错误
			return;
		}
		else if( BCallGroup == (bGroupYx+bGroupYc) )//最后一组  
		{
			r_Reqe_Erro_s104(0x64,0x07,pRxdFm->Length);

//			if( BCallGroup == bGroupYx )
//			{
//				Compe_SDYx(BCallGroup-1,BCallGroup,0x80);
//			}
//			else
//			{
//				Edit_AllYc(BCallGroup-bGroupYx-1,BCallGroup-bGroupYx, 0x80);
//			}

		}
		else//中间组
		{
			r_Reqe_Erro(0x64,0x07,pRxdFm->Length);
			
//			if( BCallGroup <= bGroupYx )
//			{
//				Compe_SDYx(BCallGroup-1,BCallGroup,0x80);
//			}
//			else
//			{
//				Edit_AllYc(BCallGroup-bGroupYx-1,BCallGroup-bGroupYx, 0x80);
//			}
		}			
		return;
	}
	else	//错误
	{
		r_Reqe_Erro_s104(0x64,0x47,pRxdFm->Length);
		return;
	}		
}

//================================================================
//  函数名称: CS104_2002nr::r_Reqe_Erro
//  函数功能: 子站对主站报文的肯定或否定确认
//  输入参数: bStyle类型标识，bReason传送原因，bLength传送长度。
//  返回值：  无
//================================================================
void r_Reqe_Erro(BYTE bStyle,BYTE bReason,BYTE bLength)
{
	BYTE i = 0;
	BYTE* pBuf;
	
	if( bLength<10 )
		return;
		
	pBuf = (BYTE *)pRxdFm;
	Txd_Head_s104(bStyle,0);
	
	for( i=0;i<bLength-10;i++ )
	{
		m_Txd.Buf[DEALSTATRT_2002nr+i]=pBuf[DEALSTATRT_2002nr+i];
	}
	
	m_Txd.WritePtr += bLength-10;
	Txd_Tail_s104(bReason, 1,1);
}



void Handle_U_Format(int fd, DWORD DevID) 
{
	unsigned char Control = 0;
	pTxdFm = (TS104APDU *)m_Txd.Buf;
	switch(pRxdFm->Control1)
	{
		case  STARTDT_ACT_2002nr: 		//主站发送STARTDT生效
			bStartFlag = 1; 	//置允许数据发送标志
			InitFinishFlag = 1;
			wR104_2002nr_RxCount = 0;
			wS104_2002nr_TxCount = 0;
			Control |= STARTDT_CON_2002nr; 

//			if(Flag_SendOneYX == 1 && (gpDevice[m_dwDevID].pSoeWrap->Ptr.dwWritePtr == 0)){
//				if(::GetDevFromAddr(1,&DevID)){//找到模块地址是m_dwDzID的设备
//					WriteBI(DevID, 0, PBI_OFF,0,0,0,0,0);
//					Flag_SendOneYX = 0;
////					sprintf(sss,"M104---DevID(%d)  m_dwDevID(%d) devtype(%d)", DevID, m_dwDevID, gpDevice[DevID].dwType);
////					LogError(MLE_NORMAL,sss);
//				}
//				
//			}


			
			break;

		case STOPDT_ACT_2002nr:		//主站发送STOPDT
			bStartFlag = 0; 	//清允许数据发送标志
			Control |= STOPDT_CON_2002nr; 	break;
			
		case TESTFR_ACT_2002nr:		//主站发送TESTFR生效
			Control |= TESTFR_CON_2002nr; 	break;

		default: return;
	}

	//组U帧应答
	m_Txd.ReadPtr=0;
	pTxdFm->Start	= STARTHEAD_2002nr;
	pTxdFm->Length =  0x04;
	pTxdFm->Control1 = Control;
	pTxdFm->Control2 = 0;
	pTxdFm->Control3 = 0;
	pTxdFm->Control4 = 0;
	m_Txd.WritePtr=6;

	IEC10X->Send(fd,(char *)m_Txd.Buf,m_Txd.WritePtr);

	if(InitFinishFlag == 1){
		InitFinishFlag = 0;
		Edit_InitFinsh(fd);
	}
//	if(pRxdFm->Control1 == TESTFR_ACT_2002nr)
//	   Edit_ChgYc();
}
static BOOL ResetProcess()
{

//	TSysTimeDBISoe sysDSoe; 
//	TSysTimeSoe sysSoe; 
//	TSysTimeMr sysMr;
//	TSysTimeAit sysAiSoe;
//	WORD wAddr;
//	DWORD TempDevID;

	r_Reqe_Erro(0x69,0x07,pRxdFm->Length);
	
//	LogError(MLE_NORMAL,"ResetProcess_end");
//
//	while(ReadSysTimeSoe(m_dwDevID, &sysSoe))//清SOE缓冲区
//		IncSoeReadPtr(m_dwDevID);
//
//	while(ReadSysTimeDBISoe(m_dwDevID, &sysDSoe))//清双点SOE缓冲区
//		IncDBISoeReadPtr(m_dwDevID);
//
//	while(ReadSysTimeMr(m_dwDevID, &sysMr))//清保护信息缓冲区
//		IncMrReadPtr(m_dwDevID);
//
//	while(ReadSysTimeAit(m_dwDevID, &sysAiSoe))
//		IncAitReadPtr(m_dwDevID);
//	
//	gpDevice[m_dwDevID].pSoeWrap->Ptr.dwWritePtr = 0;
//	gpDevice[m_dwDevID].pSoeWrap->Ptr.dwReadPtr = 0;
//	::GetDevFromAddr(5,&TempDevID);
//	bStartFlAg_s104 = 0;
//	gpDevice[TempDevID].pDZ[0]|=0x1;
//	gpDevice[TempDevID].pDZ[0]|=0x2;
//	gpDevice[TempDevID].pDZ[0]|=0x4;
//	
//	sprintf(sss,"gpDevice[TempDevID].pDZ[0] is(0x:%x)",gpDevice[TempDevID].pDZ[0]);
//	LogError(MLE_NORMAL, sss);
	
	return TRUE;
}


//===================================================================
//  函数名称: CS104_2002nr::Handle_S_Format         
//  函数功能: S格式报文处理
//  输入参数:                         
//  返回值：无                        
//===================================================================
void Handle_S_Format_s104() 
{
//	if( bComP_ReSeNum )		//作帧的计数判断
//	{
//		if( wR104_2002nr_RxCount>=(pRxdFm->Control3+pRxdFm->Control4*0x100)+IECS104_2002nr_WCOUT )
//		{
//			RetryFrame();	//重发上次帧
//			return;
//		}		
//	}
	
//	if( bWaitSFlag != 0 )	//总召唤和电度量数据要分批上传
//	{
//		if( !GenCall_GroupYxYc() )
//		{
//			GenCall_GroupMc();
//			return;
//		}
//	}	

//	Edit_ChgYc();
	return;
}
//===================================================================
//  函数名称: CS104_2002nr::r_call_TimeS         
//  函数功能: 处理主站对时
//  输入参数:                 
//  返回值：无                        
//===================================================================
void r_call_TimeS()
{
//	BYTE* pData;
//	TSysTime sysTime;	
//	WORD wMSecond;
//	TAbsTime absTime;
//	TAbsTime beforSynTime;
//	int i;	
//	DWORD dwControl;
//	
//	//zym add 判断是否接收该端口的对时命令
//	dwControl = gpDevice[m_dwDevID].dwControl;
//	dwControl &= 0x700;
//	if(dwControl==0x100)    //禁止对钟
//		return;
//	//end
//
//	pData = (BYTE *)pRxdFm;
//	//下面判断一些合法性条件
//	if( pData[8]!=0x06 )
//	{
//		r_Reqe_Erro(0x67,0x47,pRxdFm->Length);
//		return;
//	}
//		
//	for(i=0;i<BINFOADDR_2002;i++)		//若信息体地址不为0,返回错误
//	{
//		if(pData[DEALSTATRT_2002nr+i]!=0x00)
//		{
//			r_Reqe_Erro(0x67,0x44,pRxdFm->Length);
//			return;
//		}
//	}
//	
//	pData += DEALSTATRT_2002nr+BINFOADDR_2002;
//	ReadAbsTime(&beforSynTime);		//读系统绝对时间
//
//	wMSecond = MAKEWORD(pData[0], pData[1]);
//	sysTime.MSecond = (wMSecond%1000);
//	sysTime.Second  = (wMSecond/1000);
//	sysTime.Minute = pData[2];
//	sysTime.Hour = pData[3];
//	sysTime.Day = (pData[4]&0x1F);
//	sysTime.Month = (pData[5]&0x0F);
//	sysTime.Year = (pData[6]&0x7F)+2000;
//	
//	ToAbsTime(&sysTime, &absTime);	//系统时间转换成绝对时间
//
//	absTime.Lo += m_wDelayTime;
//
//	if(absTime.Lo<(DWORD)m_wDelayTime)
//		absTime.Hi += 1;
//
//	WriteAbsTime(&absTime);

	r_Reqe_Erro_s104(0x67,0x07,pRxdFm->Length);	//回复肯定确认
	return;
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


//===================================================================
//  函数名称: CS104_2002nr::Handle_S_Format         
//  函数功能: I格式报文处理
//  输入参数:                         
//  返回值：无                        
//===================================================================
void Handle_I_Format_s104(int fd, int DevID, uint8_t *buf, uint16_t len)
{
	BYTE BIType;
	if( !bStartFlag )
		return;
//		
	wR104_2002nr_RxCount += 2;
	BIType=pRxdFm->Style;
//	
//	if( (pRxdFm->Address_L!=Rtu_104_2002nr) || ((pRxdFm->Address_L==0xff)&&(BIType!=0x64) 
//		&& (BIType!=0x65) && (BIType!=0x67)) )	//RTU地址错误
//	{
//		return ;//错误
//	}
//	
//	bCall_Group = 0;	//如果分批上传被其他I中断，停止
//		
	switch(BIType)
	{
		case 100: 	//C_IC_NA_1 0x64总召唤命令
		{
			r_call_All();
			break;
		}
//		
//		case 101: 	//C_CI_NA_1 0x65电度量召唤
//		{
//			r_call_Mc();
//			break;
//		}
//		
//		case 102: 	//C_RD_NA_1 0x66读命令
//		{
//			r_call_Read();
//			break;
//		}
//		
		case 103:	//C_CS_NA_1 0x68时钟同步
  		{
  			r_call_TimeS();
  			break;
  		}
		case 105:	//0x69复位进程
				ResetProcess();
		break;
		
  		break;
//  		case 107:	//C_TS_TA_1 0x6b带时标CP56Time2a的测试命令	
//  		{
//  			r_call_TestTime();
//  			break;
//  		}
  		
		//遥控命令
  		case 45: 	//C_SC_NA_1 0x2d单点遥控
  		case 46: 	//C_DC_NA_1 0x2e双点遥控
    	case 47:	//C_RC_NA_1 0x30升降命令
    	case 58:	//C_SC_TA_1 0x3a带时标单点遥控
    	case 59:	//C_DC_TA_1 0x3b带时标双点遥控 
   		case 60:	//C_RC_TA_1 0x3c带时标升降命令
		{
//			r_Deal_DoAo();
//			break;
		}
				
//   	case 48:	//C_SE_NA_1 0x30设定归一化值命令  	
//    	case 49:	//C_SE_NB_1 0x31设定标度化值命令   	
//   	case 50:	//C_SE_NC_1 0x32设定短浮点数命令  		
//   		case 61:	//C_SE_TA_1 0x3d带时标的设定归一化值命令  		
//   		case 62:	//C_SE_TB_1 0x3e带时标的设定标度化值命令 		  			
//   		case 63:	//C_SE_TC_1 0x3f带时标的设定短浮点数命令
//   		{
   			//r_Deal_Fv();
//   			break;
//   		}
   		case 49:    //定值修改预置
   		{
//   			bCall_Flag++;
//  			if(bCall_Flag==3)
//   				Dz_Change_Prepare_Flag();
//   			else if(bCall_Flag>3)
//   				bCall_Flag=1;
   			break;
   		}

   		case 0x6c:	//0x6c参数召唤
			r_Reqe_Erro_s104(0x6c,0x07,pRxdFm->Length);
			Edit_DZ_All_s104();
   			break;
   		case 0x70:	//0x70参数预置
			r_call_DZ_YZ(pRxdFm->Length);
   			break;
   		case 0x71:	//0x71参数激活
			r_Reqe_Erro(0x71,0x07,pRxdFm->Length);
			log("(%d),(%d),(%d)\n",DZReturnAddr104.Dev1,DZReturnAddr104.Dev2,DZReturnAddr104.Dev3);
			
			if(DZReturnAddr104.Dev1 == 1){
				r_call_Dz(1,TP_DZ_CALL_EXE);
				DZReturnAddr104.Dev1 = 0;
			}
			if(DZReturnAddr104.Dev2 == 1){
				r_call_Dz(2,TP_DZ_CALL_EXE);
				DZReturnAddr104.Dev2 = 0;
			}
			if(DZReturnAddr104.Dev3 == 1){
				r_call_Dz(3,TP_DZ_CALL_EXE);
				DZReturnAddr104.Dev3 = 0;
			}
			upDateDZData104(DZ_JI_HUO);


   			break;
   		case 115:	//P_CT_NA_CALL 0x73召唤定值
		{
			break;
		}
		
		case 118: 	//G_GZ_LB_1 0x76召唤故障录波
		{
			//r_call_GzLbo();
			break;
		}
		
		case 119:			//zym 08-9-18
		{
//			Reset_Alarm();   //复归警笛警铃信号		
			break;
		}

                case 120:			//chenli 09-7-17
		{
//			START_ACT_RELAY;
//		        taskDelay(100*bAlarmTime);
//		        RESET_ACT_RELAY;                       		
			break;
		}
//                case 121:			//chenli 09-7-17
//		{
//		 
//                        START_ALARM_RELAY;
//		        taskDelay(100*bAlarmTime);
//		        RESET_ALARM_RELAY;		
//			break;
//		}
		default :	//未知的类型标识
		{
//			r_call_Undefine();
			break;
		}					
	 }		
}

//===================================================================
//  函数名称: CS104_2002nr::RxdAPCIInfo         
//  函数功能: 接收控制域判断处理 
//  输入参数:                         
//  返回值：无                        
//===================================================================
int RxdAPCIInfo(int DevID, uint8_t *buf, uint16_t len)
{
	unsigned char Control;
	int fd = gpDevice[DevID].fd;
	pRxdFm = (TS104APDU *)buf;
	Control = pRxdFm->Control1;
    localfd_s104 = fd; 
	switch(Control&RECV_CONTROL1_2002nr)	//判断控制域格式
	{ 
		case 0x3:	Handle_U_Format(fd, DevID);	break; //处理U格式报文
		case 0x1:	Handle_S_Format_s104();	break; //处理S格式报文
        default: 	Handle_I_Format_s104(fd, DevID, buf, len);	break; //缺省处理I格式报文
	}
	return RET_SUCESS;
}
int S104OnTimeOut(int DevID)
{
	DWORD TempDevID;
	DWORD RerurnDevID;
	int i;
	TimecntS104++;


	if(DZ_Comman104 == DZ_YU_ZHI){
		DZ_Comman104 = DZ_Clear;
		Edit_DZYZ104();
	}
//
//	if(TimerOn(TimecntS104, 6)){
//		for(i=0;i<342;i++){
//			S101DzValue[i].DZValue = gpDevice[6].pDZ[i];
////			log("gpDevice[6].pDZ[%d](%x)\n",i,gpDevice[6].pDZ[i]);
//		}
//	
//	}
	if(TimerOn(3)){
		Edit_ChgYc();
	}

	if(TimerOn(3)){
		if(flag_call_16){
			flag_call_16 = 0;
			Edit_call6465(DevID, 0x64, 0x07, 0x24);
			PaiXu_OldSoe();
			Edit_OldSoe();
			Edit_call6465(DevID, 0x64, 0x0a, 0x24);
		
		}
	}

	BYTE dzBrustAddr;//定值突发的地址号
	BYTE *buff;
	BYTE len;
	GetDevFromAddr(5,&TempDevID);//定值突发缓冲区
	
	if(gpDevice[TempDevID].pDZ[4] == DZ_TU_FA){//定值突发
		dzBrustAddr = gpDevice[TempDevID].pDZ[2];
		gpDevice[TempDevID].pDZ[4] = DZ_Clear;
		buff = (BYTE *)&gpDevice[TempDevID].pDZ[10];
		len = buff[1];
		Edit_BrustDz104(gpDevice[DevID].fd, buff,len+2,dzBrustAddr);
		return RET_SUCESS;
	}

	BYTE *bufftmp;
	GetDevFromAddr(6,&RerurnDevID);//定值预置确认缓冲区
	if(gpDevice[RerurnDevID].pDZ[1] == 1){//定值预置确认
		gpDevice[RerurnDevID].pDZ[1] = 0;
		if(gpDevice[RerurnDevID].pDZ[0] == 0x47){
			DZReturnReanson = 0x47;
		}else{
			DZReturnReanson = 0x7;
		}
		DZReturnAddr104.Dev1 = 1;
		bufftmp = (BYTE *)&gpDevice[RerurnDevID].pDZ[Dev1_DZ_Start];
		WriteDZReturn104(gpDevice[DevID].fd, bufftmp);
		flag_dz_return104 = 1;
	}
	if(gpDevice[RerurnDevID].pDZ[2] == 1){//定值预置确认
		gpDevice[RerurnDevID].pDZ[2]=0;
		if(gpDevice[RerurnDevID].pDZ[0] == 0x47){
			DZReturnReanson = 0x47;
		}else{
			DZReturnReanson = 0x7;
		}
		DZReturnAddr104.Dev2 = 1;
		PRINT_FUNLINE;
		bufftmp = (BYTE *)&gpDevice[RerurnDevID].pDZ[Dev2_DZ_Start];
		WriteDZReturn104(gpDevice[DevID].fd, bufftmp);
		flag_dz_return104 = 1;
	}
	if(gpDevice[RerurnDevID].pDZ[3] == 1){//定值预置确认
		gpDevice[RerurnDevID].pDZ[3] = 0;
		if(gpDevice[RerurnDevID].pDZ[0] == 0x47){
			DZReturnReanson = 0x47;
		}else{
			DZReturnReanson = 0x7;
		}
		DZReturnAddr104.Dev3 = 1;
		bufftmp = (BYTE *)&gpDevice[RerurnDevID].pDZ[Dev3_DZ_Start];
		WriteDZReturn104(gpDevice[DevID].fd, bufftmp);
		flag_dz_return104 = 1;
	}

	if(TimecntS104%2 == 0){
		if(flag_dz_return104 == 1){
			flag_dz_return104 = 0;
			TxdCallDZReturn104(gpDevice[DevID].fd);
		}
	}
	if(TimecntS104%3 == 0){
		for(i=0;i<342;i++){
			S104DzValue[i].DZValue = gpDevice[CKLogicDevID].pDZ[i];
		}
 		
	}

}

int siec104Task(int DevID)
{
	return 0;
}


