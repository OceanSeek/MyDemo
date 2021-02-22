#ifndef _MXJ103_H
#define _MXJ103_H

#define XJ103_Slaver_MAX_BUF_LEN 255

//*********************************************************************************//
//    ���ƺ��� dwCommand�����궨��
//*********************************************************************************//
#define TP_YKSELECT    	1    //ң��Ԥ��
#define TP_YKOPERATE  	2    //ң��ִ��
#define TP_YKCANCEL  	3    //ң�س���
#define TP_YKDIRECT  	4    //ң��ֱ��ִ��

#define TP_OPEN      	2   //ң�ء�ͬ�ڿ�������Ϊ��
#define TP_CLOSE      	1   //ң�ء�ͬ�ڿ�������Ϊ��
#define TP_NULL      	0   //�����ֿ�������

#define	PBI_ON			1
#define PBI_OFF			0

#define MAX_SUBDEVNO 32
#define maxDevNum		255/*�������װ���� ����Ϊ255*/


//==== ������Ķ��� ====

//��������λ����
#define PRM_MASTER     1        //����վ
#define PRM_SLAVE      0        //�Ӷ�վ

//֡������Чλ�ĺ궨��
#define FCV_VALID      1        //֡����λ��Ч
#define FCV_INVALID    0        //֡����λ��Ч

//Ҫ�����λ����
#define ACD_HAVECLASS1 1        //��1���û�����
#define ACD_NOCLASS1   0        //��1���û�����

//����������λ����
#define DFC_OK         0        //����
#define DFC_OVERFLOW   1        //��������

//������Ķ���
//FC_S ��ʾ���ӷ���(��վ)
#define FC_S_ACK          0     //ȷ��
#define FC_S_BUSY         1     //��·æ
#define FC_S_DATA         8     //��������Ӧ
#define FC_S_NODATA       9     //���ٻ�����
#define FC_S_LINKOK       11    //��·����
#define FC_S_LINKERROR    14    //��·����δ����
#define FC_S_LINKUNFIN    15    //��·����δ���


//FC_M ��ʾ���Ʒ���(��վ)
#define FC_M_RESETCU      0     //��λͨ�ŵ�Ԫ
#define FC_M_DATA         3     //��������,Ҫ��ȷ��
#define FC_M_BROADCAST    4     //��������,��Ӧ��
#define FC_M_RESETFCB     7     //��λ֡����λ
#define FC_M_CALLLINK     9     //�ٻ���·״̬
#define FC_M_CALLCLASS1   10    //�ٻ�1������
#define FC_M_CALLCLASS2   11    //�ٻ�2������

//==== Ӧ�÷������ݵ�Ԫ���� ====

//���ͱ�ʶ�Ķ���,1-31Ϊ���ݷ�Χ,32-255Ϊר�÷�Χ
//TYP_S ��ʾ���ӷ���(��վ)
#define TYP_S_TM       1        //��ʱ��ı���
#define TYP_S_TMR      2        //�����ʱ��ı���
#define TYP_S_ME1      3        //����ֵ��
#define TYP_S_TME      4        //�����ʱ��ı���ֵ
#define TYP_S_IDENT    5        //��ʶ
#define TYP_S_SST      6        //ʱ��ͬ��
#define TYP_S_CALLSTOP 8        //���ٻ���ֹ
#define TYP_S_ME2      9        //����ֵ��
#define TYP_S_GDATA    10       //ͨ�÷�������
#define TYP_S_GIDENT   11       //ͨ�÷����ʶ
///////wsnrrcs9612
#define TYP_S_ALLYC    50        //0x32ȫң������
#define TYP_S_ALLYX    44        //0x2cȫң������
#define TYP_S_YKFX     64        //0x40ң������
#define TYP_S_BWYX_NOTIME     40        //0x28��λң�Ų���ʱ��
#define TYP_S_BWYX_TIME       41        //0x29��λң�Ŵ�ʱ��
#define TYP_S_DISTURB_DATA    23        //0x17�Ŷ�����
//////wsnrrcs9621
//ZYM
#define TYP_S_ALLDD		36			//�����
#define TYP_S_YBBW_TIME 45			//ѹ���λ

//TYP_M ��ʾ���Ʒ���(��վ),
#define TYP_M_SST      6        //ʱ��ͬ��
#define TYP_M_CALL     7        //���ٻ�
#define TYP_M_GDATA    10       //ͨ�÷�������
#define TYP_M_COMMAND  20       //һ������
#define TYP_M_GCOMMAND 21       //ͨ������
//////wsnr
#define TYP_M_YK       64       //0x40ң������
#define TYP_M_FUGUI    20       //0x14��������
/////wsnr

