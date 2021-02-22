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

#ifndef _GX101Slaver_H
#define _GX101Slaver_H






#define ARM_PACK __packed
#pragma pack(push)
#pragma pack(1)


#define YC_START_ADDR 0x4001
 



 /*
 *********************************************************************************************************
 *                           FUNCTION CODE
 *********************************************************************************************************
 */
 /*
  * down
  * */
#define     GX101Slaver_CTRL_RESET_LINK          0
#define     GX101Slaver_CTRL_PULSE               2
#define     GX101Slaver_CTRL_SEND_USR_DATA_ACK   3
#define     GX101Slaver_CTRL_SEND_DATA           4
#define     GX101Slaver_CTRL_REQ_LINK_STATUS     9


/*
 * up
 * */
#define     GX101Slaver_CTRL_RES_CONFIRM        		 0x00
#define     GX101Slaver_CTRL_RESPONSE_LINK_STATUS     0x0B
/*
 * DIR, PRM, FCB, FCV, ACD, DFC
 * */
#define     GX101Slaver_CTRL_DIR_DOWN            0x01
#define     GX101Slaver_CTRL_DIR_UP              0x00
#define     GX101Slaver_CTRL_FCB_OPPO_BIT        0x01
#define     GX101Slaver_CTRL_FCB_OPPO_NONE       0x00
#define     GX101Slaver_CTRL_PRM_MASTER          0x01
#define     GX101Slaver_CTRL_PRM_SLAVE           0x00
#define     GX101Slaver_CTRL_FCV_ENABLE          0x01
#define     GX101Slaver_CTRL_FCV_DISABLE         0x00
#define     GX101Slaver_CTRL_ACD_WITH_DATA       0x01
#define     GX101Slaver_CTRL_ACD_NONE_DATA       0x00
#define     GX101Slaver_CTRL_DFC_CAN_REC         0x00
#define     GX101Slaver_CTRL_DFC_CANNOT_REC      0x01

 /*
 *********************************************************************************************************
 *                           FRAME  VALUE
 *********************************************************************************************************
 */
#define     GX101Slaver_STABLE_BEGING             0X10
#define     GX101Slaver_STABLE_END                0X16
#define     GX101Slaver_STABLE_LEN                0X06
/*
 * variable
 * */
#define     GX101Slaver_VARIABLE_BEGING           0X68
#define     GX101Slaver_VARIABLE_END              0X16
 /*
  * length of variable
  * */
#define     GX101Slaver_VARIABLE_HEAD_LEN          (sizeof(GX101Slaver_68_T)-1)
#define     GX101Slaver_VARIABLE_ASDU_LEN          (sizeof(IEC10X_ASDU_T)-1)
#define     GX101Slaver_VARIABLE_INFO_LEN          (sizeof(ASDU_INFO_T)-1)
#define     GX101Slaver_VARIABLE_LEN               (GX101Slaver_VARIABLE_HEAD_LEN+GX101Slaver_VARIABLE_ASDU_LEN+GX101Slaver_VARIABLE_INFO_LEN)        /*add cs+end*/



#define GX101Slaver_MAX_BUF_LEN  256


/*
 *  regular frame   head:10
 * */
//typedef  struct {
//
//	uint8_t _func:4;			   /*function*/
//	uint8_t _dfc:1;
//	uint8_t _acd:1;
//	uint8_t _prm:1;
//	uint8_t _dir:1;
//
//}CTRL_UP_T;
//
//typedef  struct{
//
//	uint8_t _func:4;			   /* function */
//	uint8_t _fcv:1;
//	uint8_t _fcb:1; 			   /* Frame calculate bit */
//	uint8_t _prm:1; 			   /* 1:from start station, 0:from end station */
//	uint8_t _dir:1;
//
//}CTRL_DOWN_T;


//typedef  union
//{
//	CTRL_UP_T up;
//	CTRL_DOWN_T down;
//	uint8_t val;
//} CTRL_T,*PCTRL_T;


typedef  struct
{
    uint8_t _begin;
    CTRL_T _ctrl;
    uint16_t _addr;
    uint8_t _cs;
    uint8_t _end;
}GX101Slaver_10_T,*PGX101Slaver_10_T;


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
}GX101Slaver_68_T,*PGX101Slaver_68_T;



/* GX101Slaver STATE-MACHINE */
enum {
    GX101Slaver_FLAG_LINK_CLOSED,		/*0*/
    GX101Slaver_FLAG_LINK_IDLE,			/*1*/
    GX101Slaver_FLAG_INIT_LINK,			/*2*/	
    GX101Slaver_FLAG_RESET_LINK,		/*3*/
    GX101Slaver_FLAG_REQ_LINK,			/*4*/
    GX101Slaver_FLAG_RESET_REMOTE_LINK,	/*5*/
    GX101Slaver_FLAG_INIT_FIN,			/*6*/
    GX101Slaver_FLAG_CALL_ACT,			/*7*/
    GX101Slaver_FLAG_CALL_ACT_FIN,		/*8*/
    GX101Slaver_FLAG_CALL_ACT_RET,		/*9*/
    GX101Slaver_FLAG_CALL_SIG_TOTAL,	/*10*/
    GX101Slaver_FLAG_CALL_DET_TOTAL,	/*11*/
	GX101Slaver_FLAG_CALL_REFUSE,		/*12*/
    GX101Slaver_FLAG_DELAY_ACT,			/*13*/
    GX101Slaver_FLAG_CLOCK_SYS,			/*14*/
    GX101Slaver_FLAG_PULSE,				/*15*/
    GX101Slaver_FLAG_CALL_GROUP,		/*16*/
    GX101Slaver_FLAG_CONNECT_OK,		/*17*/
	GX101Slaver_FLAG_YK_CHECK,			/*18*/
	GX101Slaver_FLAG_YK_CONFIRM,		/*19*/
	GX101Slaver_FLAG_YK_REVOCATION, 	/*20*/
	GX101Slaver_FLAG_YK_FINISH, 		/*21*/
	GX101Slaver_FLAG_YK_REFUSE, 		/*22*/
	GX101Slaver_FLAG_YK_OVER_TIME,		/*23*/
	GX101Slaver_FLAG_CALL_DZ_TOTAL,		/*24*/
	
};
#pragma pack()

/*
*********************************************************************************************************
*                                           GLOABLE VARIABLE
*********************************************************************************************************
*/
extern        uint16_t            GX101Slaver_Pulse_Cnt;
extern        uint8_t             GX101Slaver_STATE_FLAG_INIT;

extern		  int			      ttyS3_fd;


/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/
int 				Init_GX101Slaver(int DevNo);



#pragma pack(pop)


//#ifdef __cplusplus
//}
//#endif  /*GX101Slaver_STM32*/
//#endif
#endif /*_GX101Slaver_H*/
