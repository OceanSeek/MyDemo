#include "sys.h"
extern TBurstYXRecord YXDSoeRecord[MAXYXRECORD];

//==========================  与应用层的接口  =============================

static int GetNum(int DevID)
{
	int SoeNum = 0;
	SoeNum = (gpDevice[DevID].ReadDYXSoePtr > gVars.YxDSoeWritePtr) ?gVars.YxDSoeWritePtr + MAXYXRECORD - gpDevice[DevID].ReadDYXSoePtr\
		:gVars.YxDSoeWritePtr - gpDevice[DevID].ReadDYXSoePtr;
	log("gpDevice[DevID].ReadDYXSoePtr is %d gVars.YxDSoeWritePtr %d\n",gpDevice[DevID].ReadDYXSoePtr, gVars.YxDSoeWritePtr);
	return SoeNum;
}


DWORD GetDBISoeNum(DWORD dwDevNo)
{
//	DWORD DBISoeNum;
//	DWORD DBISoeMaxBuffer;
//	TPtr *pPtr;		
//
//	if(dwDevNo>=gVars.dwDevNum)
//		return 0;
//	if(gpDevice[dwDevNo].pDBISoeWrap == 0)
//		return 0;
//
////	SM_P(MPV_DBISOE);
//	
//	pPtr = &gpDevice[dwDevNo].pDBISoeWrap->Ptr;
////	DBISoeNum = GetNum(pPtr);
////	DBISoeMaxBuffer = GetDBISoeMaxBuffer(dwDevNo);
//
//	if(DBISoeNum > DBISoeMaxBuffer)
//	{
//		pPtr->dwReadPtr = pPtr->dwWritePtr - DBISoeMaxBuffer;
//		DBISoeNum = DBISoeMaxBuffer;
//	}
//
////	SM_V(MPV_DBISOE);	
//	
//	return DBISoeNum;
	if(dwDevNo>=gVars.dwDevNum)
		return 0;
			
	DWORD DBISoeNum;

	DBISoeNum=GetNum(dwDevNo);
	log("DBISoeNum is %d \n",DBISoeNum);
	return DBISoeNum;


}

void ClearDBISoe(DWORD dwDevNo)
{
	TPtr *pPtr;

	if(dwDevNo>=gVars.dwDevNum)
		return ;
	if(gpDevice[dwDevNo].pDBISoeWrap==0)
		return;
	
//	SM_P(MPV_DBISOE);		
	
	pPtr=&gpDevice[dwDevNo].pDBISoeWrap->Ptr;
	pPtr->dwReadPtr = pPtr->dwWritePtr;
	
//	SM_V(MPV_DBISOE);
}

DWORD GetDBISoeReadPtr(DWORD dwDevNo)
{
	TDevie       *pDevice;
	pDevice = pGetDevice(dwDevNo);
	DWORD Num;
	TPtr *pPtr;	

	if(pDevice==NULL)
		return 0;
	
	if(pDevice->pDBISoeWrap!=NULL)
	{
		SM_P(MPV_DBISOE);		
	
		pPtr=&gpDevice[dwDevNo].pDBISoeWrap->Ptr;
//		Num=GetNum(pPtr);
//		
//		if(Num>GetDBISoeMaxBuffer(dwDevNo))
//			pPtr->dwReadPtr=pPtr->dwWritePtr-GetDBISoeMaxBuffer(dwDevNo);
		
		SM_V(MPV_DBISOE);		
		
		return	pPtr->dwReadPtr;
	}
	else
		return 0;
}

DWORD GetDBISoeWritePtr(DWORD dwDevNo)
{
	TDevie       *pDevice;
	
	pDevice = pGetDevice(dwDevNo);
	TPtr *pPtr;
	
	if(pDevice == NULL)
		return 0;
			
	if(pDevice->pDBISoeWrap!=NULL)
	{
		SM_P(MPV_DBISOE);		

		pPtr = &gpDevice[dwDevNo].pDBISoeWrap->Ptr;

		SM_V(MPV_DBISOE);		

		return	pPtr->dwWritePtr;
	}

	return 0;
}

