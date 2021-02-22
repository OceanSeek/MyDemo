#include "sys.h"




WORD GetAIFlag(DWORD dwDevID, DWORD dwNo)
{
	TLogicAI* pLogicAI;
	DWORD dwNum;

	if(dwDevID >= gVars.dwDevNum)
		return FALSE;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return FALSE;

	dwNum = gpDevice[dwDevID].AINum;
	pLogicAI = gpDevice[dwDevID].pLogicBase->pLogicAI;

	if(pLogicAI == NULL || dwNum == 0)
		return FALSE;

	if(dwNo >= dwNum)
		return FALSE;
	
	return pLogicAI[dwNo].wFlag;
}


void SetAIFlag(DWORD dwDevID, DWORD dwNo, WORD wFlag)
{
	TLogicAI* pLogicAI;
	DWORD dwNum;

	if(dwDevID>=gVars.dwDevNum)
		return;

	if(gpDevice[dwDevID].pLogicBase == NULL)
		return;

	dwNum = gpDevice[dwDevID].AINum;
	pLogicAI = gpDevice[dwDevID].pLogicBase->pLogicAI;

	if(pLogicAI == NULL || dwNum == 0)
		return;

	if(dwNo >= dwNum)
		return;
	
	pLogicAI[dwNo].wFlag = wFlag;
}

short ReadAI(DWORD dwDevID,DWORD dwAINo, WORD* pStatus)
{
	if(dwDevID>=gVars.dwDevNum)
		return 0;
	if(dwAINo>=gpDevice[dwDevID].AINum)
		return 0;
	return (*gpDevice[dwDevID].ReadAI)(dwDevID, dwAINo, pStatus);
}



//��ʵ�ʿ�AI(��Xϵ��)
DWORD Real_ReadAI(DWORD dwDevID,DWORD dwAINo, WORD* pStatus)
{
	long lgValue;
	WORD wType;
	TRealAI *pRealAI;
	if(dwAINo>=gpDevice[dwDevID].AINum)
		return 0;

	gpDevice[dwDevID].pRealBase->pRealAI[dwAINo].a = 1;
	pRealAI=&(gpDevice[dwDevID].pRealBase->pRealAI[dwAINo]);
	wType = pRealAI->wType;
	lgValue = gpDevice[dwDevID].pAI[dwAINo];

	wType = 0;				
	if(pStatus)
		*pStatus = gpDevice[dwDevID].pAIStatus[dwAINo];

//	switch(wType)
//	{
//	case 0: //��ͨ��
//	case 1://������
//		break;
//	
//	case 2: //4_20ma  ��Ӧ0---20000
//
//		if(lgValue<3000)//������Ư���ɸ���ʵ������޸�//ͬʱ���Ǳ�������������ܵ���4mA
//			lgValue = 4000;	//4~20mA�ɼ�Ϊ4000~20000
//
//		lgValue = (lgValue-4000)*5/4;	//20000/16000	//��4000~20000 ����Ϊ0~20000
//		
//		break;
//	case 3: //4��12��20ma  ��Ӧ -10000----0----+10000
//
//		if(lgValue<3000)//�������������δ�ӵ�Դ//ͬʱ���Ǳ�������������ܵ���4mA
//			lgValue = 12000;	//12mAΪ4~12~20mA����ֵ
//		
//		lgValue = (lgValue-12000)*5/4;	//20000/16000	//��4000~20000 ����Ϊ0~20000
//		break;
//	case 4: //4��12��20ma ��Ӧ0---1000---0 COS�㷨
//
//		if(lgValue > 12000)
//		{
//			if(lgValue > 20000)
//		        lgValue = 20000;
//			lgValue = 20000 - lgValue;
//		}
//		else
//		{
//			if(lgValue < 4000)
//	        	lgValue = 4000;
//
//			lgValue = lgValue - 4000;
//		}	
//
//		lgValue = lgValue/8;
//		
//		break;
//
//	default:
//		break;
//	}

	return lgValue;
}

//������ϵ����ң��
DWORD Logic_ReadScaledAI(DWORD dwDevID,DWORD dwAINo, WORD* pStatus)
{
	float iVal;
	
	DWORD dwRealDevID;
	DWORD dwRealAINo;
	TLogicAI *pLogicAI;

	if(dwAINo>=gpDevice[dwDevID].AINum)
		return 0;
    dwRealDevID = gpDevice[dwDevID].pLogicBase->pLogicAI[dwAINo].wRealID;

	
	if(dwRealDevID>=gVars.dwDevNum)
		return 0;
    dwRealAINo  = gpDevice[dwDevID].pLogicBase->pLogicAI[dwAINo].wOffset;
	
    pLogicAI = &(gpDevice[dwDevID].pLogicBase->pLogicAI[dwAINo]);
	
	if(pStatus)
		*pStatus = 0;
//	iVal=(*gpDevice[dwRealDevID].ReadAI)(dwRealDevID,dwRealAINo);
	iVal = Real_ReadAI(dwRealDevID, dwRealAINo, pStatus); //wcp20020913
//	iVal*=10;
//	iVal *= (float)pLogicAI->a;
//	iVal /= (float)pLogicAI->b;
/*	if(iVal>=0)
	   iVal+=5;	
	else
	   iVal-=5;
	iVal/=10;
*/	
//	iVal += pLogicAI->c;
//	
//	if((iVal >= 32767) && (iVal<32900))
//		iVal = 32767;

//	if(iVal>=32900)
//	{
//		iVal = 32767;
//		
////		if(pStatus)
////			*pStatus |= OV_BIT;
//	}
//
//	if((iVal <= - 32768) && (iVal>-32900))
//		iVal = -32767;
//	
//	if(iVal<-32900)
//	{
//		iVal = -32768;
//
////		if(pStatus)
////			*pStatus |= OV_BIT;
//	}

	return 	(DWORD)iVal;
}

