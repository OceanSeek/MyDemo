#include "sys.h"

////#ifdef  IEC101_STM32
// /*
// * GLOABLE VARIALBLE
// */
uint8_t                         Iec101_Respon_Confirm = 0;
uint8_t                         Iec101_Sendbuf[IEC101_MAX_BUF_LEN];


uint8_t             IEC10X_Call_AllQoi = 0;
uint8_t             IEC10X_Call_GroupQoi = 0;
uint16_t            IEC101_Pulse_Cnt = 0;

//extern int fd;


/*
 * STATE
 * */
uint8_t             IEC101_STATE_FLAG_INIT = IEC101_FLAG_LINK_CLOSED;
uint8_t             IEC101_STATE_FLAG_CALLALL = IEC101_FLAG_LINK_CLOSED;
uint8_t             IEC101_STATE_FLAG_GROUP = IEC101_FLAG_LINK_CLOSED;
uint8_t             IEC101_STATE_FLAG_CLOCK = IEC101_FLAG_LINK_CLOSED;
uint8_t             IEC101_STATE_FLAG_DELAY = IEC101_FLAG_LINK_CLOSED;
uint8_t             IEC101_STATE_FLAG_PULSE = IEC101_FLAG_LINK_CLOSED;

int GX101_Master_Receive(int DevNo, uint8_t *buf, uint16_t len);
int GX101Master_Task(int DevNo);
int GX101Master_OnTimeOut(int DevNo);
int GX101Master_BuildYk(int DevNo, uint8_t Type, uint16_t reason, uint8_t YKData);
	
int Init_GX101Master(int DevNo)
{
	gpDevice[DevNo].Receive = GX101_Master_Receive;
	gpDevice[DevNo].Task = GX101Master_Task;
	gpDevice[DevNo].SetYK = GX101Master_BuildYk;
	gpDevice[DevNo].OnTimeOut = GX101Master_OnTimeOut;
}


uint8_t Linux_send(int ttyfd,char *buf, int len){

    if(-1 == write(ttyfd,buf,len)){
        LOG("-%s-,Send error \n",__FUNCTION__);
        return RET_ERROR;
    }
    printf("Send Ok!\r\n");
    
    return RET_SUCESS;
}



