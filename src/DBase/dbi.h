#ifndef __DBI_H__
#define __DBI_H__


void IncBurstDBIReadPtr(DWORD dwDevID);
DWORD GetBurstDBINum(DWORD dwDevID);
bool ReadBurstDBI(DWORD dwDevID,TBurstDBI *pBurstDBI);
DWORD GetBurstDBINum(DWORD dwDevID);
DWORD GetDBINum(DWORD dwDevID);
BYTE ReadDBI( DWORD dwDevID,DWORD dwDBINo, WORD* pStatus);
void SetDBIFlag(DWORD dwDevID, DWORD dwPoint, WORD wFlag);
WORD GetDBIFlag(DWORD dwDevID, DWORD dwNo);

#endif

