// Demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <SysUtilities/include/net/net.h>
#include <windows.h>
#include <SysUtilities/include/SysUtilities.h>
#include <SysUtilities/include/api.h>
#include <SysUtilities/include/ThreadPool.h>
#include <SysUtilities/include/Cond.h>
#include <SysUtilities/include/Timestamp.h>
#include <SysUtilities/include/TimerQueue.h>
#include <SysUtilities/include/AutoLock.h>

#ifdef _DEBUG
#pragma comment(lib, "SysUtilities/lib/SysUtilitiesd.lib")
#else
#pragma comment(lib, "SysUtilities/lib/SysUtilities.lib")
#endif

#include <thread>

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
	if (true)
	{
		SYS_UTL::CTaskThread _task;
		std::thread tb2([&](){
			for (int i = 0; i < 20; i++)
			{
				_task.Idle([](int v1, int v2){
					std::cout << "idle " << v1 << "  " << v2 << std::endl;
				}, i, i * 2);
				Sleep(Random(10, 2000));
			}
		});
		Sleep(Random(10, 300));
		std::thread tb1([&](){
			for (int i = 0; i < 20; i++)
			{
				_task.Task([](int v1, int v2){
					std::cout << "task " << v1 << "  " << v2 << std::endl;
				}, i, i*i);
				Sleep(Random(10, 1000));
				if (i > 10)
				{
					Sleep(1000);
				}
			}
		});

		std::cin.ignore();
	}
	std::cin.ignore();
	if (true)
	{
		/***
		* @brief 自动加锁/释放锁测试
		* 延迟加锁测试
		*/
		SYS_UTL::CCritSec cs;
		{
			SYS_UTL::CAutoLock lck(&cs);
			std::cout << "加锁成功 line:"<<__LINE__ << std::endl;
			{
				SYS_UTL::CAutoLock lck(&cs, SYS_UTL::LOCK_FLAG::lock_defer);
				lck.Lock(1000);
				if (lck.Owns())
				{
					std::cout << "加锁成功 line:" << __LINE__ << std::endl;
				}
				else{
					std::cout << "加锁失败 line:" << __LINE__ << std::endl;
				}
				{
					SYS_UTL::CAutoLock lck(&cs, SYS_UTL::LOCK_FLAG::lock_defer);
					lck.Lock(2000);
					if (lck.Owns())
					{
						std::cout << "加锁成功 line:" << __LINE__ << std::endl;
					}
					else{
						std::cout << "加锁失败 line:" << __LINE__ << std::endl;
					}
				}
			}
		}
		{
			SYS_UTL::CAutoLock lck(&cs);
			std::cout << "加锁成功 line:" << __LINE__ << std::endl;
		}
	}
	std::cin.ignore();
	if (true){// 
		SYS_UTL::ThreadPool pool(4);
		std::vector< std::future<int> > results;

		for (int i = 0; i < 8; ++i) {
			results.emplace_back(
				pool.enqueue([i] {
				std::cout << "hello " << i << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::cout << "world " << i << std::endl;
				return i*i;
			})
				);
		}

		for (auto && result : results)
			std::cout << result.get() << ' ';
		std::cout << std::endl;
	}
	std::cin.ignore();
	if (true){// 
		SYS_UTL::NET::CNetClient clt;
		std::cout << "开始连接." << std::endl;
		clt.ConnectSocket(SYS_UTL::NET::TRANS_PROTOCOL_TYPE_TCP, "192.168.1.253", 1031);
		clt.SetSktNoBlock();
		if (!clt.IsError() && clt.IsValidSkt()) {
			std::cout << "链接成功." << std::endl;
			std::string str;
			char buff[1024];
			int len = 0;

			while (true)
			{
				str = "";
				std::cout << "输入:" << std::endl;
				std::cin >> str;

				if (str.length()) {
					len = clt.Write((const char*)str.data(), str.length(), 2000);
					std::cout << "w:" << len << std::endl;
				}

				len = clt.Read(buff, 1024, 0, 2000);
				if (len > 0) {
					buff[len] = '\0';
					std::cout << "客户端接收到数据:" << buff << std::endl;
				}
			}
		}
		else {
			std::cout << "链接失败." << std::endl;
		}
	}

	std::cin.ignore();
	if (true){// 
		std::cout << "TCP." << std::endl;
		SYS_UTL::NET::CNetServer cNetServer;
		int err = cNetServer.OpenSocket(SYS_UTL::NET::TRANS_PROTOCOL_TYPE_TCP, nullptr, 5000);
		if (err != 0 || cNetServer.IsError() || !cNetServer.IsValidSkt()){
			std::cout << "OpenSocket err" << std::endl;
			return -1;
		}
		cNetServer.SetSktNoBlock();
		cNetServer.SetTcpNoDelay();
		cNetServer.SetSktReuseAddr(true);
		SYS_UTL::NET::CNetClient* pNetClient = nullptr;

		while (true)
		{
			if (cNetServer.Accept(pNetClient)){
				pNetClient->SetSktNoBlock();
				new std::thread([](SYS_UTL::NET::CNetClient*& skt){
					char szData[1024 * 8], log[1024 * 4];
					char tmp[16];
					int len = 0;
					BYTE heard_pack[] = { 0x5A, 0x51, 0x57, 0x4C, 0x2D, 0x45, 0x74, 0x68, 0x52, 0x53, 0x2D, 0x50, 0x4F, 0x52, 0x54, 0x31 };
					BYTE get_sys_par[] = { 0xFE, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0xB8, 0xEF, 0xAA };
					BYTE get_sys_par1[] = { 0xFE, 0x01, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0xB8, 0xEF, 0xAA };
					while (true)
					{
						szData[0] = '\0';
						len = skt->Read(szData, 1024 * 8);
						if (len <= 0) {
							continue;
						}

						log[0] = '\0';
						_snprintf_s(log, _TRUNCATE, "接收到数据: ");
						for (int i = 0; i < len; i++)
						{
							_snprintf_s(tmp, _TRUNCATE, "%02X ", szData[i]);
							strncat_s(log, _TRUNCATE, tmp, _TRUNCATE);
						}
						std::cout << log << std::endl;
						OutputDebugString(log);

						if (!memcmp(szData, heard_pack, sizeof(heard_pack)))
						{
							std::cout << "心跳包" << std::endl;

							int ret = skt->Write((const char*)get_sys_par, sizeof(get_sys_par), 500);
							std::cout << "发生数据长度:" << ret << " , " << ((ret == sizeof(get_sys_par)) ? "成功" : "失败") << std::endl;

							BYTE buff[16];
							int length = sizeof(get_sys_par1);
							memcpy(buff, get_sys_par1, length);
							int cnt = 0;
							for (int i = 0; i < length - 1; i++)
							{
								cnt += get_sys_par1[i];
							}
							buff[length - 1] = cnt;

							log[0] = '\0';
							_snprintf_s(log, _TRUNCATE, "xx: ");
							for (int i = 0; i < length; i++)
							{
								_snprintf_s(tmp, _TRUNCATE, "%02X ", buff[i]);
								strncat_s(log, _TRUNCATE, tmp, _TRUNCATE);
							}
							std::cout << log << std::endl;
							OutputDebugString(log);

							ret = skt->Write((const char*)buff, length, 500);
							std::cout << "发生数据长度:" << ret << " , " << ((ret == sizeof(get_sys_par)) ? "成功" : "失败") << std::endl;
						}
						//strncat_s(szData, _TRUNCATE, "—SERVER", _TRUNCATE);
						//skt->Write(szData, strlen(szData), 2000);
					}
				}, std::ref(pNetClient));
			}
			else{
				Sleep(10);
			}
		}
	}

	std::cin.ignore();
	if(true){// tcp
		std::cout << "TCP." << std::endl;
		SYS_UTL::NET::CNetServer cNetServer;
		int err = cNetServer.OpenSocket(SYS_UTL::NET::TRANS_PROTOCOL_TYPE_TCP, nullptr, 9898);
		if (err != 0 || cNetServer.IsError() || !cNetServer.IsValidSkt()){
			std::cout << "OpenSocket err" << std::endl;
			return -1;
		}
		cNetServer.SetSktNoBlock();
		cNetServer.SetTcpNoDelay();
		cNetServer.SetSktReuseAddr(true);
		SYS_UTL::NET::CNetClient* pNetClient = nullptr;
		std::thread clt([](){
			Sleep(1000);
			SYS_UTL::NET::CNetClient clt;
			std::cout << "开始连接." << std::endl;
			clt.ConnectSocket(SYS_UTL::NET::TRANS_PROTOCOL_TYPE_TCP, "127.0.0.1", 9898);
			clt.SetSktNoBlock();
			if (!clt.IsError() && clt.IsValidSkt()) {
				std::cout << "链接成功." << std::endl;
				std::string str;
				char buff[1024];
				int len = 0;

				while (true)
				{
					str = "";
					std::cout << "输入:" << std::endl;
					std::cin >> str;

					if (str.length()) {
						len = clt.Write((const char*)str.data(), str.length(), 2000);
						std::cout << "w:" << len << std::endl;
					}

					len = clt.Read(buff, 1024, 0, 2000);
					if (len > 0) {
						buff[len] = '\0';
						std::cout << "客户端接收到数据:" << buff << std::endl;
					}
				}
			}
			else {
				std::cout << "链接失败." << std::endl;
			}
		});
		while (true)
		{
			if (cNetServer.Accept(pNetClient)){
				pNetClient->SetSktNoBlock();
				new std::thread([](SYS_UTL::NET::CNetClient*& skt){
					char szData[1024];
					int len = 0;
					while (true)
					{
						szData[0] = '\0';
						len = skt->Read(szData, 1024);
						if (len <= 0) {
							continue;
						}
						szData[len] = '\0';
						std::cout << szData << std::endl;

						strncat_s(szData, _TRUNCATE, "—SERVER", _TRUNCATE);
						skt->Write(szData, strlen(szData), 2000);
					}
				}, std::ref(pNetClient));
			}
			else{
				Sleep(10);
			}
		}
	}

	std::cin.ignore();
	{
		std::cout << "UDP." << std::endl;
		char buff[1024];
		sockaddr stUdpSrc;
		int len = sizeof(sockaddr);
		SYS_UTL::NET::CNetServer cNetServer;
		int err = cNetServer.OpenSocket(SYS_UTL::NET::TRANS_PROTOCOL_TYPE_UDP, nullptr, 9898);
		if (err != 0 || cNetServer.IsError() || !cNetServer.IsValidSkt()){
			std::cout << "OpenSocket err" << std::endl;
			return -1;
		}
		cNetServer.SetSktNoBlock();
		cNetServer.SetSktReuseAddr(true);
		SYS_UTL::NET::CNetClient* pNetClient = nullptr;
		std::thread clt([](){
			Sleep(1000);
			SYS_UTL::NET::CNetClient clt;
			std::cout << "开始连接." << std::endl;
			clt.ConnectSocket(SYS_UTL::NET::TRANS_PROTOCOL_TYPE_UDP, "127.0.0.1", 9898);
			clt.SetSktNoBlock();
			if (!clt.IsError() && clt.IsValidSkt()) {
				std::cout << "链接成功." << std::endl;
				std::string str;
				char buff[1024];
				int len = 0, iAddrLen = sizeof(sockaddr);
				sockaddr stUdpSrc;

				while (true)
				{
					str = "";
					std::cout << "输入:" << std::endl;
					std::cin >> str;

					if (str.length()) {
						len = clt.Write((const char*)str.data(), str.length(), 2000);
						std::cout << "w:" << len << std::endl;
					}

					len = clt.ReadFromUDP(buff, 1024, (struct sockaddr&)stUdpSrc, iAddrLen, 2000);
					if (len > 0) {
						buff[len] = '\0';
						std::cout << "客户端接收到数据:" << buff << std::endl;
					}
				}
			}
			else {
				std::cout << "链接失败." << std::endl;
			}
		});

		while (true)
		{
			buff[0] = '\0';

			int iDataLen = cNetServer.ReadFromUDP(buff, 1024, (struct sockaddr&)stUdpSrc, len, 2000);
			if (iDataLen > 0 && iDataLen < 1024) {

				buff[iDataLen] = '\0';
				strncat_s(buff, sizeof(buff), " Server ok", strlen(" Server ok"));
				cNetServer.WriteToUDP(buff, strlen(buff), (sockaddr&)stUdpSrc, len);
			}
			else{
				//std::cout << "UDP Server Not Read Data." << std::endl;
			}
		}
	}
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
		* 条件变量测试
		*/
		std::cout << "条件变量测试" << std::endl;
		SYS_UTL::CThreadBox tb[5];
		CCond cd;
		for (int i = 0; i < 5; i++)
		{
			tb[i].Start([&](BOOL& bRun, HANDLE hWait, void* context){
				bool bRet = false;
				while (!bRet)
				{
					bRet = cd.TryWait(1000);
					std::cout << "线程: " << GetCurrentThreadId() << "等待条件变量: " << bRet << std::endl;
				}
			});
		}
		std::cin.ignore();
		std::cout << "触发单个条件变量..." << std::endl;
		cd.Signal();
		std::cin.ignore();
		std::cout << "触发单个条件变量..." << std::endl;
		cd.Signal();
		std::cin.ignore();
		std::cout << "触发所有条件变量..." << std::endl;
		cd.Broadcast();
		std::cin.ignore();
	}

	std::cin.ignore();
	{
		/**
		* 条件变量测试
		*/
		std::cout << "条件变量测试" << std::endl;
		SYS_UTL::CThreadBox tb1, tb2;
		CCond cd;
		tb1.Start([&](BOOL& bRun, HANDLE hWait, void* context){
			Sleep(5000);
			std::cout << "触发条件变量..." << std::endl;
			cd.Signal();
		});
		tb2.Start([&](BOOL& bRun, HANDLE hWait, void* context){
			bool ret = cd.TryWait(3000);
			// 这里会超时
			std::cout << "线程,条件变量等待...结束:" << ret << std::endl;
		});
		std::cout << "条件变量等待..." << std::endl;
		bool ret = cd.Wait();
		std::cout << "条件变量等待...结束:" << ret << std::endl;
	}
	std::cin.ignore();
	{
		/**
		* 线程池测试
		*/
		std::cout << "线程池测试" << std::endl;
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

