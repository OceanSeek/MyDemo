#include "sys.h"
#include "SXJ103.h"

//�ⲿ����������
extern TBurstYXRecord YXBrustRecord[MAXYXRECORD];
extern TBurstYXRecord YXSoeRecord[MAXYXRECORD];
extern TBurstYCRecord YCBrustRecord[MAXYCRECORD];
extern TransTable_T *TransYKTable;


//����������
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


static TCommIO m_Txd;				  //���ͻ������Ĵ���ṹ


uint8_t XJ103_Slaver_Sendbuf[XJ103_Slaver_MAX_BUF_LEN];


//����������
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
	
	wBINum = gVars.TransYXTableNum; 			//����ң����Ŀ
	wAINum = gVars.TransYCTableNum; 			//ң����Ŀ

	if(wBINum%16==0)								   //ң�ŵ���
		bBYTENum=wBINum/16; 							  //ң���ֽ���
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

//	ClearAllDevFlag(SF_DevErr); 		//ͨѶ�������

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

/*	if(DevNo==3)
	{
        	Check_Auto_AI();
	}*/
	Check_Auto_SOE(DevNo);	
				
}
//===================================================================
//  ��������: TxdOnTimer
//  ��������: У�鶨ʱ
//  �������:  
//  ����ֵ����                        
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
//		���ݽ��ղ���
//
//===================================================================
//  ��������: RxdMonitor        
//  ��������: ���ݽ�����Ϣ
//  �������:                         
//  ����ֵ����                        
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
        case 0x40:                         //��λͨ�ŵ�Ԫ
        case 0x47:                         //��λ�����λ
              T_Affirm(DevNo, 0x20);                  //ȷ��֡�ظ�
              T_04_flag=true;              //���ͱ�ʶ���ı�־,cot=04
              T_05_flag=true;              //���ͱ�ʶ���ı�־,cot=05
//              SystemReset(0);              //��λͨ�ŵ�Ԫ                            
          
              break;
        case 0x5b:
        case 0x7b:
               //T_nodata(); 
              if(OneClass_flag==true)                         //�ٻ��������� 
               {  
                  T_50_Deal(DevNo, 0x28);                            //����ң��
                  OneClass_flag=false;
               }
              else
                 T_50_Deal(DevNo, 0x08);
              
        	    break;  
        case 0x5a:
        case 0x7a:                         //�ٻ�һ������ 
             
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
            if(T_2C_flag==true)      //�ܲ�ѯ
              {                 
                 T_2C_Deal(DevNo);			          //ASDU44����ȫң��
                 T_2C_flag=false;
                 //T_08_flag=true;            // ���ٽ�����־
                 return true;
              }
            if(T_08_flag==true)      //���ٽ���
              {
              	 T_08_Deal(DevNo);               //ASDU08�������ٽ�������
                 T_08_flag=false;              	
                 return true;   
              }
             if(T_04_flag==true)         
              {
                 T_DeviceFlag(DevNo, 0x28, 4, 3);         //ASDU5,CTR=0x28,COT=04,INF=3Ϊ��λͨ�ŵ�Ԫ
                 T_04_flag=false;                
                 return true;
              }
             if(T_05_flag==true)     
              {
                 T_DeviceFlag(DevNo, 0x08, 5, 4);         //ASDU5,CTR=0x28,COT=05,INF=4Ϊ��������
                 T_05_flag=false;                
                 return true;
              }                                                       
              
               else
              	T_nodata(DevNo);                 //���������ٻ����� 
              break;        
               
      }
  }
  else if(recvbuf[0]==0x68)
  {
	pHead = recvbuf[6];
	switch(pHead)
	{
	case 0x06:
		Rxd_06_Deal(recvbuf,sendbuf,wLen);			//��ʱ����
		break;
	case 0x07:
		T_Affirm(DevNo, 0x10);                           //����ȷ��֡
		T_2C_flag=true;                           //����ȫң�ű��ı�־
                T_08_flag=true;
		OneClass_flag=true;                      //��ʾ��վ��һ������
		bScan=recvbuf[12];                        //��ɨ�����
		//Rxd_07_Deal(recvbuf,sendbuf,wLen);			//�ܲ�ѯ
		break;
	case 0x40:	                                //ң��ѡ��/ִ��/����
		T_Affirm(DevNo, 0x10);                          //����ȷ��֡
        OneClass_YK=true;
        OneClass_flag=true;
        bCOT=recvbuf[8];
        bINF=recvbuf[11];
        bDCC=recvbuf[12];
        bRII=recvbuf[13];                        //���ʶ��
        break;

        /*if(recvbuf[12]==0x81)                
          YKSELOpen_flag=true;                      //��ʾ��վ��һ������
        if(recvbuf[12]==0x82)                
          YKSELClose_flag=true;                      //��ʾ��վ��һ������

        if(recvbuf[12]==0x01)                
          YKEXEOpen_flag=true;                      //��ʾ��վ��һ������
        if(recvbuf[12]==0x02)                
          YKEXEClose_flag=true;                      //��ʾ��վ��һ������

        if(recvbuf[12]==0xc1)                
          YKCANOpen_flag=true;                      //��ʾ��վ��һ������
        if(recvbuf[12]==0xc2)                
          YKCANClose_flag=true;                      //��ʾ��վ��һ������*/
                
	case 0x0a:
	//	Rxd_0a_Deal(recvbuf,sendbuf,wLen);			//ͨ�÷�������
		if(recvbuf[15] ==0x09 && recvbuf[14] ==0x00)
		{
			//�źŸ���
			Set103_Reset_Control();			//ͨ�÷�������
		//	return;
		}
		break;
/*	case 0x14:
		Rxd_14_Deal(recvbuf,sendbuf,wLen);			//һ������
		break;*/
/*	case 0x15:
		T_Affirm(DevNo);                               //����ȷ��֡
		T_0A_flag=true;                           //����ȫң�ⱨ�ı�־
		bRII=recvbuf[12];                        //�淵����Ϣ��ʶ��
//		Rxd_15_Deal(recvbuf,sendbuf,wLen);			//ͨ�÷�������
		break;*/

	default:
		T_nodata(DevNo);                            //���������ٻ�����
		break;
	}
  }
	return true;
}


