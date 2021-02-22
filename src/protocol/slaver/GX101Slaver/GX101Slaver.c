#include "sys.h"
//#include "GX101Slaver.h"

////#ifdef  GX101Slaver_STM32
// /*
// * GLOABLE VARIALBLE
// */
uint8_t                         GX101Slaver_Sendbuf[GX101Slaver_MAX_BUF_LEN];
static uint8_t                  GX101Slaver_ReSendbuf[GX101Slaver_MAX_BUF_LEN];
uint8_t                         *GX101Slaver_Recbuf;

uint8_t             GX101Slaver_Call_Qoi = 0;
uint16_t            GX101Slaver_Pulse_Cnt = 0;


int				GX101Slaver_Sta_Addr = 0;
int16_t DZRemain;//DZ����
uint16_t Rec_milliseconds;

uint32_t Old_Time_cnt1;
uint32_t OldSoeStartAddr = 0;
uint32_t OldSoeEndAddr = 0;

extern TBurstYXRecord YXBrustRecord[MAXYXRECORD];
extern TBurstYXRecord YXSoeRecord[MAXYXRECORD];
extern TBurstYCRecord YCBrustRecord[MAXYCRECORD];
extern TBurstYCRecord DZBrustRecord[MAXYCRECORD];

extern TransTable_T *TransYKTable;



uint16_t flagtest = 0;

int GX101_Slaver_Receive(int DevNo, uint8_t *buf, uint16_t len);
int GX101Slaver_Task(int DevNo);
int GX101Slaver_On_Time_Out(int DevNo);
int GX101Slaver_Deal_DZ_YZ(int DevNo, PIEC10X_ASDU_101T GX101Slaver_Asdu);
uint8_t GX101Slaver_BuildBrustDZ(int DevNo, uint16_t asdu_num,uint16_t StartAddr,PTBurstYCRecord asdu_data);
int GX101ScanBrustData(int DevNo);


int Init_GX101Slaver(int DevNo)
{
	gpDevice[DevNo].STATE_FLAG_INIT = GX101Slaver_FLAG_LINK_CLOSED;
	gpDevice[DevNo].STATE_FLAG_CALLALL = GX101Slaver_FLAG_LINK_CLOSED;
	gpDevice[DevNo].STATE_FLAG_GROUP = GX101Slaver_FLAG_LINK_CLOSED;
	gpDevice[DevNo].STATE_FLAG_DELAY = GX101Slaver_FLAG_LINK_CLOSED;
	gpDevice[DevNo].STATE_FLAG_YK = GX101Slaver_FLAG_LINK_CLOSED;

	gpDevice[DevNo].Receive = GX101_Slaver_Receive;
	gpDevice[DevNo].Task = GX101Slaver_Task;
	gpDevice[DevNo].OnTimeOut = GX101Slaver_On_Time_Out;
    
	
#ifdef HUAWEI
	if(strcmp("Serial", gpDevice[DevNo].Com) == 0)HuaWei485Ctrl_Switch(DevNo, RS485CTL_Read);
#endif
}


static uint8_t CK_send(int DevNo,char *buf, int len){

#ifdef HUAWEI
	if(strcmp("Serial", gpDevice[DevNo].Com) == 0){
		HuaWei485Ctrl_Switch(DevNo, RS485CTL_Write);
		DumpHEX(buf, len);
		if(-1 == write(gpDevice[DevNo].fd,buf,len)){
			perror("Send error \n");
			log("DevNo(%d)	fd(%d)\n",DevNo, gpDevice[DevNo].fd);
			HuaWei485Ctrl_Switch(DevNo, RS485CTL_Read);
			return RET_ERROR;
		}
		usleep(10000);
		HuaWei485Ctrl_Switch(DevNo, RS485CTL_Read);
    }
#else
	if(strcmp("Serial", gpDevice[DevNo].Com) == 0){
		if(-1 == write(gpDevice[DevNo].fd,buf,len)){
			perror("Send error \n");
			log("DevNo(%d)	fd(%d)\n",DevNo, gpDevice[DevNo].fd);
			return RET_ERROR;
		}		
    }
#endif
	
	else if(strcmp("Net", gpDevice[DevNo].Com) == 0){
		if(strcmp("udp", gpDevice[DevNo].TcpType) == 0){
			struct sockaddr_in dest_addr = {};
			dest_addr.sin_family = AF_INET;//ipv4
			dest_addr.sin_port = htons(gpDevice[DevNo].UDP_Dest_PORT);//�˿ں�
			dest_addr.sin_addr.s_addr = inet_addr(gpDevice[DevNo].UDP_Dest_IP);//�ҵ�ip��ַ
			sendto(gpDevice[DevNo].fd, buf, len, 0, (struct sockaddr *)&dest_addr,sizeof(dest_addr)); 
		}
	
		else if(-1 == write(gpDevice[DevNo].fd,buf,len)){
			perror("Send error \n");
			log("DevNo(%d)	fd(%d)\n",DevNo, gpDevice[DevNo].fd);
			return RET_ERROR;
		}
    }
	

	if(buf[0] == 0x68){//��ر���
		gpDevice[DevNo].ReSendOldTime = gpDevice[DevNo].TimeCnt;
		gpDevice[DevNo].ReSendDataLen = len;
		gpDevice[DevNo].ReSendFlag = RESEND_YES;
	
		memcpy(GX101Slaver_ReSendbuf, buf, len);
	}
	MonitorTx(monitorData._TX_ID, DevNo, monitorData._fd, buf, len);
	
    return RET_SUCESS;
}
uint8_t GX101Slaver_ResetFlag(int DevNo)
{
	int i;
	for(i=0;i<gVars.dwDevNum;i++){
		gpDevice[i].ReadYCPtr = gVars.YcWritePtr = 0;
		gpDevice[i].ReadYXPtr = gVars.YxWritePtr = 0;
		gpDevice[i].ReadYXSoePtr = gVars.YxSoeWritePtr = 0;
	}
}


uint8_t GX101Slaver_RetStatusOk(int DevNo, uint16_t addr){

    uint16_t len;
    PGX101Slaver_10_T GX101Slaver = (PGX101Slaver_10_T)GX101Slaver_Sendbuf;

    len = GX101Slaver_STABLE_LEN;

    GX101Slaver->_begin = GX101Slaver_STABLE_BEGING;

    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_SLAVE;
    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_NONE_DATA;
    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CAN_REC;

    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_RESPONSE_LINK_STATUS;

    GX101Slaver->_addr = addr;
    GX101Slaver->_cs = GX101Slaver_Sendbuf[1]+GX101Slaver_Sendbuf[2]+GX101Slaver_Sendbuf[3];
    GX101Slaver->_end = GX101Slaver_STABLE_END;
	
	CK_send(DevNo, GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, GX101Slaver_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}

uint8_t GX101Slaver_AskConfirm(int DevNo, uint16_t addr){

    uint16_t len;
    PGX101Slaver_10_T GX101Slaver = (PGX101Slaver_10_T)GX101Slaver_Sendbuf;

    len = GX101Slaver_STABLE_LEN;

    GX101Slaver->_begin = GX101Slaver_STABLE_BEGING;

    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_SLAVE;
    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_NONE_DATA;
    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CAN_REC;

    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_RES_CONFIRM;

    GX101Slaver->_addr = addr;
    GX101Slaver->_cs = GX101Slaver_Sendbuf[1]+GX101Slaver_Sendbuf[2]+GX101Slaver_Sendbuf[3];
    GX101Slaver->_end = GX101Slaver_STABLE_END;

	CK_send(DevNo,GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, GX101Slaver_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}


static uint8_t GX101Slaver_ResConfirm(int DevNo){

    uint16_t len;

    PGX101Slaver_10_T GX101Slaver = (PGX101Slaver_10_T)GX101Slaver_Sendbuf;

    len = GX101Slaver_STABLE_LEN;

    GX101Slaver->_begin = GX101Slaver_STABLE_BEGING;

    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_SLAVE;
    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_NONE_DATA;
    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CAN_REC;

    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_RES_CONFIRM;

    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
    GX101Slaver->_cs = GX101Slaver_Sendbuf[1]+GX101Slaver_Sendbuf[2]+GX101Slaver_Sendbuf[3];
    GX101Slaver->_end = GX101Slaver_STABLE_END;

	
	CK_send(DevNo,GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, Prio, NULL,NULL);

    return RET_SUCESS;
}

static uint8_t GX101Slaver_Send_Mirror(int DevNo, PGX101Slaver_68_T GX101Slaver_68, uint8_t reason){

    uint16_t len, i;
	uint8_t cs_temp = 0;

    /*init struct*/
    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);
	
	len = GX101Slaver_68->_len + 6;

	memcpy(GX101Slaver_Sendbuf, GX101Slaver_68, len);


	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;

	asdu->_reason = reason;

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;
	
	CK_send(DevNo, GX101Slaver_Sendbuf,len);

    return RET_SUCESS;
}

uint8_t GX101Slaver_ReqLinkStatus(int DevNo){

    uint16_t len;

    PGX101Slaver_10_T GX101Slaver = (PGX101Slaver_10_T)GX101Slaver_Sendbuf;


    len = GX101Slaver_STABLE_LEN;

    GX101Slaver->_begin = GX101Slaver_STABLE_BEGING;

    /*Ctrol*/
    GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
    GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
    GX101Slaver->_ctrl.down._fcb = 0;
    GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_FCV_DISABLE;
    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_REQ_LINK_STATUS;
	
    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
    GX101Slaver->_cs = GX101Slaver_Sendbuf[1]+GX101Slaver_Sendbuf[2]+GX101Slaver_Sendbuf[3];
    GX101Slaver->_end = GX101Slaver_STABLE_END;

	CK_send(DevNo,GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, GX101Slaver_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}

//
uint8_t GX101Slaver_ResetLink(int DevNo, uint16_t addr){

    uint16_t len;

    PGX101Slaver_10_T GX101Slaver = (PGX101Slaver_10_T)GX101Slaver_Sendbuf;

    len = GX101Slaver_STABLE_LEN;

    GX101Slaver->_begin = GX101Slaver_STABLE_BEGING;

    /*Ctrol*/
    GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
    GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
    GX101Slaver->_ctrl.down._fcb = GX101Slaver_CTRL_FCB_OPPO_NONE;
    GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_FCV_DISABLE;

    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_RESET_LINK;

    GX101Slaver->_addr = addr;
    GX101Slaver->_cs = GX101Slaver_Sendbuf[1]+GX101Slaver_Sendbuf[2]+GX101Slaver_Sendbuf[3];
    GX101Slaver->_end = GX101Slaver_STABLE_END;
//	LOG("addr is (%d)\n",GX101Slaver->_addr);
	len = 6;
	CK_send(DevNo,GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, GX101Slaver_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}
//
//
uint8_t GX101Slaver_BuildFinInit(int DevNo){

    uint16_t len = 0;
    uint8_t cs_temp = 0,i;

    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);

    len = GX101Slaver_STABLE_LEN;

    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
//    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
//    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_MASTER;
//    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_WITH_DATA;
//    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CANNOT_REC;
//    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = 0;
	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;

    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
    /*asdu*/
    asdu->_type = Iec10x_M_EI_NA_1;
    asdu->_num._num = 1;
    asdu->_reason = IEC10X_ASDU_REASON_INIT;
    asdu->_addr = GX101Slaver_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;
    info->_element[0] = 0;

    /*len*/
    len = GX101Slaver_VARIABLE_LEN + asdu->_num._num;
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;

	CK_send(DevNo,GX101Slaver_Sendbuf,len);
    //DumpHEX(GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len ,GX101Slaver_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;

}


uint8_t GX101Slaver_BuildActConfirm(int DevNo, uint8_t type, uint8_t qoi){

    uint16_t len = 0;
    uint8_t cs_temp = 0,i;

    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);

    len = GX101Slaver_STABLE_LEN;

    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;

    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
    /*asdu*/
    asdu->_type = type;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason = IEC10X_ASDU_REASON_ACTCON;
    asdu->_addr = GX101Slaver_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;
    info->_element[0] = qoi;
    /*len*/
    len = GX101Slaver_VARIABLE_LEN + asdu->_num._num;
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;

	CK_send(DevNo,GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, Prio, NULL,NULL);

    return RET_SUCESS;

}
//
uint8_t GX101Slaver_BuildActFinish(int DevNo, uint8_t qoi){

    uint16_t len = 0;
    uint8_t cs_temp = 0,i;

    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);

    len = GX101Slaver_STABLE_LEN;

    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
//    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
//    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_MASTER;
//    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_WITH_DATA;
//    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CANNOT_REC;
//    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;

    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_C_IC_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason = IEC10X_ASDU_REASON_ACTFIN;
    asdu->_addr = GX101Slaver_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;
    info->_element[0] = qoi;

    /*len*/
    len = GX101Slaver_VARIABLE_LEN + asdu->_num._num;
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;

	
	CK_send(DevNo,GX101Slaver_Sendbuf,len);
    //DumpHEX(GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, Prio, NULL,NULL);

    return RET_SUCESS;
}

