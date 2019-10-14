#ifndef __SYS_UTILITIES_THREADPOOL_H__
#define __SYS_UTILITIES_THREADPOOL_H__

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include "ThreadBox.h"

namespace SYS_UTL
{
	typedef void(*THREAD_POOL_PROCESS)(void* lpTask, int iTaskDataLen, void* lpUsr);

	/**
	* \brief �̺߳з�װ��.
	*/
	class CThreadPool;
	class SYS_UTL_CPPAPI CThreadWrapper
	{
		friend class CThreadPool;
	private:
		CThreadWrapper();
		~CThreadWrapper();

	public:
		/**
		* \brief �����߳�.
		* \param lpProcCallBack ������ص�����
		* \param lpUsr �Զ���������
		*/
		int Start(int iPacketSize, SYS_UTL::THREAD_POOL_PROCESS lpProcCallBack, void* lpUsr);
		int Start(std::function<void(BOOL& bRun, HANDLE hWait, void* context)>, const char* lpszName = "");
		/**
		* \brief ֹͣ�߳�
		*/
		void Stop();

		/**
		* \brief �������
		* \param lpTask �������ݻ�����, ����Ϊ NULL, ���򷵻ز�������
		* \param iTaskDataLen �������ݴ�С
		*/
		int Task(void* lpTask, int iTaskDataLen);
		/**
		* \brief ��ȡ����ʱ��(�������ٺ���)
		*/
		int GetIdleTime();
		/**
		* \brief �Ƿ�������
		*/
		bool IsError();
		/**
		* \brief �Ƿ����״̬
		*/
		bool IsIdle();
		operator bool(){ return !IsError(); }
	private:
		/**
		* \brief
		*/
		void __UpdateIdleTime(unsigned int);
		/**
		* \brief
		*/
		void __UpdateError(int);
	private:
		/**�����߳�*/
		SYS_UTL::CThreadBox m_Thread;
		/**����ʱ���*/
		unsigned int m_uiIdleTimestamp;
		/**��*/
		SYS_UTL::CCritSec m_lock;
		/**��ȡ���񻺳���*/
		struct  {
			int iBufferLen;
			int iDataLen;
			BYTE buff[1024 * 8];
			BYTE* lpBuff;
		}m_Task;
		/**�ڲ���������*/
		int m_iError;
		/***/
		struct  {
			SYS_UTL::THREAD_POOL_PROCESS cb;
			void* lpUsr;
		}m_TaskCallBack;
		std::function<void(BOOL& bRun, HANDLE hWait, void* context)> m_cb;
	};

	/**
	* \brief �̳߳ط�װ��.
	*/
	class SYS_UTL_CPPAPI CThreadPool
	{
		CThreadPool(const CThreadPool &ref);
		CThreadPool &operator=(const CThreadPool &ref);
	public:
		CThreadPool();
		virtual ~CThreadPool();

		/**
		* \brief ��Դ��ʼ��.
		* \param iThreadCnt �̳߳��̸߳�������
		* \param iTaskBufferLen ���񻺳����ܴ�С(�ֽ�),С�ڵ���0x0FFFFFFF
		* \param iSingleTaskMaxLen �������񻺳�����󳤶�(�ֽ�), С�ڵ���iTaskBufferLen
		* \param iMaxIdleTime �߳�������ʱ��(����),�ﵽ������ʱ��,�̻߳��˳�. =0���������ڶ�����
		*/
		int Init(int iThreadCnt, int iTaskBufferLen, int iSingleTaskMaxLen, int iMaxIdleTime);
		/**
		* \brief ֹͣ�̳߳�
		*/
		void UnInit();

		/**
		* \brief �����̳߳�.
		*	�ڲ�����ʹ�� BOX_THREAD_PROCESS �ص�,�ٴ� function�ص�,
		* \param lpProcCallBack ������ص�����
		* \param lpUsr �Զ���������
		*/
		int Start(SYS_UTL::THREAD_POOL_PROCESS lpProcCallBack, void* lpUsr);
		int Start(std::function<void(void* lpTask, int iTaskDataLen)>);
		/**
		* \brief ֹͣ�̳߳�
		*/
		void Stop();

