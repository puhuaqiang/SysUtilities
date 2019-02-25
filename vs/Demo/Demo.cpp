// Demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <SysUtilities/include/SysUtilities.h>
#include <SysUtilities/include/api.h>
#include <SysUtilities/include/ThreadPool.h>
#include <SysUtilities/include/Cond.h>
#include <SysUtilities/include/Timestamp.h>
#include <SysUtilities/include/TimerQueue.h>
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
	std::cout << "�߳�ID: " << GetCurrentThreadId() << " Value:" << lpInfo->id << std::endl;
	Sleep(Random(500, 10000));
	std::cout << "�߳�ID: " << GetCurrentThreadId() << " Value:" << lpInfo->id << " END" << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::cin.ignore();
	{
		CTimestamp tx = CTimestamp::now();
		std::cout << tx.microSecondsSinceEpoch() << std::endl;
		std::cout << tx.toString() << std::endl;
		std::cout << tx.toFormattedString() << std::endl;

		CTimestamp t = CTimestamp::now();
		CTimerQueue tq;
		Sleep(2000);
		CTimerId tid = tq.addTimer([](){

			CTimestamp tmp = CTimestamp::now();
			std::cout << tmp.toFormattedString() << std::endl;
		}, t, 2);
		if (!tid.IsValid())
		{
			std::cout << "err" << std::endl;
		}
		CTimerId tid2 = tq.addTimer([&](){
			CTimestamp tmp = CTimestamp::now();
			std::cout << tmp.toFormattedString() << " xxx " << std::endl;
			tq.cancel(tid2);
		}, addTime(t, 3.0), 0);
		if (!tid2.IsValid())
		{
			std::cout << "errxx" << std::endl;
		}
		std::cin.ignore();
	}
	std::cin.ignore();
	{
		/**
		* ������������
		*/
		std::cout << "������������" << std::endl;
		SYS_UTL::CThreadBox tb[5];
		CCond cd;
		for (int i = 0; i < 5; i++)
		{
			tb[i].Start([&](BOOL& bRun, HANDLE hWait, void* context){
				bool bRet = false;
				while (!bRet)
				{
					bRet = cd.TryWait(1000);
					std::cout << "�߳�: " << GetCurrentThreadId() << "�ȴ���������: " << bRet << std::endl;
				}
			});
		}
		std::cin.ignore();
		std::cout << "����������������..." << std::endl;
		cd.Signal();
		std::cin.ignore();
		std::cout << "����������������..." << std::endl;
		cd.Signal();
		std::cin.ignore();
		std::cout << "����������������..." << std::endl;
		cd.Broadcast();
		std::cin.ignore();
	}

	std::cin.ignore();
	{
		/**
		* ������������
		*/
		std::cout << "������������" << std::endl;
		SYS_UTL::CThreadBox tb1, tb2;
		CCond cd;
		tb1.Start([&](BOOL& bRun, HANDLE hWait, void* context){
			Sleep(5000);
			std::cout << "������������..." << std::endl;
			cd.Signal();
		});
		tb2.Start([&](BOOL& bRun, HANDLE hWait, void* context){
			bool ret = cd.TryWait(3000);
			// ����ᳬʱ
			std::cout << "�߳�,���������ȴ�...����:" << ret << std::endl;
		});
		std::cout << "���������ȴ�..." << std::endl;
		bool ret = cd.Wait();
		std::cout << "���������ȴ�...����:" << ret << std::endl;
	}
	std::cin.ignore();
	{
		/**
		* �̳߳ز���
		*/
		std::cout << "�̳߳ز���" << std::endl;
		SYS_UTL::CThreadPool threadpool;
		int err = threadpool.Init(4, 1024 * 64, 512, 30000);
		err = threadpool.Start(ThreadPoolProc, NULL);

		int iCnt = 0;
		THREAD_POOL_INFO stInfo;
		for (int i = 0; i < 20; i++)
		{
			stInfo.id = iCnt++;
			err = threadpool.Task(&stInfo, sizeof(stInfo));
			std::cout << "������� err:" << err << std::endl;
		}
		std::cin.ignore();
	}

	return 0;
}

