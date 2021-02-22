

#ifndef _GX104Master_H
#define _GX104Master_H


#define     GX104Master_MAX_BUF_LEN          256
#define 	BINFOADDR					0x3			//信息体地址长度

#define     GX104Master_HEAD                 0X68


#define 	PBI_ON						1
#define 	PBI_OFF						0


/*
 * PRIO
 * */
#define             GX104Master_PRIO_INITLINK            0
#define             GX104Master_PRIO_CALLALL             1
#define             GX104Master_PRIO_CALLGROUP           2
#define             GX104Master_PRIO_CLOCK               3
#define             GX104Master_PRIO_DELAY               3
#define             GX104Master_PRIO_PULSE               0
#define             GX104Master_PRIO_SPON                0

/*
 * Ctrl U Function
 * */
#define             GX104Master_U_FUNC_STARTDT           0x07		//0B00000111
#define             GX104Master_U_FUNC_STOPDT            0x13		//0B00010011
#define             GX104Master_U_FUNC_TESTER            0x83		//0B01000011

#define             GX104Master_U_FUNC_STARTDT_ACK       0x0b		//0B00001011
#define             GX104Master_U_FUNC_STOPDT_ACK        0x23		//0B00100011
#define             GX104Master_U_FUNC_TESTER_ACK        0x43		//0B10000011

/*
 * ASDU Type Upload station address
 * */
#define             GX104Master_ASDU_TYPE_M_DTU_INF_1    180
#define             GX104Master_INFO_SIGNATURE           0XEB90EB90


/*
 * Info Address
 * */
#define             GX104Master_DEV_TYPE_HXGF               0X03
#define             GX104Master_DEV_TYPE_HXTM               0X01

#define             GX104Master_INFOADDR_VALUE_HXGF         0X004001
#define             GX104Master_INFOADDR_VALUE_HXTM         0X004201

#define             GX104Master_INFOADDR_STATE_HXGF         0X000001
#define             GX104Master_INFOADDR_STATE_HXTM         0X000301

/* base device information */
#define             GX104Master_INFOADDR_BASE_DEVINFO       0X102001

/*
 * device type 
 */
//#define       ENDDEVICE_TYPE_ERR                  0
//#define       ENDDEVICE_TYPE_HXPF                 2
#define         ENDDEVICE_TYPE_HXTM                 1
#define         ENDDEVICE_TYPE_HXGF                 3

#define         AP_TYPE_BASE_INFO                   4    

/* COT */
#define         AP_COT_BASE_INFO                    18  


#pragma pack(1)



/*
 *  GX104Master package
 * */
/* Control filed I type */
typedef struct {

    uint32_t Type:1;
    uint32_t SendSn:15;
    uint32_t Reserve:1;
    uint32_t RecvSn:15;
}GX104Master_CTRL_I_T, *PGX104Master_CTRL_I_T;

/* Control filed S type */
typedef struct {

    uint32_t Type1:1;
    uint32_t Type2:1;
    uint32_t Reserve:15;
    uint32_t RecvSn:15;
}GX104Master_CTRL_S_T, *PGX104Master_CTRL_S_T;

/* Control filed U type */
typedef struct {

    uint32_t Type1:1;
    uint32_t Type2:1;
    uint32_t Startdt:1;
    uint32_t StartdtAck:1;
    uint32_t Stopdt:1;
    uint32_t StopdtAck:1;
    uint32_t Tester:1;
    uint32_t TesterAck:1;
    uint32_t Reserve:24;
}GX104Master_CTRL_U_T, *PGX104Master_CTRL_U_T;

/* Control filed type */
typedef struct {

    uint32_t Type1:1;
    uint32_t Type2:1;
    uint32_t Reserve:30;

}GX104Master_CTRLType_T, *PGX104Master_CTRLType_T;


/* Control filed type */
typedef struct {

    uint32_t Func:8;
    uint32_t Reserve:24;

}GX104Master_CTRLFunc_T, *PGX104Master_CTRLFunc_T;

typedef union{

    GX104Master_CTRL_I_T     I;
    GX104Master_CTRL_S_T     S;
    GX104Master_CTRL_U_T     U;
    GX104Master_CTRLType_T   Type;
    GX104Master_CTRLFunc_T   Func;
}GX104Master_CTRL_T, *PGX104Master_CTRL_T;

typedef struct{

    uint8_t         Head;
    uint8_t         Len;
    GX104Master_CTRL_T   Ctrl;
    uint8_t         Asdu[1];
}GX104Master_DATA_T,*PGX104Master_DATA_T;

