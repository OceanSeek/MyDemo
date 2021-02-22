#include "sys.h"

extern TSysTimeAit	AitRecord[MAXYCRECORD];


//==========================  与应用层的接口  =============================
static int GetNum(int DevID)
{
	int SoeNum = 0;
	SoeNum = (gpDevice[DevID].ReadAitPtr > gVars.AitWritePtr) ?gVars.AitWritePtr + MAXYXRECORD - gpDevice[DevID].ReadAitPtr\
		:gVars.AitWritePtr - gpDevice[DevID].ReadAitPtr;
	log("gpDevice[DevID].ReadAitPtr is %d gVars.AitWritePtr %d\n",gpDevice[DevID].ReadAitPtr, gVars.AitWritePtr);
	return SoeNum;
}



DWORD GetAitNum(DWORD dwDevID)
{
//	if(dwDevID>=gVars.dwDevNum)
//		return 0;
//	if(gpDevice[dwDevID].pAitWrap==0)
//		return 0;
//	DWORD AitNum;
//	DWORD AitMaxBuffer;
//	
//	TPtr *pPtr;		
//	SM_P(MPV_AIT);
//	pPtr=&gpDevice[dwDevID].pAitWrap->Ptr;
//	AitNum=GetNum(pPtr);
//	AitMaxBuffer = GetAitMaxBuffer(dwDevID);
//	if(AitNum > AitMaxBuffer)
//	{
//		pPtr->dwReadPtr = pPtr->dwWritePtr - AitMaxBuffer;
//		AitNum = AitMaxBuffer;
//	}
//	SM_V(MPV_AIT);	
//	return AitNum;

	if(dwDevID>=gVars.dwDevNum)
		return 0;
			
	DWORD AitNum;

	AitNum=GetNum(dwDevID);
	log("AitNum is %d \n",AitNum);
	return AitNum;


}

void ClearAit(DWORD dwDevID)
{
//	if(dwDevID>=gVars.dwDevNum)
//		return ;
//	if(gpDevice[dwDevID].pAitWrap==0)
//		return;
//	TPtr *pPtr;
//	SM_P(MPV_AIT);		
//	pPtr=&gpDevice[dwDevID].pAitWrap->Ptr;
//	pPtr->dwReadPtr = pPtr->dwWritePtr;
//	SM_V(MPV_AIT);
}

DWORD GetAitReadPtr(DWORD dwDevID)
{

//	TDevie       *pDevice;
//	pDevice = pGetDevice(dwDevID);
//	if(pDevice==NULL)
//		return 0;
//	DWORD Num;
//	TPtr *pPtr;	
//	if(pDevice->pAitWrap!=NULL)
//	{
//		SM_P(MPV_AIT);		
//		pPtr=&gpDevice[dwDevID].pAitWrap->Ptr;
//		Num=GetNum(pPtr);
//		if(Num>GetAitMaxBuffer(dwDevID))
//			pPtr->dwReadPtr=pPtr->dwWritePtr-GetAitMaxBuffer(dwDevID);
//		SM_V(MPV_AIT);		
//		return	pPtr->dwReadPtr;
//	}
//	else
//		return 0;
}
DWORD GetAitWritePtr(DWORD dwDevID)
{
//	TDevice       *pDevice;
//	TPtr *pPtr;
//
//	pDevice = pGetDevice(dwDevID);
//	
//	if(pDevice == NULL)
//		return 0;
		
		
//	if(pDevice->pAitWrap!=NULL)
//	{
////		SM_P(MPV_AIT);		
//		pPtr = &gpDevice[dwDevID].pAitWrap->Ptr;
////		SM_V(MPV_AIT);		
//		return	pPtr->dwWritePtr;
//	}
//	else
//		return 0;
}

void IncAitReadPtr(DWORD dwDevID)
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
	gpDevice[dwDevID].ReadAitPtr++;
	gpDevice[dwDevID].ReadAitPtr %= MAXYCRECORD;


}

void WriteSysTimeAit(DWORD dwDevID,TSysTimeAit *pSysTimeAit)
{
	if(dwDevID>=gVars.dwDevNum || pSysTimeAit == NULL)
		return; 
	
	if(pSysTimeAit->nNo >= gpDevice[dwDevID].AINum)//点号越界
		return;
	
	if(gpDevice[dwDevID].pAitWrap==0)
		return;	

//	if(gpDevice[dwDevID].dwType == MT_LOGIC)
//		Logic_WriteSysTimeAit(dwDevID,pSysTimeAit);
//	else 
//		Real_WriteSysTimeAit(dwDevID,pSysTimeAit);
}


void Logic_WriteSysTimeAit(DWORD dwDevID,TSysTimeAit *pSysTimeAit)
{
	DWORD dwPtr;	
	
	if(pSysTimeAit == NULL)
		return;

//	if(ClockInvalidate(&pSysTimeAit->stTime)!=TRUE)
//	    return;

//	SM_P(MPV_AIT);

	TPtr *pPtr;
	pPtr=&gpDevice[dwDevID].pAitWrap->Ptr;
	
	if(pPtr == NULL)
		return;

//	dwPtr=pPtr->dwWritePtr%GetAitMaxBuffer(dwDevID);

	gpDevice[dwDevID].pSysTimeAit[dwPtr]=*pSysTimeAit;

	pPtr->dwWritePtr++;

//	SM_V(MPV_AIT);
}

//用默认的掼针读SOE
bool ReadSysTimeAit(DWORD dwDevID,TSysTimeAit *pSysTimeAit)
{
	//防错
//	if(dwDevID>=gVars.dwDevNum)
//		return FALSE; 
//	if(!gpDevice[dwDevID].pAitWrap)
//		return FALSE;
//	
//	if(*pdwReadPtr==gpDevice[dwDevID].pAitWrap->Ptr.dwWritePtr)
//		return FALSE;
//	
//	if(gpDevice[dwDevID].dwType == MT_LOGIC)
//		return Logic_ReadSysTimeAit(dwDevID,pdwReadPtr,pSysTimeAit);
	
//	return Real_ReadSysTimeAit(dwDevID,pdwReadPtr,pSysTimeAit);	
	if(dwDevID>=gVars.dwDevNum)
		return FALSE;
		
	if(gpDevice[dwDevID].ReadAitPtr==gVars.AitWritePtr)
	{
		return FALSE;
	}
	*pSysTimeAit = AitRecord[gpDevice[dwDevID].ReadAitPtr];

	return TRUE;

}

bool ReadAbsTimeAit(DWORD dwDevID,DWORD *pdwReadPtr,TAbsTimeAit *pAbsTimeAit)
{
	TSysTimeAit SysTimeAit;
	
	if(ReadSysTimeAit( dwDevID, &SysTimeAit )!=TRUE)
		return FALSE;
	
	pAbsTimeAit->nNo=SysTimeAit.nNo;
	pAbsTimeAit->sValue=SysTimeAit.sValue;
//	ToAbsTime(&SysTimeAit.stTime,&pAbsTimeAit->stTime);
	return TRUE;
}	

//bool ReadAbsTimeAit(DWORD dwDevID,TAbsTimeAit *pAbsTimeAit)
//{
//	return 	ReadAbsTimeAit( dwDevID,&gpDevice[dwDevID].pAitWrap->Ptr.dwReadPtr, pAbsTimeAit );
//}



