

#ifndef _GX104Slaver_H
#define _GX104Slaver_H


#define     GX104Slaver_MAX_BUF_LEN          256
#define 	BINFOADDR					0x3			//信息体地址长度


#define     GX104Slaver_HEAD                 0X68


#define 	PBI_ON						1
#define 	PBI_OFF						0


/*
 * PRIO
 * */
#define             GX104Slaver_PRIO_INITLINK            0
#define             GX104Slaver_PRIO_CALLALL             1
#define             GX104Slaver_PRIO_CALLGROUP           2
#define             GX104Slaver_PRIO_CLOCK               3
#define             GX104Slaver_PRIO_DELAY               3
#define             GX104Slaver_PRIO_PULSE               0
#define             GX104Slaver_PRIO_SPON                0

/*
 * Ctrl U Function
 * */
#define             GX104Slaver_U_FUNC_STARTDT           0x07		//0B00000111
#define             GX104Slaver_U_FUNC_STOPDT            0x13		//0B00010011
#define             GX104Slaver_U_FUNC_TESTER            0x83		//0B01000011

#define             GX104Slaver_U_FUNC_STARTDT_ACK       0x0b		//0B00001011
#define             GX104Slaver_U_FUNC_STOPDT_ACK        0x23		//0B00100011
#define             GX104Slaver_U_FUNC_TESTER_ACK        0x43		//0B10000011

/*
 * ASDU Type Upload station address
 * */
#define             GX104Slaver_ASDU_TYPE_M_DTU_INF_1    180
#define             GX104Slaver_INFO_SIGNATURE           0XEB90EB90


/*
 * Info Address
 * */
#define             GX104Slaver_DEV_TYPE_HXGF               0X03
#define             GX104Slaver_DEV_TYPE_HXTM               0X01

#define             GX104Slaver_INFOADDR_VALUE_HXGF         0X004001
#define             GX104Slaver_INFOADDR_VALUE_HXTM         0X004201

#define             GX104Slaver_INFOADDR_STATE_HXGF         0X000001
#define             GX104Slaver_INFOADDR_STATE_HXTM         0X000301

/* base device information */
#define             GX104Slaver_INFOADDR_BASE_DEVINFO       0X102001

/*
 * device type 
 */
//#define       ENDDEVICE_TYPE_ERR                  0
//#define       ENDDEVICE_TYPE_HXPF                 2
#define         ENDDEVICE_TYPE_HXTM                 1
#define         ENDDEVICE_TYPE_HXGF                 3
/*
 * Data type 
 */
#define         DATA_TYPE_YX                 		1
#define         DATA_TYPE_YC                 		2

#define         AP_TYPE_BASE_INFO                   4    

/* COT */
#define         AP_COT_BASE_INFO                    18  


#pragma pack(1)



/*
 *  GX104Slaver package
 * */
/* Control filed I type */
typedef struct {

    uint32_t Type:1;
    uint32_t SendSn:15;
    uint32_t Reserve:1;
    uint32_t RecvSn:15;
}GX104Slaver_CTRL_I_T, *PGX104Slaver_CTRL_I_T;

/* Control filed S type */
typedef struct {

    uint32_t Type1:1;
    uint32_t Type2:1;
    uint32_t Reserve:15;
    uint32_t RecvSn:15;
}GX104Slaver_CTRL_S_T, *PGX104Slaver_CTRL_S_T;

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
}GX104Slaver_CTRL_U_T, *PGX104Slaver_CTRL_U_T;

/* Control filed type */
typedef struct {

    uint32_t Type1:1;
    uint32_t Type2:1;
    uint32_t Reserve:30;

}GX104Slaver_CTRLType_T, *PGX104Slaver_CTRLType_T;

/* Control filed type */
typedef struct {

    uint32_t Func:8;
    uint32_t Reserve:24;

}GX104Slaver_CTRLFunc_T, *PGX104Slaver_CTRLFunc_T;

typedef union{

    GX104Slaver_CTRL_I_T     I;
    GX104Slaver_CTRL_S_T     S;
    GX104Slaver_CTRL_U_T     U;
    GX104Slaver_CTRLType_T   Type;
    GX104Slaver_CTRLFunc_T   Func;
}GX104Slaver_CTRL_T, *PGX104Slaver_CTRL_T;