typedef struct {

    float Current;
    float Temperature;    
}GX104Master_DevA_Info_T, *PGX104Master_DevA_Info_T;

#pragma pack()

/*
 * length
 * */
#define     GX104Master_HEAD_LEN          (sizeof(GX104Master_DATA_T)-1)
#define     GX104Master_ASDU_LEN          (sizeof(IEC10X_ASDU_T)-1)
#define     GX104Master_INFO_LEN          (sizeof(ASDU_INFO_T)-1)
#define     GX104Master_DATA_LEN          (GX104Master_HEAD_LEN+GX104Master_ASDU_LEN+GX104Master_INFO_LEN)


enum {
    GX104Master_FLAG_CLOSED,        /*0*/
    GX104Master_FLAG_SEND_CLOSED,        /*1*/
    GX104Master_FLAG_RECV_CLOSED,             /*2*/
    GX104Master_FLAG_LINK_INIT,          /*3*/
    GX104Master_FLAG_IDLE,               /*4*/
    GX104Master_FLAG_UPLOAD_ADDR,             /*5*/
    GX104Master_FLAG_START_LINK,             /*6*/
    GX104Master_FLAG_REQ_LINK,             /*7*/
    GX104Master_FLAG_RESET_REMOTE_LINK,             /*8*/
    GX104Master_FLAG_INIT_FIN,             /*9*/
    GX104Master_FLAG_CALL_ALLDATA,             /*10*/
    GX104Master_FLAG_CALL_GROURPDATA,             /*11*/
    GX104Master_FLAG_CALL_ACT_FIN,             /*12*/
    GX104Master_FLAG_CALL_SIG_TOTAL,             /*13*/
    GX104Master_FLAG_CALL_DET_TOTAL,             /*14*/
    GX104Master_FLAG_CLOCK_SYS,             /*15*/
    GX104Master_FLAG_TESTER,             /*16*/
    GX104Master_FLAG_TESTER_STOP,             /*17*/
    GX104Master_FLAG_CALL_GROUP,             /*18*/
    GX104Master_FLAG_CONNECT_SUCESS,             /*19*/
    GX104Master_FLAG_S_ACK,             /*20*/
};

enum {
    GX104Master_M_FLAG_RECV_CLOSED,        /*0*/
    GX104Master_M_FLAG_SEND_CLOSED,        /*1*/
    GX104Master_M_FLAG_CLOSED,             /*2*/
    GX104Master_M_FLAG_LINK_INIT,          /*3*/
    GX104Master_M_FLAG_IDLE,               /*4*/
    GX104Master_M_FLAG_UPLOAD_ADDR,             /*5*/
    GX104Master_M_FLAG_START_LINK,             /*6*/
    GX104Master_M_FLAG_REQ_LINK,             /*7*/
    GX104Master_M_FLAG_RESET_REMOTE_LINK,             /*8*/
    GX104Master_M_FLAG_CALL_GROURPDATA,             /*11*/
    GX104Master_M_FLAG_CALL_ACT_FIN,             /*12*/
    GX104Master_M_FLAG_CALL_SIG_TOTAL,             /*13*/
    GX104Master_M_FLAG_CALL_DET_TOTAL,             /*14*/
    GX104Master_M_FLAG_CLOCK_SYS,             /*15*/
    GX104Master_M_FLAG_TESTER,             /*16*/
    GX104Master_M_FLAG_TESTER_STOP,             /*17*/
    GX104Master_M_FLAG_CALL_GROUP,             /*18*/
    GX104Master_M_FLAG_CONNECT_SUCESS,             /*19*/
    GX104Master_M_FLAG_S_ACK,             /*20*/
	GX104Master_M_FLAG_REC_YX,			 /*21*/
	GX104Master_M_FLAG_REC_YC,			 /*21*/
};


/*
*********************************************************************************************************
*                          EXTERN  VARIABLE
*********************************************************************************************************
*/
extern      uint8_t             GX104Master_STATE_FLAG_INIT;
extern      uint8_t             GX104Master_STATE_FLAG_CALLALL;
extern      uint8_t             GX104Master_STATE_FLAG_GROUP;
extern      uint8_t             GX104Master_STATE_FLAG_CLOCK;
extern      uint8_t             GX104Master_STATE_FLAG_PULSE;

extern		uint8_t             GX104Master_M_STATE_FLAG_INIT;


/*
*********************************************************************************************************
*                          FUNCTION
*********************************************************************************************************
*/
void GX104Master_Start_Link(int DevNO);
int Init_GX104Master(int DevNO);


#endif
