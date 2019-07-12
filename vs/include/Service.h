#ifndef __SYS_UTILITIES_SERVICE_H__
#define __SYS_UTILITIES_SERVICE_H__

#include "SysUtilities.h"

namespace SYS_UTL{

	/**
		����װ���붨��
	*/
	enum {
		/// ��װ����
		SERVICE_INSTALL = 1,
		/// ж�ط���
		SERVICE_UNINSTALL,
		/// ���з���
		SERVICE_RUN,
		/// ֹͣ����
		SERVICE_DONE,
		/// ɾ������
		SERVICE_DELETE,
		/// ���Ʒ���
		SERVICE_CONTROL
	};

#ifdef _WIN32
	//��������״̬��ػص�
	typedef BOOL(*MONITOR_CALLBACK)(HANDLE hWaitEvent);

	typedef struct _ServiceInfo {
		char szName[50];//��������	
		char szDisplayName[MAX_PATH];//������ʾ����	
		char szDescribe[MAX_PATH];//��������		
	}SERVICE_INFO, *PSERVICE_INFO;

	//���������Ϣ����
	typedef struct _SERVICE_LIST {
		SERVICE_INFO stInfo;
		LPSERVICE_MAIN_FUNCTION pServiceProc;//����������	
		LPHANDLER_FUNCTION pHandlerProc;//������ƺ���	
		SERVICE_STATUS_HANDLE hStatusHandle;//����������	
		MONITOR_CALLBACK pMonitorCb;//��������״̬���ص�	
		HANDLE hExitEvent;//�����˳��¼�	
		HANDLE hWaitEvent;//�������еȴ��¼�	
	}SERVICE_LIST,*PSERVICE_LIST;

	//����
	typedef struct _SERVICE_CMD {
		int iCmd; //����װ���붨��
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
		//����װ�������
		static int AnalyseCmd(int argc, char* argv[]);
		static int AnalyseCmd(const char* szCmdLine, SERVICE_CMD pCmd[], int iLen);
		static int AnalyseCmdEx(const TCHAR* pCmdLine, PSERVICE_CMD pCmd);

		//���·���״̬
		static BOOL UpdateServiceStatus(SERVICE_STATUS_HANDLE hStatus, DWORD dwStatus, DWORD dwErrCode, DWORD dwWaitHint);
		//ע�����
		static BOOL RegisterService(LPCTSTR pServiceName, LPTSTR pServiceDisplayName);
		//ע������
		static BOOL UnregisterService(LPCTSTR pServiceName);
		//��������
		static BOOL StartupService(LPCTSTR pServiceName);
		//ֹͣ����
		static BOOL StopService(LPCTSTR pServiceName);
		//���÷���
		static BOOL ReconfigureService(LPCTSTR pServiceName, LPTSTR pDesc);
		//�жϷ����Ƿ�����
		static BOOL IsServiceRunning(LPCTSTR pServiceName);

		//��������ƺ���
		static void WINAPI ServiceHandler(DWORD dwControl);
		//������������
		static void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *pArgv);
		//���ܷ�����ƺ���
		static void WINAPI WatchDogServiceHandler(DWORD dwControl);
		//���ܷ���������
		static void WINAPI WatchDogServiceMain(DWORD dwArgc, LPTSTR *pArgv);

		//��������״̬���������
		static DWORD WINAPI TimeCheckProcLoop(LPVOID pParam);
		//����������״̬���
		static BOOL ServiceMonitor(HANDLE hWaitEvent);
		//���ܷ�������״̬���
		static BOOL WatchDogServiceMonitor(HANDLE hWaitEvent);

		//����������
		static int StartServer();
		//ֹͣ������
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