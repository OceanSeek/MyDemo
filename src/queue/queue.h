#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef int Elementype;        //    定义数据类型


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
//	int  nNo;			//点号
//	int  bStatus;		//状态 0:由合至分   非0：由分至合
//	char  bType;        //0x0A-无  0x0B-三相电流 0x0C-电压 0x0D-频率 0x0E-零序
//	char  bValue0;
//	char  bValue1;
//	char  bValue2;
//	char  bValue3;
//	CP56Time2a_T stTime;    	//年、月、日、时、分、秒、毫秒
//}TSysTimeSoe,*PTSysTimeSoe;

//    定义节点结构
typedef struct Node {
    Elementype Element;        //    数据域
    TSysTimeSoe YxSoe;
    struct Node * Next;
}NODE, *PNODE;

//    定义队列结构体
typedef struct QNode {
    PNODE Head, Tail;        //    队列头，尾指针
} Queue, *PQueue;

void InitQueue(PQueue);    //    创建队列函数
bool IsEmptyQueue(PQueue);    //    判断队列是否为空函数
void InsertQueue(PQueue queue,int val,TSysTimeSoe _YxSoe);    //    入队函数
void DeleteQueue(PQueue queue,int* val,TSysTimeSoe *Yxsoe);    //    出队函数
void DestroyQueue(PQueue);    //    摧毁队列函数
void TraverseQueue(PQueue);    //    遍历队列函数
void ClearQueue(PQueue);    //    清空队列函数
int LengthQueue(PQueue);    //    求队列长度函数



extern Queue YxSoeQueue;	 	//	创建YxSoe队列变量
extern Queue YxQueue;	  		//	创建Yx队列变量
extern Queue YcSoeQueue;	  	//	创建YcSoe队列变量

#endif

