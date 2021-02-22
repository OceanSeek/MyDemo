#include "sys.h"

extern TBurstBcd BCDRecord[MAXYCRECORD];


//==========================  与应用层的接口  =============================

WORD GetBcdFlag(DWORD dwDevID, DWORD dwNo)
{
	TLogicBCD* pLogicBCD;
	DWORD dwNum;

	if(dwDevID >= gVars.dwDevNum)
		return FALSE;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return FALSE;

	dwNum = gpDevice[dwDevID].dwBCDNum;
	pLogicBCD = gpDevice[dwDevID].pLogicBase->pLogicBCD;

	if(pLogicBCD == NULL || dwNum == 0)
		return FALSE;

	if(dwNo >= dwNum)
		return FALSE;
	
	return pLogicBCD[dwNo].wFlag;
}


DWORD GetBCDNum(DWORD dwDevID)
{
	if(dwDevID>=gVars.dwDevNum)
		return 0;
	return gpDevice[dwDevID].dwBCDNum;	
}


//规约层使用读BCD
DWORD ReadBCD(DWORD dwDevID,DWORD dwBCDNo, WORD* pStatus)
{
	if(dwDevID >= gVars.dwDevNum)
		return 0;
	
	if(dwBCDNo >= gpDevice[dwDevID].dwBCDNum)
		return 0;
		
	return (*gpDevice[dwDevID].ReadBCD)(dwDevID,dwBCDNo, pStatus);
}


void SetBcdFlag(DWORD dwDevID, DWORD dwNo, WORD wFlag)
{
	TLogicBCD* pLogicBCD;
	DWORD dwNum;

	if(dwDevID>=gVars.dwDevNum)
		return;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return;

	dwNum = gpDevice[dwDevID].dwBCDNum;
	pLogicBCD = gpDevice[dwDevID].pLogicBase->pLogicBCD;

	if(pLogicBCD == NULL || dwNum == 0)
		return;

	if(dwNo >= dwNum)
		return;
	
	pLogicBCD[dwNo].wFlag = wFlag;
}

TLogicBCD* GetLogicBCD(DWORD dwDevID, DWORD *dwBCDNum)
{
	TLogicBase	*pLogicBase;
	TLogicBCD* pLogicBCD;
	

	if(dwDevID >= gVars.dwDevNum)
		return NULL;

	pLogicBase = gpDevice[dwDevID].pLogicBase;

	if(pLogicBase == 0)
		return NULL;

	pLogicBCD = gpDevice[dwDevID].pLogicBase->pLogicBCD;
	*dwBCDNum = gpDevice[dwDevID].dwBCDNum;

	if(dwBCDNum == 0)
		return NULL;

	return pLogicBCD;
}

static int GetNum(int DevID)
{
	int BCDNum = 0;
	BCDNum = (gpDevice[DevID].ReadBCDPtr > gVars.BCDWritePtr) ?gVars.BCDWritePtr + MAXYCRECORD - gpDevice[DevID].ReadBCDPtr\
		:gVars.BCDWritePtr - gpDevice[DevID].ReadBCDPtr;
	log("gpDevice[DevID].ReadBCDPtr is %d gVars.BCDWritePtr %d\n",gpDevice[DevID].ReadBCDPtr, gVars.BCDWritePtr);
	return BCDNum;
}


BOOL ReadBurstBcd(DWORD dwDevID, TBurstBcd* pBurstBcd)//获取变化bcd个数
{

	if(dwDevID>=gVars.dwDevNum)
		return FALSE;
		
	if(gpDevice[dwDevID].ReadBCDPtr==gVars.BCDWritePtr)
	{
		return FALSE;
	}
	*pBurstBcd = BCDRecord[gpDevice[dwDevID].ReadBCDPtr];

	return TRUE;


}

DWORD GetBurstBcdNum(DWORD dwDevID)//获取变化遥测个数
{
//	TLogicBase	*pLogicBase;
//	TBurstBcdWrap   *pBurstBcdWrap;
//	
//	return 0;
//
//	if(dwDevID>=gVars.dwDevNum)
//		return 0;
//
//	pLogicBase=gpDevice[dwDevID].pLogicBase;
//	
//	if(pLogicBase==0)
//		return 0;
//
//	pBurstBcdWrap = &pLogicBase->burstBcdWrap;
//
//	if(!pLogicBase->burstBcdWrap.pBurstBcd)
//		return 0;
	
//	if(pBurstBcdWrap->dwBcdBurstReadPtr < pBurstBcdWrap->dwBcdBurstWritePtr)
//		return pBurstBcdWrap->dwBcdBurstWritePtr - pBurstBcdWrap->dwBcdBurstReadPtr;
//	else
//		return ScanBurstBcd(dwDevID);

	if(dwDevID>=gVars.dwDevNum)
		return 0;
			
	DWORD BCDNum;

	BCDNum=GetNum(dwDevID);
	log("BCDNum is %d \n",BCDNum);
	return BCDNum;
}

void IncBCDReadPtr(DWORD dwDevID)
{
//	TPtr *pPtr;
//
//	if(dwDevID>=gVars.dwDevNum)
//		return ;
//	if(gpDevice[dwDevID].pAitWrap==0)
//		return;
////	SM_P(MPV_AIT);
//	
//	pPtr=&gpDevice[dwDevID].pAitWrap->Ptr;
//	
//	if(pPtr == NULL)
//		return;
//
//	if(pPtr->dwReadPtr!=pPtr->dwWritePtr)
//	     pPtr->dwReadPtr++;
//	
////	SM_V(MPV_AIT);
	gpDevice[dwDevID].ReadBCDPtr++;
	gpDevice[dwDevID].ReadBCDPtr %= MAXYCRECORD;


}



