#ifndef __SYS_UTILITIES_INDEF__
#define __SYS_UTILITIES_INDEF__
#include "../include/api.h"
#if 1
#define DBG_INFO(...) dbgview_info2("SysUtilties",__FILE__,__LINE__,__VA_ARGS__);
#define DBG_ERROR DBG_INFO("ERROR")
#else
#define DBG_INFO(...)
#define DBG_ERROR
#endif

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