#ifndef __BCD_H__
#define __BCD_H__

DWORD ReadBCD(DWORD dwDevID,DWORD dwBCDNo, WORD* pStatus);
void SetBcdFlag(DWORD dwDevID, DWORD dwNo, WORD wFlag);
TLogicBCD* GetLogicBCD(DWORD dwDevID, DWORD *dwBCDNum);
WORD GetBcdFlag(DWORD dwDevID, DWORD dwNo);
BOOL ReadBurstBcd(DWORD dwDevID, TBurstBcd* pBurstBcd);
DWORD GetBurstBcdNum(DWORD dwDevID);
void IncBCDReadPtr(DWORD dwDevID);
DWORD GetBCDNum(DWORD dwDevID);


#endif