uint8_t GX101Slaver_CallRefuse(int DevNo, uint8_t qoi){

    uint16_t len = 0;
    uint8_t cs_temp = 0,i;

    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);

    len = GX101Slaver_STABLE_LEN;

    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
//    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
//    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_MASTER;
//    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_WITH_DATA;
//    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CANNOT_REC;
//    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;

    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_C_IC_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason = IEC10X_ASDU_REASON_REFUSE;
    asdu->_addr = GX101Slaver_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;
    info->_element[0] = qoi;

    /*len*/
    len = GX101Slaver_VARIABLE_LEN + asdu->_num._num;
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;

	
	CK_send(DevNo,GX101Slaver_Sendbuf,len);
    //DumpHEX(GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, Prio, NULL,NULL);

    return RET_SUCESS;
}

uint8_t GX101Slaver_BuildResetProcess(int DevNo){

    uint16_t len = 0;
    uint8_t cs_temp = 0,i;

    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);

    len = GX101Slaver_STABLE_LEN;

    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
//    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
//    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_MASTER;
//    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_WITH_DATA;
//    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CANNOT_REC;
//    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;

	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;
    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_C_RP_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = 1;
    asdu->_reason = IEC10X_ASDU_REASON_ACTCON;
    asdu->_addr = GX101Slaver_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;
    info->_element[0] = 1;

    /*len*/
    len = GX101Slaver_VARIABLE_LEN + asdu->_num._num;
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;

	
	CK_send(DevNo,GX101Slaver_Sendbuf,len);
    //DumpHEX(GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, Prio, NULL,NULL);

    return RET_SUCESS;
}


//
uint8_t GX101Slaver_BuildSignal(int DevNo, uint8_t reason,uint8_t Prio){

    uint16_t len = 0;
    uint8_t cs_temp = 0, *ptr = NULL;
	uint16_t asdu_num=0;
	uint32_t YXNum;
	uint16_t i;
	int FlagSendOver = 0 , OffSet;

    /*init struct*/
    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);

	YXNum = gVars.TransYXTableNum;
//	log("_____YX_asdunum is (%d)\n",YXNum);

	for(i=0;i<YXNum;i++){
		if(ReadYxFlag(i) == Flag_CallAll)
			break;
	}
	if(i == YXNum){
		FlagSendOver = 1;
		return FlagSendOver;
	}
		
	OffSet = i;
	
    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
//    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
//    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_MASTER;
//    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_WITH_DATA;
//    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CANNOT_REC;
//    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;

	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;
	info->_addr = STARTSYX;
	if((YXNum - i)/127){
		asdu_num = 127;
	}
	else{
		asdu_num = YXNum - i;
		FlagSendOver = 1;
	}
	
	GX101Slaver->_addr = GX101Slaver_Sta_Addr;
	/*asdu*/
	asdu->_type = IEC10X_M_SP_NA_1;
	asdu->_num._sq = 1;
	asdu->_num._num = asdu_num;
	asdu->_reason = reason;

    /*info*/
    info->_addr = OffSet + STARTSYX;
	
    /*signal value*/
    ptr = info->_element;
	
	
    for(i = 0; i < asdu_num; i++){
		if((i + OffSet) > gVars.TransYXTableNum){
			perror("(i + OffSet) > gVars.TransYXTableNum\n");
			return 0;
		}
			
		SetYXFlag(i + OffSet, Flag_Clear);
		*ptr = ReadYx(i + OffSet);
        ptr++;
    }
    /*len*/
    len = ptr + 2 - GX101Slaver_Sendbuf;
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;

    //DumpHEX(GX101Slaver_Sendbuf,len);
    
	CK_send(DevNo,GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, Prio, NULL,NULL);
    return FlagSendOver;
}

uint8_t GX101Slaver_BuildDetect(int DevNo, uint8_t reason,uint8_t type, uint8_t Prio){

    uint16_t len = 0,asdu_addr;
    uint8_t cs_temp = 0, *ptr = NULL;
	uint16_t asdu_num=0;
	uint8_t ReSend_Flag = 0;//�ְ����ͱ�־λ
    PIEC10X_DETECT_T detect = NULL;
	PIEC10X_DETECT_DW_T detect_dw = NULL;
    PIEC10X_DETECT_F_T detect_f = NULL;
	uint16_t i;
	int FlagSendOver = 0 , OffSet;
	uint32_t YCNum;
	YCNum = gVars.TransYCTableNum;
//	log("_____YX_asdunum is (%d)\n",YCNum);

	for(i=0;i<YCNum;i++){
		if(ReadYcFlag(i) == Flag_CallAll)
			break;
	}
	if(i == YCNum){
		FlagSendOver = 1;
		return FlagSendOver;
	}
		
	OffSet = i;
	
    /*init struct*/
    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);

    /*get value*/
//    GX101Slaver->GetStationInfo(&addr,0,&asdu_num);//no menber
	asdu_num = gVars.TransYCTableNum;
//	log("_____asdunum is (%d)\n",asdu_num);
    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
//    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
//    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_MASTER;
//    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_NONE_DATA;
//    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CANNOT_REC;
//    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;

	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;
    GX101Slaver->_addr = GX101Slaver_Sta_Addr;

	if((YCNum - i)/40){
		asdu_num = 40;
	}
	else{
		asdu_num = YCNum - i;
		FlagSendOver = 1;
	}
	

    /*asdu*/
    asdu->_type = type;
    asdu->_num._sq = 1;
    asdu->_num._num = asdu_num;
    asdu->_reason = reason;
    asdu->_addr = GX101Slaver_Sta_Addr;
//	log("_______asdu->_num._num is(%d)\n",asdu_num);
	
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_DET + OffSet;
    /*Detect value*/
    ptr = info->_element;

    for(i=0; i<asdu_num; i++){
		SetYCFlag(i + OffSet, Flag_Clear);
        /*short int*/
        if(type == IEC10X_M_ME_NA_1){//��һ��
            detect = (PIEC10X_DETECT_T)ptr;
            detect->_detect = ReadYcData16(i + OffSet);
            detect->_qds = 0;
            ptr += sizeof(IEC10X_DETECT_T);
        }
		else if(type == IEC10X_M_TI_BD_1){//��Ȼ�
			detect_dw = (PIEC10X_DETECT_DW_T)ptr;
			detect_dw->_detect = ReadYcData32(i + OffSet);
			detect_dw->_qds = 0;
			ptr += sizeof(IEC10X_DETECT_DW_T);
		}
		
        /*float*/
        else if(type == IEC10X_M_ME_NC_1){//�̸���
        	
			detect_dw = (PIEC10X_DETECT_DW_T)ptr;
			detect_dw->_detect = ReadYcData32(i + OffSet);
			detect_dw->_qds = 0x40;
			ptr += sizeof(IEC10X_DETECT_DW_T);

			
        }
    }
	
    /*len*/
    len = ptr + 2 - GX101Slaver_Sendbuf;                     /* add cs+end*/
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/
    /*end*/
    for(i=4; i<len-2; i++){
//		log("cs[%x] ",GX101Slaver_Sendbuf[i]);
        cs_temp += GX101Slaver_Sendbuf[i];
    }
	
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;
	
	CK_send(DevNo,GX101Slaver_Sendbuf,len);
    //DumpHEX(GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, Prio, NULL,NULL);

    return FlagSendOver;
}

uint8_t GX101Slaver_BuildDZ(int DevNo, uint8_t reason,uint8_t type){

    uint16_t len = 0;
    uint8_t cs_temp = 0, *ptr = NULL;
	uint16_t asdu_num=0;
	uint32_t Value;
	uint16_t i;
	int FlagSendOver = 0 , OffSet;
	uint32_t DZNum;
	Data_Value_T DataValue;
	
	DZNum = gVars.TransDZTableNum;

	for(i=0;i<DZNum;i++){
		if(ReadDzCallFlag(i) == Flag_CallAll)
			break;
	}
	if(i == DZNum){
		FlagSendOver = 1;
		return FlagSendOver;
	}
		
	OffSet = i;
	log("OffSet is (%d)\n",OffSet);
    /*init struct*/
    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);

    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;


	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;
    GX101Slaver->_addr = GX101Slaver_Sta_Addr;

	if((DZNum - i)/40){
		asdu_num = 40;
	}
	else{
		asdu_num = DZNum - i;
		FlagSendOver = 1;
	}

    /*asdu*/
    asdu->_type = type;
    asdu->_num._sq = 1;
    asdu->_num._num = asdu_num;
    asdu->_reason = reason;
    asdu->_addr = GX101Slaver_Sta_Addr;
