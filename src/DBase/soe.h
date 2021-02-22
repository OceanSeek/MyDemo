#ifndef __SOE_H__
#define __SOE_H__

DWORD GetSoeNum(DWORD dwDevID);
void ClearSoe(DWORD dwDevID);
DWORD GetSoeReadPtr(DWORD dwDevID);
DWORD GetSoeWritePtr(DWORD dwDevID);
void IncSoeReadPtr(DWORD dwDevID);
void WriteAbsTimeSoe(DWORD dwDevID, TAbsTimeSoe *pAbsTimeSoe);
void WriteSysTimeSoe(DWORD dwDevID, TSysTimeSoe *pSysTimeSoe);
void IncSoeReadPtr(DWORD dwDevID);
bool ReadSysTimeSoe(DWORD dwDevID,TSysTimeSoe *pSysTimeSoe);

#endif