//===================================================================
//  ��������: SearchOneFrame        
//  ��������: �жϽ��յ��������Ƿ���ȷ
//  �������:                         
//  ����ֵ����                        
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
 	    case 0x10:     //�̶�֡������
		{
		 
		if(pBuf[2]!=wAddr)
			return RET_ERROR;                       //��ַ��
		 
		if( pBuf[3] !=CheckSum(&pBuf[1],2 ))                //У�����
			return RET_ERROR;


		//�����ַ����ж�
		if( pBuf[4] != 0x16 )
			return RET_ERROR;


		//֡У����ȷ
		return RET_SUCESS;
		}
    	case 0x68:     //�ɱ�֡������
		{
		wFrameLength=pBuf[1]+6;

		//�����ַ����ж�
		if( pBuf[0] != pBuf[3] )
			return RET_ERROR;

		//���ȵ��ж�
		if( pBuf[1] !=pBuf[2] )
			return RET_ERROR;

		//���ձ��ĳ��Ȳ����ڱ�֡���ĳ���
		if(wFrameLength != sLen)
			return RET_ERROR;

		//��ַ����ж�
		/*if(pBuf[5]!=wAddr)
			return RET_ERROR;*/                      //��ַ��

		//У��͵��ж�
		if( pBuf[wFrameLength-2] !=
			  CheckSum(&pBuf[4],pBuf[1]))
		return RET_ERROR;

		//�����ַ����ж�
		if( pBuf[wFrameLength-1] != 0x16 )
			return RET_ERROR;


			//֡У����ȷ
			return RET_SUCESS;
		}
       	default:               //����֡
			return RET_ERROR;
     }

}
//===================================================================
//  ��������: Rxd_06_Deal()
//  ��������: ��ʱ�����
//  �������:                         
//  ����ֵ����                        
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

//	ReadAbsTime(&beforSynTime);		//��ϵͳ����ʱ��

	wMSecond = MAKEWORD(pData[12], pData[13]);

	sysTime.MSecond = wMSecond;
	sysTime.Minute = pData[14];
	sysTime.Hour = pData[15];
	sysTime.Day = pData[16]&0x1f;   //yxl 0x0f-->0X1F 2009.4.14
	sysTime.Month = pData[17];      //yxl 7->11 2009.4.14
	sysTime.Year = pData[18]+2000;  //yxl 8->12 2009.4.14
	
//	ToAbsTime(&sysTime, &absTime);	//ϵͳʱ��ת���ɾ���ʱ��

	absTime.Lo += wDelayTime;

	if(absTime.Lo<(DWORD)wDelayTime)
		absTime.Hi += 1;

//	WriteAbsTime(&absTime);

	return;
}

