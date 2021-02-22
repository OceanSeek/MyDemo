#ifndef __miec104gx_H
#define __miec104gx_H


//�жϷ�����վ����
#define I_FORMAT		0
#define S_FORMAT 		1
#define U_FORMAT 		3

#define  STARTDT_ACT 	0x7		//���ݿ������ж�
#define  STARTDT_CON 	0x0B
#define  STOPDT_ACT  	0x13
#define  STOPDT_CON  	0x23
#define  TESTFR_ACT  	0x43
#define  TESTFR_CON  	0x83
#define IEC104_k		12	 	//����������12֡I���ĵȴ�����
#define IEC104_t1		30		//���ͻ����APDU�����ȴ�ʱ��,��λ�룬�����ȴ�ȷ��3����
#define  STARTDT_act 	0x7
#define  STARTDT_CON 	0x0B
#define  STOPDT_ACT  	0x13
#define  STOPDT_CON  	0x23
#define  TESTFR_ACT  	0x43
#define  TESTFR_CON  	0x83
#define  Rtu_104  		1

#define Dev1_DZ_Start		10	//��ֵ��������ʼ��ַ
#define Dev2_DZ_Start		70	//��ֵ��������ʼ��ַ
#define Dev3_DZ_Start		130	//��ֵ��������ʼ��ַ
#define DZ_YZ_CONFIRM		2 //��ֵԤ��ȷ��
#define TP_DZ_CALL_SEL      2    //��ֵ�ٻ�Ԥ��
#define TP_DZ_CALL_EXE      3    //��ֵ�ٻ�ִ��

#define 	BINFOADDR					0x3			//��Ϣ���ַ����

#define Type_101		1	
#define Type_104		2	
#define Type_clear		0	
#define STARTSYX104		0x1		//����ң����Ϣ���ַ
#define STARTYC104		0x4001	//ң����Ϣ���ַ
#pragma pack(1)

//typedef union{
//	float  _float;
//	uint32_t _int32;
//}Data_Value_T, *PData_Value_T;



//typedef struct{   //��Լio 
//    unsigned short BufSize;		//��������С
//    unsigned short ReadPtr;		//��ָ��
//    unsigned short OldReadPtr;  //��ָ��
//    unsigned short WritePtr;	//дָ��
//    WORD	Address;			//��ַ��(��Ҫ���ڷ���)
//    DWORD	Flag;				//������(��Ҫ���ڷ���)
//    uint8_t  Buf[255];		//������
//}TCommIO;



int Init_miec104gx(int DevID);
BOOL GetDevFromAddr( DWORD dwDevAddr, DWORD *dwDevID);


#pragma pack()

#endif

