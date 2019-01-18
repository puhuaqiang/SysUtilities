#include "../SysUtilities/stdafx.h"
#include "InDef.h"
#include "../include/api.h"


/* uv_once initialization guards */
static SYSUTL_ONCE __init_guard = { 0, NULL };

/* Kernel32 function pointers */
sInitializeConditionVariable pInitializeConditionVariable = NULL;
sSleepConditionVariableCS pSleepConditionVariableCS = NULL;
sSleepConditionVariableSRW pSleepConditionVariableSRW = NULL;
sWakeAllConditionVariable pWakeAllConditionVariable = NULL;
sWakeConditionVariable pWakeConditionVariable = NULL;

#if !defined(__MINGW32__) || __MSVCRT_VERSION__ >= 0x800
static void __crt_invalid_parameter_handler(const wchar_t* expression,
	const wchar_t* function, const wchar_t * file, unsigned int line,
	uintptr_t reserved) {
	/* No-op. */
}
#endif

void __init(void) 
{
	/* Tell Windows that we will handle critical errors. */
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX |
		SEM_NOOPENFILEERRORBOX);

	/* Tell the CRT to not exit the application when an invalid parameter is
	* passed. The main issue is that invalid FDs will trigger this behavior.
	*/
#if !defined(__MINGW32__) || __MSVCRT_VERSION__ >= 0x800
	_set_invalid_parameter_handler(__crt_invalid_parameter_handler);
#endif

	/* We also need to setup our debug report handler because some CRT
	* functions (eg _get_osfhandle) raise an assert when called with invalid
	* FDs even though they return the proper error code in the release build.
	*/
#if defined(_DEBUG) && (defined(_MSC_VER) || defined(__MINGW64_VERSION_MAJOR))
	//_CrtSetReportHook(uv__crt_dbg_report_handler);
#endif

	/* Fetch winapi function pointers. This must be done first because other
	* initialization code might need these function pointers to be loaded.
	*/
	__winapi_init();

}

void __once_inner(LPSYSUTL_ONCE guard, void(*callback)(void))
{
	DWORD result;
	HANDLE existing_event, created_event;

	created_event = CreateEvent(NULL, 1, 0, NULL);
	if (created_event == 0) {
		DBG_INFO("errno:%d error:%s",GetLastError(), "CreateEvent");
		return;
	}

	existing_event = InterlockedCompareExchangePointer(&guard->event,
		created_event,
		NULL);

	if (existing_event == NULL) {
		/* We won the race */
		callback();

		result = SetEvent(created_event);
		//assert(result);
		guard->ran = 1;

	}
	else {
		/* We lost the race. Destroy the event we created and wait for the */
		/* existing one to become signaled. */
		CloseHandle(created_event);
		result = WaitForSingleObject(existing_event, INFINITE);
		//assert(result == WAIT_OBJECT_0);
	}
}

void __sys_once_init(void)
{
	Once(&__init_guard, __init);
}

void __winapi_init(void)
{
	HMODULE kernel32_module;

	kernel32_module = GetModuleHandleA("kernel32.dll");
	if (kernel32_module == NULL) {
		DBG_INFO("errno:%d error:%s", GetLastError(), "GetModuleHandleA");
		return;
	}

	pInitializeConditionVariable = (sInitializeConditionVariable)
		GetProcAddress(kernel32_module, "InitializeConditionVariable");
	pSleepConditionVariableCS = (sSleepConditionVariableCS)
		GetProcAddress(kernel32_module, "SleepConditionVariableCS");

	pSleepConditionVariableSRW = (sSleepConditionVariableSRW)
		GetProcAddress(kernel32_module, "SleepConditionVariableSRW");

	pWakeAllConditionVariable = (sWakeAllConditionVariable)
		GetProcAddress(kernel32_module, "WakeAllConditionVariable");

	pWakeConditionVariable = (sWakeConditionVariable)
		GetProcAddress(kernel32_module, "WakeConditionVariable");
}