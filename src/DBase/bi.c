#include "sys.h"
extern TBurstYXRecord YXBrustRecord[MAXYXRECORD];

//==========================  与应用层的接口  =============================

//void WriteOneLogicBurstBI(TReflex *pReflex,BYTE bStatus)
//{
////写入逻辑库中
////	DWORD dwDevID;
////	
////	for(;pReflex;pReflex=pReflex->pNext)
////	{
////		dwDevID=pReflex->dwDevID;
////			
////		if(dwDevID>=gVars.dwDevNum)
////			continue;
////		TDevie * pDevice=pGetDevice(dwDevID);
////		
////		if(pDevice==NULL)
////			continue;
////		
////		if(!WriteOneBurstBI(dwDevID,pReflex->dwNo,bStatus ))
////			continue;
////		
////		if(gVars.BIEventInfo.bBurstBIAppNum)
////		{
////			for(int j=0;j<gVars.BIEventInfo.bBurstBIAppNum;j++)
////			{
////				for(int i=0;i<MAX_DEVAPPINFO;i++)
////				{
////					if(pDevice->DevAppInfo[i].dwAppID==gVars.BIEventInfo.BurstBIApp[j])
////					{
////						PostEvent(gVars.BIEventInfo.BurstBIApp[j],MEV_BURSTBI);
////						break;
////					}
////				}
////			}
////		}
////	}
//}


void WriteOneLogicBurstBI(TReflex *pReflex, TBurstBI* pBurstBI)
{
////写入逻辑库中
//	DWORD dwDevID;
//	TDevie * pDevice;
//
//	if(pReflex == NULL || pBurstBI == NULL)
//		return;
//
//	for(;pReflex;pReflex=pReflex->pNext)
//	{
//		dwDevID = pReflex->dwDevID;
//			
//		if(dwDevID >= gVars.dwDevNum)
//				continue;
//		
//		pDevice = pGetDevice(dwDevID);
//		
//		if(pDevice == NULL)
//			continue;
//		
//		pBurstBI->nNo = pReflex->dwNo;
//		
//		if(!WriteOneBurstBI(dwDevID, pBurstBI))
//				continue;
//
//		if(gVars.BIEventInfo.bBurstBIAppNum)
//		{
//			for(int j=0;j<gVars.BIEventInfo.bBurstBIAppNum;j++)
//			{
//				for(int i=0;i<MAX_DEVAPPINFO;i++)
//				{
//					if(pDevice->DevAppInfo[i].dwAppID==gVars.BIEventInfo.BurstBIApp[j])
//					{
//						PostEvent(gVars.BIEventInfo.BurstBIApp[j],MEV_BURSTBI);
//						break;
//					}
//				}
//			}
//		}
//	}
}


bool WriteOneBurstBI(DWORD dwDevID,DWORD dwBINo,BYTE bStatus )
{
	TBurstBIWrap *pBurstBIWrap;
	TBurstBI *ppBurstBI;
	TPtr *pPtr;
	DWORD dwWptr;
	CP56Time2a_T sysTime;

//	ReadSysTime(&sysTime);

//	SM_P(MPV_BIBURST);
	
	pBurstBIWrap = gpDevice[dwDevID].pBurstBIWrap;
	
	if(pBurstBIWrap==0)
	{
//		SM_V(MPV_BIBURST);
		return FALSE;	
	}
	pPtr = &pBurstBIWrap->Ptr;
//	dwWptr = pPtr->dwWritePtr%GetBurstBIMaxBuffer(dwDevID);
	ppBurstBI = &gpDevice[dwDevID].pBurstBI[dwWptr];
	ppBurstBI->nNo = dwBINo;
	ppBurstBI->bStatus = bStatus;
	ppBurstBI->stTime = sysTime;
	pPtr->dwWritePtr++;

//	SM_V(MPV_BIBURST);
	
	return TRUE;
	
}