//����ԭ��,1-63Ϊ���ݷ�Χ,64-255Ϊר�÷�Χ
//COT_S ��ʾ���ӷ���(��վ)
#define COT_S_BURST    1        //ͻ��
#define COT_S_CYCLE    2        //ѭ��
#define COT_S_RESETFCB 3        //��λ֡����λ
#define COT_S_RESETCU  4        //��λͨ�ŵ�Ԫ
#define COT_S_TEST     7        //����ģʽ
#define COT_S_SST      8        //ʱ��ͬ��
#define COT_S_CALL     9        //���ٻ�
#define COT_S_CALLSTOP 10       //���ٻ���ֹ
#define COT_S_LOCAL    11       //���ز���
#define COT_S_REMOTE   12       //Զ������
#define COT_S_ACK      20       //����Ŀ϶��Ͽ�
#define COT_S_NAK      21       //����ķ��Ͽ�
#define COT_S_GWACK    40       //ͨ�÷���д����Ŀ϶��Ͽ�
#define COT_S_GWNAK    41       //ͨ�÷���д����ķ��Ͽ�
#define COT_S_GRACK    42       //ͨ�÷�����������Ч������Ӧ
#define COT_S_GRNAK    43       //ͨ�÷�����������Ч������Ӧ
#define COT_S_GWANS    44       //ͨ�÷���дȷ��

//COT_M ��ʾ���Ʒ���(��վ)
#define COT_M_SST      8        //ʱ��ͬ��
#define COT_M_CALL     9        //���ٻ�������
#define COT_M_COMMAND  20       //һ������
#define COT_M_GWRITE   40       //ͨ�÷���д����
#define COT_M_GREAD    42       //ͨ�÷��������
/////wsnrrcs9621rcs9612
#define COT_M_REMOTE   12       //Զ������

//==== �����־���� ====
#define SF_LINKSTATUS_ERROR 0   //��·״̬��־

#define SF_CALLLINK         1   //�ٻ���·״̬
#define SF_RESETCU          2   //��λͨ�ŵ�Ԫ
#define SF_RESETFCB         3   //��λ֡����λ

#define SF_CALLYC           7
#define SF_CALLALL          8   //���ٻ�
#define SF_DATA             9   //��������,Ҫ��ȷ��
#define SF_BRAODCAST        10  //��������,��Ӧ��
#define SF_CALLCLASS1       11  //�ٻ�1������
#define SF_CALLCLASS2       12  //�ٻ�2������
#define SF_SST              13  //��ʱ

//ͨ�÷������ݵ������־
#define SF_GEN_RGROUPITEM   16  //�����б�������ı���
#define SF_GEN_RGROUPVALUE  17  //��һ�����ȫ����Ŀ��ֵ������
#define SF_GEN_RENTRYITEM   18  //��������Ŀ��Ŀ¼
#define SF_GEN_RENTRYVALUE  19  //��������Ŀ��ֵ������
#define SF_GEN_CALLALL      20  //��ͨ�÷������ݵ��ܲ�ѯ
#define SF_GEN_WENTRY       21  //д��Ŀ
#define SF_GEN_WENTRYACK    22  //��ȷ�ϵ�д��Ŀ
#define SF_GEN_WENTRYEXEC   23  //��ִ�е�д��Ŀ
#define SF_GEN_WENTRYCANCEL 24  //д��Ŀ��ֹ

//֡ͬ���ַ��Ķ���
#define SYN_START_10   0x10     //�̶�֡�������ַ�
#define SYN_START_68   0x68     //�ɱ�֡�������ַ�
#define SYN_STOP       0x16     //֡�����ַ�

//��������
#define FUN_BU         1        //�����Ԫ
#define FUN_TZ         128      //���뱣��
#define FUN_I          160      //��������
#define FUN_IT         176      //��ѹ�������
#define FUN_IL         192      //��·�����
#define FUN_GEN        254      //ͨ�÷���
#define FUN_GLB        255      //ȫ��


//lgh---
#define TP_DZ_CALL_QH        1    //��ֵ�����ٻ�
#define TP_DZ_CALL_SEL      2    //��ֵ�ٻ�Ԥ��
#define TP_DZ_CALL_EXE      3    //��ֵ�ٻ�ִ��
#define TP_DZ_HAVE_DATA   4    //�����ٻ���ֵһ������
#define TP_DZ_MODI_QH       5     //��ֵ�����·�
#define TP_DZ_MODI_SEL      6    //��ֵ�޸�Ԥ��
#define TP_DZ_MODI_EXE      7    //��ֵ�޸�ִ��


//ZYM B600 ��ֵ�ٻ�
#define CALL_DZ_FLAG0    0//����
#define CALL_DZ_FLAG1    1//����
#define CALL_DZ_FLAG2    2//�ٻ����
#define CALL_DZ_ERROR    3//�ٻ�����

#define DZ_NULL         0
#define DZ_SELECT	1
#define DZ_OPERATE      2

#define DZ_CHG_NULL     0
#define DZ_CHG_PREPARE  1
#define DZ_CHG_OPERATE  2

//240-255,ͨ�÷��๦��
#define INF_M_GEN_RGROUPITEM   240   //�����б�������ı���
#define INF_M_GEN_RGROUPVALUE  241   //��һ�����ȫ����Ŀ��ֵ������
#define INF_M_GEN_RENTRYITEM   243   //��������Ŀ��Ŀ¼
#define INF_M_GEN_RENTRYVALUE  244   //��������Ŀ��ֵ������
#define INF_M_GEN_CALLALL      245   //��ͨ�÷������ݵ��ܲ�ѯ
#define INF_M_GEN_WENTRY       248   //д��Ŀ
#define INF_M_GEN_WENTRYACK    249   //��ȷ�ϵ�д��Ŀ
#define INF_M_GEN_WENTRYEXEC   250   //��ִ�е�д��Ŀ
#define INF_M_GEN_WENTRYCANCEL 251   //д��Ŀ��ֹ



//�㲥��ַ�Ķ���
#define MASTER_BROADCAST_ADDRESS  0xFF      //��վ�Դ�վ�Ĺ㲥



//Ӧ�÷������ݵ�Ԫ�ṹ
typedef struct
{
	BYTE byTYP;                 //���ͱ�ʶ
	BYTE byVSQ;                 //�ɱ�ṹ�޶���
	BYTE byCOT;                 //����ԭ��
	BYTE byADDRESS;             //Ӧ�÷���Ԫ��ַ
	BYTE byFUN;                 //��������
	BYTE byINF;                 //��Ϣ���
	BYTE byData;                //������
}TAsdu;


typedef struct
{
	BYTE RESERVED:1;
	BYTE PRM:1;
	BYTE FCB:1;
	BYTE FCV:1;
	BYTE FC:4;
}TTxdControl;


//���崦��ʱ��Ҫ�洢����Ϣ�ṹ
typedef struct
{
	BYTE byCurrentFCB;          //ÿ����ȷ���պ󱣴��FCBλ
	BYTE byScanNumber;          //ɨ�����
	BYTE byCOL;                 //���ݼ���
}TInfo;

//�̶���������֡ͷ�ṹ(����)
typedef struct
{
	BYTE byStart;               //�����ַ�
	TTxdControl Control;        //������
	BYTE byAddress;             //��ַ��
	BYTE byChecksum;            //֡У���
	BYTE byStop;                //�����ַ�
}TTxdFrameFix;

//�ɱ䳤������֡ͷ�ṹ(����)
typedef struct
{
	BYTE byStart1;              //�����ַ�
	BYTE byLength1;             //����
	BYTE byLength2;             //����(�ظ�)
	BYTE byStart2;              //�����ַ�(�ظ�)
	TTxdControl Control;        //������
	BYTE byAddress;             //��ַ��
	TAsdu Asdu;                 //Ӧ�÷������ݵ�Ԫ
}TTxdFrameVar;

//����֡ͷ�ṹ
typedef union
{
	TTxdFrameFix FrameFix_10;
	TTxdFrameVar FrameVar_68;
}TTxdFrameHead;

//����֡ͷ�ṹ
typedef union
{
	TTxdFrameFix FrameFix_10;
	TTxdFrameVar FrameVar_68;
}TRxdFrameHead;


//�������ݽṹ
typedef struct
{
	BYTE Number:7;              //��Ŀ
	BYTE Cont:1;                //����״̬λ
}TNext;


//ͨ�÷����ʶ��Žṹ����
typedef struct
{
	BYTE byGroup;               //��
	BYTE byEntry;               //��Ŀ
}TGin;

//ͨ�÷������������ṹ
typedef struct
{
	BYTE byDataType;            //��������
	BYTE byDataSize;            //���ݿ��
	TNext Next;                 //��������
}TGdd;


//ͨ�÷������ݽṹ
typedef struct
{
	TGin Gin;                   //ͨ�÷����ʶ���
	BYTE byKod;                 //���������
	TGdd Gdd;                   //ͨ�÷�����������
	BYTE byGdi;                 //ͨ�÷����ʶ����
}TGenData;



int Init_XJ103Master(int DevID);

#endif