//	log("_______asdu->_num._num is(%d)\n",asdu_num);
	
    /*info*/
    info->_addr = IEC10X_DZ_ADDR + OffSet;
    /*Detect value*/
    ptr = info->_element;
	

    for(i=0; i<asdu_num; i++){
		SetDZCallFlag(i + OffSet, Flag_Clear);
		if(type == IEC10X_TYPE_DZ){
			Value = ReadDzData32(i + OffSet);
			*(uint32_t *)ptr = Value;
			ptr += sizeof(uint32_t);
			if(Value > 0){
				DataValue._int32 = Value;
				log("DevNo(%d)(%d) value is %f\n", (i + OffSet)/114, (i + OffSet)%114, DataValue._float);
			}
		}

    }
	
    /*len*/
    len = ptr + 2 - GX101Slaver_Sendbuf;                     /* add cs+end*/
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/
    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
	
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;
	
	CK_send(DevNo,GX101Slaver_Sendbuf,len);

    return FlagSendOver;
}

uint8_t GX101Slaver_BuildBrustYX(int DevNo, uint16_t asdu_num,uint16_t StartAddr,PTBurstYXRecord asdu_data){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, *ptr = NULL;
    PTSysTime time_tmp = NULL;
	uint8_t sq = 0;

    /*init struct*/
    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);
	PASDU_INFO_8T info_8 = NULL;
	for(i=0;i<(asdu_num-1);i++){
		if((asdu_data[(StartAddr+i)%MAXYXRECORD].point + 1) == asdu_data[(StartAddr+i+1)%MAXYXRECORD].point){
			sq = 1;
		}
		else{
			sq = 0;
			break;
		}
	}
    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
//    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
//    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_MASTER;
//    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_WITH_DATA;
//    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CANNOT_REC;
//    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;

	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;
    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_M_SP_NA_1;
    asdu->_num._sq = sq;
    asdu->_num._num = asdu_num;
    asdu->_reason = IEC10X_COT_SPONT;
    asdu->_addr = GX101Slaver_Sta_Addr;

	
	if(asdu->_num._sq == 1){//����
		/*info*/
		info->_addr = asdu_data[StartAddr%MAXYXRECORD].point + STARTSYX;
		/*Detect value*/
		ptr = info->_element;
		for(i=0; i<asdu_num; i++){
			
		    *ptr = asdu_data[(StartAddr+i)%MAXYXRECORD].bStatus;
		    ptr++;
		}
	}
	else if(asdu->_num._sq == 0){//������
		ptr = (uint8_t *)&info->_addr;
		for(i=0; i<asdu_num; i++){
			info_8 = (PASDU_INFO_8T)ptr;
			info_8->_addr = asdu_data[(StartAddr+i)%MAXYXRECORD].point + STARTSYX;
			info_8->_Data8 = asdu_data[(StartAddr+i)%MAXYXRECORD].bStatus;//ң��״̬
			ptr += sizeof(ASDU_INFO_8T);
		}
	
	}

    /*len*/
    len = ptr + 2 - GX101Slaver_Sendbuf;
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;

	CK_send(DevNo,GX101Slaver_Sendbuf,len);
//    DumpHEX(GX101Slaver_Sendbuf,len);

    return RET_SUCESS;
}

uint8_t GX101Slaver_BuildBrustYXSoe(int DevNo, uint16_t asdu_num, uint8_t reason, uint16_t StartAddr,PTBurstYXRecord asdu_data){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, *ptr = NULL;
    PTSysTime time_tmp = NULL;
	uint8_t sq = 0;

    /*init struct*/
    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);
	PASDU_101SOE_T info_8 = NULL;
	PCP56Time2a_T time = NULL;
    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
//    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
//    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_MASTER;
//    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_WITH_DATA;
//    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CANNOT_REC;
//    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;

	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;

    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_M_SP_TB_1;
    asdu->_num._sq = sq;
    asdu->_num._num = asdu_num;
    asdu->_reason = reason;
    asdu->_addr = GX101Slaver_Sta_Addr;

	
	ptr = (uint8_t *)&info->_addr;
	for(i=0; i<asdu_num; i++){
		info_8 = (PASDU_101SOE_T)ptr;
		info_8->_addr = asdu_data[(StartAddr+i)%MAXYXRECORD].point + STARTSYX;
		info_8->_value = asdu_data[(StartAddr+i)%MAXYXRECORD].bStatus;//ң��״̬
		time = (PCP56Time2a_T)info_8->_element;
		*time = asdu_data[(StartAddr+i)%MAXYXRECORD].stTime;
		ptr += (sizeof(PASDU_101SOE_T)+sizeof(CP56Time2a_T)-1);
	}
	

    /*len*/
    len = ptr + 2 - GX101Slaver_Sendbuf;
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;

	CK_send(DevNo,GX101Slaver_Sendbuf,len);
//    DumpHEX(GX101Slaver_Sendbuf,len);

    return RET_SUCESS;
}

uint8_t GX101Slaver_BuildOldYXSoe(int DevNo, PTBurstYXRecord OldYXSoe){
	int SendOverFlag = 0;
	uint16_t RemainNum = 0;
	uint16_t SendNum = 0;
	
	if((OldSoeEndAddr - OldSoeStartAddr) >= 20){
		SendNum = 20;
		PRINT_FUNLINE;
	}
	else if(OldSoeEndAddr == 0){
		SendOverFlag = 1;
		PRINT_FUNLINE;
		log("SendOverFlag(%d)\n",SendOverFlag);
		return SendOverFlag;
	}else if((OldSoeEndAddr - OldSoeStartAddr) < 20){
		PRINT_FUNLINE;
		SendNum = OldSoeEndAddr - OldSoeStartAddr;
	}
	GX101Slaver_BuildBrustYXSoe(DevNo, SendNum, IEC10X_COT_GROUP16, OldSoeStartAddr, YXSoeRecord);
	
	OldSoeStartAddr += SendNum;
	if(OldSoeStartAddr = OldSoeEndAddr){
		PRINT_FUNLINE;
		SendOverFlag = 1;
	}
    return SendOverFlag;
}


uint8_t GX101Slaver_BuildBrustYC(int DevNo, uint16_t asdu_num,uint16_t StartAddr,PTBurstYCRecord asdu_data){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, *ptr = NULL;
    PIEC10X_DETECT_DW_T Value = NULL;
	PASDU_INFO_32T Value_Addr = NULL;
	uint8_t sq = 0;

    /*init struct*/
    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);
	
	for(i=0;i<(asdu_num-1);i++){
		if((asdu_data[(StartAddr+i)%MAXYCRECORD].point + 1) == asdu_data[(StartAddr+i+1)%MAXYCRECORD].point){
			sq = 1;
		}
		else{
			sq = 0;
			break;
		}
	}
    /*get value*/
    len = GX101Slaver_STABLE_LEN;

    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
//    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
//    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_MASTER;
//    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_NONE_DATA;
//    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CANNOT_REC;
//    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;

	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;
    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_M_ME_NC_1;
    asdu->_num._sq = sq;
    asdu->_num._num = asdu_num;
    asdu->_reason = IEC10X_COT_SPONT;
    asdu->_addr = GX101Slaver_Sta_Addr;


    /*Detect value*/
	if(asdu->_num._sq == 1){//����
		/*info addr*/
		info->_addr = asdu_data[StartAddr%MAXYCRECORD].point + STARTSYC;
		/*Detect value*/
		ptr = info->_element;
		for(i=0; i<asdu_num; i++){
			Value = (PIEC10X_DETECT_DW_T)ptr;
		    Value->_detect = asdu_data[(StartAddr+i)%MAXYCRECORD].Value32;
			Value->_qds = 0x40;
		    ptr +=sizeof(IEC10X_DETECT_DW_T);
		}
	}
	else if(asdu->_num._sq == 0){//������
		ptr = (uint8_t *)&info->_addr;
		for(i=0; i<asdu_num; i++){
			Value_Addr = (PASDU_INFO_32T)ptr;
			Value_Addr->_addr = asdu_data[(StartAddr+i)%MAXYCRECORD].point + STARTSYC;
			Value_Addr->_Data32 = asdu_data[(StartAddr+i)%MAXYCRECORD].Value32;
			Value_Addr->_qds = 0x40;
			ptr += sizeof(ASDU_INFO_32T);
		}
	
	}

    /*len*/
    len = ptr + 2 - GX101Slaver_Sendbuf;                     /* add cs+end*/
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;

	CK_send(DevNo,GX101Slaver_Sendbuf,len);

    return RET_SUCESS;
}

uint8_t GX101Slaver_BuildBrustDZ(int DevNo, uint16_t asdu_num, uint16_t StartAddr,PTBurstYCRecord asdu_data){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, *ptr = NULL;
	uint8_t sq = 0;
	uint16_t temp16;

    /*init struct*/
    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);
	
	for(i=0;i<(asdu_num-1);i++){
		if((asdu_data[(StartAddr+i)%MAXYCRECORD].point + 1) == asdu_data[(StartAddr+i+1)%MAXYCRECORD].point){
			sq = 1;
		}
		else{
			sq = 0;
			break;
		}
	}
    /*get value*/
    len = GX101Slaver_STABLE_LEN;

    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;
    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_TYPE_DZ_YZ;
    asdu->_num._sq = sq;
    asdu->_num._num = asdu_num;
    asdu->_reason = IEC10X_COT_SPONT;
    asdu->_addr = GX101Slaver_Sta_Addr;


    /*Detect value*/
	if(asdu->_num._sq == 1){//����
		/*info addr*/
		info->_addr = asdu_data[StartAddr%MAXYCRECORD].point + STARTSDZ;
		/*Detect value*/
		ptr = info->_element;
		for(i=0; i<asdu_num; i++){
		    *(uint32_t *)ptr = asdu_data[(StartAddr+i)%MAXYCRECORD].Value32;
		    ptr += 4;
		}
	}
	else if(asdu->_num._sq == 0){//������
		ptr = (uint8_t *)&info->_addr;
		for(i=0; i<asdu_num; i++){
			temp16 = asdu_data[(StartAddr+i)%MAXYCRECORD].point + STARTSDZ;
		    *(uint16_t *)ptr = temp16;
			ptr += 2;
		    *(uint32_t *)ptr = asdu_data[(StartAddr+i)%MAXYCRECORD].Value32;
		    ptr += 4;
		}
	
	}	
	*ptr = 6;//��ֵ������
	ptr++;
    /*len*/
    len = ptr + 2 - GX101Slaver_Sendbuf;                     /* add cs+end*/
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;

	CK_send(DevNo,GX101Slaver_Sendbuf,len);

    return RET_SUCESS;
}