typedef struct{

    uint8_t         Head;
    uint8_t         Len;
    GX104Slaver_CTRL_T   Ctrl;
    uint8_t         Asdu[1];
}GX104Slaver_DATA_T,*PGX104Slaver_DATA_T;

typedef struct {

    float Current;
    float Temperature;    
}GX104Slaver_DevA_Info_T, *PGX104Slaver_DevA_Info_T;

#pragma pack()

/*
 * length
 * */
#define     GX104Slaver_HEAD_LEN          (sizeof(GX104Slaver_DATA_T)-1)
#define     GX104Slaver_ASDU_LEN          (sizeof(IEC10X_ASDU_T)-1)
#define     GX104Slaver_INFO_LEN          (sizeof(ASDU_INFO_T)-1)
#define     GX104Slaver_DATA_LEN          (GX104Slaver_HEAD_LEN+GX104Slaver_ASDU_LEN+GX104Slaver_INFO_LEN)


enum {
    GX104Slaver_FLAG_RECV_CLOSED,        /*0*/
    GX104Slaver_FLAG_SEND_CLOSED,        /*1*/
    GX104Slaver_FLAG_CLOSED,             /*2*/
    GX104Slaver_FLAG_LINK_INIT,          /*3*/
    GX104Slaver_FLAG_IDLE,               /*4*/
    GX104Slaver_FLAG_UPLOAD_ADDR,             /*5*/
    GX104Slaver_FLAG_START_LINK,             /*6*/
    GX104Slaver_FLAG_REQ_LINK,             /*7*/
    GX104Slaver_FLAG_RESET_REMOTE_LINK,             /*8*/
    GX104Slaver_FLAG_INIT_FIN,             /*9*/
    GX104Slaver_FLAG_CALL_ALLDATA,             /*10*/
    GX104Slaver_FLAG_CALL_GROURPDATA,             /*11*/
    GX104Slaver_FLAG_CALL_ACT_FIN,             /*12*/
    GX104Slaver_FLAG_CALL_SIG_TOTAL,             /*13*/
    GX104Slaver_FLAG_CALL_DET_TOTAL,             /*14*/
    GX104Slaver_FLAG_CLOCK_SYS,             /*15*/
    GX104Slaver_FLAG_TESTER,             /*16*/
    GX104Slaver_FLAG_TESTER_STOP,             /*17*/
    GX104Slaver_FLAG_CALL_GROUP,             /*18*/
    GX104Slaver_FLAG_CONNECT_SUCESS,             /*19*/
    GX104Slaver_FLAG_S_ACK,             /*20*/
	GX104Slaver_FLAG_YK_CHECK, 			/*21*/
	GX104Slaver_FLAG_YK_CONFIRM,			/*22*/
	GX104Slaver_FLAG_YK_REVOCATION,			/*23*/
	GX104Slaver_FLAG_YK_FINISH,			/*24*/
	GX104Slaver_FLAG_YK_REFUSE, 		/*25*/
	GX104Slaver_FLAG_YK_OVER_TIME,			/*26*/
};


/*
*********************************************************************************************************
*                          EXTERN  VARIABLE
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                          FUNCTION
*********************************************************************************************************
*/
uint8_t         GX104Slaver_StateMachine(int fd);
uint8_t         GX104Slaver_BuildSignal_Spon(int fd, uint8_t TimeFlag, uint8_t signalV, uint16_t addrV);
int         	GX104Slaver_BuildDetectF_Spont(int fd, uint8_t TimeFlag, float detectV, uint16_t addrV);
int         	GX104Slaver_BuildDetect_Spont(int fd, uint8_t TimeFlag, PIEC10X_DETECT_T detectV, uint16_t addrV);
uint8_t 		GX104Slaver_M_ReadTask(int fd);
void 			GX104Slaver_Start_Clear(void);
uint8_t 		GX104Slaver_BuildYkSelect(int fd,uint8_t qoi, uint8_t reason, YK_INFO_T Yk_info);
int 			Init_GX104Slaver(int DevID);


#endif