void IncDBISoeReadPtr(DWORD dwDevNo)
{
//	TPtr *pPtr;
//
//	if(dwDevNo>=gVars.dwDevNum)
//		return ;
//	if(gpDevice[dwDevNo].pDBISoeWrap==0)
//		return;
//	
//	SM_P(MPV_DBISOE);
//	
//	pPtr=&gpDevice[dwDevNo].pDBISoeWrap->Ptr;
//	
//	if(pPtr->dwReadPtr!=pPtr->dwWritePtr)
//	     pPtr->dwReadPtr++;
//	
//	SM_V(MPV_DBISOE);
	gpDevice[dwDevNo].ReadDYXSoePtr++;
	gpDevice[dwDevNo].ReadDYXSoePtr %= MAXYXRECORD;

}

void WriteAbsTimeDBISoe(DWORD dwDevNo,TAbsTimeDBISoe *pAbsTimeDBISoe)
{
	TSysTimeDBISoe SysTimeDBISoe;

	SysTimeDBISoe.nNo=pAbsTimeDBISoe->nNo;
	SysTimeDBISoe.bStatus=pAbsTimeDBISoe->bStatus;
//	AbsTimeTo( &pAbsTimeDBISoe->stTime,&SysTimeDBISoe.stTime);		
	
	WriteSysTimeDBISoe(dwDevNo,&SysTimeDBISoe);
}

void WriteSysTimeDBISoe(DWORD dwDevNo, TSysTimeDBISoe *pSysTimeDBISoe)
{
	//防错
	if(dwDevNo>=gVars.dwDevNum)
		return; 
	
	if(pSysTimeDBISoe->nNo >= gpDevice[dwDevNo].DBINum)//点号越界
		return;
	
	if(gpDevice[dwDevNo].pDBISoeWrap==0)
		return;	

	if(gpDevice[dwDevNo].dwType == MT_LOGIC)
		Logic_WriteSysTimeDBISoe(dwDevNo,pSysTimeDBISoe);
	else 
		Real_WriteSysTimeDBISoe(dwDevNo, pSysTimeDBISoe, FALSE);
}

void Real_WriteSysTimeDBISoe(DWORD dwDevNo,TSysTimeDBISoe *pSysTimeDBISoe, BOOL bByBI)
{
	DWORD dwPtr;	
	TRealBase *pRealBase;	
	TRealDBI *pRealDBI;
	DWORD dwDBINo;//变位遥信序号
	TReflex *pReflex;
	TPtr *pPtr;	

	//防错
	pRealBase=gpDevice[dwDevNo].pRealBase;
	
	if(!pRealBase)
		return;

	dwDBINo = pSysTimeDBISoe->nNo;
	pRealDBI = &pRealBase->pRealDBI[dwDBINo];

/*
	if(pRealDBI->dwControl&0x04)//数据库第一点取反
		pSysTimeDBISoe->bStatus ^= 0x01;

	if(pRealDBI->dwControl&0x08)//数据库第二点取反
		pSysTimeDBISoe->bStatus ^= 0x02;
*/		

	if(!bByBI)
	{
//		if(!(pRealDBI->dwControl&BI_IO_SOE))//IO不产生SOE
//			return;
	}

	SM_P(MPV_DBISOE);
	pPtr=&gpDevice[dwDevNo].pDBISoeWrap->Ptr;
//	dwPtr=pPtr->dwWritePtr%GetDBISoeMaxBuffer(dwDevNo);
	gpDevice[dwDevNo].pSysTimeDBISoe[dwPtr]=*pSysTimeDBISoe;
	
	pPtr->dwWritePtr++;
	SM_V(MPV_DBISOE);

//写入逻辑库中
	pReflex=pRealDBI->pReflex;

	while(pReflex)
	{
		dwDevNo=pReflex->dwDevNo;
		pSysTimeDBISoe->nNo=pReflex->dwNo;
		WriteSysTimeDBISoe(dwDevNo,pSysTimeDBISoe);

		//发遥信变位事件
//		if(gVars.BIEventInfo.bDBISOEAppNum)
//		{
//			for(int j=0;j<gVars.BIEventInfo.bSOEAppNum;j++)
//			{
//				for(int i=0;i<MAX_DEVAPPINFO;i++)
//				{
//					if(gpDevice[dwDevNo].DevAppInfo[i].dwAppID == gVars.BIEventInfo.DBISOEApp[j])
//					{
//						PostEvent(gVars.BIEventInfo.DBISOEApp[j],MEV_DBISOE);
//						break;
//					}
//				}
//			}
//		}
		pReflex=pReflex->pNext;
	}
}

