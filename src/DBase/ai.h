#ifndef __AI_H__
#define __AI_H__

#define OVDEAD_BIT		0x04


DWORD GetBurstAINum(DWORD dwDevID);//获取变化遥测个数
void WriteOneLogicBurstAI(TReflex *pReflex, WORD Flag_Old, DWORD value);
PTDevie pGetDevice(DWORD dwDevID);
WORD GetAIFlag(DWORD dwDevID, DWORD dwNo);
short ReadAI(DWORD dwDevID,DWORD dwAINo, WORD* pStatus);
short LogicAIToScale(DWORD dwDevID, DWORD dwNo, short sValue, WORD* pStatus);
void WriteAI(DWORD dwDevID,DWORD dwAINo,DWORD sValue);
bool ReadBurstAI(DWORD dwDevID,TBurstAI *pBurstAI);
void SetAIFlag(DWORD dwDevID, DWORD dwNo, WORD wFlag);
DWORD GetAINum(DWORD dwDevID);


#endif


