//    队列的单链表实现
//    头节点：哨兵作用，不存放数据，用来初始化队列时使队头队尾指向的地方
//    首节点：头节点后第一个节点，存放数据



#include "sys.h"

Queue YxSoeQueue;	 	//	创建YxSoe队列变量
Queue YxQueue;	  		//	创建Yx队列变量
Queue YcSoeQueue;	  	//	创建YcSoe队列变量

//    声明函数体
void InitQueue(PQueue);    //    创建队列函数
bool IsEmptyQueue(PQueue);    //    判断队列是否为空函数
void InsertQueue(PQueue queue,int val,TSysTimeSoe _YxSoe);    //    入队函数
void DeleteQueue(PQueue queue,int* val,TSysTimeSoe *Yxsoe);    //    出队函数
void DestroyQueue(PQueue);    //    摧毁队列函数
void TraverseQueue(PQueue);    //    遍历队列函数
void ClearQueue(PQueue);    //    清空队列函数
int LengthQueue(PQueue);    //    求队列长度函数

//    主函数
int A_main() {
    int val = 0;    //    定义临时变量
    Queue queue;    //    创建队列变量
    TSysTimeSoe YxSoe;

	
    InitQueue(&queue);    //    调用初始化队列函数
    IsEmptyQueue(&queue);    //    调用判断队列是否为空函数
    InsertQueue(&queue, 100,YxSoe);    //调用入队函数
    InsertQueue(&queue, 200,YxSoe);
    InsertQueue(&queue, 300,YxSoe);
    InsertQueue(&queue, 400,YxSoe);
    InsertQueue(&queue, 500,YxSoe);
    InsertQueue(&queue, 600,YxSoe);
    IsEmptyQueue(&queue);
    TraverseQueue(&queue);    //    调用队列遍历函数
    DeleteQueue(&queue, &val,&YxSoe);    //    调用出队函数
    TraverseQueue(&queue);
    ClearQueue(&queue);        //    调用清空队列函数
    IsEmptyQueue(&queue);
    DestroyQueue(&queue);    //    调用摧毁队列函数

    return 0;
}

//    定义队列初始化函数
void InitQueue(PQueue queue) {
    queue->Head = queue->Tail = (PNODE)malloc(sizeof(NODE));    //    动态创建头节点，使队头，队尾指向该节点
    //头节点相当于哨兵节点的作用，不存储数据（区别于首节点）
    if (queue->Head == NULL) {        //    判断内存是否分配成功
        log("InitQueue Fail...\n");
        exit(-1);
    }
    queue->Head->Next = NULL;    //    初始队列头节点指针指向为空
    log("InitQueue Suscess...\n");
}

//    定义判断队列是否为空函数
bool IsEmptyQueue(PQueue queue) {
    if (queue->Head == queue->Tail) {
        log("queue is empty...\n");
        return true;
    }
    else {
        //log("队列不为空...\n");
        return false;
    }
        
}
//    定义入队函数
//    从队列尾部插入数据val
void InsertQueue(PQueue queue,int val,TSysTimeSoe _YxSoe) {
    PNODE P = (PNODE)malloc(sizeof(NODE));    //    创建一个新节点用于存放插入的元素
    if (P == NULL) {
        log("Ram Error can't malloc memery \n");
        exit(-1);
    }
    P->Element = val;    //    把要插入的数据放到节点数据域
    P->YxSoe = _YxSoe;
    P->Next = NULL;        //    新节点指针指向为空
    queue->Tail->Next = P;    //    使上一个队列尾部的节点指针指向新建的节点
    queue->Tail = P;    //    更新队尾指针，使其指向队列最后的节点
//    log("Insert date %d sucess\n",val);
//    log("Insert date sucess\n");
}

//    定义出队函数
//    从队列的首节点开始出队
//    若出队成功，用val返回其值
void DeleteQueue(PQueue queue,int* val,TSysTimeSoe *Yxsoe) {
    if (IsEmptyQueue(queue)) {
        log("The queue is empty...\n");
        return;
    }
    PNODE  P= queue->Head->Next;    //    临时指针
    *val = P->Element;    //    保存其值
    *Yxsoe = P->YxSoe;
    queue->Head->Next = P->Next;        //    更新头节点
    if (queue->Tail==P)
        queue->Tail = queue->Head;
    free(P);    //    释放头队列
    P = NULL;    //    防止产生野指针
//    log("The stack is pushed successfully, and the value is %d\n", *val);
}
//    定义队列遍历函数
void TraverseQueue(PQueue queue) {
    
    if (IsEmptyQueue(queue)) {
        return;
    }        
    PNODE P = queue->Head->Next;    //从队列首节点开始遍历（非头节点，注意区分）
    log("The result of queue is :\n");
    while (P != NULL) {
		log("YxTime yx(%d) value16(%d) value32(0x:%x) Point(%d) year(%d) mon(%d) min(%d)\n",P->YxSoe.bStatus,P->YxSoe.detect16._detect,\
			P->YxSoe.detect32._detect,P->YxSoe.nNo,P->YxSoe.stTime._year._year,\
			P->YxSoe.stTime._month._month,P->YxSoe.stTime._min._minutes);
        P = P->Next;
    }
    log("\n");
}
//    定义队列的摧毁函数
//    删除整个队列，包括头节点
void DestroyQueue(PQueue queue) {
    //从头节点开始删除
    while (queue->Head != NULL) {
        queue->Tail = queue->Head->Next;
        free(queue->Head);
        queue->Head = queue->Tail;
    }
    log("destroy queue ...\n");
}
//    定义清空队列函数
void ClearQueue(PQueue queue) {
    PNODE P = queue->Head->Next;    //    临时指针
    PNODE Q = NULL;        //    临时指针
    queue->Tail = queue->Head;        //    使队尾指针指向头节点
    queue->Head->Next = NULL;
    //    从首节点开始清空
    while (P != NULL) {
        Q = P;
        P = P->Next;
        free(Q);
    }
    log("clear queue sucess...\n");

}

int LengthQueue(PQueue queue){
	int Count=0;
	PNODE P = queue->Head->Next;
	while(P!=NULL){
		Count++;
		P = P->Next;
	}
	return Count;
}