//===================================================================
//  ��������: T_DeviceFlag(uint8_t bCTR,uint8_t bCOT,uint8_t bINF)
//  ��������: ȷ��֡
//  �������:  bCOT,bINF                       
//  ����ֵ����      
//  ˵��    : 
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
	pBuf[m_Txd.WritePtr++] = bCTR;                     //������
	pBuf[m_Txd.WritePtr++] = wAddr;                    //��·��ַ��
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
//  ��������: T_Affirm(uint8_t bCTR)
//  ��������: ȷ��֡
//  �������:                         
//  ����ֵ����      
//  ˵��    : 
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
//  ��������: T_nodata()
//  ��������: �����ٻ�������
//  �������:                         
//  ����ֵ����      
//  ˵��    : 
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
//  ��������: T_2C_Deal()
//  ��������: ����ȫң��
//  �������:                         
//  ����ֵ����      
//  ˵��    : 
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

	if(wBINum%16==0){                                    //ң�ŵ���
   		bBYTENum=wBINum/16;                                //ң���ֽ���
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
//  ��������: T_08_Deal()
//  ��������: �������ٽ�������
//  �������:                         
//  ����ֵ����      
//  ˵��    : 
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
	pBuf[m_Txd.WritePtr++] = 0x08;                     //������
	pBuf[m_Txd.WritePtr++] = wAddr;                    //��·��ַ��
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
//  ��������: T_50_Deal(uint8_t bCTR)
//  ��������: ����ȫң��
//  �������:                         
//  ����ֵ����      
//  ˵��    : 
//===================================================================
int T_50_Deal(int DevNo, uint8_t bCTR)
{
	short YcValue;
//	uint8_t  bNOG;                                 //��bNum֡����
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
	
	pBuf[m_Txd.WritePtr++] = wAddr;                    //��·��ַ��
	pBuf[m_Txd.WritePtr++] = 0x32;                     //TYP
	pBuf[m_Txd.WritePtr++] = wAINum&0x7f;              //VSQң�����
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
//  ��������: T_0A_Deal()
//  ��������: ����ȫң��
//  �������:                         
//  ����ֵ����      
//  ˵��    : 
//===================================================================
/*int T_0A_Deal(uint8_t bContr)
{
	short YcValue;
	uint8_t  bNOG;                                 //��bNum֡����
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
	
	pBuf[m_Txd.WritePtr++] = wAddr;                    //��·��ַ��
	pBuf[m_Txd.WritePtr++] = 0x0A;                     //TYP
	pBuf[m_Txd.WritePtr++] = 0x81;                     //VSQ
	pBuf[m_Txd.WritePtr++] = 0x2A;                     //COT
	pBuf[m_Txd.WritePtr++] = wAddr;                    
	pBuf[m_Txd.WritePtr++] = 0xFE;                     //FUN
	pBuf[m_Txd.WritePtr++] = 0xF1;                     //INF
	pBuf[m_Txd.WritePtr++] = bRII;                     //������Ϣ��ʾ��RII
	
	if(bRem==0||bNum>1)
		bNOG=12;
	else 
		bNOG=bRem;
				
	pBuf[m_Txd.WritePtr++] = bNOG;                    //ͨ�÷����ʶ��ĿNOG
	
	for(int i=0;i<bNOG;i++)
	{	   
	   pBuf[m_Txd.WritePtr++] = 0x01;                  //ͨ�÷����ʶ���GIN_H
	   pBuf[m_Txd.WritePtr++] = (bAINum-bNum)*12+i;
	   pBuf[m_Txd.WritePtr++] = 0x01;                  //�������
       pBuf[m_Txd.WritePtr++] = 0x04;                  //��������
       pBuf[m_Txd.WritePtr++] = 0x02;                  //���ݿ��
       pBuf[m_Txd.WritePtr++] = 0x01;                  //��Ŀ
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
//  ��������: Rxd_0a_Deal()
//  ��������: ͨ�÷������ݲ�ѯ�Ļظ�
//  �������:                         
//  ��   ��  ֵ : ��
//  ˵��           : �ظ�����ң�ŵ��ң�����Ϣ
//===================================================================
int T_64_Deal(int DevNo, uint8_t bCOT,uint8_t bINF,uint8_t bDCC,uint8_t bRII)
{
     	DWORD dwAttrib;
        uint8_t  bCommand;

        switch(bDCC&0x03)
	{
	  case 0x02: dwAttrib=TP_CLOSE;
                     break; //��
                    
	  case 0x01: dwAttrib=TP_OPEN; 
                     break;  //��	
        }
	
        switch(bDCC&0xC0)
        {

          case 0x80:
                     bCommand=TP_YKSELECT;          //ң��ѡ�� 
		     break;
          case 0x00:
                     bCommand=TP_YKOPERATE;          //ң��ִ��                 
		     break;
          case 0xC0:
                     bCommand=TP_YKCANCEL;           //ң��ȡ��
		     break;
       
        }
 
//        SetYK(DevNo,bCommand,bINF,dwAttrib);
        send_write(DevNo, bCOT,bINF,bDCC,bRII); 
	
       return true;
}

//===================================================================
//  ��������: Rxd_0a_Deal()
//  ��������: ͨ�÷������ݲ�ѯ�Ļظ�
//  �������:                         
//  ��   ��  ֵ : ��
//  ˵��           : �ظ�����ң�ŵ��ң�����Ϣ
//===================================================================
/*void Rxd_0a_Deal(uint8_t* rBuf,uint8_t* sBuf,WORD wLen)
{
	uint8_t cot,bAttrib;//����ԭ��
	uint8_t inf;//��Ϣ���
	uint8_t rii;//���ر�ʾ
	uint8_t ginh,ginl,bCommand;
	DWORD dwAttrib;
	
 
	inf=rBuf[11];                         //��Ϣ���
	rii=rBuf[12];                         //������Ϣ��ʶ��
//	ginh=0x01;                            //����һ��ң��
	ginl=rBuf[15];                        //ң�غ�
	bAttrib=rBuf[wLen-3];                 //ң������  

	switch(bAttrib)
	{
	  case 0x02: dwAttrib=TP_CLOSE; break; //��
	  case 0x01: dwAttrib=TP_OPEN; break; //��
	}
			
	switch(inf) 
	{
		case 249:                           //ң��ѡ�� 
			   cot=0x2c;
			   bCommand=TP_YKSELECT;
			   break;			   
		case 250:                           //ң��ִ��
			   cot=0x28;
			   bCommand=TP_YKOPERATE;                           
			   break;			   			   
		case 251:                           //ң��ȡ��
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
	pBuf[m_Txd.WritePtr++] = 0x08;                      //������
	pBuf[m_Txd.WritePtr++] = wAddr;                     //��·��ַ��
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
	pBuf[m_Txd.WritePtr++] = 0x08;                      //������
	pBuf[m_Txd.WritePtr++] = wAddr;                     //��·��ַ��
	pBuf[m_Txd.WritePtr++] = 0x0A;                      //TYP
	pBuf[m_Txd.WritePtr++] = 0x81;                      //VSQ
    pBuf[m_Txd.WritePtr++] = cot;                     //COT
	pBuf[m_Txd.WritePtr++] = wAddr;
    pBuf[m_Txd.WritePtr++] = 0xFE;                    //FUN
    pBuf[m_Txd.WritePtr++] = inf;                     //INF
    pBuf[m_Txd.WritePtr++] = rii;                     //RII
    pBuf[m_Txd.WritePtr++] = 0x01;                    //NOG
    pBuf[m_Txd.WritePtr++] = 0x01;                    //GINH=0x01,����һ��ң��
    pBuf[m_Txd.WritePtr++] = ginl;                    //ң�غ�
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
//		���ݷ��Ͳ���
//
//===================================================================
//  ��������: TxdMonitor        
//  ��������: ���ݷ�����Ϣ
//  �������:                         
//  ����ֵ����                        
//===================================================================
static int TxdMonitor(void)
{
//	if(TxdOnTimer())
//		return true;
	
	return true;
}


//===================================================================
//  ��������: Check_Auto        
//  ��������: ����Ƿ���SOE�¼�
//  �������:                         
//  ����ֵ����                        
//===================================================================
int Check_Auto_SOE(int DevNo)
{
	bAutoSendSOE = (gpDevice[DevNo].ReadYXPtr==gVars.YxWritePtr)?FALSE:TRUE;
	bAutoSendSOE1 = (gpDevice[DevNo].ReadYXSoePtr==gVars.YxSoeWritePtr)?FALSE:TRUE;
	
	return RET_SUCESS;
	
}

//===================================================================
//  ��������: Check_Auto_AI    
//  ��������: ����Ƿ���ң��Խ���¼�
//  �������:                         
//  ����ֵ����                        
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
//  ��������: Txd_Auto_YX_40()        
//  ��������: ��Ҫ�������͵ı���
//  �������:  
//  ����ֵ����                        
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
	pBuf[m_Txd.WritePtr++] = wAddr;                    //��·��ַ��
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
//  ��������: CSCOM103::Txd_Auto_SOE_41(uint8_t CTR)        
//  ��������: ��Ҫ�������͵ı���
//  �������:  
//  ����ֵ����                        
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
	pBuf[m_Txd.WritePtr++] = wAddr;                    //��·��ַ��
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
//  ��������: Txd_Auto_AI 
//  ��������: ��������ң��Խ���¼�	(ң��Խ���¼���ʱ������)
//  �������:                         
//  ����ֵ��
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
		pBuf[m_Txd.WritePtr++] = wAddr;                    //��·��ַ��
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
//  ��������: CheckSum
//  ��������: ʵ���ۼӺ�У��
//  �������: uint8_t * pBuf, ���뻺����
//            uint8_t byLength, ����������
//  ����ֵ:   uint8_t bySum, �ۼӺ�
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
//  ��������: CS103::Set103_Reset_Control
//  ��������: ����ִ��
//  �������:  
//  ����ֵ����                        
//===================================================================
//extern void SignalReset();
void Set103_Reset_Control()
{	
	//SystemReset(0); //yxl 2010.10.26 ����澯�Ͷ����̵���
//	SignalReset();
}




