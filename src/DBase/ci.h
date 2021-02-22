#ifndef __CI_H__
#define __CI_H__

DWORD GetCINum(DWORD dwDevID);
DWORD ReadCI(DWORD dwDevID,DWORD dwCINo, WORD* pStatus);
void WriteCI(DWORD dwDevID,DWORD dwCINo,DWORD dwValue);
WORD GetCIFlag(DWORD dwDevID, DWORD dwNo);
void SetCIFlag(DWORD dwDevID, DWORD dwNo, WORD wFlag);
BOOL ReadFreezeCI(DWORD dwDevID, DWORD dwNo, TFreezeCI *freezeCI);
BOOL GetFreezeType(DWORD dwDevID, DWORD dwNo);
void FreezeCI(DWORD dwDevID, BOOL bTotal);
void ScanFreeze();
DWORD GetFreezeCINum(DWORD dwDevID);

#endif



