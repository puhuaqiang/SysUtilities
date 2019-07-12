#include "../include/Service.h"
#include "../include/api.h"
#include <functional>

//共享内存数据段
/*#pragma data_seg("_shared_data")
static DWORD g_dwProcessId = 0;//主服务进程ID
static DWORD g_dwProcessWatchDogId = 0;//看管服务进程ID
#pragma data_seg()
#pragma comment(linker,"/section:incmgr_shared_data,rws")*/


namespace SYS_UTL {

	//int* CService::g_ProcessID = 0;
	//int* CService::g_ProcessWatchID = 0;
	//SERVER_STATUS CService::g_Status = {};
	SERVER_STATUS CService::g_Process = {nullptr, nullptr ,nullptr ,nullptr };
	std::function<int()> CService::gStartCB = nullptr;
	std::function<void()> CService::gStopCB = nullptr;
	std::function<void(HANDLE)> CService::gRunCB = nullptr;
	SERVICE_LIST CService::g_Service[g_ServiceCnt];

#ifdef _WIN32
	//服务命令控制列表
	const SERVICE_CMD g_CmdArry[] =
	{
		//安装服务
		{ SERVICE_INSTALL, "i", "", NULL },
		{ SERVICE_INSTALL, "I", "", NULL },
		//卸载服务
		{ SERVICE_UNINSTALL, "u", "", NULL },
		{ SERVICE_UNINSTALL, "U", "", NULL },
		//运行服务
		{ SERVICE_RUN, "r", "", NULL },
		{ SERVICE_RUN, "R", "", NULL },
		//停止服务
		{ SERVICE_DONE, "s", "", NULL },
		{ SERVICE_DONE, "S", "", NULL },
		//删除服务
		{ SERVICE_DELETE, "d", "", NULL },
		{ SERVICE_DELETE, "D", "", NULL },
		//控制服务
		{ SERVICE_CONTROL, "k", "", NULL },
		{ SERVICE_CONTROL, "K", "", NULL },
	};
	const int g_Cmd_len = sizeof(g_CmdArry) / sizeof(g_CmdArry[0]);


	CService::CService() {

	}
	CService::~CService() {

	}

	int CService::Init(PSERVICE_INFO pMain, PSERVICE_INFO pWatch, 
		PSERVER_STATUS pInfo,
		std::function<int()> cbStart,
		std::function<void()> cbStop,
		std::function<void(HANDLE)> cbRun) {
		if (nullptr != pInfo) {
			memcpy(&g_Process, pInfo, sizeof(SERVER_STATUS));
		}
		gStartCB = cbStart;
		gStopCB = cbStop;
		gRunCB = cbRun;

		memset(&g_Service[0], 0, sizeof(SERVICE_LIST));
		memset(&g_Service[1], 0, sizeof(SERVICE_LIST));
		memcpy(&g_Service[0].stInfo, pMain, sizeof(SERVICE_INFO));
		memcpy(&g_Service[1].stInfo, pWatch, sizeof(SERVICE_INFO));
		g_Service[0].pServiceProc = ServiceMain;
		g_Service[0].pHandlerProc = ServiceHandler;
		g_Service[0].pMonitorCb = ServiceMonitor;

		g_Service[1].pServiceProc = WatchDogServiceMain;
		g_Service[1].pHandlerProc = WatchDogServiceHandler;
		g_Service[1].pMonitorCb = WatchDogServiceMonitor;
		return 0;
	}

