#ifndef __SYS_UTILITIES_SERVICE_H__
#define __SYS_UTILITIES_SERVICE_H__

#include "SysUtilities.h"

namespace SYS_UTL{

	/**
		服务安装代码定义
	*/
	enum {
		/// 安装服务
		SERVICE_INSTALL = 1,
		/// 卸载服务
		SERVICE_UNINSTALL,
		/// 运行服务
		SERVICE_RUN,
		/// 停止服务
		SERVICE_DONE,
		/// 删除服务
		SERVICE_DELETE,
		/// 控制服务
		SERVICE_CONTROL
	};

#ifdef _WIN32
	//服务运行状态监控回调
	typedef BOOL(*MONITOR_CALLBACK)(HANDLE hWaitEvent);

	typedef struct _ServiceInfo {
		char szName[50];//服务名称	
		char szDisplayName[MAX_PATH];//服务显示名称	
		char szDescribe[MAX_PATH];//服务描述		
	}SERVICE_INFO, *PSERVICE_INFO;

	//服务相关信息定义
	typedef struct _SERVICE_LIST {
		SERVICE_INFO stInfo;
		LPSERVICE_MAIN_FUNCTION pServiceProc;//服务主函数	
		LPHANDLER_FUNCTION pHandlerProc;//服务控制函数	
		SERVICE_STATUS_HANDLE hStatusHandle;//服务操作句柄	
		MONITOR_CALLBACK pMonitorCb;//服务运行状态检测回调	
		HANDLE hExitEvent;//服务退出事件	
		HANDLE hWaitEvent;//服务运行等待事件	
	}SERVICE_LIST,*PSERVICE_LIST;

	//命令
	typedef struct _SERVICE_CMD {
		int iCmd; //服务安装代码定义
		char szName[128];
		char szParam[128];
		PSERVICE_LIST pService;
	}SERVICE_CMD,*PSERVICE_CMD;

	typedef struct _SERVER_STATUS {
		int* iMainProcessID;
		int* iWatchDogProcessID;
		BOOL* bStopMainProcess;
		BOOL* bStopWatchDogProccess;
	}SERVER_STATUS, *PSERVER_STATUS;

	//template class UTILS_API std::function<int()>;
	//template class UTILS_API std::function<void()>;
	//template class UTILS_API std::function<void(HANDLE)>;


	class SYS_UTL_CPPAPI CService final {

	public:
		CService();
		~CService();

		static int Init(PSERVICE_INFO, PSERVICE_INFO, 
			PSERVER_STATUS,
			std::function<int()>, std::function<void()>, std::function<void(HANDLE)>);
		//服务安装命令分析
		static int AnalyseCmd(int argc, char* argv[]);
		static int AnalyseCmd(const char* szCmdLine, SERVICE_CMD pCmd[], int iLen);
		static int AnalyseCmdEx(const TCHAR* pCmdLine, PSERVICE_CMD pCmd);

		//更新服务状态
		static BOOL UpdateServiceStatus(SERVICE_STATUS_HANDLE hStatus, DWORD dwStatus, DWORD dwErrCode, DWORD dwWaitHint);
		//注册服务
		static BOOL RegisterService(LPCTSTR pServiceName, LPTSTR pServiceDisplayName);
		//注销服务
		static BOOL UnregisterService(LPCTSTR pServiceName);
		//启动服务
		static BOOL StartupService(LPCTSTR pServiceName);
		//停止服务
		static BOOL StopService(LPCTSTR pServiceName);
		//重置服务
		static BOOL ReconfigureService(LPCTSTR pServiceName, LPTSTR pDesc);
		//判断服务是否运行
		static BOOL IsServiceRunning(LPCTSTR pServiceName);

		//主服务控制函数
		static void WINAPI ServiceHandler(DWORD dwControl);
		//主服务主函数
		static void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *pArgv);
		//看管服务控制函数
		static void WINAPI WatchDogServiceHandler(DWORD dwControl);
		//看管服务主函数
		static void WINAPI WatchDogServiceMain(DWORD dwArgc, LPTSTR *pArgv);

		//服务运行状态检测主函数
		static DWORD WINAPI TimeCheckProcLoop(LPVOID pParam);
		//主服务运行状态监控
		static BOOL ServiceMonitor(HANDLE hWaitEvent);
		//看管服务运行状态监控
		static BOOL WatchDogServiceMonitor(HANDLE hWaitEvent);

		//启动服务器
		static int StartServer();
		//停止服务器
		static void StopServer();

	private:
		const static int g_ServiceCnt = 2;
		static SERVICE_LIST g_Service[g_ServiceCnt];
		static SERVER_STATUS g_Process;
		//static int* g_ProcessWatchID;
		static std::function<int()> gStartCB;
		static std::function<void()> gStopCB;
		static std::function<void(HANDLE)> gRunCB;
		//static SERVER_STATUS g_Status;
	};
#else

#endif
} // namespace SYS_UTL
#endif