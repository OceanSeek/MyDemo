//    ���еĵ�����ʵ��
//    ͷ�ڵ㣺�ڱ����ã���������ݣ�������ʼ������ʱʹ��ͷ��βָ��ĵط�
//    �׽ڵ㣺ͷ�ڵ���һ���ڵ㣬�������



#include "sys.h"

Queue YxSoeQueue;	 	//	����YxSoe���б���
Queue YxQueue;	  		//	����Yx���б���
Queue YcSoeQueue;	  	//	����YcSoe���б���

//    ����������
void InitQueue(PQueue);    //    �������к���
bool IsEmptyQueue(PQueue);    //    �ж϶����Ƿ�Ϊ�պ���
void InsertQueue(PQueue queue,int val,TSysTimeSoe _YxSoe);    //    ��Ӻ���
void DeleteQueue(PQueue queue,int* val,TSysTimeSoe *Yxsoe);    //    ���Ӻ���
void DestroyQueue(PQueue);    //    �ݻٶ��к���
void TraverseQueue(PQueue);    //    �������к���
void ClearQueue(PQueue);    //    ��ն��к���
int LengthQueue(PQueue);    //    ����г��Ⱥ���

//    ������
int A_main() {
    int val = 0;    //    ������ʱ����
    Queue queue;    //    �������б���
    TSysTimeSoe YxSoe;

	
    InitQueue(&queue);    //    ���ó�ʼ�����к���
    IsEmptyQueue(&queue);    //    �����ж϶����Ƿ�Ϊ�պ���
    InsertQueue(&queue, 100,YxSoe);    //������Ӻ���
    InsertQueue(&queue, 200,YxSoe);
    InsertQueue(&queue, 300,YxSoe);
    InsertQueue(&queue, 400,YxSoe);
    InsertQueue(&queue, 500,YxSoe);
    InsertQueue(&queue, 600,YxSoe);
    IsEmptyQueue(&queue);
    TraverseQueue(&queue);    //    ���ö��б�������
    DeleteQueue(&queue, &val,&YxSoe);    //    ���ó��Ӻ���
    TraverseQueue(&queue);
    ClearQueue(&queue);        //    ������ն��к���
    IsEmptyQueue(&queue);
    DestroyQueue(&queue);    //    ���ôݻٶ��к���

    return 0;
}

//    ������г�ʼ������
void InitQueue(PQueue queue) {
    queue->Head = queue->Tail = (PNODE)malloc(sizeof(NODE));    //    ��̬����ͷ�ڵ㣬ʹ��ͷ����βָ��ýڵ�
    //ͷ�ڵ��൱���ڱ��ڵ�����ã����洢���ݣ��������׽ڵ㣩
    if (queue->Head == NULL) {        //    �ж��ڴ��Ƿ����ɹ�
        log("InitQueue Fail...\n");
        exit(-1);
    }
    queue->Head->Next = NULL;    //    ��ʼ����ͷ�ڵ�ָ��ָ��Ϊ��
    log("InitQueue Suscess...\n");
}

//    �����ж϶����Ƿ�Ϊ�պ���
bool IsEmptyQueue(PQueue queue) {
    if (queue->Head == queue->Tail) {
        log("queue is empty...\n");
        return true;
    }
    else {
        //log("���в�Ϊ��...\n");
        return false;
    }
        
}
//    ������Ӻ���
//    �Ӷ���β����������val
void InsertQueue(PQueue queue,int val,TSysTimeSoe _YxSoe) {
    PNODE P = (PNODE)malloc(sizeof(NODE));    //    ����һ���½ڵ����ڴ�Ų����Ԫ��
    if (P == NULL) {
        log("Ram Error can't malloc memery \n");
        exit(-1);
    }
    P->Element = val;    //    ��Ҫ��������ݷŵ��ڵ�������
    P->YxSoe = _YxSoe;
    P->Next = NULL;        //    �½ڵ�ָ��ָ��Ϊ��
    queue->Tail->Next = P;    //    ʹ��һ������β���Ľڵ�ָ��ָ���½��Ľڵ�
    queue->Tail = P;    //    ���¶�βָ�룬ʹ��ָ��������Ľڵ�
//    log("Insert date %d sucess\n",val);
//    log("Insert date sucess\n");
}

//    ������Ӻ���
//    �Ӷ��е��׽ڵ㿪ʼ����
//    �����ӳɹ�����val������ֵ
void DeleteQueue(PQueue queue,int* val,TSysTimeSoe *Yxsoe) {
    if (IsEmptyQueue(queue)) {
        log("The queue is empty...\n");
        return;
    }
    PNODE  P= queue->Head->Next;    //    ��ʱָ��
    *val = P->Element;    //    ������ֵ
    *Yxsoe = P->YxSoe;
    queue->Head->Next = P->Next;        //    ����ͷ�ڵ�
    if (queue->Tail==P)
        queue->Tail = queue->Head;
    free(P);    //    �ͷ�ͷ����
    P = NULL;    //    ��ֹ����Ұָ��
//    log("The stack is pushed successfully, and the value is %d\n", *val);
}
//    ������б�������
void TraverseQueue(PQueue queue) {
    
    if (IsEmptyQueue(queue)) {
        return;
    }        
    PNODE P = queue->Head->Next;    //�Ӷ����׽ڵ㿪ʼ��������ͷ�ڵ㣬ע�����֣�
    log("The result of queue is :\n");
    while (P != NULL) {
		log("YxTime yx(%d) value16(%d) value32(0x:%x) Point(%d) year(%d) mon(%d) min(%d)\n",P->YxSoe.bStatus,P->YxSoe.detect16._detect,\
			P->YxSoe.detect32._detect,P->YxSoe.nNo,P->YxSoe.stTime._year._year,\
			P->YxSoe.stTime._month._month,P->YxSoe.stTime._min._minutes);
        P = P->Next;
    }
    log("\n");
}
//    ������еĴݻٺ���
//    ɾ���������У�����ͷ�ڵ�
void DestroyQueue(PQueue queue) {
    //��ͷ�ڵ㿪ʼɾ��
    while (queue->Head != NULL) {
        queue->Tail = queue->Head->Next;
        free(queue->Head);
        queue->Head = queue->Tail;
    }
    log("destroy queue ...\n");
}
//    ������ն��к���
void ClearQueue(PQueue queue) {
    PNODE P = queue->Head->Next;    //    ��ʱָ��
    PNODE Q = NULL;        //    ��ʱָ��
    queue->Tail = queue->Head;        //    ʹ��βָ��ָ��ͷ�ڵ�
    queue->Head->Next = NULL;
    //    ���׽ڵ㿪ʼ���
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