//bool WriteOneBurstBI(DWORD dwDevID, TBurstBI* pBurstBI)
//{
//	TBurstBIWrap *pBurstBIWrap;
//	TBurstBI *ppBurstBI;
//	TPtr *pPtr;
//	DWORD dwWptr;
//
////	SM_P(MPV_BIBURST);
//	pBurstBIWrap=gpDevice[dwDevID].pBurstBIWrap;
//
//	if(pBurstBIWrap==0)
//	{
////		SM_V(MPV_BIBURST);
//		return FALSE;	
//	}
//	pPtr=&pBurstBIWrap->Ptr;
////	dwWptr=pPtr->dwWritePtr%GetBurstBIMaxBuffer(dwDevID);
//	ppBurstBI = &gpDevice[dwDevID].pBurstBI[dwWptr];
//	
//	*ppBurstBI = *pBurstBI;
//
//	pPtr->dwWritePtr++;
//
////	SM_V(MPV_BIBURST);
//	
//	return TRUE;
//}

DWORD GetBINum(DWORD dwDevID)
{
	if(dwDevID >= gVars.dwDevNum)
		return 0;
	return gpDevice[dwDevID].BINum;	
}

WORD GetBIFlag(DWORD dwDevID, DWORD dwNo)
{
	TLogicBI* pLogicBI;
	DWORD dwNum;

	if(dwDevID >= gVars.dwDevNum)
		return FALSE;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return FALSE;

	dwNum = gpDevice[dwDevID].BINum;
	pLogicBI = gpDevice[dwDevID].pLogicBase->pLogicBI;

	if(pLogicBI == NULL || dwNum == 0)
		return FALSE;

	if(dwNo >= dwNum)
		return FALSE;
	
	return pLogicBI[dwNo].wFlag;
}

BYTE ReadBI( DWORD dwDevID,DWORD dwBINo, WORD* pStatus)
{
	if(dwDevID >= gVars.dwDevNum)
		return 0;

	if(dwBINo >= gpDevice[dwDevID].BINum)
		return 0;

	return (*gpDevice[dwDevID].ReadBI)(dwDevID,dwBINo, pStatus);
}


bool ReadBurstBI(DWORD dwDevID,TBurstBI *pBurstBI)
{
	if(dwDevID>=gVars.dwDevNum)
		return FALSE;
		
	if(gpDevice[dwDevID].ReadYXPtr==gVars.YxWritePtr)
	{
		return FALSE;
	}
	pBurstBI->bStatus = YXBrustRecord[gpDevice[dwDevID].ReadYXPtr].bStatus;
	pBurstBI->stTime = YXBrustRecord[gpDevice[dwDevID].ReadYXPtr].stTime;
	pBurstBI->nNo = YXBrustRecord[gpDevice[dwDevID].ReadYXPtr].point;
	
    return TRUE;
}
//bool ReadBurstBI(DWORD dwDevID,DWORD *pdwReadPtr,TBurstBI *pBurstBI)
//{
//	
//	if(dwDevID>=gVars.dwDevNum)
//		return FALSE;
//	TBurstBIWrap *pBurstBIWrap=gpDevice[dwDevID].pBurstBIWrap;
//	if(pBurstBIWrap==0)
//		return FALSE;	
//	DWORD	BIBurstNum;
//	SM_P(MPV_BIBURST);
//	TPtr *pPtr=&pBurstBIWrap->Ptr;
//	if(*pdwReadPtr==pPtr->dwWritePtr)
//	{
//		SM_V(MPV_BIBURST);
//		return FALSE;
//	}
//	BIBurstNum=pPtr->dwWritePtr-*pdwReadPtr;
//	if(BIBurstNum>GetBurstBIMaxBuffer(dwDevID))
//		*pdwReadPtr=pPtr->dwWritePtr-GetBurstBIMaxBuffer(dwDevID);
//	*pBurstBI=gpDevice[dwDevID].pBurstBI[*pdwReadPtr%GetBurstBIMaxBuffer(dwDevID)];
//	SM_V(MPV_BIBURST);
//	return TRUE;
//	
//    
//}
static int GetNum(int DevID)
{
	int YXNum = 0;
	YXNum = (gpDevice[DevID].ReadYXPtr > gVars.YxWritePtr) ?gVars.YxWritePtr + MAXYXRECORD - gpDevice[DevID].ReadYXPtr\
		:gVars.YxWritePtr - gpDevice[DevID].ReadYXPtr;
	log("gpDevice[DevID].ReadYXPtr is %d gVars.YxWritePtr %d\n",gpDevice[DevID].ReadYXPtr, gVars.YxWritePtr);
	return YXNum;
}