TLogicAI* GetLogicAI(DWORD dwDevID, DWORD *dwAINum)
{
	TLogicBase	*pLogicBase;
	TLogicAI* pLogicAI;
	*dwAINum = 0;
	
	if(dwDevID >= gVars.dwDevNum)
		return NULL;

	pLogicBase = gpDevice[dwDevID].pLogicBase;
	if(pLogicBase == 0)
		return NULL;
	pLogicAI = gpDevice[dwDevID].pLogicBase->pLogicAI;
//	*dwAINum = gpDevice[dwDevID].AINum;
	*dwAINum = gVars.TransYCTableNum;

	if(*dwAINum == 0)
		return NULL;
	return pLogicAI;
}

/*****************************************************************
��������: ScanBurstAI
��������: ���仯ң��
�������: 
	dwDevID:�豸ID��
����ֵ��  ��
�޸����ڣ�2020-4-1
	����ԭ�ȼ��ͻ��ң�ⷽʽ�����������жϹ��ܣ�ֱ���ֶ�д�����־λ

*****************************************************************/

DWORD ScanBurstAI(DWORD dwDevID)//�����仯ң��
{
	DWORD dwAINo;
	DWORD dwAINum;
	TLogicAI    *pLogicAI;//RAM
	DWORD sValue;
	short sChangedValue;
	WORD wDeadLine;
	WORD wOverNum;
	WORD wStatus;


	pLogicAI = GetLogicAI(dwDevID, &dwAINum);
	if(pLogicAI == NULL)
		return 0;
	
	wOverNum = 0;
	for(dwAINo=0;dwAINo<gVars.TransYCTableNum;dwAINo++)
	{
		if(pLogicAI[dwAINo].wOldFlag & OVDEAD_BIT)
		{
			wOverNum ++;
//			continue;
		}

//		if(gpDevice[dwDevID].dwControl&LOG_CONF_AI_COE)//ң���ϵ��
//			sValue = Logic_ReadScaledAI(dwDevID,dwAINo, &wStatus);
//		else
//			sValue = Logic_ReadAI(dwDevID,dwAINo, &wStatus);

		sValue = Logic_ReadScaledAI(dwDevID,dwAINo, &wStatus);
		sChangedValue = sValue - pLogicAI[dwAINo].sOldValue;

		
//		if(!(pLogicAI[dwAINo].wSQZ & 0x8000))//��������
//			wDeadLine = pLogicAI[dwAINo].wSQZ;
//		else
//			wDeadLine = (DWORD)abs(pLogicAI[dwAINo].sOldValue)*(pLogicAI[dwAINo].wSQZ & 0x7FFF)/1000;

		if(sChangedValue < 0)
			sChangedValue =- sChangedValue;
		
		//�޸����ڣ�2020-4-1
//		if(sChangedValue>wDeadLine)
//		{			
//			pLogicAI[dwAINo].sOldValue = sValue; //���µ�ֵ
//			pLogicAI[dwAINo].wOldFlag = (wStatus|OVDEAD_BIT);
//			ReadSysTime(&pLogicAI[dwAINo].stTime);
//			
//			wOverNum ++;
//
//		}
		//�޸����ڣ�2020-4-1
		if(wStatus & OVDEAD_BIT)
		{			
//			pLogicAI[dwAINo].sOldValue = sValue; //���µ�ֵ
//			pLogicAI[dwAINo].wOldFlag = wStatus;
//			wOverNum ++;
		}



		
	}
	
	return wOverNum ++;
}


/*****************************************************************
��������: GetBurstAINum
��������: ��ȡ�仯ң��
�������: 
	dwDevID:�豸ID��
����ֵ��  ��
�޸����ڣ�2020-4-1
	����ԭ�ȼ��ͻ��ң�ⷽʽ��ԭ��Ϊ��ʵ�ʿ��л�ȡң��ֵ�����߼����У������߼����ң��ֵ�����Ƿ�Խ�磬�Ӷ�����Խ��ң��
	�޸�Ϊ���ʵ�ʿ��е�ң��status����statusΪOVDEAD_BIT����ͻ��ң�������һ��

*****************************************************************/

