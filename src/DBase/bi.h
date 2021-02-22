#ifndef __BI_H__
#define __BI_H__


//void WriteOneLogicBurstBI(TReflex *pReflex,BYTE bStatus);
void WriteOneLogicBurstBI(TReflex *pReflex, TBurstBI* pBurstBI);
bool WriteOneBurstBI(DWORD dwDevID,DWORD dwBINo,BYTE bStatus );
//bool WriteOneBurstBI(DWORD dwDevID, TBurstBI* pBurstBI);
bool ReadBurstBI(DWORD dwDevID,TBurstBI *pBurstBI);
//bool ReadBurstBI(DWORD dwDevID,DWORD *pdwReadPtr,TBurstBI *pBurstBI);
DWORD GetBurstBINum(DWORD dwDevID);
DWORD GetBurstBIReadPtr(DWORD dwDevID);
void ClearBurstBI(DWORD dwDevID);
void IncBurstBIReadPtr(DWORD dwDevID);
void SetBINum(DWORD dwDevID,DWORD MaxNum);
DWORD GetBINum(DWORD dwDevID);
WORD GetBIFlag(DWORD dwDevID, DWORD dwNo);
BYTE ReadBI( DWORD dwDevID,DWORD dwBINo, WORD* pStatus);
void SetBIFlag(DWORD dwDevID, DWORD dwPoint, WORD wFlag);

#endif