	int CService::AnalyseCmd(int argc, char* argv[]) {
		SERVICE_CMD stCmd[10], *pCmd = NULL;
		char szCmdLine[MAX_PATH] = { 0 };
		SERVICE_LIST* pServiceList = NULL;
		SERVICE_TABLE_ENTRY ste[2];
		switch (argc)
		{
		case 1:
			_snprintf_s(szCmdLine, _TRUNCATE, "%s", argv[0]);
			break;
		case 2:
			_snprintf_s(szCmdLine, _TRUNCATE, "%s %s", argv[0], argv[1]);
			break;
		case 3:
			_snprintf_s(szCmdLine, _TRUNCATE, "%s %s %s", argv[0], argv[1], argv[2]);
			break;
		case 4:
			_snprintf_s(szCmdLine, _TRUNCATE, "%s %s %s %s", argv[0], argv[1], argv[2], argv[3]);
			break;
		default:
			break;
		}

		//分析命令行参数
		int iCmdCnt = AnalyseCmd(szCmdLine, stCmd, 10);
		if (iCmdCnt <= 0){
			return -1;
		}

		int err = 0;
		for (int i = 0; i < iCmdCnt; i++){
			pCmd = &stCmd[i];
			pServiceList = stCmd[i].pService;
			if (NULL == pServiceList) continue;
			switch (pCmd->iCmd)
			{
			case SERVICE_INSTALL:
				for (size_t j = 0; j < g_ServiceCnt; j++) {
					pServiceList = &g_Service[j];
					RegisterService(pServiceList->stInfo.szName, pServiceList->stInfo.szDisplayName);
					ReconfigureService(pServiceList->stInfo.szName, pServiceList->stInfo.szDescribe);
				}
				break;
			case SERVICE_UNINSTALL:
				for (size_t j = 0; j < g_ServiceCnt; j++) {
					pServiceList = &g_Service[j];
					UnregisterService(pServiceList->stInfo.szName);
				}
				break;
			case SERVICE_RUN:
				for (size_t j = 0; j < g_ServiceCnt; j++) {
					pServiceList = &g_Service[j];
					StartupService(pServiceList->stInfo.szName);
				}
				break;
			case SERVICE_DONE:
				for (size_t j = 0; j < g_ServiceCnt; j++) {
					pServiceList = &g_Service[j];
					StopService(pServiceList->stInfo.szName);
				}
				break;
			case SERVICE_DELETE:
				for (size_t j = 0; j < g_ServiceCnt; j++) {
					pServiceList = &g_Service[j];
					StopService(pServiceList->stInfo.szName);
					UnregisterService(pServiceList->stInfo.szName);
				}
				break;
			case SERVICE_CONTROL:
				if (!IsServiceRunning(pServiceList->stInfo.szName)) {
					ste[0].lpServiceName = pServiceList->stInfo.szName;
					ste[0].lpServiceProc = pServiceList->pServiceProc;
					ste[1].lpServiceName = NULL;
					ste[1].lpServiceProc = NULL;
					StartServiceCtrlDispatcher(ste);
				}
				break;
			default:
				break;
			}
		}

		return 0;
	}

	//服务安装命令分析
	int CService::AnalyseCmd(const char* ptszCmdLine, SERVICE_CMD pCmd[], int iLen){
		size_t iCmdLen = 0;
		size_t i, k, iRet = 0, index = 0;
		char Mess[MAX_PATH];
		if (NULL == ptszCmdLine){
			return 0;
		}
		iCmdLen = strlen(ptszCmdLine);
		if (iCmdLen <= 0){
			return 0;
		}

		i = 0;
		while (i < iCmdLen) {
			if (ptszCmdLine[i++] == '-') {
				k = 0;
				while (i < iCmdLen) {
					if (ptszCmdLine[i - 1] == ' ' && ptszCmdLine[i] == '-') {
						Mess[k] = '\0';
						k = 0;
						iRet = AnalyseCmdEx(Mess, &pCmd[index]);
						if (iRet == 0) index += 1;
						break;
					}
					Mess[k++] = ptszCmdLine[i];
					i += 1;
				}
				if (k > 0) {
					Mess[k] = '\0';
					k = 0;
					iRet = AnalyseCmdEx(Mess, &pCmd[index]);
					if (iRet == 0) index += 1;
				}
			}
		}
		return index;
	}