DWORD GetBurstBINum(DWORD dwDevID)
{
	if(dwDevID>=gVars.dwDevNum)
		return 0;
			
	DWORD BIBurstNum;

	BIBurstNum=GetNum(dwDevID);
	log("BIBurstNum is %d \n",BIBurstNum);
	return BIBurstNum;
	
}
DWORD GetBurstBIReadPtr(DWORD dwDevID)
{

//	TDevie       *pDevice;
//	pDevice = pGetDevice(dwDevID);
//	if(pDevice==NULL)
//		return 0;
//	DWORD Num;
//	TPtr *pPtr;	
//	if(pDevice->pBurstBIWrap!=NULL)
//	{
//		SM_P(MPV_BIBURST);
//		pPtr=&pDevice->pBurstBIWrap->Ptr;
//		Num=GetNum(pPtr);
//		if(Num>GetBurstBIMaxBuffer(dwDevID))
//			pPtr->dwReadPtr=pPtr->dwWritePtr-GetBurstBIMaxBuffer(dwDevID);
//		SM_V(MPV_BIBURST);
//		return	pPtr->dwReadPtr;
//	}
//	else
//		return 0;
}

void ClearBurstBI(DWORD dwDevID)
{
//	if(dwDevID>=gVars.dwDevNum)
//		return;
//			
//	TDevie       *pDevice;
//	pDevice = pGetDevice(dwDevID);
//	if(pDevice==NULL)
//		return;
//	TBurstBIWrap *pBurstBIWrap=pDevice->pBurstBIWrap;
//	if(pBurstBIWrap==0)
//		return;	
//	SM_P(MPV_BIBURST);
//	TPtr *pPtr=&pBurstBIWrap->Ptr;
//	pPtr->dwReadPtr = pPtr->dwWritePtr;
//	SM_V(MPV_BIBURST);
}

void IncBurstBIReadPtr(DWORD dwDevID)
{
//	TDevie       *pDevice;
//	pDevice = pGetDevice(dwDevID);
//	if(pDevice==NULL)
//		return;
//	TBurstBIWrap *pBurstBIWrap=pDevice->pBurstBIWrap;
//	if(pBurstBIWrap==0)
//		return;	
//	
//	SM_P(MPV_BIBURST);
//	TPtr *pPtr=&pBurstBIWrap->Ptr;
//	if(pPtr->dwReadPtr!=pPtr->dwWritePtr)
//	     pPtr->dwReadPtr++;
//	SM_V(MPV_BIBURST);
	gpDevice[dwDevID].ReadYXPtr++;
	gpDevice[dwDevID].ReadYXPtr %= MAXYXRECORD;

}
void SetBINum(DWORD dwDevID,DWORD MaxNum)
{
//	TDevie *pDevice;
//
//	pDevice=pGetDevice(dwDevID);
//	if(pDevice==0)
//		return;
//	if(pDevice->dwBINum>MaxNum)
//		pDevice->dwBINum=MaxNum;
	
}

/*以下三个函数是给，发送数据时做标记用的*/
void SetBIFlag(DWORD dwDevID, DWORD dwPoint, WORD wFlag)
{
	TLogicBI* pLogicBI;
	int i;
	DWORD dwNum;

	if(dwDevID>=gVars.dwDevNum)
		return;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return;

	dwNum = gpDevice[dwDevID].BINum;
	pLogicBI = gpDevice[dwDevID].pLogicBase->pLogicBI;

	if(pLogicBI == NULL || dwNum == 0)
		return;

	for(i=0; i<dwNum; i++)
		pLogicBI[i].wFlag = wFlag;
}


