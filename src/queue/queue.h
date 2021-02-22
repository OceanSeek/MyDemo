#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef int Elementype;        //    ������������


//typedef struct{
//
//    int _milliseconds;
//    char _min;
//    char _hour;
//    char _day;
//    char _month;
//    char _year;
//
//}CP56Time2a_T, *PCP56Time2a_T;

//typedef struct {
//	int  nNo;			//���
//	int  bStatus;		//״̬ 0:�ɺ�����   ��0���ɷ�����
//	char  bType;        //0x0A-��  0x0B-������� 0x0C-��ѹ 0x0D-Ƶ�� 0x0E-����
//	char  bValue0;
//	char  bValue1;
//	char  bValue2;
//	char  bValue3;
//	CP56Time2a_T stTime;    	//�ꡢ�¡��ա�ʱ���֡��롢����
//}TSysTimeSoe,*PTSysTimeSoe;

//    ����ڵ�ṹ
typedef struct Node {
    Elementype Element;        //    ������
    TSysTimeSoe YxSoe;
    struct Node * Next;
}NODE, *PNODE;

//    ������нṹ��
typedef struct QNode {
    PNODE Head, Tail;        //    ����ͷ��βָ��
} Queue, *PQueue;

void InitQueue(PQueue);    //    �������к���
bool IsEmptyQueue(PQueue);    //    �ж϶����Ƿ�Ϊ�պ���
void InsertQueue(PQueue queue,int val,TSysTimeSoe _YxSoe);    //    ��Ӻ���
void DeleteQueue(PQueue queue,int* val,TSysTimeSoe *Yxsoe);    //    ���Ӻ���
void DestroyQueue(PQueue);    //    �ݻٶ��к���
void TraverseQueue(PQueue);    //    �������к���
void ClearQueue(PQueue);    //    ��ն��к���
int LengthQueue(PQueue);    //    ����г��Ⱥ���



extern Queue YxSoeQueue;	 	//	����YxSoe���б���
extern Queue YxQueue;	  		//	����Yx���б���
extern Queue YcSoeQueue;	  	//	����YcSoe���б���

#endif

