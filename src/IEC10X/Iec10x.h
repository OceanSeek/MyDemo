/*******************************************************************
Copyright (C):    
File name    :    Iec10x.h
DESCRIPTION  :
AUTHOR       :
Version      :    1.0
Date         :    2014/07/23
Others       :
History      :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) Date:          Author: 
   content:
            

*******************************************************************/

#ifndef _IEC10X_H
#define _IEC10X_H
//#define IEC101_STM32


//#include "common_datatype.h"
//#include "PRIO_QUEUE_Iec10x.h"

#define     STARTSYX          			1
#define     STARTSYC          			0x4001
#define     STARTSDZ          			0x5001
#define     STARTSYK          			0x6001
#define     wAllSYxNum          		128			//单台设备遥信数量,应更改读取数据库中的参数
#define     wAllSYcNum          		128			//单台设备遥c测数量,应更改读取数据库中的参数
#define     wAllSDzNum          		128			//单台设备定值数量,应更改读取数据库中的参数


#define     Flag_Spon          		1			//遥信突发标志
#define     Flag_CallAll          	2			//总召唤标志
#define     Flag_Spon_Clear       	0			//遥信突发标志清零
#define     Flag_Clear       		0			//标志清零
#define     Flag_DZ_YZ          	3			//定值预置标志位
#define     Flag_DZ_RET          	4			//定值预置返回标志位
#define     Flag_DZ_REFUSE          5			//定值预置拒绝返回标志位

#define     ACTIVATE          			1			//激活
#define     LinkReady          			1			//链路已建立
#define     LinkOverTime         		2			//超时
#define     LinkDisconect          		0			//断开链接

#define 	LINK_COLSE				0
#define 	LINK_SUCESS				1


#define     ENABLE          		1			
#define     DISABLE          		0			

#define     MAX_SLAVE_DEVICE_NUM		6			//最大子站数

#define 	MAXYXRECORD				1000		//最大遥信记录
#define 	MAXYCRECORD				255			//最大遥测记录

#define 	YKOPEN					1			//遥控分
#define 	YKCLOSE					2			//遥控合

#define 	YKWAIT					0			//遥控等待
#define 	YKACKTIVE				1			//遥控激活
#define 	YKREVOCATION			2			//遥控撤销

#define 	YKSELECT				1			//遥控选择
#define 	YKEXECUCE				0			//遥控执行
/*

 * YK return
 * */
#define 	YKACKCONFIRM			1			//激活确认
#define 	YKREFUSE				2			//拒绝激活
#define 	YKFINISH				3			//遥控结束
/*

 * CONFIGURE
 * */
//#define             IEC10XLOCK
#define             PRIO_QUEUE
#define             IEC10X_PRIO_MAX                 7
#define             IEC10X_HEADER_LENGTH            1

/*
 * PRIO
 * */
#define             IEC10X_PRIO_INITLINK            0
#define             IEC10X_PRIO_CALLALL             1
#define             IEC10X_PRIO_CALLGROUP           2
#define             IEC10X_PRIO_CLOCK               3
#define             IEC10X_PRIO_DELAY               3
#define             IEC10X_PRIO_PULSE               0
#define             IEC10X_PRIO_SPON                0

typedef union{
	float  _float;
	uint32_t _int32;
}Data_Int_To_Float_T, *PData_Int_To_Float_T;

typedef  struct {

	uint8_t _func:4;			   /*function*/
	uint8_t _dfc:1;
	uint8_t _acd:1;
	uint8_t _prm:1;
	uint8_t _dir:1;

}CTRL_UP_T;
typedef  struct{

	uint8_t _func:4;			   /* function */
	uint8_t _fcv:1;
	uint8_t _fcb:1; 			   /* Frame calculate bit */
	uint8_t _prm:1; 			   /* 1:from start station, 0:from end station */
	uint8_t _dir:1;

}CTRL_DOWN_T;


typedef  union
{
	CTRL_UP_T up;
	CTRL_DOWN_T down;
	uint8_t val;
} CTRL_T,*PCTRL_T;


/*
 *  Transmission  Priority Queue.
 */
typedef struct IEC10X_CallbackArg{
    uint8_t *value;
    uint32_t PicSn;
    uint32_t FramSerialNum;
}Iec10x_CallbackArg_T;

