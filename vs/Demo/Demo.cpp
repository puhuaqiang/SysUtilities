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
	std::cout << "�߳�ID: " << GetCurrentThreadId() << " Value:" << lpInfo->id << std::endl;
	Sleep(Random(500, 10000));
	std::cout << "�߳�ID: " << GetCurrentThreadId() << " Value:" << lpInfo->id << " END" << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
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
			std::cout << "��ʼ����." << std::endl;
			clt.ConnectSocket(SYS_UTL::NET::TRANS_PROTOCOL_TYPE_TCP, "127.0.0.1", 9898);
			clt.SetSktNoBlock();
			if (!clt.IsError() && clt.IsValidSkt()) {
				std::cout << "���ӳɹ�." << std::endl;
				std::string str;
				char buff[1024];
				int len = 0;

				while (true)
				{
					str = "";
					std::cout << "����:" << std::endl;
					std::cin >> str;

					if (str.length()) {
						len = clt.Write((const char*)str.data(), str.length(), 2000);
						std::cout << "w:" << len << std::endl;
					}

					len = clt.Read(buff, 1024, 0, 2000);
					if (len > 0) {
						buff[len] = '\0';
						std::cout << "�ͻ��˽��յ�����:" << buff << std::endl;
					}
				}
			}
			else {
				std::cout << "����ʧ��." << std::endl;
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

						strncat_s(szData, _TRUNCATE, "��SERVER", _TRUNCATE);
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
			std::cout << "��ʼ����." << std::endl;
			clt.ConnectSocket(SYS_UTL::NET::TRANS_PROTOCOL_TYPE_UDP, "127.0.0.1", 9898);
			clt.SetSktNoBlock();
			if (!clt.IsError() && clt.IsValidSkt()) {
				std::cout << "���ӳɹ�." << std::endl;
				std::string str;
				char buff[1024];
				int len = 0, iAddrLen = sizeof(sockaddr);
				sockaddr stUdpSrc;

				while (true)
				{
					str = "";
					std::cout << "����:" << std::endl;
					std::cin >> str;

					if (str.length()) {
						len = clt.Write((const char*)str.data(), str.length(), 2000);
						std::cout << "w:" << len << std::endl;
					}

					len = clt.ReadFromUDP(buff, 1024, (struct sockaddr&)stUdpSrc, iAddrLen, 2000);
					if (len > 0) {
						buff[len] = '\0';
						std::cout << "�ͻ��˽��յ�����:" << buff << std::endl;
					}
				}
			}
			else {
				std::cout << "����ʧ��." << std::endl;
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

