#include "sys.h"

//==========================  与应用层的接口  =============================

DWORD GetCINum(DWORD dwDevID)
{
	if(dwDevID>=gVars.dwDevNum)
		return 0;
	return gpDevice[dwDevID].dwCINum;	
}

DWORD ReadCI(DWORD dwDevID,DWORD dwCINo, WORD* pStatus)
{
	if(dwDevID>=gVars.dwDevNum)
			return 0;

	return (*gpDevice[dwDevID].ReadCI)(dwDevID,dwCINo, pStatus);
}

void WriteCI(DWORD dwDevID,DWORD dwCINo,DWORD dwValue)
{
	if(dwDevID>=gVars.dwDevNum)
		return ;
	(*gpDevice[dwDevID].WriteCI)(dwDevID,dwCINo,dwValue);
}


WORD GetCIFlag(DWORD dwDevID, DWORD dwNo)
{
	TLogicCI* pLogicCI;
	DWORD dwNum;
	

	if(dwDevID >= gVars.dwDevNum)
		return FALSE;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return FALSE;

	dwNum = gpDevice[dwDevID].dwCINum;
	pLogicCI = gpDevice[dwDevID].pLogicBase->pLogicCI;

	if(pLogicCI == NULL || dwNum == 0)
		return FALSE;

	if(dwNo >= dwNum)
		return FALSE;
	
	return pLogicCI[dwNo].wFlag;
}


void SetCIFlag(DWORD dwDevID, DWORD dwNo, WORD wFlag)
{
	TLogicCI* pLogicCI;
	DWORD dwNum;

	if(dwDevID>=gVars.dwDevNum)
		return;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return;

	dwNum = gpDevice[dwDevID].dwCINum;
	pLogicCI = gpDevice[dwDevID].pLogicBase->pLogicCI;

	if(pLogicCI == NULL || dwNum == 0)
		return;

	if(dwNo >= dwNum)
		return;
	
	pLogicCI[dwNo].wFlag = wFlag;
}

BOOL ReadFreezeCI(DWORD dwDevID, DWORD dwNo, TFreezeCI* freezeCI)
{
	TLogicCI* pLogicCI;
	DWORD dwNum;

	if(dwDevID >= gVars.dwDevNum)
		return FALSE;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return 0;
	
	dwNum = gpDevice[dwDevID].dwCINum;
	pLogicCI = gpDevice[dwDevID].pLogicBase->pLogicCI;
	
	if((pLogicCI == NULL) || (dwNo >= dwNum))
		return FALSE;
	
//	if(!bForce)
//	{
//		if(pLogicCI[dwNo].bFreezeFlag == FALSE)
//			return FALSE;
//	}
	
	pLogicCI[dwNo].bFreezeFlag = FALSE;
	freezeCI->dwValue = pLogicCI[dwNo].dwFreezeValue;
	freezeCI->stTime = pLogicCI[dwNo].stTime;
	freezeCI->wStatus = pLogicCI[dwNo].wStatus;
	

	return TRUE;
}

BOOL GetFreezeType(DWORD dwDevID, DWORD dwNo)/*bReset=1发送差值, bReset=0发送总数*/
{
	DWORD dwNum;
	TLogicCI* pLogicCI;

	if(dwDevID >= gVars.dwDevNum)
		return FALSE;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return 0;
	
	dwNum = gpDevice[dwDevID].dwCINum;
	pLogicCI = gpDevice[dwDevID].pLogicBase->pLogicCI;

	if(pLogicCI == NULL || dwNo >= dwNum)
		return FALSE;
	
	return pLogicCI[dwNo].bReset;
}

