#ifndef __AIT_H__
#define __AIT_H__


DWORD GetAitNum(DWORD dwDevID);
void ClearAit(DWORD dwDevID);
DWORD GetAitReadPtr(DWORD dwDevID);
DWORD GetAitWritePtr(DWORD dwDevID);
void IncAitReadPtr(DWORD dwDevID);
void WriteSysTimeAit(DWORD dwDevID,TSysTimeAit *pSysTimeAit);
void Logic_WriteSysTimeAit(DWORD dwDevID,TSysTimeAit *pSysTimeAit);
bool ReadSysTimeAit(DWORD dwDevID,TSysTimeAit *pSysTimeAit);
bool ReadAbsTimeAit(DWORD dwDevID,DWORD *pdwReadPtr,TAbsTimeAit *pAbsTimeAit);
//bool ReadAbsTimeAit(DWORD dwDevID,TAbsTimeAit *pAbsTimeAit);

#endif