uint8_t IEC10X_RetStatusOk(int DevNo, uint16_t addr){

    uint16_t len;
    PIEC101_10_T Iec10x = (PIEC101_10_T)Iec101_Sendbuf;

    len = IEC101_STABLE_LEN;

    Iec10x->_begin = IEC101_STABLE_BEGING;

    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_RESPONSE_LINK_STATUS;

    Iec10x->_addr = addr;
    Iec10x->_cs = Iec101_Sendbuf[1]+Iec101_Sendbuf[2]+Iec101_Sendbuf[3];
    Iec10x->_end = IEC101_STABLE_END;
	
	Linux_send(gpDevice[DevNo].fd,Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}

uint8_t IEC10X_AskConfirm(int DevNo, uint16_t addr){

    uint16_t len;
    PIEC101_10_T Iec10x = (PIEC101_10_T)Iec101_Sendbuf;

    len = IEC101_STABLE_LEN;

    Iec10x->_begin = IEC101_STABLE_BEGING;

    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_RES_CONFIRM;

    Iec10x->_addr = addr;
    Iec10x->_cs = Iec101_Sendbuf[1]+Iec101_Sendbuf[2]+Iec101_Sendbuf[3];
    Iec10x->_end = IEC101_STABLE_END;

	Linux_send(gpDevice[DevNo].fd,Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}


static uint8_t IEC10X_ResConfirm(int DevNo, uint8_t Prio){

    uint16_t len;

    PIEC101_10_T Iec10x = (PIEC101_10_T)Iec101_Sendbuf;

    len = IEC101_STABLE_LEN;

    Iec10x->_begin = IEC101_STABLE_BEGING;

    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_SLAVE;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CAN_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_RES_CONFIRM;

    Iec10x->_addr = Iec10x_Sta_Addr;
    Iec10x->_cs = Iec101_Sendbuf[1]+Iec101_Sendbuf[2]+Iec101_Sendbuf[3];
    Iec10x->_end = IEC101_STABLE_END;

	
	Linux_send(gpDevice[DevNo].fd,Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    IEC10X_Enqueue(Iec101_Sendbuf, len, Prio, NULL,NULL);

    return RET_SUCESS;
}
uint8_t IEC10X_ReqLinkStatus(int DevNo){

    uint16_t len;

    PIEC101_10_T Iec10x = (PIEC101_10_T)Iec101_Sendbuf;

    LOG("%s \n",__FUNCTION__);

    len = IEC101_STABLE_LEN;

    Iec10x->_begin = IEC101_STABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.down._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.down._fcb = IEC101_CTRL_FCB_OPPO_NONE;
    Iec10x->_ctrl.down._fcv = IEC101_CTRL_FCV_DISABLE;

    Iec10x->_ctrl.up._func = IEC101_CTRL_REQ_LINK_STATUS;

    Iec10x->_addr = Iec10x_Sta_Addr;
    Iec10x->_cs = Iec101_Sendbuf[1]+Iec101_Sendbuf[2]+Iec101_Sendbuf[3];
    Iec10x->_end = IEC101_STABLE_END;

	Linux_send(gpDevice[DevNo].fd,Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}

//
uint8_t IEC10X_ResetLink(int DevNo, uint16_t addr){

    uint16_t len;

    PIEC101_10_T Iec10x = (PIEC101_10_T)Iec101_Sendbuf;

    len = IEC101_STABLE_LEN;

    Iec10x->_begin = IEC101_STABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.down._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.down._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.down._fcb = IEC101_CTRL_FCB_OPPO_NONE;
    Iec10x->_ctrl.down._fcv = IEC101_CTRL_FCV_DISABLE;

    Iec10x->_ctrl.up._func = IEC101_CTRL_RESET_LINK;

    Iec10x->_addr = addr;
    Iec10x->_cs = Iec101_Sendbuf[1]+Iec101_Sendbuf[2]+Iec101_Sendbuf[3];
    Iec10x->_end = IEC101_STABLE_END;
	LOG("addr is (%d)\n",Iec10x->_addr);
	len = 6;
	Linux_send(gpDevice[DevNo].fd,Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;
}
//
//
uint8_t IEC101_BuildFinInit(int DevNo){

    uint16_t len = 0;
    uint8_t cs_temp = 0,i;

    PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(Iec10x->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);

    len = IEC101_STABLE_LEN;

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_WITH_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    /*asdu*/
    asdu->_type = Iec10x_M_EI_NA_1;
    asdu->_num._num = 1;
    asdu->_reason = IEC10X_ASDU_REASON_INIT;
    asdu->_addr = Iec10x_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;
    info->_element[0] = 0;

    /*len*/
    len = IEC101_VARIABLE_LEN + asdu->_num._num;
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += Iec101_Sendbuf[i];
    }
    Iec101_Sendbuf[len-2] = cs_temp;
    Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

	Linux_send(gpDevice[DevNo].fd,Iec101_Sendbuf,len);
    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    IEC10X_Enqueue(Iec101_Sendbuf, len ,IEC10X_PRIO_INITLINK, NULL,NULL);

    return RET_SUCESS;

}


uint8_t IEC101_BuildDelayAct(int DevNo,uint16_t delay_time){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, asdu_num = 0, *ptr = NULL;
    uint16_t cp16time2a = delay_time;

    /*init struct*/
    PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(Iec10x->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);

    /*get value*/
    asdu_num = 1;

    len = IEC101_STABLE_LEN;

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_C_CD_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason = IEC10X_COT_ACTCON;
    asdu->_addr = Iec10x_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;

    /*delay value*/
    ptr = info->_element;
    *(uint16_t *)ptr = IEC10X_Cp16time2a;
    ptr+=2;

    /*len*/
    len = ptr + 2 - Iec101_Sendbuf;
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += Iec101_Sendbuf[i];
    }
    Iec101_Sendbuf[len-2] = cs_temp;
    Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

	
	Linux_send(gpDevice[DevNo].fd,Iec101_Sendbuf,len);
    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_DELAY, NULL,NULL);

    return RET_SUCESS;
}
uint8_t IEC101_BuildClockAct(int DevNo){

    uint16_t len = 0;
    uint8_t cs_temp = 0, i, asdu_num = 0, *ptr = NULL;

    /*init struct*/
    PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
    PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(Iec10x->_asdu);
    PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);
    PCP56Time2a_T time = (PCP56Time2a_T)(info->_element);

    /*get value*/
    asdu_num = 1;

    len = IEC101_STABLE_LEN;

    /*head*/
    Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

    /*Ctrol*/
    Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
    Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
    Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_NONE_DATA;
    Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

    Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

    Iec10x->_addr = Iec10x_Sta_Addr;
    /*asdu*/
    asdu->_type = IEC10X_C_CS_NA_1;
    asdu->_num._sq = 0;
    asdu->_num._num = asdu_num;
    asdu->_reason = IEC10X_COT_ACTCON;
    asdu->_addr = Iec10x_Sta_Addr;
    /*info*/
    info->_addr = IEC10X_INFO_ADDR_NONE;

    /*clock value*/
    ptr = info->_element;
    //time;
//    IEC10X->GetTime(time);//获取系统时间，有待完善
    ptr+=sizeof(CP56Time2a_T);


    /*len*/
    len = ptr + 2 - Iec101_Sendbuf;
    Iec10x->_len = Iec10x->_len_cfm = len-4-2;          /*-start-len-len-start   -cs-end*/

    /*end*/
    for(i=4; i<len-2; i++){
        cs_temp += Iec101_Sendbuf[i];
    }
    Iec101_Sendbuf[len-2] = cs_temp;
    Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

	Linux_send(gpDevice[DevNo].fd,Iec101_Sendbuf,len);
    //DumpHEX(Iec101_Sendbuf,len);
    /* enqueue to the transmisson queue */
//    IEC10X_Enqueue(Iec101_Sendbuf, len, IEC10X_PRIO_CLOCK, NULL,NULL);

    return RET_SUCESS;
}

uint8_t IEC101_BuildYkReturn(int DevNo,uint8_t qoi, PYKReturn_T YK_Return, uint8_t info_addr){

	uint16_t len = 0;
	uint8_t cs_temp = 0,i;

	PIEC101_68_T Iec10x = (PIEC101_68_T)Iec101_Sendbuf;
	PIEC10X_ASDU_101T asdu = (PIEC10X_ASDU_101T)(Iec10x->_asdu);
	PASDU_INFO_101T info = (PASDU_INFO_101T)(asdu->_info);

	len = IEC101_STABLE_LEN;

	/*head*/
	Iec10x->_begin = Iec10x->_begin_cfm = IEC101_VARIABLE_BEGING;

	/*Ctrol*/
	Iec10x->_ctrl.up._dir = IEC101_CTRL_DIR_UP;
	Iec10x->_ctrl.up._prm = IEC101_CTRL_PRM_MASTER;
	Iec10x->_ctrl.up._acd = IEC101_CTRL_ACD_WITH_DATA;
	Iec10x->_ctrl.up._dfc = IEC101_CTRL_DFC_CANNOT_REC;

	Iec10x->_ctrl.up._func = IEC101_CTRL_SEND_USR_DATA_ACK;

	Iec10x->_addr = Iec10x_Sta_Addr;
	/*asdu*/
	asdu->_type = qoi;
	asdu->_num._num = 1;
	asdu->_reason = YK_Return->_reason;
	asdu->_addr = Iec10x_Sta_Addr;
	/*info*/
	info->_addr = info_addr + STARTSYK;
	info->_element[0] = YK_Return->_data;

	/*len*/
	len = IEC101_VARIABLE_LEN + asdu->_num._num;
	Iec10x->_len = Iec10x->_len_cfm = len-4-2;			/*-start-len-len-start	 -cs-end*/

	/*end*/
	for(i=4; i<len-2; i++){
		cs_temp += Iec101_Sendbuf[i];
	}
	Iec101_Sendbuf[len-2] = cs_temp;
	Iec101_Sendbuf[len-1] = IEC101_VARIABLE_END;

	Linux_send(gpDevice[DevNo].fd,Iec101_Sendbuf,len);
	//DumpHEX(Iec101_Sendbuf,len);
	/* enqueue to the transmisson queue */
//	  IEC10X_Enqueue(Iec101_Sendbuf, len ,IEC10X_PRIO_INITLINK, NULL,NULL);



    return RET_SUCESS;
}

int IEC10X_ASDU_CALL_Qoi(uint8_t qoi){
	
    switch(qoi){
        case IEC10X_CALL_QOI_TOTAL:
            break;
        case IEC10X_CALL_QOI_GROUP1:
            break;
        case IEC10X_CALL_QOI_GROUP2:
            break;
        case IEC10X_CALL_QOI_GROUP9:
            break;
        case IEC10X_CALL_QOI_GROUP10:
            break;
        default:
            LOG("-%s- call cmd active error(%d) \n" ,__FUNCTION__ ,qoi);
            break;
    }
}

uint8_t IEC10X_ASDU_Call(int DevNo, PIEC10X_ASDU_101T Iec10x_Asdu){

    PASDU_INFO_101T asdu_info = (PASDU_INFO_101T)(Iec10x_Asdu->_info);
    uint8_t qoi = asdu_info->_element[0];
    uint8_t Prio = 0;

    if(asdu_info->_addr != 0){
        LOG("-%s- call cmd active error addr(%x) \n" ,__FUNCTION__ ,asdu_info->_addr);
        return RET_ERROR;
    }
//	LOG("Iec10x_Asdu->_reason is (%02x)\n",Iec10x_Asdu->_reason);
//	LOG("qoi (%02x)\n",qoi);
	
    switch(Iec10x_Asdu->_reason){

        case IEC10X_ASDU_REASON_ACT:
            switch(qoi){
                case IEC10X_CALL_QOI_TOTAL:
                    break;
                case IEC10X_CALL_QOI_GROUP1:
                case IEC10X_CALL_QOI_GROUP2:
                case IEC10X_CALL_QOI_GROUP9:
                case IEC10X_CALL_QOI_GROUP10:
                    break;
                default:
                    LOG("-%s- call cmd error qoi(%d) \n", __FUNCTION__,qoi);
                    return RET_ERROR;
            }
            break;

        default:
            LOG("-%s- call cmd error reason(%d) \n", __FUNCTION__,Iec10x_Asdu->_reason);
            break;
    }
    return RET_SUCESS;
}

uint8_t IEC10X_ASDU_Delay(int DevNo, PIEC10X_ASDU_101T Iec10x_Asdu){

    PASDU_INFO_101T asdu_info = (PASDU_INFO_101T)(Iec10x_Asdu->_info);

    if(asdu_info->_addr != 0){
        LOG("-%s- delay cmd error addr(%d) \n", __FUNCTION__,asdu_info->_addr);
        return RET_ERROR;
    }

    switch(Iec10x_Asdu->_reason){

        case IEC10X_COT_ACT:
            IEC10X_Cp16time2a = *(uint16_t *)(asdu_info->_element);
            LOG("-%s- delay cmd (0x%x%x)(%d)ms \n", __FUNCTION__,asdu_info->_element[0],asdu_info->_element[1],IEC10X_Cp16time2a);
            IEC101_STATE_FLAG_DELAY = IEC101_FLAG_DELAY_ACT;
            IEC10X_ResConfirm(DevNo, IEC10X_PRIO_DELAY);
            IEC101_BuildDelayAct(DevNo, IEC10X_Cp16time2a);
            break;
        case IEC10X_COT_SPONT:
            IEC10X_Cp16time2a_V = *(uint16_t *)(asdu_info->_element);
            LOG("-%s- delay cmd delay value(%d)ms \n", __FUNCTION__,IEC10X_Cp16time2a_V);
            IEC10X_ResConfirm(DevNo, IEC10X_PRIO_DELAY);
            break;
        default:
            LOG("-%s- delay cmd error reason(%d) \n", __FUNCTION__,Iec10x_Asdu->_reason);
            break;
    }
    return RET_SUCESS;
}

uint8_t IEC10X_ASDU_CLOCK(int DevNo, PIEC10X_ASDU_101T Iec10x_Asdu){

    PASDU_INFO_101T asdu_info = (PASDU_INFO_101T)(Iec10x_Asdu->_info);
    PCP56Time2a_T time = (PCP56Time2a_T)(asdu_info->_element);

    memcpy(&IEC10X_Cp56time2a,asdu_info->_element, sizeof(CP56Time2a_T));


    if(asdu_info->_addr != 0){
        LOG("-%s- Clock cmd error addr(%d) \n", __FUNCTION__,asdu_info->_addr);
		LOG("asdu_info->_addr is (%d)\n",asdu_info->_addr);
        return RET_ERROR;
    }

    switch(Iec10x_Asdu->_reason){

        case IEC10X_COT_ACT:
            LOG("-%s- Clock cmd (20%d-%d-%d %d %d:%d:%d) delay(%d) \n", __FUNCTION__,IEC10X_Cp56time2a._year._year,IEC10X_Cp56time2a._month._month,IEC10X_Cp56time2a._day._dayofmonth,
                    IEC10X_Cp56time2a._day._dayofweek,IEC10X_Cp56time2a._hour._hours,IEC10X_Cp56time2a._min._minutes,IEC10X_Cp56time2a._milliseconds,IEC10X_Cp16time2a_V);
            IEC10X_Cp56time2a._milliseconds += IEC10X_Cp16time2a_V;
            /*get time*/
            /*...*/
//            IEC10X->SetTime(&IEC10X_Cp56time2a);//设置送检时间，有待完善
//            IEC101_STATE_FLAG_DELAY = IEC101_FLAG_CLOCK_SYS;
            IEC10X_ResConfirm(DevNo, IEC10X_PRIO_CLOCK);
            IEC101_BuildClockAct(DevNo);
            break;
        case IEC10X_COT_SPONT:
            LOG("-%s- Clock cmd spont \n", __FUNCTION__);
            break;
        default:
            LOG("-%s- Clock cmd error reason(%d) \n", __FUNCTION__,Iec10x_Asdu->_reason);
            break;
    }
    return RET_SUCESS;
}

uint8_t IEC101_Deal_Yk(PIEC10X_ASDU_101T Iec10x_Asdu){

	PASDU_INFO_8T asdu_info = (PASDU_INFO_8T)(Iec10x_Asdu->_info);
	uint16_t info_addr;
	int i;

	info_addr = asdu_info->_addr - STARTSYK;
	
	if((info_addr+1) > MAX_SLAVE_DEVICE_NUM){
		PERROR("ERROR____max slave device!\n");
		PRINT_FUNLINE;
		return -1 ;
	}
//	Device_Yk_Info[info_addr]._flag = ACTIVATE;
//	Device_Yk_Info[info_addr] = Iec10x_Asdu->_reason;
//	Device_Yk_Info[info_addr]._Data8 = asdu_info->_Data8;
	return RET_SUCESS;

}


uint8_t Iec10x_Deal_10(int DevNo, uint8_t *buf, uint16_t len){

    uint8_t cfun, sta,i;
    uint8_t cs_temp = 0;
    PIEC101_10_T Iec10x_10 = NULL;

    Iec10x_10 = (PIEC101_10_T)buf;
    /* check check_sum*/
    for(i=1; i<len-2; i++){
        cs_temp += buf[i];
    }
    if(Iec10x_10->_cs != cs_temp){
        LOG("-%s-,check sum error(%x) \n",__FUNCTION__,cs_temp);
        return RET_ERROR;
    }
    /* check end of the frame*/
    if(Iec10x_10->_end != 0x16){
        LOG("-%s-,iec10x_10 end error(%d) \n",__FUNCTION__,Iec10x_10->_end);
        return RET_ERROR;
    }
    /*deal the function code*/
    cfun = Iec10x_10->_ctrl.down._func;
    sta = Iec10x_10->_ctrl.down._prm;
	Iec10x_Sta_Addr = Iec10x_10->_addr;
	
//	LOG("cfun is (%d)\n",cfun);
//	LOG("sta is (%d)\n",sta);
    //IEC10X->GetLinkAddr();
#ifdef IEC10XLOCK
//	LOG("-%s-,line is (%d) \n",__FUNCTION__,__LINE__);
    if(IEC10X->LOCK != NULL)
        IEC10X->LOCK();
	LOG("-%s-,line is (%d) \n",__FUNCTION__,__LINE__);
#endif
    if(sta == IEC101_CTRL_PRM_MASTER){
        switch(cfun){
            case IEC101_CTRL_RESET_LINK:
//                LOG("++++Reset link... \n");
                IEC101_STATE_FLAG_INIT = IEC101_FLAG_RESET_LINK;
                break;
            case IEC101_CTRL_PULSE:
//                LOG("++++PULSE... \n");
                IEC101_STATE_FLAG_PULSE = IEC101_FLAG_PULSE;
                IEC10X_ResConfirm(DevNo, IEC10X_PRIO_PULSE);
                break;
            case IEC101_CTRL_SEND_DATA:
//                LOG("++++Send data... \n");
                break;
            case IEC101_CTRL_REQ_LINK_STATUS:
                LOG("++++Request link status...\n");
				IEC10X_RetStatusOk(DevNo, Iec10x_Sta_Addr);
//                IEC101_STATE_FLAG_INIT = IEC101_FLAG_INIT_LINK;
//                Iec10x_Sta_Addr = IEC10X->GetLinkAddr();
//				LOG("Iec10x_Sta_Addr is %d \n",Iec10x_Sta_Addr);
                break;
            default:
                LOG("++++error DOWN function code (%d)... \n",cfun);
                break;
        }
    }else if(sta == IEC101_CTRL_PRM_SLAVE){//从动站
//		LOG("-%s-,line is (%d) \n",__FUNCTION__,__LINE__);
        switch(cfun){
            case IEC101_CTRL_RESPONSE_LINK_STATUS:
//                LOG("++++Respon link status... \n");
                IEC10X_ResetLink(DevNo, Iec10x_Sta_Addr);
                IEC101_STATE_FLAG_INIT = IEC101_FLAG_RESET_REMOTE_LINK;
                break;
            case IEC101_CTRL_RES_CONFIRM:
//                LOG("++++Respon confirm... \n");//
                Iec101_Respon_Confirm = 1;
                break;
            default:
                LOG("++++error UP function code (%d)... \n",cfun);
                break;
        }
    }
#ifdef IEC10XLOCK
    if(IEC10X->UNLOCK != NULL)
        IEC10X->UNLOCK();
#endif
    return RET_SUCESS;
}

uint8_t Iec10x_Deal_68(int DevNo, uint8_t *buf, uint16_t len){

    uint8_t cfun, sta, i;
    uint8_t cs_temp = 0;
    PIEC101_68_T Iec10x_68 = NULL;
    PIEC10X_ASDU_101T Iec10x_Asdu = NULL;
	char *buf_temp;
	
    Iec10x_68 = (PIEC101_68_T)buf;
    Iec10x_Asdu = (PIEC10X_ASDU_101T)Iec10x_68->_asdu;
    /* check check_sum*/
    for(i=4; i<len-2; i++){
        cs_temp += buf[i];
    }
    if(buf[len-2] != cs_temp){
        LOG("-%s-,iec10x_68 check sum error (%d)(%d) \n",__FUNCTION__,buf[len-2],cs_temp);
        return RET_ERROR;
    }
    /* check end of the frame*/
    if(buf[len-1] != IEC101_VARIABLE_END){
        LOG("-%s-,iec10x_68 end error(%d) \n",__FUNCTION__,buf[len-1]);
        return RET_ERROR;
    }
    /* check len of the receive frame */
    if(Iec10x_68->_len+6 != len){
        LOG("-%s-,iec10x_68 rec len error(%d)(%d) \n",__FUNCTION__, Iec10x_68->_len, len);
        return RET_ERROR;
    }
    /* check len of the frame */
    if(Iec10x_68->_len != Iec10x_68->_len_cfm){
        LOG("-%s-,iec10x_68 len error(%d)(%d) \n",__FUNCTION__, Iec10x_68->_len, Iec10x_68->_len_cfm);
        return RET_ERROR;
    }
	
	
    /*deal the function code*/
    cfun = Iec10x_68->_ctrl.down._func;
    sta = Iec10x_68->_ctrl.down._prm;
	Iec10x_Sta_Addr = Iec10x_68->_addr;
	
#ifdef IEC10XLOCK
    if(IEC10X->LOCK != NULL)
        IEC10X->LOCK();
#endif


//	LOG("Iec10x_68->_begin is (%02x)\n",Iec10x_68->_begin);
//	LOG("Iec10x_68->len is (%02x)\n",Iec10x_68->_len);
//	LOG("Iec10x_68->len-cfm is (%02x)\n",Iec10x_68->_len_cfm);
//	LOG("Iec10x_68->_begin_cfm is (%02x)\n",Iec10x_68->_begin_cfm);
//	LOG("Iec10x_68->_ctrl is (%02x)\n",Iec10x_68->_ctrl);
//	LOG("Iec10x_68->_addr is (%02x)\n",Iec10x_68->_addr);

    switch(cfun){
        case IEC101_CTRL_SEND_USR_DATA_ACK:
            Iec101_Respon_Confirm = 0;
            LOG("++++Send user data need ack... \n");
			printf("Iec10x_Asdu->_type is (%d)\n",Iec10x_Asdu->_type);
            switch(Iec10x_Asdu->_type){
                case IEC10X_C_IC_NA_1:
                    LOG("++++asdu type call cmd... \n");
					buf_temp = (char *)Iec10x_Asdu;
					log("buf_asdu,1,2,[%02x],[%02x],[%02x]\n",buf_temp[0],buf_temp[1],buf_temp[2]);
                    IEC10X_ASDU_Call(DevNo, Iec10x_Asdu);
                    break;
                case IEC10X_C_CD_NA_1:
                    LOG("++++asdu type delay active... \n");
                    IEC10X_ASDU_Delay(DevNo, Iec10x_Asdu);
                    break;
                case IEC10X_C_CS_NA_1:
                    LOG("++++asdu type clock active... \n");
                    IEC10X_ASDU_CLOCK(DevNo, Iec10x_Asdu);
                    break;
				case IEC10X_C_SC_NA_1://单点遥控
					LOG("++++asdu type yk active... \n");
					log("Iec10x_Asdu_addr(%d),info(%d)\n",Iec10x_Asdu->_addr,Iec10x_Asdu->_info[0]);
					IEC10X_AskConfirm(DevNo, Iec10x_Sta_Addr);
					IEC101_Deal_Yk(Iec10x_Asdu);
					
					break;
					
                default:
					printf("Iec10x_Asdu->_type is (%d)\n",Iec10x_Asdu->_type);
                    break;
            }
            break;
        default:
            LOG("++++error function code (%d)... \n", cfun);
            break;
    }
#ifdef IEC10XLOCK
    if(IEC10X->UNLOCK != NULL)
        IEC10X->UNLOCK();
#endif
    return RET_SUCESS;
}

int GX101_Master_Receive(int DevNo, uint8_t *buf, uint16_t len){

    uint8_t *BufTemp = NULL;
    int16_t LenRemain,LenTmp;
    if(buf == NULL){
        perror("-%s-,buffer (null)",__FUNCTION__);
        return RET_ERROR;
    }
    if(len <= 0 || len>IEC101_MAX_BUF_LEN){
        perror("-%s-,buffer len error(%d) \n",__FUNCTION__,len);
        return RET_ERROR;
    }

    BufTemp = buf;
    LenRemain = len;
    while(BufTemp<buf+len){
//		LOG("-%s-,LINE %s\n",__FUNCTION__,__LINE__);
        if(BufTemp[0] == IEC101_STABLE_BEGING){
            LenTmp = 6;
            IEC101_Pulse_Cnt = 0;
            if(LenRemain<6){
                perror("_%s_,len error(%d) \n",__FUNCTION__,len);
                return RET_ERROR;
            }
            Iec10x_Deal_10(DevNo, BufTemp, LenTmp);
        }else if(BufTemp[0] == IEC101_VARIABLE_BEGING){
            LenTmp = BufTemp[1]+6;
            IEC101_Pulse_Cnt = 0;
            if(BufTemp[1]!=BufTemp[2]){
                perror("_%s_,len error(%d)(%d) \n",__FUNCTION__,BufTemp[1],BufTemp[2]);
                return RET_ERROR;
            }
            Iec10x_Deal_68(DevNo, BufTemp, LenTmp);
        }else{
            return RET_ERROR;
        }
        BufTemp+=LenTmp;
        LenRemain-=LenTmp;
    }
    return RET_SUCESS;
}

int GX101Master_Task(int DevNo){

    /*Init link*/

    switch(IEC101_STATE_FLAG_INIT){

        case IEC101_FLAG_LINK_CLOSED:
            break;
        case IEC101_FLAG_INIT_LINK:
            Iec101_Respon_Confirm = 0;
//            Iec10x_Sta_Addr = IEC10X->GetLinkAddr();
            IEC10X_RetStatusOk(DevNo, Iec10x_Sta_Addr);
            IEC101_STATE_FLAG_INIT = IEC101_FLAG_LINK_IDLE;
            break;
        case IEC101_FLAG_RESET_LINK:
//            IEC10X_ClearQ();
            IEC10X_ResConfirm(DevNo, IEC10X_PRIO_INITLINK);
			
            IEC101_STATE_FLAG_CALLALL = IEC101_FLAG_LINK_CLOSED;
            IEC101_STATE_FLAG_GROUP   = IEC101_FLAG_LINK_CLOSED;
            IEC101_STATE_FLAG_DELAY   = IEC101_FLAG_LINK_CLOSED;
            IEC101_STATE_FLAG_PULSE   = IEC101_FLAG_LINK_CLOSED;

            IEC101_STATE_FLAG_INIT = IEC101_FLAG_REQ_LINK;
            break;
        case IEC101_FLAG_REQ_LINK:
            LOG("IEC101_FLAG_REQ_LINK \n");
            IEC10X_ReqLinkStatus(DevNo);
            IEC101_STATE_FLAG_INIT = IEC101_FLAG_LINK_IDLE;
            break;
        case IEC101_FLAG_RESET_REMOTE_LINK:
            if(Iec101_Respon_Confirm){
                Iec101_Respon_Confirm = 0;
                IEC101_STATE_FLAG_INIT = IEC101_FLAG_INIT_FIN;
                LOG("IEC101_FLAG_INIT_FIN! \n");
                IEC101_BuildFinInit(DevNo);//初始化结束
            }
            break;
        case IEC101_FLAG_INIT_FIN:
            if(Iec101_Respon_Confirm){
                Iec101_Respon_Confirm = 0;
                IEC101_STATE_FLAG_INIT = IEC101_FLAG_CONNECT_OK;
            }
            break;
        case IEC101_FLAG_CONNECT_OK:
        case IEC101_FLAG_LINK_IDLE:
        default:
            break;
    }

    /*total call*/
    switch(IEC101_STATE_FLAG_CALLALL){
        case IEC101_FLAG_CALL_ACT:
            break;
        case IEC101_FLAG_CALL_ACT_RET://发送遥信
            break;
        case IEC101_FLAG_CALL_SIG_TOTAL://发送遥测
            break;
        case IEC101_FLAG_CALL_DET_TOTAL://总召唤结束
            break;
        case IEC101_FLAG_CALL_ACT_FIN:
//            LOG("IEC101_FLAG_CALL_ACT_FIN \n");
            if(Iec101_Respon_Confirm){
                Iec101_Respon_Confirm = 0;
                IEC101_STATE_FLAG_CALLALL = IEC101_FLAG_LINK_IDLE;
            }
            break;
        case IEC101_FLAG_LINK_IDLE:
        default:
            break;
    }

    /*group call*/
    switch(IEC101_STATE_FLAG_GROUP){
        case IEC101_FLAG_CALL_ACT:
            break;
        case IEC101_FLAG_CALL_ACT_RET:
//            if(Iec101_Respon_Confirm){
//                Iec101_Respon_Confirm = 0;
//                IEC10X_ASDU_CALL_Qoi(IEC10X_Call_GroupQoi);
//            }
            break;
        case IEC101_FLAG_CALL_GROUP:
//            if(Iec101_Respon_Confirm){
//                Iec101_Respon_Confirm = 0;
//                IEC101_STATE_FLAG_GROUP = IEC101_FLAG_CALL_ACT_FIN;
//                IEC101_BuildActFinish(IEC10X_Call_GroupQoi,IEC10X_PRIO_CALLGROUP);
//            }
            break;
        case IEC101_FLAG_CALL_ACT_FIN:
//            if(Iec101_Respon_Confirm){
//                Iec101_Respon_Confirm = 0;
//                IEC101_STATE_FLAG_GROUP = IEC101_FLAG_LINK_IDLE;
//            }
            break;
        case IEC101_FLAG_LINK_IDLE:
        default:
            break;
    }

    /*delay and clock*/
    switch(IEC101_STATE_FLAG_DELAY){

        case IEC101_FLAG_DELAY_ACT:
            break;
        case IEC101_FLAG_CLOCK_SYS:
//            IEC101_STATE_FLAG_DELAY = IEC101_FLAG_LINK_IDLE;
            break;
        case IEC101_FLAG_LINK_IDLE:
        default:
            break;
    }

    /*pulse*/
    switch(IEC101_STATE_FLAG_PULSE){
        case IEC101_FLAG_PULSE:
            break;
        case IEC101_FLAG_LINK_IDLE:
            break;
        default:
            break;
    }
    return RET_SUCESS;
}

int GX101Master_OnTimeOut(int DevNo){
	
	
}
/*******************************************************************  
*名称：      		GX101Master_BuildYk  
*功能：			处理遥控任务  
*入口参数：         
*	@DevNo		设备ID号
*	@Type 		遥控类型
*	@reason 	传输原因
*	@YKData 	遥控命令
*出口参数：正确返回为0，错误返回为-1 
*******************************************************************/ 
int GX101Master_BuildYk(int DevNo, uint8_t Type, uint16_t reason, uint8_t YKData)
{


    return RET_SUCESS;
}


////#endif

