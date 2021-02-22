#ifndef __siec104_2002nr_H
#define __siec104_2002nr_H


#define RECV_CONTROL1_2002nr 	3		//接收报文控制字1的判断
#define STARTHEAD 		0x68
#define  STARTDT_ACT_2002nr 	0x7		//根据控制字判断
#define  STARTDT_CON_2002nr 	0x0B
#define  STOPDT_ACT_2002nr  	0x13
#define  STOPDT_CON_2002nr  	0x23
#define  TESTFR_ACT_2002nr  	0x43
#define  TESTFR_CON_2002nr  	0x83
#define  STARTHEAD_2002nr 		0x68
#define DEALSTATRT_2002nr  	12		//接收I报文数据第13字节,即信息体地址第1个
#define STARTDZ104_2002nr		0x5001	//定值信息体地址
#define DEALSTATRT  	12		//接收I报文数据第13字节,即信息体地址第1个
#define STARTDZ104		0x5001	//定值信息体地址

#define DZ_Clear	0
#define DZ_YU_ZHI	1
#define DZ_JI_HUO	2
#define DZ_STOP		3
#define DZ_TU_FA	1 
#define DZ_YZ_CONFIRM	2 //定值预置确认
#define TP_DZ_CALL_SEL      2    //定值召唤预置
#define TP_DZ_CALL_EXE      3    //定值召唤执行
#define 	BINFOADDR					0x3			//信息体地址长度

#define Type_101		1	
#define Type_104		2	
#define Type_clear		0	
#define BI_REVERT			0x40000000  //信号复归
#define STARTSYX104_2002nr		0x1		//单点遥信信息体地址

#define 	PBI_ON						1
#define 	PBI_OFF						0

#define BINFOADDR_2002		0x3		//信息体地址字节

#define Dz_DevID_3      3    //设备ID号3的设备用作定值缓存

#define Dev1_DZ_Start		10	//定值缓冲区起始地址
#define Dev2_DZ_Start		70	//定值缓冲区起始地址
#define Dev3_DZ_Start		130	//定值缓冲区起始地址
#define STARTYC104_2002nr		0x4001	//遥测信息体地址
#define GENGRP_YC_2002nr  		10		//分组召唤每组最多上传遥测个数


#pragma pack(1)  //按1字节对齐

//typedef struct{   //规约io 
//    unsigned short BufSize;		//缓冲区大小
//    unsigned short ReadPtr;		//读指针
//    unsigned short OldReadPtr;  //读指针
//    unsigned short WritePtr;	//写指针
//    WORD	Address;			//地址码(主要用于发送)
//    DWORD	Flag;				//控制码(主要用于发送)
//    uint8_t  Buf[255];		//缓冲区
//}TCommIO;

typedef struct
{
	DWORD DZValue;
	BYTE Flag;//1:定值预置     2:定值激活
}S104DZDATA_T;

typedef struct	//I104S规约的可变帧长结构  
{
    unsigned char Start; 		//启动字符
    unsigned char Length; 		//长度
    unsigned char Control1;		 //控制域1
    unsigned char Control2; 	//控制域2
    unsigned char Control3; 	//控制域3
    unsigned char Control4; 	//控制域4
    unsigned char Style;   		//类型标识，7
    unsigned char Definitive; 	//结构限定词
    unsigned char Reason_L; 	//传送原因
    unsigned char Reason_H; 	//传送原因
    unsigned char Address_L; 	//公共地址（子站站址）
    unsigned char Address_H; 	//公共地址（子站站址）
    unsigned char AddInfom1;  	//信息体地址，第13字节
    unsigned char AddInfom2;
    unsigned char AddInfom3;
    unsigned char Data1;
    unsigned char Data2;
    
}APDU104_T;


typedef struct
{
	WORD addr;
	DWORD DZValue;
}S104DZVALUE_T;


typedef struct 
{
	BYTE Dev1;
	BYTE Dev2;
	BYTE Dev3;
	
}DZRETURNADDR_T;



//typedef union{
//	float  _float;
//	uint32_t _int32;
//}Data_Value_T, *PData_Value_T;


typedef struct 
{

    unsigned char Start; 		//启动字符
    unsigned char Length; 		//长度
    unsigned char Control1;		//控制域1
    unsigned char Control2; 	//控制域2
    unsigned char Control3; 	//控制域3
    unsigned char Control4; 	//控制域4
    unsigned char Style;   		//类型标识，7
    unsigned char Definitive; 	//结构限定词
    unsigned char Reason_L; 	//传送原因
    unsigned char Reason_H; 	//传送原因
    unsigned char Address_L; 	//公共地址（子站站址）
    unsigned char Address_H; 	//公共地址（子站站址）
    unsigned char AddInfom1;  	//信息体地址，第13字节
    unsigned char AddInfom2;
    unsigned char AddInfom3;
    unsigned char Data1;
    unsigned char Data2;
      
}TS104_2002nrRxdFm,TS104_2002nrTxdFm;			 //I104_2002nrS规约的可变帧长结构  

int 	Init_siec104_2002nr(int DevID);


#pragma pack()	//解除按1字节对齐

#endif