typedef struct IEC10X_NODE{
    uint16_t Length;
    struct IEC10X_NODE *Next;
    void(* CallBack)(Iec10x_CallbackArg_T *Arg);
    Iec10x_CallbackArg_T CallBackArg;
    uint8_t value[1];
}Iec10x_PrioNode_T;

typedef struct IEC10X_QUEUE{
    struct IEC10X_NODE *Header;
    struct IEC10X_NODE *Tail;
    unsigned char ElementNum;
}Iec10x_PrioQueue_T;


typedef struct{
    int32_t BuildSendSn;
    int32_t BuildRecvSn;
    int32_t DealSendSn;
    int32_t DealRecvSn;
    uint32_t TesterCount;
}SendRecvSn_T,*PSendRecvSn_T;


/*
*********************************************************************************************************
*                                           BLOCK SIZE AND NUMBER
*
* if add part, please change OS_MAX_MEM_PART in ucosii.h.
*********************************************************************************************************
*/

/* IEC10X_PARTITION 1   size==256*8 */
#define IEC10X_PARTITION_NUM_1       5
#define IEC10X_PARTITION_SIZE_1      512
#define IEC10X_PARTITION_1           (IEC10X_PARTITION_SIZE_1-16)

/* IEC10X_PARTITION 2   size==(1300+16)*1 */
#define IEC10X_PARTITION_NUM_2       20
#define IEC10X_PARTITION_2           128
#define IEC10X_PARTITION_SIZE_2      (IEC10X_PARTITION_2+sizeof(Iec10x_PrioNode_T))

/* IEC10X_PARTITION 1   size==64*20 */
#define IEC10X_PARTITION_NUM_3       10
#define IEC10X_PARTITION_3           32
#define IEC10X_PARTITION_SIZE_3      (IEC10X_PARTITION_3+sizeof(Iec10x_PrioNode_T))

/*
*********************************************************************************************************
*                                           ASDU
*********************************************************************************************************
*/

/*
 * ASDU TYPE
 * */
#define     Iec10x_M_EI_NA_1                0X46
#define     IEC10X_C_IC_NA_1                0X64        /*总召*/
#define     IEC10X_TYPE_DZ_CALL             0X6C        /*定值召唤*/
#define     IEC10X_TYPE_DZ_YZ             	0X70        /*定值预置*/
#define     IEC10X_TYPE_DZ_JH             	0X71        /*定值激活*/
#define     IEC10X_TYPE_DEVICE_ADDR_CALL    0X78        /*终端设备编码召唤*/
#define     IEC10X_TYPE_DEVICE_ADDR_DATA    0X79        /*终端设备编码召唤*/
#define     IEC10X_C_CS_NA_1                0x67		/*时钟*/
#define     IEC10X_C_RP_NA_1                0x69		/*复位进程*/
#define     IEC10X_C_CD_NA_1                0x6a         /**/
#define     IEC10X_TYPE_DZ_DATA             0x70         /*定值数据*/


#define     IEC10X_C_SC_NA_1                0x2D         
#define     IEC10X_C_DC_NA_1                0x2E         

/*Set data*/
#define     IEC10X_C_SE_NA_1                46
#define     IEC10X_C_SE_NC_1                50

/*
 * ASDU REASON
 * */
#define     IEC10X_ASDU_REASON_INIT         0X04
#define     IEC10X_ASDU_REASON_ACT          0X06
#define     IEC10X_ASDU_REASON_ACTCON       0X07        /*CALL*/
#define     IEC10X_ASDU_REASON_REFUSEACT	0X09        /**/
#define     IEC10X_ASDU_REASON_ACTFIN       0X0a        /*CALL FINISH*/
#define     IEC10X_ASDU_COT_UNKNOW      	45
#define     IEC10X_ASDU_REASON_REFUSE       0X47
#define     IEC10X_ASDU_REASON_REFUSE_FIN   0X4a


/*
* INFO ADDR
* */
#define     IEC10X_INFO_ADDR_NONE           0X00

/*
*********************************************************************************************************
*                           REMOTE SIGNAL
*********************************************************************************************************
*/
/*
 * Asdu type (TI)
 * */
#define     IEC10X_M_SP_NA_1                1//单点信息
#define     IEC10X_M_DP_NA_1                3
#define     IEC10X_M_SP_TB_1                30//带CP56Time2a时标的单点信息
#define     IEC10X_M_DP_TB_1                31
/*
 * Asdu reason (COT)
 * */