		/**
		* \brief �������
		* \param lpTask �������ݻ�����, ����Ϊ NULL, ���򷵻ز�������
		* \param iTaskDataLen �������ݴ�С
		* \return 0�ɹ� ,����ʧ��
		*/
		int Task(void* lpTask, int iTaskDataLen);

	private:

		/**�����̴߳�����*/
		static void __CtrlThreadProc(BOOL& bRun, HANDLE hWait, void* context);
		void __CtrlThreadProcLoop(BOOL& bRun, HANDLE hWait);
		/****/
		static void __ThreadCallBack(void* lpTask, int iTaskDataLen, void* lpUsr);
		void __ThreadCallBackProc(void* lpTask, int iTaskDataLen);

		/**
		* \brief �õ�һ���߳�ʵ��
		*/
		SYS_UTL::CThreadWrapper* __GetThread();

		/**
		* \brief ��⴦���߳�
		*	�������Ŀ���ʱ��,�ͷ��߳�
		*/
		void __CheckProcThread();

		/***/
		BOOL __IsInit() const { return m_bInit; }
		/***/
		BOOL __IsStartUp() const { return m_bStartUp; }

	private:
		/**�̳߳ؿ����߳�*/
		SYS_UTL::CThreadBox m_CtrlThread;
		/**�����̼߳���*/
		std::list<SYS_UTL::CThreadWrapper*> m_lstThreads;
		/**�Ƿ��ʼ��*/
		BOOL m_bInit;
		/**�Ƿ���������*/
		BOOL m_bStartUp;
		/**�߳�������ʱ��(����)*/
		int m_iMaxIdleTime;
		/**�̳߳�����߳���,����256*/
		int m_iMaxThreadCnt;
		int m_iSingleTaskMaxLen;
		SYS_UTL::CCritSec m_lock;
		/**������ص�����*/
		struct {
			SYS_UTL::THREAD_POOL_PROCESS lpPorc;
			void* lpUsr;
		}m_TaskCallBack;
		std::function<void(void* lpTask, int iTaskDataLen)> m_cb;
		/**��ȡ���񻺳���*/
		struct  {
			int iBufferLen;
			int iDataLen;
			BYTE buff[1024 * 8];
			BYTE* lpBuff;
		}m_Task;
	};
}


namespace SYS_UTL
{
/**
* @brief A simple C++11 Thread Pool implementation.
*	@Author https://github.com/progschj/ThreadPool
*/
class SYS_UTL_CPPAPI ThreadPool {
public:
	ThreadPool(size_t);
	template<class F, class... Args>
	auto enqueue(F&& f, Args&&... args)
		->std::future<typename std::result_of<F(Args...)>::type>;
	~ThreadPool();
private:
	// need to keep track of threads so we can join them
	std::vector< std::thread > workers;
	// the task queue
	std::queue< std::function<void()> > tasks;

	// synchronization
	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
	: stop(false)
{
	for (size_t i = 0; i<threads; ++i)
		workers.emplace_back(
		[this]
	{
		for (;;)
		{
			std::function<void()> task;

			{
				std::unique_lock<std::mutex> lock(this->queue_mutex);
				this->condition.wait(lock,
					[this]{ return this->stop || !this->tasks.empty(); });
				if (this->stop && this->tasks.empty())
					return;
				task = std::move(this->tasks.front());
				this->tasks.pop();
			}

			task();
		}
	}
	);
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
	-> std::future<typename std::result_of<F(Args...)>::type>
{
	using return_type = typename std::result_of<F(Args...)>::type;

	auto task = std::make_shared< std::packaged_task<return_type()> >(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(queue_mutex);

		// don't allow enqueueing after stopping the pool
		if (stop)
			throw std::runtime_error("enqueue on stopped ThreadPool");

		tasks.emplace([task](){ (*task)(); });
	}
	condition.notify_one();
	return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (std::thread &worker : workers)
		worker.join();
}
}
#endif