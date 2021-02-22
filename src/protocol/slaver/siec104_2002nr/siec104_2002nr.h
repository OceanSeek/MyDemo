#ifndef __siec104_2002nr_H
#define __siec104_2002nr_H


#define RECV_CONTROL1_2002nr 	3		//���ձ��Ŀ�����1���ж�
#define STARTHEAD 		0x68
#define  STARTDT_ACT_2002nr 	0x7		//���ݿ������ж�
#define  STARTDT_CON_2002nr 	0x0B
#define  STOPDT_ACT_2002nr  	0x13
#define  STOPDT_CON_2002nr  	0x23
#define  TESTFR_ACT_2002nr  	0x43
#define  TESTFR_CON_2002nr  	0x83
#define  STARTHEAD_2002nr 		0x68
#define DEALSTATRT_2002nr  	12		//����I�������ݵ�13�ֽ�,����Ϣ���ַ��1��
#define STARTDZ104_2002nr		0x5001	//��ֵ��Ϣ���ַ
#define DEALSTATRT  	12		//����I�������ݵ�13�ֽ�,����Ϣ���ַ��1��
#define STARTDZ104		0x5001	//��ֵ��Ϣ���ַ

#define DZ_Clear	0
#define DZ_YU_ZHI	1
#define DZ_JI_HUO	2
#define DZ_STOP		3
#define DZ_TU_FA	1 
#define DZ_YZ_CONFIRM	2 //��ֵԤ��ȷ��
#define TP_DZ_CALL_SEL      2    //��ֵ�ٻ�Ԥ��
#define TP_DZ_CALL_EXE      3    //��ֵ�ٻ�ִ��
#define 	BINFOADDR					0x3			//��Ϣ���ַ����

#define Type_101		1	
#define Type_104		2	
#define Type_clear		0	
#define BI_REVERT			0x40000000  //�źŸ���
#define STARTSYX104_2002nr		0x1		//����ң����Ϣ���ַ

#define 	PBI_ON						1
#define 	PBI_OFF						0

#define BINFOADDR_2002		0x3		//��Ϣ���ַ�ֽ�

#define Dz_DevID_3      3    //�豸ID��3���豸������ֵ����

#define Dev1_DZ_Start		10	//��ֵ��������ʼ��ַ
#define Dev2_DZ_Start		70	//��ֵ��������ʼ��ַ
#define Dev3_DZ_Start		130	//��ֵ��������ʼ��ַ
#define STARTYC104_2002nr		0x4001	//ң����Ϣ���ַ
#define GENGRP_YC_2002nr  		10		//�����ٻ�ÿ������ϴ�ң�����


#pragma pack(1)  //��1�ֽڶ���

//typedef struct{   //��Լio 
//    unsigned short BufSize;		//��������С
//    unsigned short ReadPtr;		//��ָ��
//    unsigned short OldReadPtr;  //��ָ��
//    unsigned short WritePtr;	//дָ��
//    WORD	Address;			//��ַ��(��Ҫ���ڷ���)
//    DWORD	Flag;				//������(��Ҫ���ڷ���)
//    uint8_t  Buf[255];		//������
//}TCommIO;

typedef struct
{
	DWORD DZValue;
	BYTE Flag;//1:��ֵԤ��     2:��ֵ����
}S104DZDATA_T;

typedef struct	//I104S��Լ�Ŀɱ�֡���ṹ  
{
    unsigned char Start; 		//�����ַ�
    unsigned char Length; 		//����
    unsigned char Control1;		 //������1
    unsigned char Control2; 	//������2
    unsigned char Control3; 	//������3
    unsigned char Control4; 	//������4
    unsigned char Style;   		//���ͱ�ʶ��7
    unsigned char Definitive; 	//�ṹ�޶���
    unsigned char Reason_L; 	//����ԭ��
    unsigned char Reason_H; 	//����ԭ��
    unsigned char Address_L; 	//������ַ����վվַ��
    unsigned char Address_H; 	//������ַ����վվַ��
    unsigned char AddInfom1;  	//��Ϣ���ַ����13�ֽ�
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

    unsigned char Start; 		//�����ַ�
    unsigned char Length; 		//����
    unsigned char Control1;		//������1
    unsigned char Control2; 	//������2
    unsigned char Control3; 	//������3
    unsigned char Control4; 	//������4
    unsigned char Style;   		//���ͱ�ʶ��7
    unsigned char Definitive; 	//�ṹ�޶���
    unsigned char Reason_L; 	//����ԭ��
    unsigned char Reason_H; 	//����ԭ��
    unsigned char Address_L; 	//������ַ����վվַ��
    unsigned char Address_H; 	//������ַ����վվַ��
    unsigned char AddInfom1;  	//��Ϣ���ַ����13�ֽ�
    unsigned char AddInfom2;
    unsigned char AddInfom3;
    unsigned char Data1;
    unsigned char Data2;
      
}TS104_2002nrRxdFm,TS104_2002nrTxdFm;			 //I104_2002nrS��Լ�Ŀɱ�֡���ṹ  

int 	Init_siec104_2002nr(int DevID);


#pragma pack()	//�����1�ֽڶ���

#endif