#define     IEC10X_COT_BACK                 2
#define     IEC10X_COT_SPONT                3
#define     IEC10X_COT_ACT                  6
#define     IEC10X_COT_ACTCON               7
#define     IEC10X_COT_STOPACT              8
#define     IEC10X_COT_STOPACTCONFIRM       9
#define     IEC10X_COT_ACTFIN               10
#define     IEC10X_COT_REQ                  5
#define     IEC10X_COT_REASON               10
#define     IEC10X_COT_RETREM               11
#define     IEC10X_COT_RETLOC               12
#define     IEC10X_COT_INTROGEN             20
#define     IEC10X_COT_INTRO1               21
#define     IEC10X_COT_INTRO2               22
#define     IEC10X_COT_GROUP16              0x24
#define     IEC10X_COT_REFUSE               0x47

/*
 * Asdu addr
 * */
#define     IEC10X_INFO_ADDR_SIG_BASE           0X0001
#define     IEC10X_INFO_ADDR_SIG_TEMP_HX_OFF    0x1000

/*
*********************************************************************************************************
*                           REMOTE DECTET
*********************************************************************************************************
*/
/*
 * Asdu type (TI)
 * */
#define     IEC10X_M_ME_NA_1                9	//归一化测量值
#define     IEC10X_M_TI_BD_1                11 	//标度化测量值
#define     IEC10X_M_ME_NC_1                13	//浮点型
#define     IEC10X_M_ME_TD_1                34
#define     IEC10X_M_ME_TF_1                36
#define     IEC10X_TYPE_DZ                0x70	//定值
/*
 * Asdu reason (COT)
 * */
#define     IEC10X_COT_PER_CYC               1
#define     IEC10X_COT_BACK                  2
#define     IEC10X_COT_SPONT                 3
#define     IEC10X_COT_REQ                   5
#define     IEC10X_COT_INTROGEN              20
#define     IEC10X_COT_INTRO9                29
#define     IEC10X_COT_INTRO10               30

/*
 * Asdu addr
 * */
#define     IEC10X_INFO_ADDR_DET             	0X4001
#define     IEC10X_DZ_ADDR		               	0X5001
#define     IEC10X_INFO_ADDR_DET_TEMP_HX_OFF  	0x1000

/*
*********************************************************************************************************
*                           CALL QOI
*********************************************************************************************************
*/

#define IEC10X_CALL_QOI_TOTAL               0x14
#define IEC10X_CALL_QOI_GROUP1              0x15
#define IEC10X_CALL_QOI_GROUP2              0x16
#define IEC10X_CALL_QOI_GROUP9              29
#define IEC10X_CALL_QOI_GROUP10             30
#define IEC10X_CALL_QOI_GROUP16             0x24
#define IEC10X_CALL_QOI_M_TOTAL             100

/*
*********************************************************************************************************
*                           INFO ADDREST        
*               TD -- temprature device
*               PD -- pullotion device
*********************************************************************************************************
*/
#define IEC10X_SET_TD_UPCYCLE               0X100001
#define IEC10X_SET_TD_MAXIMUM               0X100002
#define IEC10X_SET_TD_MINIMUM               0X100003
#define IEC10X_SET_TD_DETCYCLE              0X100004
#define IEC10X_SET_TD_TEMPRISE              0X100005
#define IEC10X_SET_PD_DETCYCLE              0X100006
#define IEC10X_SET_PD_PULPSE                0X100007
#define IEC10X_SET_PD_SELFDETCYCLE          0X100008
/*
*********************************************************************************************************
*                             
*********************************************************************************************************
*/
/* TI. */
#define     IEC10X_TI_FIRM_UPDATE              128
#define     IEC10X_TI_AP_FIRM_BACKOFF          129
#define     IEC10X_TI_AP_BASE_INFO             130
/* COT. */
#define     IEC10X_COT_ACT_TERMINAL            8
#define     IEC10X_COT_ACT_TERMINAL_ACK        9

#define     IEC10X_COT_DATA                    14
#define     IEC10X_COT_DATA_ACK                15
#define     IEC10X_COT_DATA_FIN                16
#define     IEC10X_COT_DATA_FIN_ACK            17
#define     IEC10X_COT_DATA_NEEDACK            0X100E

#define MAKELONG(low, high) ((uint32_t)(((uint16_t)(low)) | (((uint32_t)((uint16_t)(high))) << 16)))
#define MAKEDWORD(ll,lh,hl,hh) (MAKELONG(MAKEWORD(ll,lh),MAKEWORD(hl,hh)))
#define MAKEWORD(low, high) ((uint16_t)(((uint8_t)(low)) | (((uint16_t)((uint8_t)(high))) << 8)))

