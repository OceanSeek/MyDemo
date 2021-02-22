#include "sys.h"
#include "SXJ103.h"

//外部变量声明区
extern TBurstYXRecord YXBrustRecord[MAXYXRECORD];
extern TBurstYXRecord YXSoeRecord[MAXYXRECORD];
extern TBurstYCRecord YCBrustRecord[MAXYCRECORD];
extern TransTable_T *TransYKTable;


//变量声明区
int bAutoSendAI;
int bAutoSendSOE;
int bAutoSendSOE1;
int T_04_flag;
int T_05_flag;
int T_2C_flag;
int T_08_flag;
int T_0A_flag;
int OneClass_YK;
int OneClass_flag;

uint32_t bAddSOENum;
uint32_t bAddAINum;
uint32_t wBINum;	  
uint32_t wAINum;
uint16_t wDelayTime;	  
uint16_t *wYXOldValue;
   
uint32_t dwSOENum;
uint32_t dwAIOVERNum;
uint8_t	bNum;
uint8_t	bAINum;
uint8_t	bRem;
uint8_t	bScan;
uint8_t	bRII;
uint8_t	bCOT;
uint8_t	bINF;
uint8_t	bDCC;
uint32_t bBYTENum;
uint8_t	bSOENum1;
uint8_t	bSOENum2;

BYTE BIT8_R1[]={
	0x1,		0x2,		0x4,		0x8,
	0x10,		0x20,		0x40,		0x80};

BYTE BIT8_L1[]={
	0x80,		0x40,		0x20,		0x10,
	0x8,		0x4,		0x2,		0x1};

WORD BIT16_R1[]={
	0x1,		0x2,		0x4,		0x8,
	0x10,		0x20,		0x40,		0x80,
	0x100,		0x200,		0x400,		0x800,
	0x1000,		0x2000,		0x4000,		0x8000};

WORD BIT16_L1[]={
	0x8000,		0x4000,		0x2000,		0x1000,
	0x800,		0x400,		0x200,		0x100,
	0x80,		0x40,		0x20,		0x10,
	0x8,		0x4,		0x2,		0x1};

DWORD BIT32_R1[]={
	0x1,		0x2,		0x4,		0x8,
	0x10,		0x20,		0x40,		0x80,
	0x100,		0x200,		0x400,		0x800,
	0x1000,		0x2000,		0x4000,		0x8000,
	0x10000,	0x20000,	0x40000,	0x80000,
	0x100000,	0x200000,	0x400000,	0x800000,
	0x1000000,	0x2000000,	0x4000000,	0x8000000,
	0x10000000,	0x20000000,	0x40000000,	0x80000000};

DWORD BIT32_L1[]={
	0x80000000,	0x40000000,	0x20000000,	0x10000000,
	0x8000000,	0x4000000,	0x2000000,	0x1000000,
	0x800000,	0x400000,	0x200000,	0x100000,
	0x80000,	0x40000,	0x20000,	0x10000,
	0x8000,		0x4000,		0x2000,		0x1000,
	0x800,		0x400,		0x200,		0x100,
	0x80,		0x40,		0x20,		0x10,
	0x8,		0x4,		0x2,		0x1};


static TCommIO m_Txd;				  //发送缓冲区的处理结构


uint8_t XJ103_Slaver_Sendbuf[XJ103_Slaver_MAX_BUF_LEN];


//函数声明区
static DWORD SearchOneFrame(int DevNo, uint8_t* pBuf,short sLen);
void Rxd_06_Deal(uint8_t* rBuf,uint8_t* sBuf,WORD wLen);
int T_Affirm(int DevNo, uint8_t bCTR);
int T_nodata(int DevNo);
int T_2C_Deal(int DevNo);
int T_08_Deal(int DevNo);
int T_50_Deal(int DevNo, uint8_t bCTR);
int T_64_Deal(int DevNo, uint8_t bCOT,uint8_t bINF,uint8_t bDCC,uint8_t bRII);
int send_write(int DevNo, uint8_t bCOT,uint8_t bINF,uint8_t bDCC,uint8_t bRII);
int Check_Auto_SOE(int DevNo);
int Txd_Auto_YX_40(int DevNo);
int Txd_Auto_SOE_41(int DevNo, uint8_t CTR);
static uint8_t CheckSum(uint8_t * pBuf,uint8_t byLength);
int T_DeviceFlag(int DevNo, uint8_t bCTR,uint8_t bCOT,uint8_t bINF);
void Set103_Reset_Control();
int RxdMonitor_XJ103(int DevNo, uint8_t *buf, uint16_t len);
static int OnTimeOut_XJ103(int DevNo);