	int CService::AnalyseCmdEx(const TCHAR* ptszCmdLine, PSERVICE_CMD pCmd){
		size_t iLen = strlen(ptszCmdLine);
		if (iLen <= 0){
			return -1;
		}
		if (ptszCmdLine[0] == ' '){
			return -2;
		}
		char szCmd[MAX_PATH] = { 0 };
		char szParam[MAX_PATH] = { 0 };
		size_t i = 0;
		size_t j = 0;
		while (i < iLen) {
			if (ptszCmdLine[i] == ' ') break;
			szCmd[j++] = ptszCmdLine[i++];
		}
		szCmd[j] = '\0';

		j = 0;
		szParam[0] = '\0';
		while (i < iLen) {
			if (ptszCmdLine[i] != ' ') break;
			i += 1;
		}
		sprintf_s(szParam, MAX_PATH, "%s", ptszCmdLine + i);
		iLen = strlen(szParam);
		if (iLen > 0) {
			for (i = iLen - 1; i >= 0; i--) {
				if (szParam[i] != ' ')
					break;
				else
					szParam[i] = '\0';
			}
		}
		pCmd->pService = &g_Service[0];
		pCmd->iCmd = 0;
		sprintf_s(pCmd->szName, MAX_PATH, "%s", szCmd);
		sprintf_s(pCmd->szParam, MAX_PATH, "%s", szParam);
		for (i = 0; i < g_ServiceCnt; i++) {
			if (_stricmp(g_Service[i].stInfo.szName, szParam) == 0) {
				pCmd->pService = (PSERVICE_LIST)&g_Service[i];
				break;
			}
		}
		for (i = 0; i < g_Cmd_len; i++) {
			if (_stricmp(g_CmdArry[i].szName, pCmd->szName) == 0) {
				pCmd->iCmd = g_CmdArry[i].iCmd;
				break;
			}
		}
		return 0;
	}

	//更新服务状态
	BOOL CService::UpdateServiceStatus(SERVICE_STATUS_HANDLE hStatus, DWORD dwStatus, DWORD dwErrCode, DWORD dwWaitHint)
	{
		if (NULL == hStatus){
			return FALSE;
		}

		SERVICE_STATUS	ss = { 0 };
		DWORD dwCheckPoint = 1;
		DWORD dwControls = SERVICE_ACCEPT_STOP;
		ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;
		ss.dwCurrentState = dwStatus;
		ss.dwWaitHint = dwWaitHint;
		if (dwErrCode) {
			ss.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
			ss.dwServiceSpecificExitCode = dwErrCode;
		}
		switch (dwStatus)
		{
		case SERVICE_START_PENDING:
			dwControls = 0;
			break;
		case SERVICE_RUNNING:
		case SERVICE_STOPPED:
			dwCheckPoint = 0;
			break;
		}
		ss.dwCheckPoint = dwCheckPoint++;
		ss.dwControlsAccepted = dwControls;
		return SetServiceStatus(hStatus, &ss);
	}

	//注册服务
	BOOL CService::RegisterService(LPCTSTR pServiceName, LPTSTR pServiceDisplayName)
	{
		SC_HANDLE hSCManager = NULL;
		SC_HANDLE hSCService = NULL;
		//PCTSTRpDependencies = TEXT("DusService\0");
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (NULL != hSCManager) {
			char szServiceName[1024] = { 0 };
			char szServicePath[MAX_PATH] = { 0 };
			GetModuleFileName(NULL, szServicePath, MAX_PATH);
			_snprintf_s(szServiceName, _TRUNCATE, "%s -k %s", szServicePath, pServiceName);
			hSCService = CreateService(
				hSCManager,
				pServiceName,
				pServiceDisplayName,
				SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
				SERVICE_AUTO_START,
				SERVICE_ERROR_NORMAL,
				szServiceName,	//tszServicePath,
				NULL,// no load ordering group 
				NULL,// no tag identifier 
				NULL,// no dependencies 
				NULL,// LocalSystem account 
				NULL// no password
			);
		}
		if (NULL != hSCService) {
			SERVICE_DELAYED_AUTO_START_INFO info = { TRUE };
			::ChangeServiceConfig2(hSCService, SERVICE_CONFIG_DELAYED_AUTO_START_INFO, &info);
			CloseServiceHandle(hSCService);
			hSCService = NULL;
		}
		if (NULL != hSCManager) {
			CloseServiceHandle(hSCManager);
			hSCManager = NULL;
		}
		return (NULL != hSCService);
	}

