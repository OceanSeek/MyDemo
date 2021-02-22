#include "sys.h"
extern TBurstYXRecord YXSoeRecord[MAXYXRECORD];

//==========================  与应用层的接口  =============================

static int GetNum(int DevID)
{
	int SoeNum = 0;
	SoeNum = (gpDevice[DevID].ReadYXSoePtr > gVars.YxSoeWritePtr) ?gVars.YxSoeWritePtr + MAXYXRECORD - gpDevice[DevID].ReadYXSoePtr\
		:gVars.YxSoeWritePtr - gpDevice[DevID].ReadYXSoePtr;
	log("gpDevice[DevID].ReadYXSoePtr is %d gVars.YxSoeWritePtr %d\n",gpDevice[DevID].ReadYXSoePtr, gVars.YxSoeWritePtr);
	return SoeNum;
}


DWORD GetSoeNum(DWORD dwDevID)
{
//	if(dwDevID>=gVars.dwDevNum)
//		return 0;
//	if(gpDevice[dwDevID].pSoeWrap==0)
//		return 0;
//	DWORD SoeNum;
//	DWORD SoeMaxBuffer;
//	
//	TPtr *pPtr;		
//	SM_P(MPV_SOE);
//	pPtr=&gpDevice[dwDevID].pSoeWrap->Ptr;
//	SoeNum=GetNum(pPtr);
//	SoeMaxBuffer = GetSoeMaxBuffer(dwDevID);
//	if(SoeNum > SoeMaxBuffer)
//	{
//		pPtr->dwReadPtr = pPtr->dwWritePtr - SoeMaxBuffer;
//		SoeNum = SoeMaxBuffer;
//	}
//	SM_V(MPV_SOE);	
//	return SoeNum;

	if(dwDevID>=gVars.dwDevNum)
		return 0;
			
	DWORD SoeNum;

	SoeNum=GetNum(dwDevID);
	log("SoeNum is %d \n",SoeNum);
	return SoeNum;

}

void ClearSoe(DWORD dwDevID)
{
//	if(dwDevID>=gVars.dwDevNum)
//		return ;
//	if(gpDevice[dwDevID].pSoeWrap==0)
//		return;
//	TPtr *pPtr;
//	SM_P(MPV_SOE);		
//	pPtr=&gpDevice[dwDevID].pSoeWrap->Ptr;
//	pPtr->dwReadPtr = pPtr->dwWritePtr;
//	SM_V(MPV_SOE);
}

DWORD GetSoeReadPtr(DWORD dwDevID)
{
//	TDevice       *pDevice;
//	DWORD Num;
//	TPtr *pPtr;	
//
//	pDevice = pGetDevice(dwDevID);
//
//	if(pDevice == NULL)
//		return 0;
//
//	if(pDevice->pSoeWrap == NULL)
//		return 0;
//
//	SM_P(MPV_SOE);		
//	pPtr = &gpDevice[dwDevID].pSoeWrap->Ptr;
//	Num = GetNum(pPtr);
//	
//	if(Num>GetSoeMaxBuffer(dwDevID))
//		pPtr->dwReadPtr = pPtr->dwWritePtr - GetSoeMaxBuffer(dwDevID);
//	
//	SM_V(MPV_SOE);		
//	
//	return	pPtr->dwReadPtr;
}

DWORD GetSoeWritePtr(DWORD dwDevID)
{
//	TDevice       *pDevice;
//	TPtr *pPtr;
//
//	pDevice = pGetDevice(dwDevID);
//	
//	if(pDevice == NULL)
//		return 0;
//		
//	if(pDevice->pSoeWrap == NULL)
//		return 0;
//
//	SM_P(MPV_SOE);		
//	pPtr = &gpDevice[dwDevID].pSoeWrap->Ptr;
//	SM_V(MPV_SOE);		
//		
//	return	pPtr->dwWritePtr;
}



void WriteAbsTimeSoe(DWORD dwDevID, TAbsTimeSoe *pAbsTimeSoe)
{
//	TSysTimeSoe SysTimeSoe;
//	
//	SysTimeSoe.nNo = pAbsTimeSoe->nNo;
//	SysTimeSoe.bStatus = pAbsTimeSoe->bStatus;
//	AbsTimeTo(&pAbsTimeSoe->stTime,&SysTimeSoe.stTime);		
//
//	WriteSysTimeSoe(dwDevID, &SysTimeSoe);
}

void WriteSysTimeSoe(DWORD dwDevID, TSysTimeSoe *pSysTimeSoe)
{
//	if(dwDevID >= gVars.dwDevNum)
//		return; 
//	
//	if(pSysTimeSoe->nNo >= gpDevice[dwDevID].dwBINum)//点号越界
//		return;
//	
//	if(gpDevice[dwDevID].pSoeWrap==0)
//		return;	
//
//	if(gpDevice[dwDevID].dwType == MT_LOGIC)
//		Logic_WriteSysTimeSoe(dwDevID, pSysTimeSoe);
//	else 
//		Real_WriteSysTimeSoe(dwDevID, pSysTimeSoe, FALSE);
}

void IncSoeReadPtr(DWORD dwDevID)
{
//	TPtr *pPtr;
//
//	if(dwDevID >= gVars.dwDevNum)
//		return;
//	if(gpDevice[dwDevID].pSoeWrap == 0)
//		return;
//
//	pPtr = &gpDevice[dwDevID].pSoeWrap->Ptr;
//
//	if(pPtr->dwReadPtr != pPtr->dwWritePtr)//wcpwcp
//		pPtr->dwReadPtr++;
	gpDevice[dwDevID].ReadYXPtr++;
	gpDevice[dwDevID].ReadYXPtr %= MAXYXRECORD;

}

bool ReadSysTimeSoe(DWORD dwDevID,TSysTimeSoe *pSysTimeSoe)
{
	if(dwDevID>=gVars.dwDevNum)
		return FALSE;
		
	if(gpDevice[dwDevID].ReadYXSoePtr==gVars.YxSoeWritePtr)
	{
		return FALSE;
	}
	pSysTimeSoe->bStatus = YXSoeRecord[gpDevice[dwDevID].ReadYXSoePtr].bStatus;
	pSysTimeSoe->stTime = YXSoeRecord[gpDevice[dwDevID].ReadYXSoePtr].stTime;
	pSysTimeSoe->nNo = YXSoeRecord[gpDevice[dwDevID].ReadYXSoePtr].point;

	return TRUE;

}