uint8_t GX101Slaver_Build_DZ_Return(int DevNo)
{
	int i, len, num = 0, old = -1, sq = 1;
	int offset;
	uint16_t Temp16;
	PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
	PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
	PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);
	uint8_t *ptr = NULL;
	uint8_t cs_temp = 0;

	
	for(i = 0; i < gVars.TransDZTableNum; i++){
		if(ReadDzFlag(i) == Flag_DZ_RET){
			if(old == -1){
				old = i;
			}else if((old+1) != i){
				sq = 0;
			}
			old = i;
			num++;
		}
	}
	if(num == 0)
		return 0;

	/*head*/
	GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

	/*Ctrol*/
	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;
	
	GX101Slaver->_addr = GX101Slaver_Sta_Addr;
	/*asdu*/
	asdu->_type = IEC10X_TYPE_DZ_DATA;
	asdu->_num._sq = sq;
	asdu->_num._num = num;
	asdu->_reason = IEC10X_COT_ACTCON;
	asdu->_addr = GX101Slaver_Sta_Addr;
	
	if(sq){//����
		for(i = 0; i < gVars.TransDZTableNum; i++){
			if(ReadDzFlag(i) == Flag_DZ_RET){
				break;
			}
		}
		offset = i;
		Temp16 = i + STARTSDZ;
		info->_addr = Temp16;
		ptr = info->_element;
		for(i = 0 ; i < num; i++){
			*(int32_t *)ptr = ReadDzYZData32(offset+i);
			log("dz sq1 point is (%d)\n", offset+i);
			SetDZFlag(offset + i, Flag_Clear);
			ptr += 4;
		}
		
	}else{//������
		ptr = (uint8_t *)&(info->_addr);
		for(i = 0; i < gVars.TransDZTableNum; i++){
			if(ReadDzFlag(i) == Flag_DZ_RET){
				SetDZFlag(i, Flag_Clear);
				Temp16 = i + STARTSDZ;
				*(uint16_t *)ptr = Temp16;
				ptr += 2;
				*(int32_t *)ptr = ReadDzYZData32(i);
				ptr += 4;
				log("dz sq0  point is (%d)\n", i);
			}
		}
	}
	
	*ptr = 9;
	ptr++;
	
    /*len*/
    len = ptr + 2 - GX101Slaver_Sendbuf;                     /* add cs+end*/
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/
    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;

	log("len is GX101Slaver->_len(%d) fd(%d)\n",len, gpDevice[DevNo].fd);
	CK_send(DevNo,GX101Slaver_Sendbuf,len);

}

static int Check_Brust_Yc(void)
{
	int YCNum = 0;
	uint16_t i;
	uint32_t Value;

	
	for(i=0;i<gVars.TransYCTableNum;i++){
		if(ReadYcFlag(i) == Flag_Spon){
			Value = ReadYcData32(i);
//			SetYCFlag(i, Flag_Clear);
			YCNum++;
		}
	}
	return YCNum;
}

//ͻ��ң��
uint8_t GX101laver_BuildBrustYC(int DevNo,uint8_t ValueType){

    uint8_t len = 0;
    uint8_t cs_temp = 0, i, asdu_num = 0, *ptr = NULL;
    PIEC10X_DETECT_F_T detect = NULL;
    PCP56Time2a_T time = NULL;
	int YCNum = 0;
	int YCSendNum = 20;
	uint16_t Temp16= 0;
	PIEC10X_DETECT_DW_T detect32 = NULL;
	TBurstAI burstAI;
	/*get yc num*/
  	YCNum= GetBurstAINum(DevNo);
//	log("YCNum is (%d)\n",YCNum);
	if(YCNum == 0){
		return 0;
	}
	if(YCNum >= YCSendNum){
		YCSendNum = 20;
	}else{
		YCSendNum = YCNum;
	}
    /*init struct*/
    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);


    len = GX101Slaver_STABLE_LEN;

    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
//    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
//    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_MASTER;
//    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_NONE_DATA;
//    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CANNOT_REC;
//    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;

	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;
    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
	
    /*asdu*/
    asdu->_type = ValueType;
    asdu->_num._sq = 0;
    asdu->_num._num = YCSendNum;
    asdu->_reason = IEC10X_COT_SPONT;
    asdu->_addr = gpDevice[DevNo].Address;

	ptr = (uint8_t *)&(info->_addr);
	for(i=0;i<YCSendNum;i++){
		if(ReadBurstAI(DevNo, &burstAI) != TRUE)
			break;
		Temp16 = YC_START_ADDR + burstAI.dwNo;
		memcpy(ptr, &Temp16, 2);
		ptr += 2;
		detect32 = (PIEC10X_DETECT_DW_T)ptr;
		detect32->_detect = burstAI.sValue;
		detect32->_qds = 0;
		ptr += sizeof(IEC10X_DETECT_F_T);
	}

    /*len*/
    len = ptr + 2 - GX101Slaver_Sendbuf;                     /* add cs+end*/
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/
    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;
	CK_send(DevNo, GX101Slaver_Sendbuf,len);
    //DumpHEX(GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, GX101Slaver_PRIO_SPON, NULL,NULL);

    return 1;
}

uint8_t GX101Slaver_BuildCallQoi(int DevNo, uint8_t qoi){
	uint8_t SendOverFlag = 0;
	switch(qoi){
		case IEC10X_CALL_QOI_GROUP1:
		case IEC10X_CALL_QOI_GROUP2:
		case IEC10X_CALL_QOI_GROUP9:
		case IEC10X_CALL_QOI_GROUP10:
			LOG("IEC10X_CALL_QOI_GROUP\n");
			gpDevice[DevNo].STATE_FLAG_GROUP = GX101Slaver_FLAG_CALL_ACT_RET;
			GX101Slaver_Call_Qoi = qoi;
			SendOverFlag = 1;
			break;
		case IEC10X_CALL_QOI_GROUP16:
			GX101Slaver_Call_Qoi = qoi;
			SendOverFlag = GX101Slaver_BuildOldYXSoe(DevNo, YXSoeRecord);
			log("SendOverFlag(%d)\n",SendOverFlag);
			break;
			
		default:
			LOG("-%s- call cmd error qoi(%d) \n", __FUNCTION__,qoi);
			PRINT_FUNLINE;
			SendOverFlag = 1;
			return SendOverFlag;
	}
	return SendOverFlag;

}
uint8_t GX101Slaver_BuildDelayAct(int DevNo, uint16_t delay_time){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, asdu_num = 0, *ptr = NULL;
    uint16_t cp16time2a = delay_time;

    /*init struct*/
    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);

    /*get value*/
    asdu_num = 1;

    len = GX101Slaver_STABLE_LEN;

    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
//    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
//    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_MASTER;
//    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_NONE_DATA;
//    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CANNOT_REC;
//    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;

	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;
    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_C_CD_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason = IEC10X_COT_ACTCON;
    asdu->_addr = GX101Slaver_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;

    /*delay value*/
    ptr = info->_element;
    *(uint16_t *)ptr = IEC10X_Cp16time2a;
    ptr+=2;

    /*len*/
    len = ptr + 2 - GX101Slaver_Sendbuf;
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;

	
	CK_send(DevNo,GX101Slaver_Sendbuf,len);
    //DumpHEX(GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, GX101Slaver_PRIO_DELAY, NULL,NULL);

    return RET_SUCESS;
}
uint8_t GX101Slaver_BuildClockAct(int DevNo){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, asdu_num = 0, *ptr = NULL;

    /*init struct*/
    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);
    PCP56Time2a_T time = (PCP56Time2a_T)(info->_element);

    /*get value*/
    asdu_num = 1;

    len = GX101Slaver_STABLE_LEN;

    /*head*/
    GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

    /*Ctrol*/
//    GX101Slaver->_ctrl.up._dir = GX101Slaver_CTRL_DIR_UP;
//    GX101Slaver->_ctrl.up._prm = GX101Slaver_CTRL_PRM_MASTER;
//    GX101Slaver->_ctrl.up._acd = GX101Slaver_CTRL_ACD_NONE_DATA;
//    GX101Slaver->_ctrl.up._dfc = GX101Slaver_CTRL_DFC_CANNOT_REC;
//    GX101Slaver->_ctrl.up._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;

	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;
	
    GX101Slaver->_addr = GX101Slaver_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_C_CS_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason = IEC10X_COT_ACTCON;
    asdu->_addr = GX101Slaver_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;

    /*clock value*/
    ptr = info->_element;
//    time;
    IEC10X->GetTime(time);//��ȡϵͳʱ�䣬�д�����
	time->_milliseconds = (Rec_milliseconds + IEC10X_Cp16time2a_V);
    ptr+=sizeof(CP56Time2a_T);

    /*len*/
    len = ptr + 2 - GX101Slaver_Sendbuf;
    GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/
    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += GX101Slaver_Sendbuf[i];
    }
    GX101Slaver_Sendbuf[len-2] = cs_temp;
    GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;

	CK_send(DevNo,GX101Slaver_Sendbuf,len);
    //DumpHEX(GX101Slaver_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len, GX101Slaver_PRIO_CLOCK, NULL,NULL);

    return RET_SUCESS;
}

uint8_t GX101Slaver_BuildYkReturn(int DevNo,uint8_t qoi,uint8_t reason, YK_INFO_T Yk_info){

	uint16_t len = 0;
	uint8_t cs_temp = 0,i;

	PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Sendbuf;
	PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
	PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);
	uint8_t *ptr = NULL;

	len = GX101Slaver_STABLE_LEN;

	/*head*/
	GX101Slaver->_begin = GX101Slaver->_begin_cfm = GX101Slaver_VARIABLE_BEGING;

	/*Ctrol*/
	GX101Slaver->_ctrl.down._dir = GX101Slaver_CTRL_DIR_UP;
	GX101Slaver->_ctrl.down._prm = GX101Slaver_CTRL_PRM_MASTER;
	GX101Slaver->_ctrl.down._fcb = gpDevice[DevNo].fcb&0x1;
	GX101Slaver->_ctrl.down._fcv = GX101Slaver_CTRL_DFC_CANNOT_REC;
	GX101Slaver->_ctrl.down._func = GX101Slaver_CTRL_SEND_USR_DATA_ACK;
	gpDevice[DevNo].fcb = ~gpDevice[DevNo].fcb;
	
	GX101Slaver->_addr = GX101Slaver_Sta_Addr;
	/*asdu*/
	asdu->_type = qoi;
	asdu->_num._num = 1;
	asdu->_reason = reason;
	asdu->_addr = GX101Slaver_Sta_Addr;
	/*info*/
	info->_addr = Yk_info._addr + STARTSYK;

    ptr = info->_element;
	memcpy(ptr, &Yk_info._YKData, 1);

	/*len*/
	len = GX101Slaver_VARIABLE_LEN + asdu->_num._num;
	GX101Slaver->_len = GX101Slaver->_len_cfm = len-4-2;			/*-start-len-len-start	 -cs-end*/

	/*end*/
	for(i=4; i<len-2; i++){
		cs_temp += GX101Slaver_Sendbuf[i];
	}
	GX101Slaver_Sendbuf[len-2] = cs_temp;
	GX101Slaver_Sendbuf[len-1] = GX101Slaver_VARIABLE_END;
	usleep(5000);
	CK_send(DevNo,GX101Slaver_Sendbuf,len);