	//注销服务
	BOOL CService::UnregisterService(LPCTSTR pServiceName)
	{
		BOOL br = FALSE;
		SC_HANDLE hSCManager = NULL;
		SC_HANDLE hSCService = NULL;
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (NULL != hSCManager) {
			hSCService = OpenService(hSCManager, pServiceName, SERVICE_ALL_ACCESS);
			if (NULL != hSCService)
				br = DeleteService(hSCService);
		}
		if (NULL != hSCService) {
			CloseServiceHandle(hSCService);
			hSCService = NULL;
		}
		if (NULL != hSCManager) {
			CloseServiceHandle(hSCManager);
			hSCManager = NULL;
		}
		return br;
	}

	//开始服务
	BOOL CService::StartupService(LPCTSTR pServiceName)
	{
		SC_HANDLE hSCManager = NULL;
		SC_HANDLE hSCService = NULL;
		BOOL br = FALSE;
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (NULL != hSCManager) {
			hSCService = OpenService(hSCManager, pServiceName, SERVICE_ALL_ACCESS);
			if (NULL != hSCService)
				br = StartService(hSCService, 0, NULL);
		}
		if (NULL != hSCService) {
			CloseServiceHandle(hSCService);
			hSCService = NULL;
		}
		if (NULL != hSCManager) {
			CloseServiceHandle(hSCManager);
			hSCManager = NULL;
		}
		return br;
	}

	//停止服务
	BOOL CService::StopService(LPCTSTR pServiceName)
	{
		SC_HANDLE hSCManager = NULL;
		SC_HANDLE hSCService = NULL;
		BOOL br = FALSE;
		SERVICE_STATUS	ss;
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (NULL != hSCManager) {
			hSCService = OpenService(hSCManager, pServiceName, SERVICE_ALL_ACCESS);
			if (NULL != hSCService) {
				br = QueryServiceStatus(hSCService, &ss);
				if (br) {
					if (SERVICE_RUNNING == ss.dwCurrentState)
						br = ControlService(hSCService, SERVICE_CONTROL_STOP, &ss);
				}
			}
		}
		if (NULL != hSCService) {
			CloseServiceHandle(hSCService);
			hSCService = NULL;
		}
		if (NULL != hSCManager) {
			CloseServiceHandle(hSCManager);
			hSCManager = NULL;
		}
		return br;
	}

	//重置服务
	BOOL CService::ReconfigureService(LPCTSTR pServiceName, LPTSTR pDesc)
	{
		SC_HANDLE hSCManager = NULL;
		SC_HANDLE hSCService = NULL;
		BOOL br = FALSE;
		SERVICE_DESCRIPTION sd;
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (NULL != hSCManager) {
			hSCService = OpenService(hSCManager, pServiceName, SERVICE_ALL_ACCESS);
			if (NULL != hSCService) {
				sd.lpDescription = pDesc;
				br = ChangeServiceConfig2(hSCService, SERVICE_CONFIG_DESCRIPTION, &sd);
			}
		}
		if (NULL != hSCService) {
			CloseServiceHandle(hSCService);
			hSCService = NULL;
		}
		if (NULL != hSCManager) {
			CloseServiceHandle(hSCManager);
			hSCManager = NULL;
		}

		return br;
	}