void Logic_WriteSysTimeDBISoe(DWORD dwDevNo,TSysTimeDBISoe *pSysTimeDBISoe)
{
	DWORD dwPtr;
	TPtr *pPtr;
	DWORD dwTempSoe;
	DWORD dwTempBuf;

//	if(ClockInvalidate(&pSysTimeDBISoe->stTime)!=TRUE)
//	    return;
//
//	if(!(gpDevice[dwDevNo].dwControl & MSOE_COVER))
//	{
//		dwTempSoe = GetDBISoeNum(dwDevNo);
//		dwTempBuf = GetDBISoeMaxBuffer(dwDevNo);
//		
//		if(dwTempSoe == dwTempBuf)
//			return;
//	}
//
//	SM_P(MPV_DBISOE);
//
//	pPtr = &gpDevice[dwDevNo].pDBISoeWrap->Ptr;
//	dwPtr = pPtr->dwWritePtr%GetDBISoeMaxBuffer(dwDevNo);
//	gpDevice[dwDevNo].pSysTimeDBISoe[dwPtr]=*pSysTimeDBISoe;
//
//	pPtr->dwWritePtr++;
//
//	SM_V(MPV_DBISOE);

    return;
}

//产生一个SOE
void MakeDBISoe(DWORD dwDevNo,DWORD dwDBINo, BYTE bStatus)
{
	TSysTimeDBISoe SysTimeDBISoe;
	
//	ReadSysTime(&SysTimeDBISoe.stTime);	
	
	SysTimeDBISoe.bStatus = bStatus;
	SysTimeDBISoe.nNo = dwDBINo;
	
	Real_WriteSysTimeDBISoe(dwDevNo, &SysTimeDBISoe, TRUE);
}

//带自动修正读指针功能的ReadSysTimeDBISoe For 实际库
bool Real_ReadSysTimeDBISoe(DWORD dwDevNo,DWORD *pdwReadPtr,TSysTimeDBISoe *pSysTimeDBISoe)
{
//	DWORD DBISoeNum;
//		
//	SM_P(MPV_DBISOE);
//
//	DBISoeNum = gpDevice[dwDevNo].pDBISoeWrap->Ptr.dwWritePtr-*pdwReadPtr;
//
//	if(DBISoeNum>GetDBISoeMaxBuffer(dwDevNo))
//		*pdwReadPtr=gpDevice[dwDevNo].pDBISoeWrap->Ptr.dwWritePtr-GetDBISoeMaxBuffer(dwDevNo);
//	*pSysTimeDBISoe = gpDevice[dwDevNo].pSysTimeDBISoe[ (*pdwReadPtr)%GetDBISoeMaxBuffer(dwDevNo) ];
//
//	SM_V(MPV_DBISOE);

    return TRUE;
}


bool ReadSysTimeDBISoe(DWORD dwDevNo,TSysTimeDBISoe *pSysTimeDBISoe)
{
//	//防错
//	if(dwDevNo>=gVars.dwDevNum)
//		return FALSE; 
//	if(!gpDevice[dwDevNo].pDBISoeWrap)
//		return FALSE;
//	
//	if(*pdwReadPtr==gpDevice[dwDevNo].pDBISoeWrap->Ptr.dwWritePtr)
//		return FALSE;
//	
//	if(gpDevice[dwDevNo].dwType == MT_LOGIC)
//		return Logic_ReadSysTimeDBISoe(dwDevNo,pdwReadPtr,pSysTimeDBISoe);
	
//	return Real_ReadSysTimeDBISoe(dwDevNo,pdwReadPtr,pSysTimeDBISoe);	
	if(dwDevNo>=gVars.dwDevNum)
		return FALSE;
		
	if(gpDevice[dwDevNo].ReadDYXSoePtr==gVars.YxDSoeWritePtr)
	{
		return FALSE;
	}
	pSysTimeDBISoe->bStatus = YXDSoeRecord[gpDevice[dwDevNo].ReadDYXSoePtr].bStatus;
	pSysTimeDBISoe->stTime = YXDSoeRecord[gpDevice[dwDevNo].ReadDYXSoePtr].stTime;
	pSysTimeDBISoe->nNo = YXDSoeRecord[gpDevice[dwDevNo].ReadDYXSoePtr].point;

	return TRUE;


}



