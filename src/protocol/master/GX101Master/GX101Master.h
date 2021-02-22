/*******************************************************************
Copyright (C):    
File name    :    gx101slave.h
DESCRIPTION  :
AUTHOR       :
Version      :    1.0
Date         :    2020/02/18
Others       :
History      :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) Date: 2020/02/18         Author: weitao
   content:

*******************************************************************/

//#ifndef _IEC101_H
//#define _IEC101_H



#define ARM_PACK __packed
#pragma pack(push)
#pragma pack(1)




 /*
 *********************************************************************************************************
 *                           FUNCTION CODE
 *********************************************************************************************************
 */
 /*
  * down
  * */
#define     IEC101_CTRL_RESET_LINK          0
#define     IEC101_CTRL_PULSE               2
#define     IEC101_CTRL_SEND_USR_DATA_ACK   3
#define     IEC101_CTRL_SEND_DATA           4
#define     IEC101_CTRL_REQ_LINK_STATUS     9


/*
 * up
 * */
#define     IEC101_CTRL_RES_CONFIRM        		 0x00
#define     IEC101_CTRL_RESPONSE_LINK_STATUS     0x0B
/*
 * DIR, PRM, FCB, FCV, ACD, DFC
 * */
#define     IEC101_CTRL_DIR_DOWN            0x01
#define     IEC101_CTRL_DIR_UP              0x00
#define     IEC101_CTRL_FCB_OPPO_BIT        0x01
#define     IEC101_CTRL_FCB_OPPO_NONE       0x00
#define     IEC101_CTRL_PRM_MASTER          0x01
#define     IEC101_CTRL_PRM_SLAVE           0x00
#define     IEC101_CTRL_FCV_ENABLE          0x01
#define     IEC101_CTRL_FCV_DISABLE         0x00
#define     IEC101_CTRL_ACD_WITH_DATA       0x01
#define     IEC101_CTRL_ACD_NONE_DATA       0x00
#define     IEC101_CTRL_DFC_CAN_REC         0x00
#define     IEC101_CTRL_DFC_CANNOT_REC      0x01

 /*
 *********************************************************************************************************
 *                           FRAME  VALUE
 *********************************************************************************************************
 */
#define     IEC101_STABLE_BEGING             0X10
#define     IEC101_STABLE_END                0X16
#define     IEC101_STABLE_LEN                0X06
/*
 * variable
 * */
#define     IEC101_VARIABLE_BEGING           0X68
#define     IEC101_VARIABLE_END              0X16
 /*
  * length of variable
  * */
#define     IEC101_VARIABLE_HEAD_LEN          (sizeof(IEC101_68_T)-1)
#define     IEC101_VARIABLE_ASDU_LEN          (sizeof(IEC10X_ASDU_T)-1)
#define     IEC101_VARIABLE_INFO_LEN          (sizeof(ASDU_INFO_T)-1)
#define     IEC101_VARIABLE_LEN               (IEC101_VARIABLE_HEAD_LEN+IEC101_VARIABLE_ASDU_LEN+IEC101_VARIABLE_INFO_LEN)        /*add cs+end*/

#pragma pack(1)

typedef struct{
    uint8_t _flag;
    uint8_t _data;
	uint8_t _reason;
	uint8_t _TI;
}YKReturn_T,*PYKReturn_T;


#define IEC101_MAX_BUF_LEN  256

/*
 * Control Code
 * */
//typedef  struct{
//
//    uint8_t _func:4;               /* function */
//    uint8_t _fcv:1;
//    uint8_t _fcb:1;                /* Frame calculate bit */
//    uint8_t _prm:1;                /* 1:from start station, 0:from end station */
//    uint8_t _dir:1;
//
//}CTRL_DOWN_T;
//
//typedef  struct {
//
//    uint8_t _func:4;               /*function*/
//    uint8_t _dfc:1;
//    uint8_t _acd:1;
//    uint8_t _prm:1;
//    uint8_t _dir:1;
//
//}CTRL_UP_T;

//typedef  union
//{
//    CTRL_UP_T up;
//    CTRL_DOWN_T down;
//    uint8_t val;
//} CTRL_T,*PCTRL_T;

/*
 *  regular frame   head:10
 * */
typedef  struct
{
    uint8_t _begin;
    CTRL_T _ctrl;
    uint16_t _addr;
    uint8_t _cs;
    uint8_t _end;
}IEC101_10_T,*PIEC101_10_T;

/*
 *  variable fram   head:68
 * */
typedef struct
{
    uint8_t _begin;
    uint8_t _len;
    uint8_t _len_cfm;
    uint8_t _begin_cfm;
    CTRL_T _ctrl;
    uint16_t _addr;
    uint8_t _asdu[1];
}IEC101_68_T,*PIEC101_68_T;

#pragma pack()


/* IEC101 STATE-MACHINE */
enum {
    IEC101_FLAG_LINK_CLOSED,
    IEC101_FLAG_LINK_IDLE,
    IEC101_FLAG_INIT_LINK,
    IEC101_FLAG_RESET_LINK,
    IEC101_FLAG_REQ_LINK,
    IEC101_FLAG_RESET_REMOTE_LINK,
    IEC101_FLAG_INIT_FIN,
    IEC101_FLAG_CALL_ACT,
    IEC101_FLAG_CALL_ACT_FIN,
    IEC101_FLAG_CALL_ACT_RET,
    IEC101_FLAG_CALL_SIG_TOTAL,
    IEC101_FLAG_CALL_DET_TOTAL,
    IEC101_FLAG_DELAY_ACT,
    IEC101_FLAG_CLOCK_SYS,
    IEC101_FLAG_PULSE,
    IEC101_FLAG_CALL_GROUP,
    IEC101_FLAG_CONNECT_OK,
};

/*
*********************************************************************************************************
*                                           GLOABLE VARIABLE
*********************************************************************************************************
*/
extern        uint16_t            IEC101_Pulse_Cnt;
extern        uint8_t             IEC101_STATE_FLAG_INIT;

extern		  int			      ttyS3_fd;


/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/

//void                Iex101_main(uint8_t *buf, uint16_t len);

int		          	IEC101_BuildSignal_Spont(uint8_t TimeFlag,uint8_t sq, uint8_t asdu_num, PTSysTimeSoe asdu_data);
int             	IEC101_BuildDetectDW_Spont(uint8_t reason,uint8_t type, uint8_t Prio, uint8_t asdu_num, PTSysTimeSoe asdu_data);
int             	IEC101_BuildDetectF_Spont(uint8_t TimeFlag,        float detectV, uint16_t addrV);
int 				Init_GX101Master(int DevID);

#pragma pack(pop)


//#ifdef __cplusplus
//}
//#endif  /*IEC101_STM32*/
//#endif
//#endif /*_IEC101_H*/
