// Demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <SysUtilities/include/SysUtilities.h>
#include <SysUtilities/include/api.h>
#include <SysUtilities/include/ThreadPool.h>
#ifdef _DEBUG
#pragma comment(lib, "SysUtilities/lib/SysUtilitiesd.lib")
#else
#pragma comment(lib, "SysUtilities/lib/SysUtilities.lib")
#endif

using namespace SYS_UTL;
struct  THREAD_POOL_INFO
{
	int id;
};
void ThreadPoolProc(void* lpTask, int iTaskDataLen, void* lpUsr)
{
	THREAD_POOL_INFO* lpInfo = (THREAD_POOL_INFO*)lpTask;
	std::cout << "线程ID: " << GetCurrentThreadId() << " Value:" << lpInfo->id << std::endl;
	Sleep(Random(500, 10000));
	std::cout << "线程ID: " << GetCurrentThreadId() << " Value:" << lpInfo->id << " END" << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::cin.ignore();
	{
		/**
		* 线程池测试
		*/
		SYS_UTL::CThreadPool threadpool;
		int err = threadpool.Init(4, 1024 * 64, 512, 30000);
		err = threadpool.Start(ThreadPoolProc, NULL);

		int iCnt = 0;
		THREAD_POOL_INFO stInfo;
		for (int i = 0; i < 20; i++)
		{
			stInfo.id = iCnt++;
			err = threadpool.Task(&stInfo, sizeof(stInfo));
			std::cout << "添加任务 err:" << err << std::endl;
		}
		std::cin.ignore();
	}
	return 0;
}