int Init_XJ103Slaver(int DevNo)
{
	gpDevice[DevNo].Receive = RxdMonitor_XJ103; 	
	gpDevice[DevNo].OnTimeOut = OnTimeOut_XJ103; 	
	
	wBINum = gVars.TransYXTableNum; 			//单点遥信数目
	wAINum = gVars.TransYCTableNum; 			//遥测数目

	if(wBINum%16==0)								   //遥信点数
		bBYTENum=wBINum/16; 							  //遥信字节数
	else 
		bBYTENum=(wBINum/16)+1;
		
	wYXOldValue= malloc(bBYTENum*sizeof(WORD));
	memset(wYXOldValue, 0, bBYTENum*sizeof(WORD));	 
//			dwSOENum = gpDevice[DevNo].pSoeWrap->Ptr.dwWritePtr; 
//			dwAIOVERNum = gpDevice[DevNo].pAitWrap->Ptr.dwWritePtr;
	bAddSOENum = 0;
	bAddAINum = 0;
	wDelayTime = 0;
	bSOENum1=bSOENum2=0;
//	bCycleSend = false;
	bAutoSendAI = false;
	bAutoSendSOE = false;
	bAutoSendSOE1 = false;
	T_05_flag = false;
	T_04_flag = false;
	T_2C_flag = false;
	T_08_flag = false;
	T_0A_flag = false;
	OneClass_YK = false;
	OneClass_flag = false;
//	bYKSelect = false;

//	ClearAllDevFlag(SF_DevErr); 		//通讯正常标记

	return TRUE;

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
//		定时数据处理部分
//
//===================================================================
//  函数名称: OnTimeOut
//  函数功能: 定时时间到
//  输入参数:                         
//  返回值：无                        
//===================================================================
static int OnTimeOut_XJ103(int DevNo)
{

/*	if(DevNo==3)
	{
        	Check_Auto_AI();
	}*/
	Check_Auto_SOE(DevNo);	
				
}
//===================================================================
//  函数名称: TxdOnTimer
//  函数功能: 校验定时
//  输入参数:  
//  返回值：无                        
//===================================================================
/*int TxdOnTimer()
{
	if(bCycleSend)
	{
		bCycleSend = false;
		Txd_Cycle_BI();
//		Txd_Cycle_DBI();
		Txd_Cycle_AI();

		return true;
	}
	return false;
}*/

//===================================================================
//
//		数据接收部分
//
//===================================================================
//  函数名称: RxdMonitor        
//  函数功能: 数据接收消息
//  输入参数:                         
//  返回值：无                        
//===================================================================
int RxdMonitor_XJ103(int DevNo, uint8_t *buf, uint16_t len)
{
	uint8_t* recvbuf;
	uint8_t* sendbuf;
	uint8_t  pHead;
	WORD wLen;


	recvbuf = buf;	
	wLen = len;
    sendbuf = XJ103_Slaver_Sendbuf;
    m_Txd.ReadPtr = m_Txd.WritePtr = 0;
  
	if(SearchOneFrame(DevNo, buf, len) != RET_SUCESS)
		return RET_ERROR;
	
 	if(recvbuf[0]==0x10){
  		switch(recvbuf[1]){
        case 0x40:                         //复位通信单元
        case 0x47:                         //复位桢记数位
              T_Affirm(DevNo, 0x20);                  //确认帧回复
              T_04_flag=true;              //上送标识报文标志,cot=04
              T_05_flag=true;              //上送标识报文标志,cot=05
//              SystemReset(0);              //复位通信单元                            
          
              break;
        case 0x5b:
        case 0x7b:
               //T_nodata(); 
              if(OneClass_flag==true)                         //召唤二级数据 
               {  
                  T_50_Deal(DevNo, 0x28);                            //上送遥测
                  OneClass_flag=false;
               }
              else
                 T_50_Deal(DevNo, 0x08);
              
        	    break;  
        case 0x5a:
        case 0x7a:                         //召唤一级数据 
             
            if(bAutoSendSOE==true)
              {               	             
                Txd_Auto_YX_40(DevNo);
//                bSOENum1--;
//                if(bSOENum1==0)
//                {
//                  bAutoSendSOE=false; 
//                  bAutoSendSOE1=true;
//                }
                return true;
              }
            if(bAutoSendSOE1==true)
              {
              	Txd_Auto_SOE_41(DevNo, 0x08); 
                return true;
              }
            if( OneClass_YK==true)
              {             		
             	T_64_Deal(DevNo, bCOT,bINF,bDCC,bRII);
             	OneClass_YK=false; 
                return true;            		
              }
            if(T_2C_flag==true)      //总查询
              {                 
                 T_2C_Deal(DevNo);			          //ASDU44上送全遥信
                 T_2C_flag=false;
                 //T_08_flag=true;            // 总召结束标志
                 return true;
              }
            if(T_08_flag==true)      //总召结束
              {
              	 T_08_Deal(DevNo);               //ASDU08上送总召结束报文
                 T_08_flag=false;              	
                 return true;   
              }
             if(T_04_flag==true)         
              {
                 T_DeviceFlag(DevNo, 0x28, 4, 3);         //ASDU5,CTR=0x28,COT=04,INF=3为复位通信单元
                 T_04_flag=false;                
                 return true;
              }
             if(T_05_flag==true)     
              {
                 T_DeviceFlag(DevNo, 0x08, 5, 4);         //ASDU5,CTR=0x28,COT=05,INF=4为重新启动
                 T_05_flag=false;                
                 return true;
              }                                                       
              
               else
              	T_nodata(DevNo);                 //上送无所召唤数据 
              break;        
               
      }
  }
  else if(recvbuf[0]==0x68)
  {
	pHead = recvbuf[6];
	switch(pHead)
	{
	case 0x06:
		Rxd_06_Deal(recvbuf,sendbuf,wLen);			//对时命令
		break;
	case 0x07:
		T_Affirm(DevNo, 0x10);                           //上送确认帧
		T_2C_flag=true;                           //上送全遥信报文标志
                T_08_flag=true;
		OneClass_flag=true;                      //提示主站召一级数据
		bScan=recvbuf[12];                        //存扫描序号
		//Rxd_07_Deal(recvbuf,sendbuf,wLen);			//总查询
		break;
	case 0x40:	                                //遥控选择/执行/撤消
		T_Affirm(DevNo, 0x10);                          //上送确认帧
        OneClass_YK=true;
        OneClass_flag=true;
        bCOT=recvbuf[8];
        bINF=recvbuf[11];
        bDCC=recvbuf[12];
        bRII=recvbuf[13];                        //存标识符
        break;

        /*if(recvbuf[12]==0x81)                
          YKSELOpen_flag=true;                      //提示主站召一级数据
        if(recvbuf[12]==0x82)                
          YKSELClose_flag=true;                      //提示主站召一级数据

        if(recvbuf[12]==0x01)                
          YKEXEOpen_flag=true;                      //提示主站召一级数据
        if(recvbuf[12]==0x02)                
          YKEXEClose_flag=true;                      //提示主站召一级数据

        if(recvbuf[12]==0xc1)                
          YKCANOpen_flag=true;                      //提示主站召一级数据
        if(recvbuf[12]==0xc2)                
          YKCANClose_flag=true;                      //提示主站召一级数据*/
                
	case 0x0a:
	//	Rxd_0a_Deal(recvbuf,sendbuf,wLen);			//通用分类数据
		if(recvbuf[15] ==0x09 && recvbuf[14] ==0x00)
		{
			//信号复归
			Set103_Reset_Control();			//通用分类数据
		//	return;
		}
		break;
/*	case 0x14:
		Rxd_14_Deal(recvbuf,sendbuf,wLen);			//一般命令
		break;*/
/*	case 0x15:
		T_Affirm(DevNo);                               //上送确认帧
		T_0A_flag=true;                           //上送全遥测报文标志
		bRII=recvbuf[12];                        //存返回信息标识符
//		Rxd_15_Deal(recvbuf,sendbuf,wLen);			//通用分类命令
		break;*/

	default:
		T_nodata(DevNo);                            //上送无所召唤数据
		break;
	}
  }
	return true;
}


//===================================================================
//  函数名称: SearchOneFrame        
//  函数功能: 判断接收到的数据是否正确
//  输入参数:                         
//  返回值：无                        
//===================================================================
static DWORD SearchOneFrame(int DevNo, uint8_t* pBuf,short sLen)
{
	WORD wFrameLength;
	WORD wAddr;
	
       wAddr = gpDevice[DevNo].Address;
	
	if(sLen<5)
		return RET_ERROR;

	if(pBuf == NULL)
		return RET_ERROR;

	switch(pBuf[0])
	{
 	    case 0x10:     //固定帧长报文
		{
		 
		if(pBuf[2]!=wAddr)
			return RET_ERROR;                       //地址错
		 
		if( pBuf[3] !=CheckSum(&pBuf[1],2 ))                //校验码错
			return RET_ERROR;


		//结束字符的判定
		if( pBuf[4] != 0x16 )
			return RET_ERROR;


		//帧校验正确
		return RET_SUCESS;
		}
    	case 0x68:     //可变帧长报文
		{
		wFrameLength=pBuf[1]+6;

		//启动字符的判定
		if( pBuf[0] != pBuf[3] )
			return RET_ERROR;

		//长度的判定
		if( pBuf[1] !=pBuf[2] )
			return RET_ERROR;

		//接收报文长度不等于本帧报文长度
		if(wFrameLength != sLen)
			return RET_ERROR;

		//地址域的判定
		/*if(pBuf[5]!=wAddr)
			return RET_ERROR;*/                      //地址错

		//校验和的判定
		if( pBuf[wFrameLength-2] !=
			  CheckSum(&pBuf[4],pBuf[1]))
		return RET_ERROR;

		//结束字符的判定
		if( pBuf[wFrameLength-1] != 0x16 )
			return RET_ERROR;


			//帧校验正确
			return RET_SUCESS;
		}
       	default:               //错误帧
			return RET_ERROR;
     }

}
//===================================================================
//  函数名称: Rxd_06_Deal()
//  函数功能: 对时命令处理
//  输入参数:                         
//  返回值：无                        
//===================================================================
void Rxd_06_Deal(uint8_t* rBuf,uint8_t* sBuf,WORD wLen)
{
	uint8_t* pData;
	TSysTime sysTime;	
	WORD wMSecond;
	TAbsTime absTime;
	TAbsTime beforSynTime;

	DWORD dwControl;
	

	

	pData = rBuf;

//	ReadAbsTime(&beforSynTime);		//读系统绝对时间

	wMSecond = MAKEWORD(pData[12], pData[13]);

	sysTime.MSecond = wMSecond;
	sysTime.Minute = pData[14];
	sysTime.Hour = pData[15];
	sysTime.Day = pData[16]&0x1f;   //yxl 0x0f-->0X1F 2009.4.14
	sysTime.Month = pData[17];      //yxl 7->11 2009.4.14
	sysTime.Year = pData[18]+2000;  //yxl 8->12 2009.4.14
	
//	ToAbsTime(&sysTime, &absTime);	//系统时间转换成绝对时间

	absTime.Lo += wDelayTime;

	if(absTime.Lo<(DWORD)wDelayTime)
		absTime.Hi += 1;

//	WriteAbsTime(&absTime);

	return;
}

//===================================================================
//  函数名称: T_DeviceFlag(uint8_t bCTR,uint8_t bCOT,uint8_t bINF)
//  函数功能: 确认帧
//  输入参数:  bCOT,bINF                       
//  返回值：无      
//  说明    : 
//===================================================================
int T_DeviceFlag(int DevNo, uint8_t bCTR,uint8_t bCOT,uint8_t bINF)
{
	uint8_t* pBuf;
	WORD wAddr;
	
  	wAddr = gpDevice[DevNo].Address;
	m_Txd.ReadPtr = m_Txd.WritePtr = 0;
	pBuf = XJ103_Slaver_Sendbuf;
	
	pBuf[m_Txd.WritePtr++] = 0x68;
	pBuf[m_Txd.WritePtr++] = 0x15;
	pBuf[m_Txd.WritePtr++] = 0x15;
	pBuf[m_Txd.WritePtr++] = 0x68;
	pBuf[m_Txd.WritePtr++] = bCTR;                     //控制域
	pBuf[m_Txd.WritePtr++] = wAddr;                    //链路地址域
	pBuf[m_Txd.WritePtr++] = 0x05;                     //TYP
	pBuf[m_Txd.WritePtr++] = 0x81;                     //VSQ
    pBuf[m_Txd.WritePtr++] = bCOT;                     //COT
	pBuf[m_Txd.WritePtr++] = wAddr;
    pBuf[m_Txd.WritePtr++] = 0xB8;                     //FUN
    pBuf[m_Txd.WritePtr++] = bINF;                     //INF
    pBuf[m_Txd.WritePtr++] = 0x03;                    //COL
    pBuf[m_Txd.WritePtr++] = 0x00;
    pBuf[m_Txd.WritePtr++] = 0x00;
    pBuf[m_Txd.WritePtr++] = 0x00;
    pBuf[m_Txd.WritePtr++] = 0x00;
    pBuf[m_Txd.WritePtr++] = 0x00;
    pBuf[m_Txd.WritePtr++] = 0x00;
    pBuf[m_Txd.WritePtr++] = 0x00;
    pBuf[m_Txd.WritePtr++] = 0x00;
    pBuf[m_Txd.WritePtr++] = 0x00;
    pBuf[m_Txd.WritePtr++] = 0x01;
    pBuf[m_Txd.WritePtr++] = 0x06;
    pBuf[m_Txd.WritePtr++] = 0x03;
    
    pBuf[m_Txd.WritePtr++] =CheckSum(&pBuf[4],21);
    pBuf[m_Txd.WritePtr++] = 0x16;
	XJ103_send(DevNo, pBuf, m_Txd.WritePtr);
	
	return true;	
	 
}


//===================================================================
//  函数名称: T_Affirm(uint8_t bCTR)
//  函数功能: 确认帧
//  输入参数:                         
//  返回值：无      
//  说明    : 
//===================================================================
int T_Affirm(int DevNo, uint8_t bCTR)
{
	uint8_t* pBuf;
	WORD wAddr;
	
  	wAddr = gpDevice[DevNo].Address;
	m_Txd.ReadPtr = m_Txd.WritePtr = 0;
	pBuf = XJ103_Slaver_Sendbuf;
	
	pBuf[m_Txd.WritePtr++] = 0x10;
	pBuf[m_Txd.WritePtr++] = bCTR;
	pBuf[m_Txd.WritePtr++] = wAddr;
	pBuf[m_Txd.WritePtr++] =CheckSum(&pBuf[1],2);
	pBuf[m_Txd.WritePtr++] = 0x16;
	
	XJ103_send(DevNo, pBuf, m_Txd.WritePtr);	
	return true;		
}

//===================================================================
//  函数名称: T_nodata()
//  函数功能: 无所召唤的数据
//  输入参数:                         
//  返回值：无      
//  说明    : 
//===================================================================
int T_nodata(int DevNo)
{	
  uint8_t* pBuf;
	WORD wAddr;
	
  	wAddr = gpDevice[DevNo].Address;
	m_Txd.ReadPtr = m_Txd.WritePtr = 0;
	pBuf = XJ103_Slaver_Sendbuf;
	
	pBuf[m_Txd.WritePtr++] = 0x10;
	pBuf[m_Txd.WritePtr++] = 0x09;
	pBuf[m_Txd.WritePtr++] = wAddr;
	pBuf[m_Txd.WritePtr++] =CheckSum(&pBuf[1],2);
	pBuf[m_Txd.WritePtr++] = 0x16;
	
	XJ103_send(DevNo, pBuf, m_Txd.WritePtr);	
	return true;
}

//===================================================================
//  函数名称: T_2C_Deal()
//  函数功能: 上送全遥信
//  输入参数:                         
//  返回值：无      
//  说明    : 
//===================================================================
int T_2C_Deal(int DevNo)
{
        uint8_t* pBuf;
	WORD wAddr,wValue=0,wValue1=0,wStChange=0,wStChange1=0;
	int i,j;
	
	WORD BIT16_R1[]={
	0x1,		0x2,		0x4,		0x8,
	0x10,		0x20,		0x40,		0x80,
	0x100,  0x200,  0x400,  0x800,
	0x1000, 0x2000, 0x4000, 0x8000};
	
  	wAddr = gpDevice[DevNo].Address;
	m_Txd.ReadPtr = m_Txd.WritePtr = 0;
	pBuf = XJ103_Slaver_Sendbuf;

	pBuf[m_Txd.WritePtr++] = 0x68;
	m_Txd.WritePtr++;
	m_Txd.WritePtr++;	
	pBuf[m_Txd.WritePtr++] = 0x68;
	pBuf[m_Txd.WritePtr++] = 0x28;
	pBuf[m_Txd.WritePtr++] = wAddr;
	pBuf[m_Txd.WritePtr++] = 0x2C;	 	 	
	pBuf[m_Txd.WritePtr++] = bBYTENum;                   //VSQ
	pBuf[m_Txd.WritePtr++] = 0x09;                       //COT
	pBuf[m_Txd.WritePtr++] = wAddr;
	pBuf[m_Txd.WritePtr++] = 0x01;                       //FUN
	pBuf[m_Txd.WritePtr++] = 0x95;                       //INF               

	if(wBINum%16==0){                                    //遥信点数
   		bBYTENum=wBINum/16;                                //遥信字节数
  		for( j = 0; j < bBYTENum; j++){
        	wValue=0;
        	wStChange=0;
        	for( i = 0; i < 16; i++){
	  	     	if(ReadYx(i+j*16) & PBI_ON)
	  	       		wValue |= BIT16_R1[i]; 
	  	    	if((wYXOldValue[j]&(1<<i))!=(wValue&(1<<i)))
	  	      		wStChange|= BIT16_R1[i]; 	        
        	}
	  		pBuf[m_Txd.WritePtr++] = wValue;
	   		pBuf[m_Txd.WritePtr++] = wValue>>8;
	   		pBuf[m_Txd.WritePtr++] =wStChange;
        	pBuf[m_Txd.WritePtr++] =wStChange>>8; 
            pBuf[m_Txd.WritePtr++] =0; 
            wYXOldValue[j]=wValue;
     	}
   }	    
	    
   else 
   {
//	   bBYTENum=(wBINum/16)+1;
    	for(j=0;j<wBINum/16;j++){
        	wValue=0;
        	wStChange=0;
			int i;
        	for( i=0;i<16;i++){
  	      		if(ReadYx(i+j*16) & PBI_ON)
  	         		wValue |= BIT16_R1[i];
  	      		if((wYXOldValue[j]&(1<<i))!=(wValue&(1<<i)))
  	      	 		wStChange|= BIT16_R1[i]; 
          	}
	    	pBuf[m_Txd.WritePtr++] = wValue;
	    	pBuf[m_Txd.WritePtr++] = wValue>>8;
	    	pBuf[m_Txd.WritePtr++] =wStChange;
            pBuf[m_Txd.WritePtr++] =wStChange>>8; 
            pBuf[m_Txd.WritePtr++] =0;
            wYXOldValue[j]=wValue;
     	}

    	for( i=0;i<wBINum%16;i++){
  	      if(ReadYx(i+(wBINum/16)*16) & PBI_ON)
  	         wValue1 |= BIT8_R1[i];
  	      if((wYXOldValue[wBINum/16]&(1<<i))!=(wValue1&(1<<i)))
  	      	 wStChange1|= BIT16_R1[i];
      	}         
     	pBuf[m_Txd.WritePtr++] = wValue1;
       	pBuf[m_Txd.WritePtr++] = wValue1>>8;
       	pBuf[m_Txd.WritePtr++] =wStChange1;
      	pBuf[m_Txd.WritePtr++] =wStChange1>>8; 
      	pBuf[m_Txd.WritePtr++] =0;
      	wYXOldValue[wBINum/16]=wValue1;      
   }
  	pBuf[m_Txd.WritePtr++] = bScan;                    //SCN
	pBuf[1]	=pBuf[2]=9+bBYTENum*5;	
	pBuf[m_Txd.WritePtr++] =CheckSum(&pBuf[4],9+bBYTENum*5);
	pBuf[m_Txd.WritePtr++] = 0x16;
	
	XJ103_send(DevNo, pBuf, m_Txd.WritePtr);
	
	return true;

}

//===================================================================
//  函数名称: T_08_Deal()
//  函数功能: 上送总召结束报文
//  输入参数:                         
//  返回值：无      
//  说明    : 
//===================================================================
int T_08_Deal(int DevNo)
{
	uint8_t* pBuf;
	WORD wAddr;
	
  	wAddr = gpDevice[DevNo].Address;
	m_Txd.ReadPtr = m_Txd.WritePtr = 0;
	pBuf = XJ103_Slaver_Sendbuf;
	
	pBuf[m_Txd.WritePtr++] = 0x68;
	pBuf[m_Txd.WritePtr++] = 0x09;
	pBuf[m_Txd.WritePtr++] = 0x09;
	pBuf[m_Txd.WritePtr++] = 0x68;
	pBuf[m_Txd.WritePtr++] = 0x08;                     //控制域
	pBuf[m_Txd.WritePtr++] = wAddr;                    //链路地址域
	pBuf[m_Txd.WritePtr++] = 0x08;                     //TYP
	pBuf[m_Txd.WritePtr++] = 0x81;                     //VSQ
    pBuf[m_Txd.WritePtr++] = 0x0A;                     //COT
	pBuf[m_Txd.WritePtr++] = wAddr;
    pBuf[m_Txd.WritePtr++] = 0xFF;                     //FUN
    pBuf[m_Txd.WritePtr++] = 0x00;                     //INF
    pBuf[m_Txd.WritePtr++] = bScan;                    //SCN
    pBuf[m_Txd.WritePtr++] =CheckSum(&pBuf[4],9);
    pBuf[m_Txd.WritePtr++] = 0x16;
    
    XJ103_send(DevNo, pBuf, m_Txd.WritePtr);	

    return true;	 
}

//===================================================================
//  函数名称: T_50_Deal(uint8_t bCTR)
//  函数功能: 上送全遥测
//  输入参数:                         
//  返回值：无      
//  说明    : 
//===================================================================
int T_50_Deal(int DevNo, uint8_t bCTR)
{
	short YcValue;
//	uint8_t  bNOG;                                 //分bNum帧传送
	uint8_t* pBuf;
	WORD wAddr;
	
  	wAddr = gpDevice[DevNo].Address;
	m_Txd.ReadPtr = m_Txd.WritePtr = 0;
	pBuf = XJ103_Slaver_Sendbuf;
	
	pBuf[m_Txd.WritePtr++] = 0x68;
	m_Txd.WritePtr++;
	m_Txd.WritePtr++;	
	pBuf[m_Txd.WritePtr++] = 0x68;	
	pBuf[m_Txd.WritePtr++] = bCTR;
	
	pBuf[m_Txd.WritePtr++] = wAddr;                    //链路地址域
	pBuf[m_Txd.WritePtr++] = 0x32;                     //TYP
	pBuf[m_Txd.WritePtr++] = wAINum&0x7f;              //VSQ遥测个数
	pBuf[m_Txd.WritePtr++] = 0x02;                     //COT
	pBuf[m_Txd.WritePtr++] = wAddr;                    
	pBuf[m_Txd.WritePtr++] = 0x01;                     //FUN
	pBuf[m_Txd.WritePtr++] = 0x5c;                     //INF
	int i;
	for( i = 0; i < wAINum; i++)
	{
	     YcValue = ReadYcData16(i);
	     YcValue=YcValue<<3;		   	   
             pBuf[m_Txd.WritePtr++] = YcValue;
  	     pBuf[m_Txd.WritePtr++] = YcValue>>8;
	}
	pBuf[1]=pBuf[2]=wAINum*2+8;	
	pBuf[m_Txd.WritePtr++] =CheckSum(&pBuf[4],wAINum*2+8);
    pBuf[m_Txd.WritePtr++] = 0x16; 
    XJ103_send(DevNo, pBuf, m_Txd.WritePtr);	
    return true;

}

//===================================================================
//  函数名称: T_0A_Deal()
//  函数功能: 上送全遥测
//  输入参数:                         
//  返回值：无      
//  说明    : 
//===================================================================
/*int T_0A_Deal(uint8_t bContr)
{
	short YcValue;
	uint8_t  bNOG;                                 //分bNum帧传送
	uint8_t* pBuf;
	WORD wAddr;
	
  wAddr = gpDevice[DevNo].Address;
	m_Txd.ReadPtr = m_Txd.WritePtr = 0;
	pBuf = XJ103_Slaver_Sendbuf;
	
	pBuf[m_Txd.WritePtr++] = 0x68;
	m_Txd.WritePtr++;
	m_Txd.WritePtr++;	
	pBuf[m_Txd.WritePtr++] = 0x68;	
	pBuf[m_Txd.WritePtr++] = bContr;
	
	pBuf[m_Txd.WritePtr++] = wAddr;                    //链路地址域
	pBuf[m_Txd.WritePtr++] = 0x0A;                     //TYP
	pBuf[m_Txd.WritePtr++] = 0x81;                     //VSQ
	pBuf[m_Txd.WritePtr++] = 0x2A;                     //COT
	pBuf[m_Txd.WritePtr++] = wAddr;                    
	pBuf[m_Txd.WritePtr++] = 0xFE;                     //FUN
	pBuf[m_Txd.WritePtr++] = 0xF1;                     //INF
	pBuf[m_Txd.WritePtr++] = bRII;                     //返回信息标示符RII
	
	if(bRem==0||bNum>1)
		bNOG=12;
	else 
		bNOG=bRem;
				
	pBuf[m_Txd.WritePtr++] = bNOG;                    //通用分类标识数目NOG
	
	for(int i=0;i<bNOG;i++)
	{	   
	   pBuf[m_Txd.WritePtr++] = 0x01;                  //通用分类标识序号GIN_H
	   pBuf[m_Txd.WritePtr++] = (bAINum-bNum)*12+i;
	   pBuf[m_Txd.WritePtr++] = 0x01;                  //描述类别
       pBuf[m_Txd.WritePtr++] = 0x04;                  //数据类型
       pBuf[m_Txd.WritePtr++] = 0x02;                  //数据宽度
       pBuf[m_Txd.WritePtr++] = 0x01;                  //数目
	   YcValue = ::ReadAI((DWORD)DevNo,(bAINum-bNum)*12+i);
       pBuf[m_Txd.WritePtr++] = YcValue;
  	   pBuf[m_Txd.WritePtr++] = YcValue>>8;
	}
	
	pBuf[1]=pBuf[2]=bNOG*8+10;	
	pBuf[m_Txd.WritePtr++] =CheckSum(&pBuf[4],bNOG*8+10);
    pBuf[m_Txd.WritePtr++] = 0x16;
 
    XJ103_send(DevNo, pBuf, m_Txd.WritePtr);	

    return true;

}*/


//===================================================================
//  函数名称: Rxd_0a_Deal()
//  函数功能: 通用分类数据查询的回复
//  输入参数:                         
//  返   回  值 : 无
//  说明           : 回复所有遥信点和遥测点信息
//===================================================================
int T_64_Deal(int DevNo, uint8_t bCOT,uint8_t bINF,uint8_t bDCC,uint8_t bRII)
{
     	DWORD dwAttrib;
        uint8_t  bCommand;

        switch(bDCC&0x03)
	{
	  case 0x02: dwAttrib=TP_CLOSE;
                     break; //合
                    
	  case 0x01: dwAttrib=TP_OPEN; 
                     break;  //分	
        }
	
        switch(bDCC&0xC0)
        {

          case 0x80:
                     bCommand=TP_YKSELECT;          //遥控选择 
		     break;
          case 0x00:
                     bCommand=TP_YKOPERATE;          //遥控执行                 
		     break;
          case 0xC0:
                     bCommand=TP_YKCANCEL;           //遥控取消
		     break;
       
        }
 
//        SetYK(DevNo,bCommand,bINF,dwAttrib);
        send_write(DevNo, bCOT,bINF,bDCC,bRII); 
	
       return true;
}

//===================================================================
//  函数名称: Rxd_0a_Deal()
//  函数功能: 通用分类数据查询的回复
//  输入参数:                         
//  返   回  值 : 无
//  说明           : 回复所有遥信点和遥测点信息
//===================================================================
/*void Rxd_0a_Deal(uint8_t* rBuf,uint8_t* sBuf,WORD wLen)
{
	uint8_t cot,bAttrib;//传送原因
	uint8_t inf;//信息序号
	uint8_t rii;//返回表示
	uint8_t ginh,ginl,bCommand;
	DWORD dwAttrib;
	
 
	inf=rBuf[11];                         //信息序号
	rii=rBuf[12];                         //返回信息标识符
//	ginh=0x01;                            //仅有一组遥控
	ginl=rBuf[15];                        //遥控号
	bAttrib=rBuf[wLen-3];                 //遥控属性  

	switch(bAttrib)
	{
	  case 0x02: dwAttrib=TP_CLOSE; break; //合
	  case 0x01: dwAttrib=TP_OPEN; break; //分
	}
			
	switch(inf) 
	{
		case 249:                           //遥控选择 
			   cot=0x2c;
			   bCommand=TP_YKSELECT;
			   break;			   
		case 250:                           //遥控执行
			   cot=0x28;
			   bCommand=TP_YKOPERATE;                           
			   break;			   			   
		case 251:                           //遥控取消
			   cot=0x2c;
			   bCommand=TP_YKCANCEL;
			   break;				
	}

  send_write(DevNo cot,inf,rii,ginl,bAttrib); 
	SetYK(DevNo,bCommand,ginl,dwAttrib);
			
}*/	
int send_write(int DevNo, uint8_t bCOT,uint8_t bINF,uint8_t bDCC,uint8_t bRII)
{
        uint8_t* pBuf;
	WORD wAddr;
	
  	wAddr = gpDevice[DevNo].Address;
	m_Txd.ReadPtr = m_Txd.WritePtr = 0;
	pBuf = XJ103_Slaver_Sendbuf;
	
	pBuf[m_Txd.WritePtr++] = 0x68;
	pBuf[m_Txd.WritePtr++] = 0x0a;
	pBuf[m_Txd.WritePtr++] = 0x0a;
	pBuf[m_Txd.WritePtr++] = 0x68;
	pBuf[m_Txd.WritePtr++] = 0x08;                      //控制域
	pBuf[m_Txd.WritePtr++] = wAddr;                     //链路地址域
	pBuf[m_Txd.WritePtr++] = 0x40;                      //TYP
	pBuf[m_Txd.WritePtr++] = 0x01;                      //VSQ
        pBuf[m_Txd.WritePtr++] = bCOT;                      //COT
	pBuf[m_Txd.WritePtr++] = wAddr;
        pBuf[m_Txd.WritePtr++] = 0x01;                      //FUN
        pBuf[m_Txd.WritePtr++] = bINF;                      //INF
        pBuf[m_Txd.WritePtr++] = bDCC;                      //DCC
        pBuf[m_Txd.WritePtr++] = bRII;                      //RII

        pBuf[m_Txd.WritePtr++] =CheckSum(&pBuf[4],10);
        pBuf[m_Txd.WritePtr++] = 0x16;
	  
	XJ103_send(DevNo, pBuf, m_Txd.WritePtr);	
	return true;


}
/*
int send_write(uint8_t cot,uint8_t inf,uint8_t rii,uint8_t ginl,uint8_t bAttrib)
{
	uint8_t* pBuf;
	WORD wAddr;
	
        wAddr = GetAddress();
	m_Txd.ReadPtr = m_Txd.WritePtr = 0;
	pBuf = XJ103_Slaver_Sendbuf;
	
	pBuf[m_Txd.WritePtr++] = 0x68;
	pBuf[m_Txd.WritePtr++] = 0x11;
	pBuf[m_Txd.WritePtr++] = 0x11;
	pBuf[m_Txd.WritePtr++] = 0x68;
	pBuf[m_Txd.WritePtr++] = 0x08;                      //控制域
	pBuf[m_Txd.WritePtr++] = wAddr;                     //链路地址域
	pBuf[m_Txd.WritePtr++] = 0x0A;                      //TYP
	pBuf[m_Txd.WritePtr++] = 0x81;                      //VSQ
    pBuf[m_Txd.WritePtr++] = cot;                     //COT
	pBuf[m_Txd.WritePtr++] = wAddr;
    pBuf[m_Txd.WritePtr++] = 0xFE;                    //FUN
    pBuf[m_Txd.WritePtr++] = inf;                     //INF
    pBuf[m_Txd.WritePtr++] = rii;                     //RII
    pBuf[m_Txd.WritePtr++] = 0x01;                    //NOG
    pBuf[m_Txd.WritePtr++] = 0x01;                    //GINH=0x01,仅有一组遥控
    pBuf[m_Txd.WritePtr++] = ginl;                    //遥控号
    pBuf[m_Txd.WritePtr++] = 0x01;
    pBuf[m_Txd.WritePtr++] = 0x09;
    pBuf[m_Txd.WritePtr++] = 0x01;
    pBuf[m_Txd.WritePtr++] = 0x01;
    pBuf[m_Txd.WritePtr++] = bAttrib;
       
    pBuf[m_Txd.WritePtr++] =CheckSum(&pBuf[4],17);
    pBuf[m_Txd.WritePtr++] = 0x16;
	  
	  XJ103_send(DevNo, pBuf, m_Txd.WritePtr);	
	  return true;	
	
}*/


//===================================================================
//
//		数据发送部分
//
//===================================================================
//  函数名称: TxdMonitor        
//  函数功能: 数据发送消息
//  输入参数:                         
//  返回值：无                        
//===================================================================
static int TxdMonitor(void)
{
//	if(TxdOnTimer())
//		return true;
	
	return true;
}


//===================================================================
//  函数名称: Check_Auto        
//  函数功能: 检测是否有SOE事件
//  输入参数:                         
//  返回值：无                        
//===================================================================
int Check_Auto_SOE(int DevNo)
{
	bAutoSendSOE = (gpDevice[DevNo].ReadYXPtr==gVars.YxWritePtr)?FALSE:TRUE;
	bAutoSendSOE1 = (gpDevice[DevNo].ReadYXSoePtr==gVars.YxSoeWritePtr)?FALSE:TRUE;
	
	return RET_SUCESS;
	
}

//===================================================================
//  函数名称: Check_Auto_AI    
//  函数功能: 检测是否有遥测越限事件
//  输入参数:                         
//  返回值：无                        
//===================================================================
/*int Check_Auto_AI()
{
	DWORD ai;
	ai = gpDevice[DevNo].pAitWrap->Ptr.dwWritePtr;
	if(ai == dwAIOVERNum)
		bAutoSendAI = false;
	else
	{
		bAddAINum = ai - dwAIOVERNum;
		dwAIOVERNum = ai;
		bAutoSendAI = true;
		return true;
	}
  
  if(GetBurstAINum(DevNo)!=0)
  {
  	bAutoSendAI = true;
  	return true; 
  }      
	return false;
}*/
//===================================================================
//  函数名称: Txd_Auto_YX_40()        
//  函数功能: 需要立即上送的报文
//  输入参数:  
//  返回值：无                        
//===================================================================
int Txd_Auto_YX_40(int DevNo)
{
	DWORD dwReadPtr;
//	WORD wMillisecond;
	uint8_t* pBuf;
	TSysTimeSoe SysTimeSoe;
	WORD wAddr;
  	wAddr = gpDevice[DevNo].Address;

	m_Txd.ReadPtr =0;
	m_Txd.WritePtr=0;
	pBuf = XJ103_Slaver_Sendbuf;

		
        pBuf[m_Txd.WritePtr++] = 0x68;
        m_Txd.WritePtr++;
	m_Txd.WritePtr++;
	pBuf[m_Txd.WritePtr++] = 0x68;
	pBuf[m_Txd.WritePtr++] = 0x28;
	pBuf[m_Txd.WritePtr++] = wAddr;                    //链路地址域
	pBuf[m_Txd.WritePtr++] = 0x28;                     //TYP
	pBuf[m_Txd.WritePtr++] = 0x81;                      //VSQ
	pBuf[m_Txd.WritePtr++] = 0x01;                     //COT
	pBuf[m_Txd.WritePtr++] = wAddr;
	pBuf[m_Txd.WritePtr++] = 0x01;                     //FUN


	if(ReadSysTimeSoe((DWORD)DevNo, &SysTimeSoe))
	{
	   pBuf[m_Txd.WritePtr++] = SysTimeSoe.nNo+0x95;								
	     if(SysTimeSoe.bStatus&PBI_ON)
		pBuf[m_Txd.WritePtr++] = 1;
	     else
		pBuf[m_Txd.WritePtr++] = 0;
						
	}
		
   	pBuf[m_Txd.WritePtr++] = 0x00;
   	pBuf[1]=pBuf[2]=0x0a;	
   	pBuf[m_Txd.WritePtr++] =CheckSum(&pBuf[4],10);
   	pBuf[m_Txd.WritePtr++] = 0x16;

   	XJ103_send(DevNo, pBuf, m_Txd.WritePtr);
  	return true;

}

//===================================================================
//  函数名称: CSCOM103::Txd_Auto_SOE_41(uint8_t CTR)        
//  函数功能: 需要立即上送的报文
//  输入参数:  
//  返回值：无                        
//===================================================================
int Txd_Auto_SOE_41(int DevNo, uint8_t CTR)
{
	DWORD dwReadPtr;
	WORD wMillisecond;
	uint8_t* pBuf;
	TSysTimeSoe SysTimeSoe;
	
	WORD wAddr = gpDevice[DevNo].Address;

	m_Txd.ReadPtr = 0;
	m_Txd.WritePtr=0;
	pBuf = XJ103_Slaver_Sendbuf;

        pBuf[m_Txd.WritePtr++] = 0x68;
	m_Txd.WritePtr++;
	m_Txd.WritePtr++;
	pBuf[m_Txd.WritePtr++] = 0x68;
	pBuf[m_Txd.WritePtr++] = CTR;
	pBuf[m_Txd.WritePtr++] = wAddr;                    //链路地址域
	pBuf[m_Txd.WritePtr++] = 0x29;                     //TYP
	pBuf[m_Txd.WritePtr++] = 0x81;                     //VSQ
	pBuf[m_Txd.WritePtr++] = 0x01;                     //COT
	pBuf[m_Txd.WritePtr++] = wAddr;
	pBuf[m_Txd.WritePtr++] = 0x01;                     //FUN

	int num = 0;
        
	while(ReadSysTimeSoe((DWORD)DevNo,&SysTimeSoe)){
	  	pBuf[m_Txd.WritePtr++] = SysTimeSoe.nNo+0x95;	
	  	pBuf[m_Txd.WritePtr++] = (SysTimeSoe.bStatus&PBI_ON)?1:0;
		wMillisecond = SysTimeSoe.stTime._milliseconds;
       	pBuf[m_Txd.WritePtr++] = (uint8_t)wMillisecond;
       	uint8_t temp = wMillisecond>>8;
       	pBuf[m_Txd.WritePtr++] = temp;
       	pBuf[m_Txd.WritePtr++] = SysTimeSoe.stTime._min._minutes;
       	pBuf[m_Txd.WritePtr++] = SysTimeSoe.stTime._hour._hours;
		num++;
		if(num>9)
			break;
	}
             
   	pBuf[m_Txd.WritePtr++] = 0x01;

    pBuf[1]=pBuf[2]=0x0e;;	
    pBuf[m_Txd.WritePtr++] =CheckSum(&pBuf[4],14);
    pBuf[m_Txd.WritePtr++] = 0x16;
	XJ103_send(DevNo, pBuf, m_Txd.WritePtr);

	return true;

}

/*//===================================================================
//  函数名称: Txd_Auto_AI 
//  函数功能: 主动发送遥测越限事件	(遥测越限事件暂时不上送)
//  输入参数:                         
//  返回值：
//===================================================================
int Txd_Auto_AI()
{
//	DWORD dwReadPtr;
	short YcValue;
	uint8_t* pBuf;
//	TSysTimeAit pSysTimeAit;
	
	WORD wAddr = GetAddress();

	m_Txd.ReadPtr = 0;
	m_Txd.WritePtr=0;
	pBuf = XJ103_Slaver_Sendbuf;


        pBuf[m_Txd.WritePtr++] = 0x68;
		m_Txd.WritePtr++;
		m_Txd.WritePtr++;
		pBuf[m_Txd.WritePtr++] = 0x68;
		pBuf[m_Txd.WritePtr++] = 0x28;
		pBuf[m_Txd.WritePtr++] = wAddr;                    //链路地址域
	    pBuf[m_Txd.WritePtr++] = 0x09;                     //TYP
		pBuf[m_Txd.WritePtr++] = wAINum;                   //VSQ
		pBuf[m_Txd.WritePtr++] = 0x01;                     //COT
	    pBuf[m_Txd.WritePtr++] = wAddr;
	    pBuf[m_Txd.WritePtr++] = 0x01;                     //FUN
	    pBuf[m_Txd.WritePtr++] = 0x2E;                     //INF

        for(int i=0;i<wAINum;i++)
        {

          YcValue = ::ReadAI((DWORD)DevNo,i);
  		  pBuf[m_Txd.WritePtr++] = YcValue;
  		  pBuf[m_Txd.WritePtr++] = YcValue>>8;
        }

  		pBuf[1]=pBuf[2]=wAINum*2+8;	
	    pBuf[m_Txd.WritePtr++] =CheckSum(&pBuf[4],wAINum*2+8);
        pBuf[m_Txd.WritePtr++] = 0x16;

        XJ103_send(DevNo, pBuf, m_Txd.WritePtr);
  		return true;

}*/

//*******************************************************************************************//
//  函数名称: CheckSum
//  函数功能: 实现累加和校验
//  输入参数: uint8_t * pBuf, 输入缓冲区
//            uint8_t byLength, 缓冲区长度
//  返回值:   uint8_t bySum, 累加和
//*******************************************************************************************//
static uint8_t CheckSum(uint8_t * pBuf,uint8_t byLength)
{
	uint8_t bySum=0;
	int i;
	for(i = 0; i < byLength; i++)
		bySum += pBuf[i];
	return bySum;
}

//===================================================================
//  函数名称: CS103::Set103_Reset_Control
//  函数功能: 复归执行
//  输入参数:  
//  返回值：无                        
//===================================================================
//extern void SignalReset();
void Set103_Reset_Control()
{	
	//SystemReset(0); //yxl 2010.10.26 复归告警和动作继电器
//	SignalReset();
}