//	DumpHEX(GX101Slaver_Sendbuf,len);
	/* enqueue to the transmisson queue */
//	  GX101Slaver_Enqueue(GX101Slaver_Sendbuf, len ,GX101Slaver_PRIO_INITLINK, NULL,NULL);



    return RET_SUCESS;
}

int GX101Slaver_Send_Brust_Info(int DevNo){
	//���ٹ����н�ֹ��ң��ң��
	if( gpDevice[DevNo].Respon_Confirm == 1 && gpDevice[DevNo].STATE_FLAG_CALLALL == GX101Slaver_FLAG_LINK_IDLE){
	    if(gpDevice[DevNo].TimeCnt%1 == 0){
			if(GX101ScanBrustData(DevNo)){
				gpDevice[DevNo].Respon_Confirm = 0;
				return 1;
			}
	    }
		
	    if(gpDevice[DevNo].TimeCnt%5 == 0){
			if(GX101laver_BuildBrustYC(DevNo, IEC10X_M_ME_NC_1)){
				gpDevice[DevNo].Respon_Confirm = 0;
				return 1;
			}
	    }
	}

}
int GX101Slaver_ReSend_Data(int DevNo){

	if((gpDevice[DevNo].TimeCnt - gpDevice[DevNo].ReSendOldTime) > 4 && gpDevice[DevNo].ReSendFlag == RESEND_YES){
		log("GX101Slaver_ReSend_Data\n");
		CK_send(DevNo, GX101Slaver_ReSendbuf, gpDevice[DevNo].ReSendDataLen);
		gpDevice[DevNo].ReSendCnt++;
		gpDevice[DevNo].ReSendOldTime = gpDevice[DevNo].TimeCnt; 
	}

	if(gpDevice[DevNo].ReSendCnt > 4){
		gpDevice[DevNo].Flag_Link_Ready = LinkOverTime;
		gpDevice[DevNo].ReSendFlag = RESEND_NO;
		gpDevice[DevNo].ReSendCnt = 0;
	}
}

int GX101Slaver_ASDU_Call(int DevNo, PIEC10X_ASDU_101T GX101Slaver_Asdu){

    PASDU_INFO_101T asdu_info = (PASDU_INFO_101T)(GX101Slaver_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];

    if(asdu_info->_addr != 0){
        LOG("-%s- call cmd active error addr(%x) \n" ,__FUNCTION__ ,asdu_info->_addr);
        return RET_ERROR;
    }
//	LOG("GX101Slaver_Asdu->_reason is (%02x)\n",GX101Slaver_Asdu->_reason);
	LOG("qoi (%02x)\n",qoi);
	
    switch(GX101Slaver_Asdu->_reason){

        case IEC10X_ASDU_REASON_ACT:
			GX101Slaver_Call_Qoi = qoi;
            switch(qoi){
                case IEC10X_CALL_QOI_TOTAL:
					LOG("IEC10X_CALL_QOI_TOTAL\n");
                    gpDevice[DevNo].STATE_FLAG_CALLALL = GX101Slaver_FLAG_CALL_ACT_RET;
                    break;
                case IEC10X_CALL_QOI_GROUP1:
                case IEC10X_CALL_QOI_GROUP2:
                case IEC10X_CALL_QOI_GROUP9:
                case IEC10X_CALL_QOI_GROUP10:
					LOG("IEC10X_CALL_QOI_GROUP\n");
                    gpDevice[DevNo].STATE_FLAG_GROUP = GX101Slaver_FLAG_CALL_ACT_RET;
                    break;
				case IEC10X_CALL_QOI_GROUP16:
                    gpDevice[DevNo].STATE_FLAG_GROUP = GX101Slaver_FLAG_CALL_ACT_RET;
					OldSoeEndAddr = gpDevice[DevNo].ReadYXSoePtr;
					if(OldSoeEndAddr >= 100){
						OldSoeStartAddr = OldSoeEndAddr - 100;
					}else{
						OldSoeStartAddr = 0;
					}
					log("OldSoeEndAddr(%d)(%d)\n",OldSoeEndAddr,OldSoeStartAddr);
					break;
					
                default:
                    gpDevice[DevNo].STATE_FLAG_GROUP = GX101Slaver_FLAG_CALL_REFUSE;
            }
            GX101Slaver_ResConfirm(DevNo);
            break;

        default:
            LOG("-%s- call cmd error reason(%d) \n", __FUNCTION__,GX101Slaver_Asdu->_reason);
            break;
    }
    return RET_SUCESS;
}

int GX101Slaver_ASDU_DZCall(int DevNo, PIEC10X_ASDU_101T GX101Slaver_Asdu){

    PASDU_INFO_101T asdu_info = (PASDU_INFO_101T)(GX101Slaver_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];

    if(asdu_info->_addr != 0){
        LOG("-%s- call cmd active error addr(%x) \n" ,__FUNCTION__ ,asdu_info->_addr);
        return RET_ERROR;
    }
//	LOG("GX101Slaver_Asdu->_reason is (%02x)\n",GX101Slaver_Asdu->_reason);
	LOG("qoi (%02x)\n",qoi);
	
    switch(GX101Slaver_Asdu->_reason){

        case IEC10X_ASDU_REASON_ACT:
			GX101Slaver_Call_Qoi = qoi;
            switch(qoi){
                case IEC10X_CALL_QOI_TOTAL:
					LOG("IEC10X_CALL_QOI_TOTAL\n");
                    gpDevice[DevNo].STATE_FLAG_DZCALLALL = GX101Slaver_FLAG_CALL_ACT_RET;
                    break;

                default:
					perror("wrong asdu reason (0x:%2x)\n", GX101Slaver_Asdu->_reason);
            }
            GX101Slaver_ResConfirm(DevNo);
            break;

        default:
            LOG("-%s- call cmd error reason(%d) \n", __FUNCTION__,GX101Slaver_Asdu->_reason);
            break;
    }
    return RET_SUCESS;
}

int GX101Slaver_ASDU_DZ_YZ(int DevNo, PGX101Slaver_68_T GX101Slaver_68){

    PIEC10X_ASDU_101T GX101Slaver_Asdu = NULL;
    GX101Slaver_Asdu = (PIEC10X_ASDU_101T)GX101Slaver_68->_asdu;
    PASDU_INFO_101T asdu_info = (PASDU_INFO_101T)(GX101Slaver_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];

	GX101Slaver_ResConfirm(DevNo);
    switch(GX101Slaver_Asdu->_reason){
        case IEC10X_ASDU_REASON_ACT:
			GX101Slaver_Deal_DZ_YZ(DevNo, GX101Slaver_Asdu);
            break;
        case IEC10X_COT_STOPACT:
			GX101Slaver_Send_Mirror(DevNo, GX101Slaver_68, IEC10X_ASDU_REASON_REFUSEACT);
            break;

        default:
            LOG("-%s- call cmd error reason(%d) \n", __FUNCTION__,GX101Slaver_Asdu->_reason);
            break;
    }
    return RET_SUCESS;
}

int GX101Slaver_ASDU_DZ_JH(int DevNo, PIEC10X_ASDU_101T GX101Slaver_Asdu){

    PASDU_INFO_101T asdu_info = (PASDU_INFO_101T)(GX101Slaver_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];
	int Dest_DevID;//Ԥ�ö���ID��
	
    switch(GX101Slaver_Asdu->_reason){
        case IEC10X_ASDU_REASON_ACT:
            GX101Slaver_ResConfirm(DevNo);
			GX101Slaver_BuildActConfirm(DevNo, IEC10X_TYPE_DZ_JH,1);
			Dest_DevID = gpDevice[DevNo].DZ_YZ_DEVID;
			if(gpDevice[Dest_DevID].SetDZ != NULL)
				gpDevice[Dest_DevID].SetDZ(Dest_DevID);
            break;


        default:
            LOG("-%s- call cmd error reason(%d) \n", __FUNCTION__,GX101Slaver_Asdu->_reason);
            break;
    }
    return RET_SUCESS;
}

int GX101Slaver_ASDU_Delay(int DevNo){

    /*init struct*/
    PGX101Slaver_68_T GX101Slaver = (PGX101Slaver_68_T)GX101Slaver_Recbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(GX101Slaver->_asdu);
    PASDU_INFO_101T asdu_info = (PASDU_INFO_101T)(asdu->_info);


    if(asdu_info->_addr != 0){
        perror("delay cmd error addr\n");
        return RET_ERROR;
    }

    switch(asdu->_reason){

        case IEC10X_COT_ACT:
            IEC10X_Cp16time2a = *(uint16_t *)(asdu_info->_element);
            LOG("-%s- delay cmd (0x%x%x)(%d)ms \n", __FUNCTION__,asdu_info->_element[0],asdu_info->_element[1],IEC10X_Cp16time2a);
            gpDevice[DevNo].STATE_FLAG_DELAY = GX101Slaver_FLAG_DELAY_ACT;
            GX101Slaver_ResConfirm(DevNo);
            GX101Slaver_BuildDelayAct(DevNo, IEC10X_Cp16time2a);
            break;
        case IEC10X_COT_SPONT:
            IEC10X_Cp16time2a_V = *(uint16_t *)(asdu_info->_element);
            LOG("-%s- delay cmd delay value(%d)ms \n", __FUNCTION__,IEC10X_Cp16time2a_V);
            GX101Slaver_ResConfirm(DevNo);
			gpDevice[DevNo].Respon_Confirm = 1;//�յ���վͻ���ı��ģ���������ñ�־λ����Ҫǿ����Ϊ1
            break;
        default:
            LOG("-%s- delay cmd error reason(%x) \n", __FUNCTION__,asdu->_reason);
            break;
    }
    return RET_SUCESS;
}