	//判断服务是否运行
	BOOL CService::IsServiceRunning(LPCTSTR pServiceName)
	{
		SC_HANDLE hSCManager = NULL;
		SC_HANDLE hSCService = NULL;
		BOOL br = FALSE;
		SERVICE_STATUS	ss;
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (NULL != hSCManager) {
			hSCService = OpenService(hSCManager, pServiceName, SERVICE_ALL_ACCESS);
			if (NULL != hSCService) {
				br = QueryServiceStatus(hSCService, &ss);
				if (br) br = (SERVICE_RUNNING == ss.dwCurrentState);
			}
		}
		if (NULL != hSCService) {
			CloseServiceHandle(hSCService);
			hSCService = NULL;
		}
		if (NULL != hSCManager) {
			CloseServiceHandle(hSCManager);
			hSCManager = NULL;
		}
		return br;
	}

	//主服务控制函数
	void WINAPI CService::ServiceHandler(DWORD dwControl)
	{
		PSERVICE_LIST pService = (PSERVICE_LIST)&g_Service[0];
		switch (dwControl)
		{
		case SERVICE_CONTROL_STOP:// Requests the pService to Stop. 		
		case SERVICE_CONTROL_SHUTDOWN:// Requests the pService to perform cleanup tasks, because the system is shutting down. 
									  //For more information, see Remarks.
			if (g_Process.bStopMainProcess != nullptr) {
				*g_Process.bStopMainProcess = TRUE;
			}
			if (NULL != pService->hWaitEvent)
				SetEvent(pService->hWaitEvent);
			if (NULL != pService->hStatusHandle)
				UpdateServiceStatus(pService->hStatusHandle, SERVICE_STOP, NO_ERROR, 0);
			if (NULL != pService->hExitEvent)
				WaitForSingleObject(pService->hExitEvent, 5000);
			break;
		case SERVICE_CONTROL_PAUSE:// Requests the pService to pause.  
			break;
		case SERVICE_CONTROL_CONTINUE:// Requests the paused pService to resume.
			break;
		case SERVICE_CONTROL_INTERROGATE:// Requests the pService to update immediately its current status information to the pService control manager.  
										 //SetServiceStatus(g_ssh, &g_ss);
			break;
		case SERVICE_CONTROL_PARAMCHANGE:// Windows 2000: Notifies the pService that pService-specific startup parameters have changed. The pService should reread its startup parameters. 
			break;
		case SERVICE_CONTROL_NETBINDADD:// Windows 2000: Notifies a network pService that there is a new component for binding. The pService should bind to the new component.  
			break;
		case SERVICE_CONTROL_NETBINDREMOVE:// Windows 2000: Notifies a network pService that a component for binding has been removed. The pService should reread its binding information and unbind iAlarmFrom the removed component.  
			break;
		case SERVICE_CONTROL_NETBINDENABLE:// Windows 2000: Notifies a network pService that a disabled binding has been enabled. The pService should reread its binding information and add the new binding.  
			break;
		case SERVICE_CONTROL_NETBINDDISABLE:// 
			break;
		default:
			break;
		}
	}

	//主服务主函数
	void WINAPI CService::ServiceMain(DWORD dwArgc, LPTSTR *pArgv)
	{
		PSERVICE_LIST pService = &g_Service[0];
		//获取服务控制句柄
		pService->hStatusHandle = RegisterServiceCtrlHandler(pService->stInfo.szName, pService->pHandlerProc);
		if (NULL == pService->hStatusHandle)
		{
			return;
		}
		if (g_Process.bStopMainProcess != nullptr) {
			*g_Process.bStopMainProcess = FALSE;
		}
		//更新服务状态
		UpdateServiceStatus(pService->hStatusHandle, SERVICE_START_PENDING, NO_ERROR, 0);
		//保存主服务进程ID
		if (nullptr != g_Process.iMainProcessID) {
			*g_Process.iMainProcessID = GetCurrentProcessId();
		}
		//创建主服务检测线程退出事件
		pService->hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		//创建主服务检测线程等待事件
		pService->hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		//启动服务器
		int err = StartServer();
		if (0 == err) {
			//更新服务当前状态
			UpdateServiceStatus(pService->hStatusHandle, SERVICE_RUNNING, NO_ERROR, 0);
			if (gRunCB) {
				gRunCB(pService->hWaitEvent);
			}
			else {
				//进入服务状态检测循环函数
				TimeCheckProcLoop(pService);
			}
		}
		//停止服务器
		StopServer();
		SetEvent(pService->hExitEvent);
		UpdateServiceStatus(pService->hStatusHandle, SERVICE_STOPPED, NO_ERROR, 0);
		pService->hStatusHandle = NULL;
		CloseHandle(pService->hWaitEvent);
		pService->hWaitEvent = NULL;
		CloseHandle(pService->hExitEvent);
		pService->hExitEvent = NULL;
	}

