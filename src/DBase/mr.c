#include "sys.h"

//==========================  与应用层的接口  =============================
extern TSysTimeMr MrRecord[MAXYXRECORD];

static int GetNum(int DevID)
{
	int MrNum = 0;
	MrNum = (gpDevice[DevID].ReadMrPtr > gVars.MrWritePtr) ?gVars.MrWritePtr + MAXYXRECORD - gpDevice[DevID].ReadMrPtr\
		:gVars.MrWritePtr - gpDevice[DevID].ReadMrPtr;
	log("gpDevice[DevID].ReadMrPtr is %d gVars.MrWritePtr %d\n",gpDevice[DevID].ReadMrPtr, gVars.MrWritePtr);
	return MrNum;
}


//取事故简报个数
DWORD GetMrNum(DWORD dwDevID)
{
//	DWORD MrNum;
//
//	if(dwDevID>=gVars.dwDevNum)
//		return 0;
//	if(gpDevice[dwDevID].pMrWrap==0)
//		return 0;
//		
//	TPtr *pPtr;		
//	SM_P(MPV_MR);
//	pPtr=&gpDevice[dwDevID].pMrWrap->Ptr;
//	MrNum=GetNum(pPtr);
//	if(MrNum>MAX_DEV_MR_NUM)
//	{
//		pPtr->dwReadPtr=pPtr->dwWritePtr-MAX_DEV_MR_NUM;
//		MrNum=MAX_DEV_MR_NUM;
//	}
//	SM_V(MPV_MR);	
//	return MrNum;

	if(dwDevID>=gVars.dwDevNum)
		return 0;
			
	DWORD MrNum;

	MrNum=GetNum(dwDevID);
	log("MrNum is %d \n",MrNum);
	return MrNum;
	
}


void IncMrReadPtr(DWORD dwDevID)
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
	gpDevice[dwDevID].ReadMrPtr++;
	gpDevice[dwDevID].ReadMrPtr %= MAXYXRECORD;

}

bool ReadSysTimeMr(DWORD dwDevID,TSysTimeMr *pTSysTimeMr)
{
	if(dwDevID>=gVars.dwDevNum)
		return FALSE;
		
	if(gpDevice[dwDevID].ReadMrPtr==gVars.MrWritePtr)
	{
		return FALSE;
	}
	*pTSysTimeMr = MrRecord[gpDevice[dwDevID].ReadMrPtr];

	return TRUE;

}