int GX101Slaver_ASDU_CLOCK(int DevNo, PIEC10X_ASDU_101T GX101Slaver_Asdu){

    PASDU_INFO_101T asdu_info = (PASDU_INFO_101T)(GX101Slaver_Asdu->_info);
    PCP56Time2a_T time = (PCP56Time2a_T)(asdu_info->_element);

    memcpy(&IEC10X_Cp56time2a,asdu_info->_element, sizeof(CP56Time2a_T));


    if(asdu_info->_addr != 0){
        LOG("-%s- Clock cmd error addr(%d) \n", __FUNCTION__,asdu_info->_addr);
		LOG("asdu_info->_addr is (%d)\n",asdu_info->_addr);
        return RET_ERROR;
    }

    switch(GX101Slaver_Asdu->_reason){

        case IEC10X_COT_ACT:
//            LOG("-%s- Clock cmd (20%d-%d-%d %d %d:%d:%d) delay(%d) \n", __FUNCTION__,IEC10X_Cp56time2a._year._year,IEC10X_Cp56time2a._month._month,IEC10X_Cp56time2a._day._dayofmonth,
//                    IEC10X_Cp56time2a._day._dayofweek,IEC10X_Cp56time2a._hour._hours,IEC10X_Cp56time2a._min._minutes,IEC10X_Cp56time2a._milliseconds,IEC10X_Cp16time2a_V);
            IEC10X_Cp56time2a._milliseconds += IEC10X_Cp16time2a_V;
			Rec_milliseconds = IEC10X_Cp56time2a._milliseconds;
			
            IEC10X->SetTime(&IEC10X_Cp56time2a);//�����ͼ�ʱ��
            GX101Slaver_ResConfirm(DevNo);
			gpDevice[DevNo].STATE_FLAG_CLOCK = GX101Slaver_FLAG_CLOCK_SYS;
            break;
        case IEC10X_COT_SPONT:
            LOG("-%s- Clock cmd spont \n", __FUNCTION__);
            break;
        default:
            LOG("-%s- Clock cmd error reason(%d) \n", __FUNCTION__,GX101Slaver_Asdu->_reason);
            break;
    }
    return RET_SUCESS;
}