DWORD GetBurstAINum(DWORD dwDevID)//��ȡ�仯ң�����
{
	TLogicAI* pLogicAI;
	DWORD dwAINum;
	DWORD dwNum;
	DWORD dwOverNum;
	pLogicAI = GetLogicAI(dwDevID, &dwAINum);
	
	if(pLogicAI == NULL){
		return 0;
	}
	dwOverNum = 0;

//	for(dwNum=0; dwNum<dwAINum; dwNum++)
//	{
//		if(pLogicAI[dwNum].wOldFlag & OVDEAD_BIT)
//			dwOverNum ++;
//	}
	
//	if(bScan && !dwOverNum)
//		dwOverNum = ScanBurstAI(dwDevID);

	dwOverNum = ScanBurstAI(dwDevID);

	return dwOverNum;
}

//***********************************************************************************//
//  �����ݿ���дһ��ң������
//  ���ܣ����ݵĸ��£�ң��Խ��/�ָ��¼��ļ���������ǿ��/ȡ������
//***********************************************************************************//
void WriteAI(DWORD dwDevID,DWORD dwAINo,DWORD sValue)
{
	if(dwDevID>=gVars.dwDevNum)
		return ;
	
	if(dwAINo>=gpDevice[dwDevID].AINum)
		return;
//	(*gpDevice[dwDevID].WriteAI)(dwDevID,dwAINo,sValue);
	gpDevice[dwDevID].pAI[dwAINo] = sValue;
}

DWORD GetAINum(DWORD dwDevID)
{
	if(dwDevID>=gVars.dwDevNum)
		return 0;
	return gpDevice[dwDevID].AINum;	
}

bool ReadBurstAI(DWORD dwDevID,TBurstAI *pBurstAI)
{
	TLogicAI* pLogicAI;
	DWORD dwAINum;
	DWORD dwNum;

	if(pBurstAI == NULL)
		return FALSE;

	pLogicAI = GetLogicAI(dwDevID, &dwAINum);

	if(pLogicAI == NULL)
		return FALSE;
	for(dwNum=0; dwNum<dwAINum; dwNum++)
	{
		if(pLogicAI[dwNum].wOldFlag&OVDEAD_BIT)
		{
//			log("dwNum is (%d)\n",dwNum);
//			PRINT_FUNLINE;
			pBurstAI->dwNo = dwNum;
			pBurstAI->sValue = pLogicAI[dwNum].sOldValue;
			pBurstAI->wStatus = pLogicAI[dwNum].wOldFlag&(~OVDEAD_BIT);
			pBurstAI->stTime = pLogicAI[dwNum].stTime;
			pLogicAI[dwNum].wOldFlag = 0;
			return TRUE;
		}
	}

	return FALSE;
}

PTDevie pGetDevice(DWORD dwDevID)
{
    if( dwDevID >= gVars.dwDevNum ) 
		return NULL;
    
    return &gpDevice[dwDevID];
}

void WriteOneLogicBurstAI(TReflex *pReflex, WORD Flag_Old, DWORD value)//5512��valueҪ�ĳ�short
{
//д���߼�����
	DWORD dwDevID;
	TDevie *pDevice;
	DWORD dwNo;

	if(pReflex == NULL)
		return;

	for(;pReflex;pReflex=pReflex->pNext)
	{
		dwDevID = pReflex->dwDevNo;
			
		if(dwDevID >= gVars.dwDevNum)
				continue;
		
		pDevice = pGetDevice(dwDevID);
		
		if(pDevice == NULL)
			continue;
		
		dwNo = pReflex->dwNo;
//		log("dwDevID(%d)dwNo(%d)\n",dwDevID, dwNo);
		gpDevice[dwDevID].pLogicBase->pLogicAI[dwNo].wOldFlag = Flag_Old;
		gpDevice[dwDevID].pLogicBase->pLogicAI[dwNo].sOldValue = value;
		
	}
}

short LogicAIToScale(DWORD dwDevID, DWORD dwNo, short sValue, WORD* pStatus)
{
	TLogicAI* pLogicAI;
	float fVal;
	
//	pLogicAI = GetLogicAIT(dwDevID, dwNo);
	
	if(pLogicAI == NULL)
		return 0;

	if(pStatus)
		*pStatus = 0;

//	if(!(gpDevice[dwDevID].dwControl&LOG_CONF_AI_COE))//ң�ⲻ��ϵ��
//		return sValue;
	
	if(pLogicAI->b == 0)
		return sValue;

	fVal = sValue;

	fVal *= (float)pLogicAI->a;
	fVal /= (float)pLogicAI->b;
	fVal += pLogicAI->c;
	
	if((fVal >= 32767) && (fVal<32900))
		fVal = 32767;

	if(fVal >= 32900)
	{
		fVal = 32767;
		
//		if(pStatus)
//			*pStatus |= OV_BIT;
	}

	if((fVal <= - 32768) && (fVal>-32900))
		fVal = -32767;
	
	if(fVal<-32900)
	{
		fVal = -32768;

//		if(pStatus)
//			*pStatus |= OV_BIT;
	}
	
	return (short)fVal;
}