//带自动修正读指针功能的ReadSysTimeDBISoe For 逻辑库
bool Logic_ReadSysTimeDBISoe(DWORD dwDevNo,DWORD *pdwReadPtr,TSysTimeDBISoe *pSysTimeDBISoe)
{
//	TSysTimeDBISoe SysTimeDBISoe;
//	DWORD DBISoeNum;
//	TLogicBase *pLogic;
//	TAbsTime t1, t2;
//	
//	SM_P(MPV_DBISOE);
//
//	DBISoeNum = gpDevice[dwDevNo].pDBISoeWrap->Ptr.dwWritePtr-*pdwReadPtr;
//
//	if(DBISoeNum>GetDBISoeMaxBuffer(dwDevNo))
//		*pdwReadPtr = gpDevice[dwDevNo].pDBISoeWrap->Ptr.dwWritePtr-GetDBISoeMaxBuffer(dwDevNo);
//
//	SysTimeDBISoe = gpDevice[dwDevNo].pSysTimeDBISoe[ *pdwReadPtr%GetDBISoeMaxBuffer(dwDevNo) ];
//	
//    // 检查逻辑库是否存在，是否有独立时钟
//	if((gpDevice[dwDevNo].dwControl & LOG_CONF_MASKTIME)==LOG_CONF_SELFTIME )
//    {
//	    // Get logic base pointer
//		pLogic = gpDevice[dwDevNo].pLogicBase;
//   
//        // Get old abstime of system
//        ToAbsTime(&SysTimeDBISoe.stTime,&t2);
//
//        // Calucate the difference between new timer and system timer
//		if(pLogic->stRelTime.LessMagic==1)//逻辑库时间<系统时间
//		{
//			t1.Hi =t2.Hi- pLogic->stRelTime.RelTimeDif.Hi;
//			t1.Lo = t2.Lo-pLogic->stRelTime.RelTimeDif.Lo ;
//			if( pLogic->stRelTime.RelTimeDif.Lo > t2.Lo ) t1.Hi--;
//		}
//		else
//		{
//			  t1.Hi = pLogic->stRelTime.RelTimeDif.Hi + t2.Hi;
//			  t1.Lo = pLogic->stRelTime.RelTimeDif.Lo + t2.Lo;
//			  if( pLogic->stRelTime.RelTimeDif.Lo > t1.Lo ) t1.Hi++;
//		}
//
//        // Convert new systime to abstime
//        AbsTimeTo( &t1, &SysTimeDBISoe.stTime);
//    }
//
//	*pSysTimeDBISoe = SysTimeDBISoe;   
//
//	SM_V(MPV_DBISOE);

    return TRUE;
}



bool ReadAbsTimeDBISoe(DWORD dwDevNo,DWORD *pdwReadPtr,TAbsTimeDBISoe *pAbsTimeDBISoe)
{
//	TSysTimeDBISoe SysTimeDBISoe;
//	
//	if(ReadSysTimeDBISoe( dwDevNo,pdwReadPtr, &SysTimeDBISoe )!=TRUE)
//		return FALSE;
//	
//	pAbsTimeDBISoe->nNo=SysTimeDBISoe.nNo;
//	pAbsTimeDBISoe->bStatus=SysTimeDBISoe.bStatus;
//	ToAbsTime(&SysTimeDBISoe.stTime,&pAbsTimeDBISoe->stTime);
	
	return TRUE;
}	



