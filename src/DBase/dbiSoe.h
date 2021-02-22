#ifndef __DBISOE_H__
#define __DBISOE_H__


DWORD GetDBISoeNum(DWORD dwDevID);
void ClearDBISoe(DWORD dwDevID);
DWORD GetDBISoeReadPtr(DWORD dwDevID);
DWORD GetDBISoeWritePtr(DWORD dwDevID);
void IncDBISoeReadPtr(DWORD dwDevID);
void WriteAbsTimeDBISoe(DWORD dwDevID,TAbsTimeDBISoe *pAbsTimeDBISoe);
void WriteSysTimeDBISoe(DWORD dwDevID, TSysTimeDBISoe *pSysTimeDBISoe);
void Real_WriteSysTimeDBISoe(DWORD dwDevID,TSysTimeDBISoe *pSysTimeDBISoe, BOOL bByBI);
void Logic_WriteSysTimeDBISoe(DWORD dwDevID,TSysTimeDBISoe *pSysTimeDBISoe);
bool ReadAbsTimeDBISoe(DWORD dwDevID,DWORD *pdwReadPtr,TAbsTimeDBISoe *pAbsTimeDBISoe);
bool ReadSysTimeDBISoe(DWORD dwDevID,TSysTimeDBISoe *pSysTimeDBISoe);

#endif
