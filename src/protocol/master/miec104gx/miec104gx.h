#ifndef __miec104gx_H
#define __miec104gx_H


//判断分析子站报文
#define I_FORMAT		0
#define S_FORMAT 		1
#define U_FORMAT 		3

#define  STARTDT_ACT 	0x7		//根据控制字判断
#define  STARTDT_CON 	0x0B
#define  STOPDT_ACT  	0x13
#define  STOPDT_CON  	0x23
#define  TESTFR_ACT  	0x43
#define  TESTFR_CON  	0x83
#define IEC104_k		12	 	//可以允许有12帧I报文等待处理
#define IEC104_t1		30		//发送或测试APDU后最大等待时间,单位秒，包括等待确认3分钟
#define  STARTDT_act 	0x7
#define  STARTDT_CON 	0x0B
#define  STOPDT_ACT  	0x13
#define  STOPDT_CON  	0x23
#define  TESTFR_ACT  	0x43
#define  TESTFR_CON  	0x83
#define  Rtu_104  		1

#define Dev1_DZ_Start		10	//定值缓冲区起始地址
#define Dev2_DZ_Start		70	//定值缓冲区起始地址
#define Dev3_DZ_Start		130	//定值缓冲区起始地址
#define DZ_YZ_CONFIRM		2 //定值预置确认
#define TP_DZ_CALL_SEL      2    //定值召唤预置
#define TP_DZ_CALL_EXE      3    //定值召唤执行

#define 	BINFOADDR					0x3			//信息体地址长度

#define Type_101		1	
#define Type_104		2	
#define Type_clear		0	
#define STARTSYX104		0x1		//单点遥信信息体地址
#define STARTYC104		0x4001	//遥测信息体地址
#pragma pack(1)

//typedef union{
//	float  _float;
//	uint32_t _int32;
//}Data_Value_T, *PData_Value_T;



//typedef struct{   //规约io 
//    unsigned short BufSize;		//缓冲区大小
//    unsigned short ReadPtr;		//读指针
//    unsigned short OldReadPtr;  //读指针
//    unsigned short WritePtr;	//写指针
//    WORD	Address;			//地址码(主要用于发送)
//    DWORD	Flag;				//控制码(主要用于发送)
//    uint8_t  Buf[255];		//缓冲区
//}TCommIO;



int Init_miec104gx(int DevID);
BOOL GetDevFromAddr( DWORD dwDevAddr, DWORD *dwDevID);


#pragma pack()

#endif