	//看管服务控制函数
	void WINAPI CService::WatchDogServiceHandler(DWORD dwControl)
	{
		PSERVICE_LIST pService = (PSERVICE_LIST)&g_Service[1];
		switch (dwControl)
		{
		case SERVICE_CONTROL_STOP:// Requests the pService to Stop. 		
		case SERVICE_CONTROL_SHUTDOWN:// Requests the pService to perform cleanup tasks, because the system is shutting down. 
									  //For more information, see Remarks.
			if (nullptr != g_Process.bStopWatchDogProccess) {
				*g_Process.bStopWatchDogProccess = TRUE;
			}
			if (NULL != pService->hWaitEvent)
				SetEvent(pService->hWaitEvent);
			if (NULL != pService->hStatusHandle)
				UpdateServiceStatus(pService->hStatusHandle, SERVICE_STOP, NO_ERROR, 0);
			if (NULL != pService->hExitEvent)
				WaitForSingleObject(pService->hExitEvent, 5000);
			break;
		case SERVICE_CONTROL_PAUSE:// Requests the pService to pause.  
			break;
		case SERVICE_CONTROL_CONTINUE:// Requests the paused pService to resume. 
			break;
		case SERVICE_CONTROL_INTERROGATE:// Requests the pService to update immediately its current status information to the pService control manager.  
										 //SetServiceStatus(g_ssh, &g_ss);
			break;
		case SERVICE_CONTROL_PARAMCHANGE:// Windows 2000: Notifies the pService that pService-specific startup parameters have changed. The pService should reread its startup parameters. 
			break;
		case SERVICE_CONTROL_NETBINDADD:// Windows 2000: Notifies a network pService that there is a new component for binding. The pService should bind to the new component.  
			break;
		case SERVICE_CONTROL_NETBINDREMOVE:// Windows 2000: Notifies a network pService that a component for binding has been removed. The pService should reread its binding information and unbind iAlarmFrom the removed component.  
			break;
		case SERVICE_CONTROL_NETBINDENABLE:// Windows 2000: Notifies a network pService that a disabled binding has been enabled. The pService should reread its binding information and add the new binding.  
			break;
		case SERVICE_CONTROL_NETBINDDISABLE:// 
			break;
		default:
			break;
		}
	}

	//看管服务主函数
	void WINAPI CService::WatchDogServiceMain(DWORD dwArgc, LPTSTR *pArgv)
	{
		PSERVICE_LIST pService = &g_Service[1];
		//获取服务控制句柄
		pService->hStatusHandle = RegisterServiceCtrlHandler(pService->stInfo.szName, pService->pHandlerProc);
		if (NULL == pService->hStatusHandle)
		{
			return;
		}
		if (nullptr != g_Process.bStopWatchDogProccess) {
			*g_Process.bStopWatchDogProccess = FALSE;
		}

		//更新服务状态
		UpdateServiceStatus(pService->hStatusHandle, SERVICE_START_PENDING, NO_ERROR, 0);
		//保存主服务进程ID
		if (nullptr != g_Process.iWatchDogProcessID) {
			*g_Process.iWatchDogProcessID = GetCurrentProcessId();
		}
		//创建主服务检测线程退出事件
		pService->hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		//创建主服务检测线程等待事件
		pService->hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		//更新服务当前状态
		UpdateServiceStatus(pService->hStatusHandle, SERVICE_RUNNING, NO_ERROR, 0);

		//进入服务状态检测循环函数
		TimeCheckProcLoop(pService);

		SetEvent(pService->hExitEvent);
		UpdateServiceStatus(pService->hStatusHandle, SERVICE_STOPPED, NO_ERROR, 0);
		CloseHandle(pService->hWaitEvent);
		pService->hWaitEvent = NULL;
		CloseHandle(pService->hExitEvent);
		pService->hExitEvent = NULL;
	}

