#include "sys.h"
extern TBurstYXRecord DYXBrustRecord[MAXYXRECORD];

//==========================  与应用层的接口  =============================

static int GetNum(int DevID)
{
	int YXNum = 0;
	YXNum = (gpDevice[DevID].ReadDYXPtr > gVars.DYxWritePtr) ?gVars.DYxWritePtr + MAXYXRECORD - gpDevice[DevID].ReadDYXPtr\
		:gVars.DYxWritePtr - gpDevice[DevID].ReadDYXPtr;
	log("gpDevice[DevID].ReadDYXPtr is %d gVars.DYxWritePtr %d\n",gpDevice[DevID].ReadDYXPtr, gVars.DYxWritePtr);
	return YXNum;
}


DWORD GetBurstDBINum(DWORD dwDevID)
{
//	if(dwDevID>=gVars.dwDevNum)
//		return 0;
//			
//	TDevie       *pDevice;
//	pDevice = pGetDevice(dwDevID);
//	if(pDevice==NULL)
//		return 0;
//	TBurstDBIWrap *pBurstDBIWrap=pDevice->pBurstDBIWrap;
//	if(pBurstDBIWrap==0)
//		return 0;	
//	DWORD DBIBurstNum;
////	SM_P(MPV_DBIBURST);
////	TPtr *pPtr=&pBurstDBIWrap->Ptr;
////	DBIBurstNum=GetNum(pPtr);
////	if(DBIBurstNum>GetBurstDBIMaxBuffer(dwDevID))
////	{
////		pPtr->dwReadPtr=pPtr->dwWritePtr-GetBurstDBIMaxBuffer(dwDevID);
////		DBIBurstNum = GetBurstDBIMaxBuffer(dwDevID);
////	}
//		
////	SM_V(MPV_DBIBURST);
	if(dwDevID>=gVars.dwDevNum)
		return 0;
			
	DWORD DBIBurstNum;

	DBIBurstNum=GetNum(dwDevID);
	log("DBIBurstNum is %d \n",DBIBurstNum);

	return DBIBurstNum;
	
}

bool ReadBurstDBI(DWORD dwDevID,TBurstDBI *pBurstDBI)
{
	
//	if(dwDevID>=gVars.dwDevNum)
//		return FALSE;
//	TBurstDBIWrap *pBurstDBIWrap=gpDevice[dwDevID].pBurstDBIWrap;
//	if(pBurstDBIWrap==0)
//		return FALSE;	
//	DWORD	DBIBurstNum;
//	SM_P(MPV_DBIBURST);
//	TPtr *pPtr=&pBurstDBIWrap->Ptr;
//	if(*pdwReadPtr==pPtr->dwWritePtr)
//	{
//		SM_V(MPV_DBIBURST);
//		return FALSE;
//	}
//	DBIBurstNum=pPtr->dwWritePtr-*pdwReadPtr;
//	if(DBIBurstNum>GetBurstDBIMaxBuffer(dwDevID))
//		*pdwReadPtr=pPtr->dwWritePtr-GetBurstDBIMaxBuffer(dwDevID);
//	*pBurstDBI=gpDevice[dwDevID].pBurstDBI[*pdwReadPtr%GetBurstDBIMaxBuffer(dwDevID)];
//	SM_V(MPV_DBIBURST);


	if(dwDevID>=gVars.dwDevNum)
		return FALSE;
		
	if(gpDevice[dwDevID].ReadDYXPtr==gVars.DYxWritePtr)
	{
		return FALSE;
	}
	pBurstDBI->bStatus = DYXBrustRecord[gpDevice[dwDevID].ReadYXPtr].bStatus;
	pBurstDBI->stTime = DYXBrustRecord[gpDevice[dwDevID].ReadYXPtr].stTime;
	pBurstDBI->nNo = DYXBrustRecord[gpDevice[dwDevID].ReadYXPtr].point;
	
    return TRUE;

	return TRUE;
	
    
}


void IncBurstDBIReadPtr(DWORD dwDevID)
{
//	TDevie      *pDevice;
//	pDevice = pGetDevice(dwDevID);
//	if(pDevice==NULL)
//		return;
//	TBurstDBIWrap *pBurstDBIWrap=pDevice->pBurstDBIWrap;
//	if(pBurstDBIWrap==0)
//		return;	
//	
//	SM_P(MPV_DBIBURST);
//	TPtr *pPtr=&pBurstDBIWrap->Ptr;
//	if(pPtr->dwReadPtr!=pPtr->dwWritePtr)
//	     pPtr->dwReadPtr++;
//	SM_V(MPV_DBIBURST);
	gpDevice[dwDevID].ReadDYXPtr++;
	gpDevice[dwDevID].ReadDYXPtr %= MAXYXRECORD;


}

/*以下三个函数是给，发送数据时做标记用的*/
void SetDBIFlag(DWORD dwDevID, DWORD dwPoint, WORD wFlag)
{
	TLogicDBI* pLogicDBI;
	DWORD dwNum;

	if(dwDevID>=gVars.dwDevNum)
		return;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return;

	dwNum = gpDevice[dwDevID].DBINum;
	pLogicDBI = gpDevice[dwDevID].pLogicBase->pLogicDBI;

	if(pLogicDBI == NULL || dwNum == 0)
		return;

//	for(i=0; i<dwNum; i++)
		pLogicDBI[dwPoint].wFlag = wFlag;
}


DWORD GetDBINum(DWORD dwDevID)
{
	if(dwDevID>=gVars.dwDevNum)
		return 0;
	return gpDevice[dwDevID].DBINum;	
}
WORD GetDBIFlag(DWORD dwDevID, DWORD dwNo)
{
	TLogicDBI* pLogicDBI;
	DWORD dwNum;

	if(dwDevID >= gVars.dwDevNum)
		return FALSE;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return FALSE;

	dwNum = gpDevice[dwDevID].DBINum;
	pLogicDBI = gpDevice[dwDevID].pLogicBase->pLogicDBI;

	if(pLogicDBI == NULL || dwNum == 0)
		return FALSE;

	if(dwNo >= dwNum)
		return FALSE;
	
	return pLogicDBI[dwNo].wFlag;
}


BYTE ReadDBI( DWORD dwDevID,DWORD dwDBINo, WORD* pStatus)
{
	if(dwDevID>=gVars.dwDevNum)
		return 0;

	if(dwDBINo>=gpDevice[dwDevID].DBINum)
		return 0;
		
	return (*gpDevice[dwDevID].ReadDBI)(dwDevID,dwDBINo, pStatus);
}