#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)((WORD)(w) >> 8))
#define LLBYTE(w)			((BYTE)(w))
#define LHBYTE(w)			((BYTE)((WORD)(w) >> 8))
#define HLBYTE(w)			((BYTE)((DWORD)(w) >> 16))
#define HHBYTE(w)			((BYTE)((DWORD)(w) >>24))


#pragma pack(1)

typedef union{
	float  _float;
	uint32_t _int32;
}Data_Value_T, *PData_Value_T;


/*
 *  asdu number
 * */
typedef struct {

    uint8_t _num:7;
    uint8_t _sq:1;

}ASDU_NUM_T;
/*
 *  asdu reason
 * */
typedef   struct {
    uint16_t _reason:14;
    uint16_t _pn:1;
    uint16_t _test:1;
}ASDU_REASON_T;

typedef   struct {
    uint8_t _reason:6;
    uint8_t _pn:1;
    uint8_t _test:1;
}ASDU_REASON_101T;

/*
 *  asdu
 * */
typedef   struct{

    uint8_t         _type;
    ASDU_NUM_T      _num;
    ASDU_REASON_T   _reason;
    uint16_t        _addr;
    uint8_t         _info[1];
}IEC10X_ASDU_T, *PIEC10X_ASDU_T;

typedef   struct{

    uint8_t         _type;	//类型标识TI
    ASDU_NUM_T      _num;	//可变结构限定词
    uint8_t   		_reason;	//传输原因
    uint16_t        _addr;		//公共地址
    uint8_t         _info[1];	//信息对象
}IEC10X_ASDU_101T, *PIEC10X_ASDU_101T;

/*
 *  asdu info
 * */
typedef   struct{
	
    uint8_t  _addr[3];
    uint8_t _element[1];
}ASDU_INFO_T, *PASDU_INFO_T;

typedef   struct{
	
    uint8_t  _addr[3];
    uint8_t  _value;
    uint8_t _element[1];
}ASDU_SOE_T, *PASDU_SOE_T;

typedef   struct{
	
    uint16_t  _addr;
    uint8_t  _value;
    uint8_t _element[1];
}ASDU_101SOE_T, *PASDU_101SOE_T;
typedef   struct{
	
    uint8_t  _addr[3];
    uint8_t  _value;
    uint8_t _element[1];
}ASDU_104SOE_T, *PASDU_104SOE_T;

typedef   struct {
    uint8_t _type:1;//0:execute 1:select
    uint8_t _backup:6;
    uint8_t _command:1;//0:oepn 1:close
}YK_DATA_T, *PYK_DATA_T;
typedef   struct {
    uint8_t _refusefinish:4;//0:wait
    uint8_t _refuse:1;//0:wait 1:refuse
    uint8_t _finish:1;//0:wait 1:finish
    uint8_t _ackconfirm:1;//0:wait 1:ackconfirm
    uint8_t _stopackconfirm:1;//0:wait 1:stopackconfirm
}YK_REATURN_T, *PYK_REATURN_T;


typedef   struct{
	uint8_t _flag;	//0:waite 1:ack 2:revocation
	YK_REATURN_T _Return;	
	YK_DATA_T _YKData;
	uint8_t _addr;
}YK_INFO_T, *PYK_INFO_T;

typedef   struct{
	
    uint16_t _addr;
    uint8_t _element[1];
}ASDU_INFO_101T, *PASDU_INFO_101T;

typedef   struct{
	
    uint16_t _addr;
    YK_DATA_T _Data8;//8位信息值
}ASDU_INFO_YK_T, *PASDU_INFO_YK_T;

typedef   struct{
	
    uint16_t _addr;
    uint8_t _Data8;//8位信息值
}ASDU_INFO_8T, *PASDU_INFO_8T;
typedef   struct{
	
    uint8_t _addr[3];
    uint8_t _Data8;//8位信息值
}ASDU_104INFO_8T, *PASDU_104INFO_8T;

typedef   struct{
	
    uint16_t _addr;
    uint16_t _Data16;//16位信息值
    uint8_t _qds;
}ASDU_INFO_16T, *PASDU_INFO_16T;


/*
 *  asdu info remote signal with time flag
 * */