	//服务运行状态检测主函数
	DWORD WINAPI CService::TimeCheckProcLoop(LPVOID pParam)
	{
		DWORD dwWait = 0;
		PSERVICE_LIST pService = (PSERVICE_LIST)pParam;
		while (TRUE) {
			dwWait = WaitForSingleObject(pService->hWaitEvent, 100);
			if (WAIT_OBJECT_0 == dwWait)
			{
				break;
			}
			if (!pService->pMonitorCb(pService->hWaitEvent))
			{
				break;
			}
		}
		return 0;
	}

	//主服务运行状态监控
	BOOL CService::ServiceMonitor(HANDLE hWaitEvent)
	{
		if (nullptr == g_Process.iWatchDogProcessID) {
			return TRUE;
		}

		if (*g_Process.bStopWatchDogProccess) {
			return TRUE;
		}

		if (*g_Process.iWatchDogProcessID <= 0) {
			*g_Process.iWatchDogProcessID = 0;
			StartupService(g_Service[1].stInfo.szName);
			return TRUE;
		}

		HANDLE hProcess = ::OpenProcess(SYNCHRONIZE, FALSE, *g_Process.iWatchDogProcessID);
		HANDLE handles[] = { hWaitEvent, hProcess };
		DWORD dwWait = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
		CloseHandle(hProcess);
		BOOL br = TRUE;
		switch (dwWait)
		{
		case WAIT_OBJECT_0 + 1:
			if (!(*g_Process.bStopWatchDogProccess)) {
				*g_Process.iWatchDogProcessID = 0;
				StartupService(g_Service[1].stInfo.szName);
			}
			break;
		case WAIT_OBJECT_0:
		default:
			br = FALSE;
			break;
		}
		return br;
	}


	//看管服务运行状态监控
	BOOL CService::WatchDogServiceMonitor(HANDLE hWaitEvent)
	{
		if (nullptr == g_Process.iMainProcessID) {
			return TRUE;
		}

		if (*g_Process.bStopMainProcess) {
			return TRUE;
		}

		if (*g_Process.iMainProcessID <= 0) {
			*g_Process.iMainProcessID = 0;
			StartupService(g_Service[0].stInfo.szName);
			return TRUE;
		}

		HANDLE hProcess = ::OpenProcess(SYNCHRONIZE, FALSE, *g_Process.iMainProcessID);
		HANDLE handles[] = { hWaitEvent, hProcess };
		DWORD dwWait = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
		CloseHandle(hProcess);
		BOOL br = TRUE;
		switch (dwWait)
		{
		case WAIT_OBJECT_0 + 1:
			if (!(*g_Process.bStopMainProcess)) {
				*g_Process.iMainProcessID = 0;
				StartupService(g_Service[0].stInfo.szName);
			}
			break;
		case WAIT_OBJECT_0:
		default:
			br = FALSE;
			break;
		}
		return br;
	}

	int CService::StartServer()
	{
		if (!gStartCB) {
			return -1;
		}
		return gStartCB();
	}

	void CService::StopServer()
	{
		if (!gStopCB) {
			return;
		}
		gStopCB();
	}
#else

#endif
}