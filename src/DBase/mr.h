#ifndef __MR_H__
#define __MR_H__


DWORD GetMrNum(DWORD dwDevID);
void IncMrReadPtr(DWORD dwDevID);
bool ReadSysTimeMr(DWORD dwDevID,TSysTimeMr *pTSysTimeMr);

#endif