typedef   struct{

    uint8_t _signal;
    uint8_t _time;
}SIGNAL_TIME_T, *PSIGNAL_TIME_T;


/*
 *  asdu info remote detect with time flag
 * */
/*int */
typedef   struct{

    uint16_t _detect;
    uint8_t _qds;

}IEC10X_DETECT_T, *PIEC10X_DETECT_T;

typedef   struct{

    uint32_t _detect;
    uint8_t _qds;

}IEC10X_DETECT_DW_T, *PIEC10X_DETECT_DW_T;


/*float*/
typedef   struct{

    float _detect;
    uint8_t _qds;

}IEC10X_DETECT_F_T, *PIEC10X_DETECT_F_T;


/*int  sq=0 */
typedef   struct{

    uint8_t  _addr[3];
    int16_t _detect;
    uint8_t _qds;

}IEC10X_DETECT_SQ0_T, *PIEC10X_DETECT_SQ0_T;

typedef   struct{

    uint16_t _addr;
    int16_t _detect;
    uint8_t _qds;

}IEC10X_DETECT_SQ0_101T, *PIEC10X_DETECT_SQ0_101T;

typedef   struct{

    uint8_t  _addr[3];
    float _detect;
    uint8_t _qds;

}IEC10X_DETECT_SQ0_F_T, *PIEC10X_DETECT_SQ0_F_T;

typedef   struct{

    uint16_t _addr;
    float _detect;
    uint8_t _qds;

}IEC10X_DETECT_SQ0_101F_T, *PIEC10X_DETECT_SQ0_101F_T;

typedef   struct{
	
    uint16_t _addr;
    uint32_t _Data32;//32位信息值
    uint8_t _qds;
}ASDU_INFO_32T, *PASDU_INFO_32T;


typedef   struct{

    uint8_t  _addr[3];
    uint32_t _detect;
    uint8_t _qds;

}GX_104_INFO_YC32_T, *PGX_104_INFO_YC32_T;

/*
 *  asdu info CP56Time2a
 * */
typedef struct{

    uint8_t _minutes:6;
    uint8_t _res:1;
    uint8_t _iv:1;
}IEC10X_Time_Min_T, *PIEC10X_Time_Min_T;

typedef struct{

    uint8_t _hours:5;
    uint8_t _res:2;
    uint8_t _su:1;
}IEC10X_Time_Hour_T, *PIEC10X_Time_Hour_T;
typedef struct{

    uint8_t _dayofmonth:5;
    uint8_t _dayofweek:3;
}IEC10X_Time_Day_T, *PIEC10X_Time_Day_T;
typedef struct{

    uint8_t _month:4;
    uint8_t _res:3;
}IEC10X_Time_Month_T, *PIEC10X_Time_Month_T;
typedef struct{

    uint8_t _year:7;
    uint8_t _res:1;
}IEC10X_Time_Year_T, *PIEC10X_Time_Year_T;
typedef struct{

    uint16_t _milliseconds;
    IEC10X_Time_Min_T _min;
    IEC10X_Time_Hour_T _hour;
    IEC10X_Time_Day_T _day;
    IEC10X_Time_Month_T _month;
    IEC10X_Time_Year_T _year;

}CP56Time2a_T, *PCP56Time2a_T;

/*
 *  asdu info remote detect with time flag
 * */
typedef   struct{

    uint16_t _detect;
    uint8_t _qds;
    CP56Time2a_T _time;
}DETECT_TIME_T, *PDETECT_TIME_T;


#pragma pack()

/*
 * Iec10x init
 */
