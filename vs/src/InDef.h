#ifndef __SYS_UTILITIES_INDEF__
#define __SYS_UTILITIES_INDEF__
#include "../include/api.h"
#include "../include/Debug.h"
#ifndef DBG_I
#define DBG_I(...) SYS_UTL::dbgview_info2("SysUtility", __FILE__, __LINE__, __VA_ARGS__);
#endif
#ifndef DBG_E
#define DBG_E DBG_I("ERROR");
#endif
#ifndef DBG_F
#define DBG_F DBG_I("FATAL ERROR");
#endif

extern HINSTANCE ghInstance;

typedef VOID(WINAPI* sInitializeConditionVariable)(PCONDITION_VARIABLE ConditionVariable);
typedef BOOL(WINAPI* sSleepConditionVariableCS)
(PCONDITION_VARIABLE ConditionVariable,
PCRITICAL_SECTION CriticalSection,
DWORD dwMilliseconds);

typedef BOOL(WINAPI* sSleepConditionVariableSRW)
(PCONDITION_VARIABLE ConditionVariable,
PSRWLOCK SRWLock,
DWORD dwMilliseconds,
ULONG Flags);

typedef VOID(WINAPI* sWakeAllConditionVariable)(PCONDITION_VARIABLE ConditionVariable);

typedef VOID(WINAPI* sWakeConditionVariable)(PCONDITION_VARIABLE ConditionVariable);

/* Kernel32 function pointers */
extern sInitializeConditionVariable pInitializeConditionVariable;
extern sSleepConditionVariableCS pSleepConditionVariableCS;
extern sSleepConditionVariableSRW pSleepConditionVariableSRW;
extern sWakeAllConditionVariable pWakeAllConditionVariable;
extern sWakeConditionVariable pWakeConditionVariable;
/*
* Winapi and ntapi utility functions
*/
void __winapi_init(void);

void __sys_once_init(void);
void __once_inner(LPSYSUTL_ONCE guard, void(*callback)(void));

#endif