uint8_t GX101Slaver_Deal_Yk(int DevNo, PIEC10X_ASDU_101T GX101Slaver_Asdu){

	PASDU_INFO_YK_T asdu_info = (PASDU_INFO_YK_T)(GX101Slaver_Asdu->_info);
	uint16_t YKDevID;//ң��Ŀ���豸
	uint16_t YKIndex;//

	YKIndex = asdu_info->_addr - STARTSYK;
	//����ң�ص�ַ
	gpDevice[DevNo].YKInfo._addr = YKIndex;
	gpDevice[DevNo].YKInfo._YKData = asdu_info->_Data8;
	if(YKIndex >= gVars.TransYKTableNum){
		perror("YKIndex > gVars.TransYKTableNum,YKID (%d)\n",YKIndex);
		gpDevice[DevNo].YKInfo._Return._refuse = 1;
		sprintf(gpDevice[DevNo].logbuf, "GX101Slaver YKIndex > gVars.TransYKTableNum \n");
		write_log(gpDevice[DevNo].logbuf, LOGMARNING, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	YKDevID = TransYKTable[YKIndex].wRealID;
	
	if(YKDevID >= MAX_SLAVE_DEVICE_NUM ){
		PERROR("YKID > MaxDeviceID!,YKID (%d)\n",YKDevID);
		gpDevice[DevNo].YKInfo._Return._refuse = 1;
		return -1 ;
	}
	if(gpDevice[YKDevID].SetYK == NULL){
		perror("gpDevice[YKDevID].SetYK == NULL, protocol(%s)\n", gpDevice[DevNo].Protocol);
		return -1;
	}
	gpDevice[YKDevID].SetYK(YKDevID, GX101Slaver_Asdu->_type, (uint16_t)GX101Slaver_Asdu->_reason, *(uint8_t *)&asdu_info->_Data8);
	gpDevice[YKDevID].YK_FROM_ID = DevNo;

	return RET_SUCESS;

}


int GX101Slaver_Deal_10(int DevNo, uint8_t *buf, uint16_t len){

    uint8_t cfun, sta,i,dir;
    uint8_t cs_temp = 0;
    PGX101Slaver_10_T GX101Slaver_10 = NULL;

    GX101Slaver_10 = (PGX101Slaver_10_T)buf;
    /* check check_sum*/
    for(i=1; i<len-2; i++){
        cs_temp += buf[i];
    }
    if(GX101Slaver_10->_cs != cs_temp){
        LOG("-%s-,check sum error(%x) \n",__FUNCTION__,cs_temp);
        return RET_ERROR;
    }
    /* check end of the frame*/
    if(GX101Slaver_10->_end != 0x16){
        LOG("-%s-,GX101Slaver_10 end error(%d) \n",__FUNCTION__,GX101Slaver_10->_end);
        return RET_ERROR;
    }
    /*deal the function code*/
    cfun = GX101Slaver_10->_ctrl.down._func;
    sta = GX101Slaver_10->_ctrl.down._prm;
	dir = GX101Slaver_10->_ctrl.down._dir;
	GX101Slaver_Sta_Addr = GX101Slaver_10->_addr;
	
//	LOG("cfun is (%d)\n",cfun);
//	LOG("sta is (%d)\n",sta);
    //GX101Slaver->GetLinkAddr();
#ifdef GX101SlaverLOCK
//	LOG("-%s-,line is (%d) \n",__FUNCTION__,__LINE__);
    if(IEC10X->LOCK != NULL)
        IEC10X->LOCK();
	LOG("-%s-,line is (%d) \n",__FUNCTION__,__LINE__);
#endif
    if(sta == GX101Slaver_CTRL_PRM_MASTER){
        switch(cfun){
            case GX101Slaver_CTRL_RESET_LINK:
                LOG("++++Reset link... \n");
                gpDevice[DevNo].STATE_FLAG_INIT = GX101Slaver_FLAG_RESET_LINK;
                break;
            case GX101Slaver_CTRL_PULSE:
//                LOG("++++PULSE... \n");
				if(gpDevice[DevNo].Flag_Link_Ready == LinkReady){
                	GX101Slaver_ResConfirm(DevNo);
				}
                
                break;
            case GX101Slaver_CTRL_SEND_DATA:
//                LOG("++++Send data... \n");
                break;
            case GX101Slaver_CTRL_REQ_LINK_STATUS://������·
                LOG("++++Request link status...DevNo(%d)\n", DevNo);
				gpDevice[DevNo].STATE_FLAG_INIT = GX101Slaver_FLAG_LINK_CLOSED;
				gpDevice[DevNo].STATE_FLAG_CALLALL = GX101Slaver_FLAG_LINK_CLOSED;
				gpDevice[DevNo].STATE_FLAG_DZCALLALL = GX101Slaver_FLAG_LINK_CLOSED;
				gpDevice[DevNo].ReSendFlag = RESEND_NO;
			
				GX101Slaver_RetStatusOk(DevNo, GX101Slaver_Sta_Addr);
				sprintf(gpDevice[DevNo].logbuf, "GX101Slaver Start link \n");
				write_log(gpDevice[DevNo].logbuf, LOGDEBUG, __FILE__, __FUNCTION__, __LINE__);
                break;

            default:
                LOG("++++error DOWN function code (%d)... \n",cfun);
                break;
        }
    }else if(sta == GX101Slaver_CTRL_PRM_SLAVE){//�Ӷ�վ
//		LOG("-%s-,line is (%d) \n",__FUNCTION__,__LINE__);
	    switch(cfun){
	        case GX101Slaver_CTRL_RESPONSE_LINK_STATUS:
//	            LOG("++++Respon link status... DevNo(%d)\n",DevNo);
	            GX101Slaver_ResetLink(DevNo, GX101Slaver_Sta_Addr);
	            gpDevice[DevNo].STATE_FLAG_INIT = GX101Slaver_FLAG_RESET_REMOTE_LINK;
	            break;
	        case GX101Slaver_CTRL_RES_CONFIRM:
				gpDevice[DevNo].Respon_Confirm = 1;
				if(dir == GX101Slaver_CTRL_DIR_DOWN){
					gpDevice[DevNo].ReSendOldTime = gpDevice[DevNo].TimeCnt;
					gpDevice[DevNo].ReSendCnt = 0;
					gpDevice[DevNo].ReSendFlag = RESEND_NO;
				}
	            
	            break;
	        default:
	            LOG("++++error UP function code (%d)... \n",cfun);
	            break;
	    }

		
    }
#ifdef IEC10XLOCK
		if(IEC10X->LOCK != NULL)
			IEC10X->LOCK();
#endif
    return RET_SUCESS;
}

int GX101Slaver_Deal_68(int DevNo, uint8_t *buf, uint16_t len){

    uint8_t cfun, sta, i;
    uint8_t cs_temp = 0;
    PGX101Slaver_68_T GX101Slaver_68 = NULL;
    PIEC10X_ASDU_101T GX101Slaver_Asdu = NULL;
	char *buf_temp;

	if(gpDevice[DevNo].Flag_Link_Ready != LinkReady)
		return RET_ERROR;
	
    GX101Slaver_68 = (PGX101Slaver_68_T)buf;
    GX101Slaver_Asdu = (PIEC10X_ASDU_101T)GX101Slaver_68->_asdu;
    /* check check_sum*/
    for(i=4; i<len-2; i++){
        cs_temp += buf[i];
    }
    if(buf[len-2] != cs_temp){
        perror("GX101Slaver_68 check sum error \n");
        return RET_ERROR;
    }
    /* check end of the frame*/
    if(buf[len-1] != GX101Slaver_VARIABLE_END){
        perror("GX101Slaver_68 end error\n");
        return RET_ERROR;
    }
    /* check len of the receive frame */
    if(GX101Slaver_68->_len+6 != len){
        perror("GX101Slaver_68 rec len error \n");
        return RET_ERROR;
    }
    /* check len of the frame */
    if(GX101Slaver_68->_len != GX101Slaver_68->_len_cfm){
        perror("GX101Slaver_68 len error \n");
        return RET_ERROR;
    }
    
	
	
    /*deal the function code*/
    cfun = GX101Slaver_68->_ctrl.down._func;
    sta = GX101Slaver_68->_ctrl.down._prm;
	GX101Slaver_Sta_Addr = GX101Slaver_68->_addr;
	
#ifdef IEC10XLOCK
		if(IEC10X->LOCK != NULL)
			IEC10X->LOCK();
#endif


    switch(cfun){
        case GX101Slaver_CTRL_SEND_USR_DATA_ACK:
//			log("GX101Slaver_Asdu->_type is (%d)\n",GX101Slaver_Asdu->_type);
            switch(GX101Slaver_Asdu->_type){
                case IEC10X_C_IC_NA_1://���ٻ�
                    LOG("++++asdu type call cmd... \n");
                    GX101Slaver_ASDU_Call(DevNo, GX101Slaver_Asdu);
                    break;
				case IEC10X_TYPE_DZ_CALL://��ֵ�ٻ�
					LOG("++++asdu type DZ call cmd... \n");
					GX101Slaver_ASDU_DZCall(DevNo, GX101Slaver_Asdu);
					break;
				case IEC10X_TYPE_DZ_YZ://��ֵԤ��
					LOG("++++asdu type DZ yuzhi cmd... \n");
					GX101Slaver_ASDU_DZ_YZ(DevNo, GX101Slaver_68);
					break;
				case IEC10X_TYPE_DZ_JH://��ֵ����
					LOG("++++asdu type DZ jihuo cmd... \n");
					GX101Slaver_ASDU_DZ_JH(DevNo, GX101Slaver_Asdu);
					gpDevice[DevNo].Flag_Select_DZ ==  Flag_Clear;
					break;
                case IEC10X_C_CD_NA_1://��ʱ���
                    LOG("++++asdu type delay active... \n");
					gpDevice[DevNo].STATE_FLAG_CLOCK = GX101Slaver_FLAG_DELAY_ACT;
                    break;
                case IEC10X_C_CS_NA_1://��ʱ
                    LOG("++++asdu type clock active... \n");
                    GX101Slaver_ASDU_CLOCK(DevNo, GX101Slaver_Asdu);
                    break;
				case IEC10X_C_SC_NA_1://����ң��
					LOG("++++asdu type yk active... \n");
					GX101Slaver_AskConfirm(DevNo, Iec10x_Sta_Addr);
					GX101Slaver_Deal_Yk(DevNo, GX101Slaver_Asdu);
					break;
				case IEC10X_C_RP_NA_1://��λ����
					LOG("++++Asdu Reset Process cmd... \n");
					GX101Slaver_ResetFlag(DevNo);
					GX101Slaver_BuildResetProcess(DevNo);
					break;
						
					
                default:
					log("GX101Slaver_Asdu->_type is (%d)\n",GX101Slaver_Asdu->_type);
                    break;
            }
            break;
        default:
            LOG("++++error function code (%d)... \n", cfun);
            break;
    }
#ifdef GX101SlaverLOCK
    if(IEC10X->UNLOCK != NULL)
        IEC10X->UNLOCK();
#endif
    return RET_SUCESS;
}

int GX101Slaver_Deal_DZ_YZ(int DevNo, PIEC10X_ASDU_101T GX101Slaver_Asdu)
{
	uint8_t *pData;
	uint8_t sq;
	uint8_t num;
	int i;
	int info_addr;
	uint32_t value;
	Data_Value_T DataValue;
	
	sq = GX101Slaver_Asdu->_num._sq;
	num = GX101Slaver_Asdu->_num._num;
	pData = GX101Slaver_Asdu->_info;

	if(sq){//����
		info_addr = MAKEWORD(pData[0], pData[1]) - IEC10X_DZ_ADDR;
		pData += 2;
		for(i = 0; i < num ; i++){
			value = MAKEDWORD(pData[0], pData[1], pData[2], pData[3]);
			WriteDzYz(DevNo, info_addr + i, value);	
			SetDZFlag(info_addr + i, Flag_DZ_YZ);
			pData += 4;
			gpDevice[DevNo].DZ_YZ_DEVID = GetDZRealID(info_addr + i);
//			DataValue._int32 = value;
//			log("dz(%d) value is %f\n", info_addr + i, DataValue._float);
		}
	}else{//������
		for(i = 0; i < num ; i++){
			info_addr = MAKEWORD(pData[0], pData[1]) - IEC10X_DZ_ADDR;
			value = MAKEDWORD(pData[2], pData[3], pData[4], pData[5]);
			WriteDzYz(DevNo, info_addr, value);
			SetDZFlag(info_addr, Flag_DZ_YZ);
			pData += 6;
			gpDevice[DevNo].DZ_YZ_DEVID = GetDZRealID(info_addr);
//			DataValue._int32 = value;
//			log("dz(%d) value is %f\n", info_addr, DataValue._float);
		}
	}
	gpDevice[DevNo].Flag_Select_DZ = ACTIVATE;
}



int GX101_Slaver_Receive(int DevNo, uint8_t *buf, uint16_t len){

    uint8_t *BufTemp = NULL;
    int16_t LenRemain,LenTmp;
	int LenHeadAndTail = 6;//����ͷ����4�ֽںͱ���β2�ֽ�
	GX101Slaver_Recbuf = buf;
	
    if(buf == NULL){
        LOG("-%s-,buffer (null)",__FUNCTION__);
        return RET_ERROR;
    }
    if(len <= 0 || len>GX101Slaver_MAX_BUF_LEN){
        LOG("-%s-,buffer len error(%d) \n",__FUNCTION__,len);
		log("DevNo(%d)  fd(%d)\n",DevNo, gpDevice[DevNo].fd);
        return RET_ERROR;
    }

    BufTemp = buf;
    LenRemain = len;
    while(BufTemp<buf+len){
//		LOG("-%s-,LINE %s\n",__FUNCTION__,__LINE__);
        if(BufTemp[0] == GX101Slaver_STABLE_BEGING){
            LenTmp = 6;
            GX101Slaver_Pulse_Cnt = 0;
            if(LenRemain < LenHeadAndTail){
                perror("_%s_,len error(%d) \n",__FUNCTION__,len);
                return RET_ERROR;
            }
            GX101Slaver_Deal_10(DevNo, BufTemp, LenTmp);
        }else if(BufTemp[0] == GX101Slaver_VARIABLE_BEGING){
            LenTmp = BufTemp[1] + LenHeadAndTail;
            GX101Slaver_Pulse_Cnt = 0;
            if(BufTemp[1]!=BufTemp[2]){
                perror("_%s_,len error(%d)(%d) \n",__FUNCTION__,BufTemp[1],BufTemp[2]);
                return RET_ERROR;
            }
            GX101Slaver_Deal_68(DevNo, BufTemp, LenTmp);
        }else{
            return RET_ERROR;
        }
        BufTemp+=LenTmp;
        LenRemain-=LenTmp;
    }
    return RET_SUCESS;
}
int CheckYKReturn101(int DevNo)
{
	if(gpDevice[DevNo].Respon_Confirm){
		if(gpDevice[DevNo].YKInfo._Return._ackconfirm == 1){
			GX101Slaver_BuildYkReturn(DevNo, IEC10X_C_SC_NA_1, IEC10X_ASDU_REASON_ACTCON, gpDevice[DevNo].YKInfo);
			gpDevice[DevNo].YKInfo._Return._ackconfirm = 0;
			gpDevice[DevNo].Respon_Confirm = 0;
		}else if(gpDevice[DevNo].YKInfo._Return._stopackconfirm == 1){
			 GX101Slaver_BuildYkReturn(DevNo, IEC10X_C_SC_NA_1, IEC10X_ASDU_REASON_REFUSEACT, gpDevice[DevNo].YKInfo);
			 gpDevice[DevNo].YKInfo._Return._stopackconfirm = 0;
			 gpDevice[DevNo].Respon_Confirm = 0;
		}else if(gpDevice[DevNo].YKInfo._Return._finish == 1){
			 GX101Slaver_BuildYkReturn(DevNo, IEC10X_C_SC_NA_1, IEC10X_ASDU_REASON_ACTFIN, gpDevice[DevNo].YKInfo);
			 gpDevice[DevNo].YKInfo._Return._finish = 0;
			 gpDevice[DevNo].Respon_Confirm = 0;
		}else if(gpDevice[DevNo].YKInfo._Return._refuse == 1){
			 GX101Slaver_BuildYkReturn(DevNo, IEC10X_C_SC_NA_1, IEC10X_ASDU_REASON_REFUSE, gpDevice[DevNo].YKInfo);
			 gpDevice[DevNo].YKInfo._Return._refuse = 0;
			 gpDevice[DevNo].Respon_Confirm = 0;
		}else if(gpDevice[DevNo].YKInfo._Return._refusefinish == 1){
			 GX101Slaver_BuildYkReturn(DevNo, IEC10X_C_SC_NA_1, IEC10X_ASDU_REASON_REFUSE_FIN, gpDevice[DevNo].YKInfo);
			 gpDevice[DevNo].YKInfo._Return._refusefinish = 0;
			 gpDevice[DevNo].Respon_Confirm = 0;
		}

	}
}

int CheckDZYZReturn101()
{
	int i, num = 0;

	for(i = 0; i < gVars.TransDZTableNum; i++){
		if(ReadDzFlag(i) == Flag_DZ_RET) num++;
	}
	if(num == 0)
		return 0;
	
	return num;
}

int GX101ScanBrustData(int DevNo)
{
	uint16_t StartAddr = 0,BrustYxNum = 0, BrustYxSoeNum = 0, BrustYcNum = 0, BrustDzNum = 0;

	
	if(ScanBrustYx(DevNo, &gpDevice[DevNo].ReadYXPtr, &StartAddr, &BrustYxNum)){
		GX101Slaver_BuildBrustYX(DevNo, BrustYxNum, StartAddr, YXBrustRecord);
		return 1;
	}
	if(ScanBrustYxSoe(DevNo, &gpDevice[DevNo].ReadYXSoePtr, &StartAddr, &BrustYxSoeNum)){
		GX101Slaver_BuildBrustYXSoe(DevNo, BrustYxSoeNum, IEC10X_COT_SPONT, StartAddr, YXSoeRecord);
		return 1;
	}
	if(ScanBrustDZ(DevNo, &gpDevice[DevNo].ReadDZPtr, &StartAddr, &BrustDzNum)){
		GX101Slaver_BuildBrustDZ(DevNo, BrustDzNum, StartAddr, DZBrustRecord);
		return 1;
	}
	
	return 0;
}

int GX101Slaver_Task(int DevNo){
	uint8_t FlagSendOver = 0;
	
	
    /*��ʼ��*/
    switch(gpDevice[DevNo].STATE_FLAG_INIT){

        case GX101Slaver_FLAG_LINK_CLOSED:
            break;
        case GX101Slaver_FLAG_INIT_LINK:
            gpDevice[DevNo].Respon_Confirm = 0;
//            GX101Slaver_Sta_Addr = IEC10X->GetLinkAddr();
            GX101Slaver_RetStatusOk(DevNo, GX101Slaver_Sta_Addr);
            gpDevice[DevNo].STATE_FLAG_INIT = GX101Slaver_FLAG_LINK_IDLE;
            break;
        case GX101Slaver_FLAG_RESET_LINK:
            GX101Slaver_ResConfirm(DevNo);
			
            gpDevice[DevNo].STATE_FLAG_CALLALL = GX101Slaver_FLAG_LINK_CLOSED;
			gpDevice[DevNo].STATE_FLAG_DZCALLALL = GX101Slaver_FLAG_LINK_CLOSED;
            gpDevice[DevNo].STATE_FLAG_GROUP   = GX101Slaver_FLAG_LINK_CLOSED;
            gpDevice[DevNo].STATE_FLAG_DELAY   = GX101Slaver_FLAG_LINK_CLOSED;
			gpDevice[DevNo].Respon_Confirm = 0;

            gpDevice[DevNo].STATE_FLAG_INIT = GX101Slaver_FLAG_REQ_LINK;
            break;
        case GX101Slaver_FLAG_REQ_LINK:
            LOG("GX101Slaver_FLAG_REQ_LINK \n");
            GX101Slaver_ReqLinkStatus(DevNo);
            gpDevice[DevNo].STATE_FLAG_INIT = GX101Slaver_FLAG_LINK_IDLE;
            break;
        case GX101Slaver_FLAG_RESET_REMOTE_LINK:
            if(gpDevice[DevNo].Respon_Confirm){
                gpDevice[DevNo].Respon_Confirm = 0;
                gpDevice[DevNo].STATE_FLAG_INIT = GX101Slaver_FLAG_INIT_FIN;
                LOG("GX101Slaver_FLAG_INIT_FIN! \n");
                GX101Slaver_BuildFinInit(DevNo);//��ʼ������
                gpDevice[DevNo].Flag_Link_Ready = LinkReady;
            }
            break;
        case GX101Slaver_FLAG_INIT_FIN:
       		gpDevice[DevNo].STATE_FLAG_INIT = GX101Slaver_FLAG_LINK_IDLE;
            break;
        case GX101Slaver_FLAG_CONNECT_OK:
        case GX101Slaver_FLAG_LINK_IDLE:
        default:
            break;
    }

    /*���ٻ�*/
    switch(gpDevice[DevNo].STATE_FLAG_CALLALL){
        case GX101Slaver_FLAG_CALL_ACT:
            break;
        case GX101Slaver_FLAG_CALL_ACT_RET:/*����ȷ��*/
            if(gpDevice[DevNo].Respon_Confirm){
				gpDevice[DevNo].Respon_Confirm = 0;
				GX101Slaver_BuildActConfirm(DevNo, IEC10X_C_IC_NA_1,IEC10X_CALL_QOI_TOTAL);
				SetCallAllFlag();
				gpDevice[DevNo].STATE_FLAG_CALLALL = GX101Slaver_FLAG_CALL_SIG_TOTAL;
			}
		
            break;
        case GX101Slaver_FLAG_CALL_SIG_TOTAL://����ң��
            if(gpDevice[DevNo].Respon_Confirm){
                gpDevice[DevNo].Respon_Confirm = 0;
				FlagSendOver = GX101Slaver_BuildSignal(DevNo, IEC10X_COT_INTROGEN,IEC10X_PRIO_CALLALL);//ң��
			  	if(FlagSendOver == 1){
					gpDevice[DevNo].STATE_FLAG_CALLALL = GX101Slaver_FLAG_CALL_DET_TOTAL;
				}
            }
		
            break;
        case GX101Slaver_FLAG_CALL_DET_TOTAL://����ң��

			if(gpDevice[DevNo].Respon_Confirm){
				gpDevice[DevNo].Respon_Confirm = 0;
				FlagSendOver = GX101Slaver_BuildDetect(DevNo, IEC10X_COT_INTROGEN,IEC10X_M_ME_NC_1,IEC10X_PRIO_CALLALL);
				if(FlagSendOver == 1)
					gpDevice[DevNo].STATE_FLAG_CALLALL = GX101Slaver_FLAG_CALL_ACT_FIN;
			}

            break;
        case GX101Slaver_FLAG_CALL_ACT_FIN://���ٻ�����
			if(gpDevice[DevNo].Respon_Confirm){
				gpDevice[DevNo].Respon_Confirm = 0;
				GX101Slaver_BuildActFinish(DevNo, GX101Slaver_Call_Qoi);
				gpDevice[DevNo].STATE_FLAG_CALLALL = GX101Slaver_FLAG_LINK_IDLE;
			}

            break;
        case GX101Slaver_FLAG_LINK_IDLE:
        default:
            break;
    }

    /*��ֵ���ٻ�*/
    switch(gpDevice[DevNo].STATE_FLAG_DZCALLALL){
        case GX101Slaver_FLAG_CALL_ACT:
            break;
        case GX101Slaver_FLAG_CALL_ACT_RET:/*����ȷ��*/
            if(gpDevice[DevNo].Respon_Confirm){
				gpDevice[DevNo].Respon_Confirm = 0;
				GX101Slaver_BuildActConfirm(DevNo, IEC10X_TYPE_DZ_CALL, IEC10X_CALL_QOI_TOTAL);
				SetCallDZAllFlag();
				gpDevice[DevNo].STATE_FLAG_DZCALLALL = GX101Slaver_FLAG_CALL_DZ_TOTAL;
			}
            break;
			
        case GX101Slaver_FLAG_CALL_DZ_TOTAL://���Ͷ�ֵ
            if(gpDevice[DevNo].Respon_Confirm){
                gpDevice[DevNo].Respon_Confirm = 0;
				FlagSendOver = GX101Slaver_BuildDZ(DevNo, IEC10X_COT_INTROGEN, IEC10X_TYPE_DZ);//��ֵ
			  	if(FlagSendOver == 1){
					gpDevice[DevNo].STATE_FLAG_DZCALLALL = GX101Slaver_FLAG_CALL_ACT_FIN;
				}
            }
            break;

        case GX101Slaver_FLAG_CALL_ACT_FIN://���ٻ�����
			if(gpDevice[DevNo].Respon_Confirm){
				gpDevice[DevNo].Respon_Confirm = 0;
				GX101Slaver_BuildActFinish(DevNo, GX101Slaver_Call_Qoi);
				gpDevice[DevNo].STATE_FLAG_DZCALLALL = GX101Slaver_FLAG_LINK_IDLE;
			}
            break;
			
        case GX101Slaver_FLAG_LINK_IDLE:
        default:
            break;
    }

    /*���ٻ�*/
    switch(gpDevice[DevNo].STATE_FLAG_GROUP){
        case GX101Slaver_FLAG_CALL_ACT:
            break;
        case GX101Slaver_FLAG_CALL_ACT_RET:
            if(gpDevice[DevNo].Respon_Confirm){
				gpDevice[DevNo].Respon_Confirm = 0;
	        	GX101Slaver_BuildActConfirm(DevNo, IEC10X_C_IC_NA_1,GX101Slaver_Call_Qoi);
				gpDevice[DevNo].STATE_FLAG_GROUP = GX101Slaver_FLAG_CALL_ACT_FIN;
            }
            break;
        case GX101Slaver_FLAG_CALL_GROUP:
            if(gpDevice[DevNo].Respon_Confirm){
                gpDevice[DevNo].Respon_Confirm = 0;
				FlagSendOver = GX101Slaver_BuildCallQoi(DevNo, GX101Slaver_Call_Qoi);
				if(FlagSendOver == 1){
					gpDevice[DevNo].STATE_FLAG_GROUP = GX101Slaver_FLAG_CALL_ACT_FIN;
				}
            }
            break;
        case GX101Slaver_FLAG_CALL_ACT_FIN:
            if(gpDevice[DevNo].Respon_Confirm){
                gpDevice[DevNo].Respon_Confirm = 0;
                gpDevice[DevNo].STATE_FLAG_GROUP = GX101Slaver_FLAG_LINK_IDLE;
				GX101Slaver_BuildActFinish(DevNo, GX101Slaver_Call_Qoi);
            }
            break;
		case GX101Slaver_FLAG_CALL_REFUSE:
			if(gpDevice[DevNo].Respon_Confirm){
				gpDevice[DevNo].Respon_Confirm = 0;
				gpDevice[DevNo].STATE_FLAG_GROUP = GX101Slaver_FLAG_LINK_IDLE;
				GX101Slaver_CallRefuse(DevNo, GX101Slaver_Call_Qoi);
			}
			break;

        case GX101Slaver_FLAG_LINK_IDLE:
        default:
            break;
    }


    /*��ʱ*/
    switch(gpDevice[DevNo].STATE_FLAG_CLOCK){

        case GX101Slaver_FLAG_DELAY_ACT:
            if(gpDevice[DevNo].Respon_Confirm){
                gpDevice[DevNo].Respon_Confirm = 0;
				GX101Slaver_ASDU_Delay(DevNo);
				gpDevice[DevNo].STATE_FLAG_CLOCK = GX101Slaver_FLAG_LINK_IDLE;
				return 0; 
            }
	
            break;
        case GX101Slaver_FLAG_CLOCK_SYS:
            if(gpDevice[DevNo].Respon_Confirm){
                gpDevice[DevNo].Respon_Confirm = 0;
				GX101Slaver_BuildClockAct(DevNo);
				gpDevice[DevNo].STATE_FLAG_CLOCK = GX101Slaver_FLAG_LINK_IDLE;
				return 0; 
            }
		
            break;
        case GX101Slaver_FLAG_LINK_IDLE:
        default:
            break;
    }
	
	/* ң�� */
	CheckYKReturn101(DevNo);
	
    /*ͻ����Ϣ*/
	GX101Slaver_Send_Brust_Info(DevNo);

    /*�ط�����*/
	GX101Slaver_ReSend_Data(DevNo);

	
    return RET_SUCESS;
}
//
////#endif

int GX101Slaver_On_Time_Out(int DevNo){
	int crlfd, iDataLen, ctrlvalue ;
	gpDevice[DevNo].TimeCnt++;
//	if(gpDevice[DevNo].TimeCnt%5 == 0){
//		GX101Slaver_Sendbuf[0] = 1;
//		GX101Slaver_Sendbuf[1] = 2;
//		GX101Slaver_Sendbuf[2] = 3;
//		GX101Slaver_Sendbuf[3] = 4;
////		HuaWei485Ctrl_Switch(DevNo, RS485CTL_Write);
////		write(gpDevice[DevNo].fd,GX101Slaver_Sendbuf,5);
//		CK_send(DevNo, GX101Slaver_Sendbuf, 5);
//	}
	
	

	if(gpDevice[DevNo].Flag_Select_DZ ==  ACTIVATE){
		if(CheckDZYZReturn101()){
			GX101Slaver_Build_DZ_Return(DevNo);
			return 0;
		}
	}

}