typedef struct _iec10x {

    char * name;
    int (* Init)(void);
    void (* Delay_ms)(uint16_t);
    void (* CloseLink)(void);
    void *(* Malloc)(uint8_t NumByte);
    void (* Free)(void *buffer);
    uint8_t (* enqueue)(Iec10x_PrioQueue_T *QueueHdr, Iec10x_PrioNode_T *NewNode);
    Iec10x_PrioNode_T *(* dequeue)(Iec10x_PrioQueue_T * QueueHdr);
    Iec10x_PrioNode_T *(* FindQHead)(Iec10x_PrioQueue_T * QueueHdr);
    char (* GetPrio)(void);
    void (* InitQueue)(Iec10x_PrioQueue_T *PrioQueue);
    void (* ClearQueue)(Iec10x_PrioQueue_T * QueueHdr);
    int (* Send)(int socketfd,char *data,int len);
    uint32_t (* SetTime)(PCP56Time2a_T time);
    uint32_t (* GetTime)(PCP56Time2a_T time);
    int8_t (* GetStationState)(uint16_t *Addr,uint8_t n,uint8_t *MaxNum);
    float (* GetStaValue)(uint16_t Addr, uint8_t DevType);
    uint16_t (* GetLinkAddr)(void);
    uint16_t (* GetInfoNum)(uint16_t *InfoNum, uint8_t DevType);
    int8_t (* SetConfig)(long Value, uint32_t addr);
    int8_t (* SaveFirmware)(uint8_t FirmLen, uint8_t *buf,uint32_t FirmwareType, uint32_t Iec10x_Update_SeekAddr);
    int8_t (* CheckFirmware)(uint32_t FirmwareType, uint32_t TotalLen);
    int8_t (* UpdateFirmware)(uint32_t FirmwareType);
    int8_t (* BackoffFirmware)(uint32_t FirmwareType);

#ifdef IEC10XLOCK
    void (* LOCK)(void);
    void (* UNLOCK)(void);
#endif

} *PIEC10X_T, IEC10X_T;


/*
 * BIG ENDIAN
 * */
#ifdef BIG_ENDIAN
#define   n2hs(x)    (x)
#define   h2ns(x)    (x)

#define   n2hl(x)    (x)
#define   h2nl(x)    (x)
/* LITTLE ENDIAN */
#else
#define   n2hs(x)    ((((x)>>8) & 0xFF) | (((x) & 0xFF)<<8))
#define   h2ns(x)    ((((x)>>8) & 0xFF) | (((x) & 0xFF)<<8))

#define   n2hl(x)    ( (((x)>>24) & 0xFF) | (((x)>>8) & 0xFF00) | \
                        (((x) & 0xFF00)<<8) | (((x) & 0xFF)<<24)   \
                      )
#define   h2nl(x)    ( (((x)>>24) & 0xFF) | (((x)>>8) & 0xFF00) | \
                        (((x) & 0xFF00)<<8) | (((x) & 0xFF)<<24)   \
                      )
#endif  /* BIG_ENDIAN */

/*
*********************************************************************************************************
*                                           EXTERN VARIABLE
*********************************************************************************************************
*/
extern      PIEC10X_T           IEC10X;
extern      uint16_t            Iec10x_Sta_Addr;
extern      int32_t             Iec104_BuildSendSn;
extern      int32_t             Iec104_BuildRecvSn;

extern      uint16_t            IEC10X_Cp16time2a;
extern      uint16_t            IEC10X_Cp16time2a_V;
extern      CP56Time2a_T        IEC10X_Cp56time2a;
extern      uint8_t             Iec10x_FirmwareUpdateFlag;


/*
*********************************************************************************************************
*                                          QUEUE FUNCTION PROTOTYPES
*********************************************************************************************************
*/
/*
 * Prio
 * */
uint8_t             IEC10X_PrioEnQueue(Iec10x_PrioQueue_T *QueueHdr, Iec10x_PrioNode_T * new_p);
Iec10x_PrioNode_T   *IEC10X_PrioDeQueue(Iec10x_PrioQueue_T * QueueHdr);
char                IEC10X_HighestPrio(void);
void                IEC10X_PrioInitQueue(Iec10x_PrioQueue_T *PrioQueue);
void                IEC10X_Prio_ClearQueue(Iec10x_PrioQueue_T * QueueHdr);
Iec10x_PrioNode_T   *IEC10X_PrioFindQueueHead(Iec10x_PrioQueue_T * QueueHdr);
/*
 * Iec10x queue
 * */
void                IEC10X_InitQ(void);
void                IEC10X_ClearQ(void);
uint8_t             IEC10X_GetPrio(uint8_t State);
void                IEC10X_Enqueue(uint8_t *EnQBuf, uint16_t Length,uint8_t Prio,
                            void(* IEC10XCallBack)(Iec10x_CallbackArg_T *Arg),Iec10x_CallbackArg_T *CallbackArg);
void                Iec10x_Scheduled(int socketfd);
Iec10x_PrioNode_T   *IEC10X_Dequeue(void);
int32_t             RegisterIEC10XMoudle(void * _IEC10X);
void                IEC10X_ClearQ(void);

void Iec10x_Lock(void);
void Iec10x_UnLock(void);
void DumpHEX(uint8_t *buffer, uint32_t len);



#endif  /*_IEC10X_H*/