void FreezeCI(DWORD dwDevID, BOOL bTotal)
{
	TLogicCI* pLogicCI;
	int i;
	DWORD dwNum;
	DWORD dwValue;
	WORD wStatus;

	if(dwDevID>=gVars.dwDevNum)
		return;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return;

	dwNum = gpDevice[dwDevID].dwCINum;
	pLogicCI = gpDevice[dwDevID].pLogicBase->pLogicCI;

	if(pLogicCI == NULL || dwNum == 0)
		return;

	for(i=0; i<dwNum; i++)
	{
		dwValue = ReadCI(dwDevID, i, &wStatus);
//		ReadSysTime(&pLogicCI[i].stTime);
		
		if(bTotal)//发送总值
		{
			pLogicCI[i].dwFreezeValue = dwValue;
		}
		else
		{
			if(!pLogicCI[i].bFreezeFlag)//数据已读走
			{
				if(dwValue >= pLogicCI[i].dwTotalValue)
					pLogicCI[i].dwFreezeValue =  dwValue - pLogicCI[i].dwTotalValue;
				else
					pLogicCI[i].dwFreezeValue = 0;

				pLogicCI[i].dwTotalValue = dwValue;
			}
			else//数据没有读走, pLogic
			{
				if(dwValue >= pLogicCI[i].dwTotalValue)
					pLogicCI[i].dwFreezeValue += (dwValue - pLogicCI[i].dwTotalValue);
				else
					pLogicCI[i].dwFreezeValue = pLogicCI[i].dwFreezeValue;
				pLogicCI[i].dwTotalValue = dwValue;
			}
		}
		pLogicCI[i].bFreezeFlag = TRUE;//置冻结标志
		pLogicCI[i].wStatus = wStatus;
	}
}

DWORD GetFreezeCINum(DWORD dwDevID)
{
	int i;
	DWORD dwNum;
	TLogicCI* pLogicCI;
	int nFreezeNum;

	if(dwDevID>=gVars.dwDevNum)
		return 0;
	
	if(gpDevice[dwDevID].pLogicBase == NULL)
		return 0;

	dwNum = gpDevice[dwDevID].dwCINum;

	pLogicCI = gpDevice[dwDevID].pLogicBase->pLogicCI;

	if(pLogicCI == NULL || dwNum == 0)
		return 0;
	
	nFreezeNum = 0;

	for(i=0; i<dwNum; i++)
	{
		if(pLogicCI[i].bFreezeFlag == TRUE)
			nFreezeNum ++;
	}

	return nFreezeNum;
}


void ScanFreezeTime(DWORD dwDevID)//1分钟扫描1次
{
	TLogicCI* pLogicCI;
	int i;
	DWORD dwNum;
	DWORD dwValue;
	WORD wStatus;

	if(dwDevID>=gVars.dwDevNum)
		return;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return;

	dwNum = gpDevice[dwDevID].dwCINum;
	pLogicCI = gpDevice[dwDevID].pLogicBase->pLogicCI;

	if(pLogicCI == NULL || dwNum == 0)
		return;

	for(i=0; i<dwNum; i++)
	{
		dwValue = ReadCI(dwDevID, i, &wStatus);
//		ReadSysTime(&pLogicCI[i].stTime);

		pLogicCI[i].wCountTime ++;

		if(pLogicCI[i].wCountTime >= pLogicCI[i].wFreezeTime)//冻结时间到
		{
			pLogicCI[i].wCountTime = 0;
		
			if(pLogicCI[i].bReset == 0)//发送总值
			{
				pLogicCI[i].dwFreezeValue = dwValue;
			}
			else
			{
				if(!pLogicCI[i].bFreezeFlag)//数据已读走
				{
					if(dwValue >= pLogicCI[i].dwTotalValue)
						pLogicCI[i].dwFreezeValue =  dwValue - pLogicCI[i].dwTotalValue;
					else
						pLogicCI[i].dwFreezeValue = 0;

					pLogicCI[i].dwTotalValue = dwValue;
				}
				else//数据没有读走, pLogic
				{
					if(dwValue >= pLogicCI[i].dwTotalValue)
						pLogicCI[i].dwFreezeValue += (dwValue - pLogicCI[i].dwTotalValue);
					else
						pLogicCI[i].dwFreezeValue = pLogicCI[i].dwFreezeValue;
					pLogicCI[i].dwTotalValue = dwValue;
				}
			}
			pLogicCI[i].bFreezeFlag = TRUE;//置冻结标志
			pLogicCI[i].wStatus = wStatus;
		}
	}
}

void ScanFreeze()
{
	DWORD dwDevNo;

	for(dwDevNo = 0; dwDevNo < gVars.dwDevNum; dwDevNo++)
		ScanFreezeTime(dwDevNo);
}

