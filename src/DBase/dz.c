#include "sys.h"
//
//
//
////读实际库AI(不X系数)
//DWORD Real_ReadAI(DWORD dwDevID,DWORD dwAINo, WORD* pStatus)
//{
//	long lgValue;
//	WORD wType;
//	TRealAI *pRealAI;
//	if(dwAINo>=gpDevice[dwDevID].AINum)
//		return 0;
//
//	gpDevice[dwDevID].pRealBase->pRealAI[dwAINo].a = 1;
//	pRealAI=&(gpDevice[dwDevID].pRealBase->pRealAI[dwAINo]);
//	wType = pRealAI->wType;
//	lgValue = gpDevice[dwDevID].pAI[dwAINo];
//
//	wType = 0;				
//	if(pStatus)
//		*pStatus = gpDevice[dwDevID].pAIStatus[dwAINo];
//
////	switch(wType)
////	{
////	case 0: //普通型
////	case 1://电流型
////		break;
////	
////	case 2: //4_20ma  对应0---20000
////
////		if(lgValue<3000)//考虑零漂，可根据实际情况修改//同时考虑变送器的输出可能低于4mA
////			lgValue = 4000;	//4~20mA采集为4000~20000
////
////		lgValue = (lgValue-4000)*5/4;	//20000/16000	//将4000~20000 换算为0~20000
////		
////		break;
////	case 3: //4—12—20ma  对应 -10000----0----+10000
////
////		if(lgValue<3000)//变送器无输出或未接电源//同时考虑变送器的输出可能低于4mA
////			lgValue = 12000;	//12mA为4~12~20mA的零值
////		
////		lgValue = (lgValue-12000)*5/4;	//20000/16000	//将4000~20000 换算为0~20000
////		break;
////	case 4: //4—12—20ma 对应0---1000---0 COS算法
////
////		if(lgValue > 12000)
////		{
////			if(lgValue > 20000)
////		        lgValue = 20000;
////			lgValue = 20000 - lgValue;
////		}
////		else
////		{
////			if(lgValue < 4000)
////	        	lgValue = 4000;
////
////			lgValue = lgValue - 4000;
////		}	
////
////		lgValue = lgValue/8;
////		
////		break;
////
////	default:
////		break;
////	}
//
//	return lgValue;
//}
//
////读乘完系数的遥测
//DWORD Logic_ReadScaledAI(DWORD dwDevID,DWORD dwAINo, WORD* pStatus)
//{
//	float iVal;
//	
//	DWORD dwRealDevID;
//	DWORD dwRealAINo;
//	TLogicAI *pLogicAI;
//
//	if(dwAINo>=gpDevice[dwDevID].AINum)
//		return 0;
//    dwRealDevID = gpDevice[dwDevID].pLogicBase->pLogicAI[dwAINo].wRealID;
//
//	
//	if(dwRealDevID>=gVars.dwDevNum)
//		return 0;
//    dwRealAINo  = gpDevice[dwDevID].pLogicBase->pLogicAI[dwAINo].wOffset;
//	
//    pLogicAI = &(gpDevice[dwDevID].pLogicBase->pLogicAI[dwAINo]);
//	
//	if(pStatus)
//		*pStatus = 0;
////	iVal=(*gpDevice[dwRealDevID].ReadAI)(dwRealDevID,dwRealAINo);
//	iVal = Real_ReadAI(dwRealDevID, dwRealAINo, pStatus); //wcp20020913
////	iVal*=10;
////	iVal *= (float)pLogicAI->a;
////	iVal /= (float)pLogicAI->b;
///*	if(iVal>=0)
//	   iVal+=5;	
//	else
//	   iVal-=5;
//	iVal/=10;
//*/	
////	iVal += pLogicAI->c;
////	
////	if((iVal >= 32767) && (iVal<32900))
////		iVal = 32767;
//
////	if(iVal>=32900)
////	{
////		iVal = 32767;
////		
//////		if(pStatus)
//////			*pStatus |= OV_BIT;
////	}
////
////	if((iVal <= - 32768) && (iVal>-32900))
////		iVal = -32767;
////	
////	if(iVal<-32900)
////	{
////		iVal = -32768;
////
//////		if(pStatus)
//////			*pStatus |= OV_BIT;
////	}
//
//	return 	(DWORD)iVal;
//}
//
//TLogicAI* GetLogicAI(DWORD dwDevID, DWORD *dwAINum)
//{
//	TLogicBase	*pLogicBase;
//	TLogicAI* pLogicAI;
//	*dwAINum = 0;
//	
//	if(dwDevID >= gVars.dwDevNum)
//		return NULL;
//
//	pLogicBase = gpDevice[dwDevID].pLogicBase;
//	if(pLogicBase == 0)
//		return NULL;
//	pLogicAI = gpDevice[dwDevID].pLogicBase->pLogicAI;
////	*dwAINum = gpDevice[dwDevID].AINum;
//	*dwAINum = gVars.TransYCTableNum;
//
//	if(*dwAINum == 0)
//		return NULL;
//	return pLogicAI;
//}
//
///*****************************************************************
//函数名称: ScanBurstAI
//函数功能: 检测变化遥测
//输入参数: 
//	dwDevID:设备ID号
//返回值：  无
//修改日期：2020-4-1
//	屏蔽原先检测突发遥测方式，屏蔽死区判断功能，直接手动写溢出标志位
//
//*****************************************************************/
//
//DWORD ScanBurstAI(DWORD dwDevID)//搜索变化遥测
//{
//	DWORD dwAINo;
//	DWORD dwAINum;
//	TLogicAI    *pLogicAI;//RAM
//	DWORD sValue;
//	short sChangedValue;
//	WORD wDeadLine;
//	WORD wOverNum;
//	WORD wStatus;
//
//
//	pLogicAI = GetLogicAI(dwDevID, &dwAINum);
//	if(pLogicAI == NULL)
//		return 0;
//	
//	wOverNum = 0;
//	for(dwAINo=0;dwAINo<gVars.TransYCTableNum;dwAINo++)
//	{
//		if(pLogicAI[dwAINo].wOldFlag & OVDEAD_BIT)
//		{
//			wOverNum ++;
////			continue;
//		}
//
////		if(gpDevice[dwDevID].dwControl&LOG_CONF_AI_COE)//遥测乘系数
////			sValue = Logic_ReadScaledAI(dwDevID,dwAINo, &wStatus);
////		else
////			sValue = Logic_ReadAI(dwDevID,dwAINo, &wStatus);
//
//		sValue = Logic_ReadScaledAI(dwDevID,dwAINo, &wStatus);
//		sChangedValue = sValue - pLogicAI[dwAINo].sOldValue;
//
//		
////		if(!(pLogicAI[dwAINo].wSQZ & 0x8000))//绝对死区
////			wDeadLine = pLogicAI[dwAINo].wSQZ;
////		else
////			wDeadLine = (DWORD)abs(pLogicAI[dwAINo].sOldValue)*(pLogicAI[dwAINo].wSQZ & 0x7FFF)/1000;
//
//		if(sChangedValue < 0)
//			sChangedValue =- sChangedValue;
//		
//		//修改日期：2020-4-1
////		if(sChangedValue>wDeadLine)
////		{			
////			pLogicAI[dwAINo].sOldValue = sValue; //最新的值
////			pLogicAI[dwAINo].wOldFlag = (wStatus|OVDEAD_BIT);
////			ReadSysTime(&pLogicAI[dwAINo].stTime);
////			
////			wOverNum ++;
////
////		}
//		//修改日期：2020-4-1
//		if(wStatus & OVDEAD_BIT)
//		{			
////			pLogicAI[dwAINo].sOldValue = sValue; //最新的值
////			pLogicAI[dwAINo].wOldFlag = wStatus;
////			wOverNum ++;
//		}
//
//
//
//		
//	}
//	
//	return wOverNum ++;
//}
//
//
///*****************************************************************
//函数名称: GetBurstAINum
//函数功能: 获取变化遥测
//输入参数: 
//	dwDevID:设备ID号
//返回值：  无
//修改日期：2020-4-1
//	屏蔽原先检测突发遥测方式：原先为从实际库中获取遥测值存入逻辑库中，再用逻辑库的遥测值计算是否越界，从而产生越限遥测
//	修改为检测实际库中的遥测status，若status为OVDEAD_BIT，则突发遥测计数加一。
//
//*****************************************************************/
//
//DWORD GetBurstAINum(DWORD dwDevID)//获取变化遥测个数
//{
//	TLogicAI* pLogicAI;
//	DWORD dwAINum;
//	DWORD dwNum;
//	DWORD dwOverNum;
//	pLogicAI = GetLogicAI(dwDevID, &dwAINum);
//	
//	if(pLogicAI == NULL){
//		return 0;
//	}
//	dwOverNum = 0;
//
////	for(dwNum=0; dwNum<dwAINum; dwNum++)
////	{
////		if(pLogicAI[dwNum].wOldFlag & OVDEAD_BIT)
////			dwOverNum ++;
////	}
//	
////	if(bScan && !dwOverNum)
////		dwOverNum = ScanBurstAI(dwDevID);
//
//	dwOverNum = ScanBurstAI(dwDevID);
//
//	return dwOverNum;
//}
//
////***********************************************************************************//
////  向数据库中写一个遥测数据
////  功能：数据的更新，遥测越限/恢复事件的检测与产生，强制/取消置数
////***********************************************************************************//
//void WriteAI(DWORD dwDevID,DWORD dwAINo,DWORD sValue)
//{
//	if(dwDevID>=gVars.dwDevNum)
//		return ;
//	
//	if(dwAINo>=gpDevice[dwDevID].AINum)
//		return;
////	(*gpDevice[dwDevID].WriteAI)(dwDevID,dwAINo,sValue);
//	gpDevice[dwDevID].pAI[dwAINo] = sValue;
//}
//
//DWORD GetAINum(DWORD dwDevID)
//{
//	if(dwDevID>=gVars.dwDevNum)
//		return 0;
//	return gpDevice[dwDevID].AINum;	
//}
//
//bool ReadBurstAI(DWORD dwDevID,TBurstAI *pBurstAI)
//{
//	TLogicAI* pLogicAI;
//	DWORD dwAINum;
//	DWORD dwNum;
//
//	if(pBurstAI == NULL)
//		return FALSE;
//
//	pLogicAI = GetLogicAI(dwDevID, &dwAINum);
//
//	if(pLogicAI == NULL)
//		return FALSE;
//	for(dwNum=0; dwNum<dwAINum; dwNum++)
//	{
//		if(pLogicAI[dwNum].wOldFlag&OVDEAD_BIT)
//		{
////			log("dwNum is (%d)\n",dwNum);
////			PRINT_FUNLINE;
//			pBurstAI->dwNo = dwNum;
//			pBurstAI->sValue = pLogicAI[dwNum].sOldValue;
//			pBurstAI->wStatus = pLogicAI[dwNum].wOldFlag&(~OVDEAD_BIT);
//			pBurstAI->stTime = pLogicAI[dwNum].stTime;
//			pLogicAI[dwNum].wOldFlag = 0;
//			return TRUE;
//		}
//	}
//
//	return FALSE;
//}
//

void WriteOneLogicBurstDZ(TReflex *pReflex, WORD Flag_Old, DWORD value)//5512的value要改成short
{
//写入逻辑库中
	DWORD dwDevNo;
	DWORD dwNo;

	if(pReflex == NULL)
		return;

	for(;pReflex;pReflex=pReflex->pNext)
	{
		dwDevNo = pReflex->dwDevNo;
			
		if(dwDevNo >= gVars.dwDevNum)
				continue;
		
		dwNo = pReflex->dwNo;
//		log("dwDevNo(%d)dwNo(%d)\n",dwDevNo, dwNo);
		gpDevice[dwDevNo].pLogicBase->pLogicAI[dwNo].wOldFlag = Flag_Old;
		gpDevice[dwDevNo].pLogicBase->pLogicAI[dwNo].sOldValue = value;
		
	}
}




