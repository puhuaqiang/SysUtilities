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
	* \brief 线程盒封装类.
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
		* \brief 启动线程.
		* \param lpProcCallBack 任务处理回调函数
		* \param lpUsr 自定义上下文
		*/
		int Start(int iPacketSize, SYS_UTL::THREAD_POOL_PROCESS lpProcCallBack, void* lpUsr);
		int Start(std::function<void(BOOL& bRun, HANDLE hWait, void* context)>, const char* lpszName = "");
		/**
		* \brief 停止线程
		*/
		void Stop();

		/**
		* \brief 添加任务
		* \param lpTask 任务数据缓冲区, 不能为 NULL, 否则返回参数错误
		* \param iTaskDataLen 任务数据大小
		*/
		int Task(void* lpTask, int iTaskDataLen);
		/**
		* \brief 读取空闲时间(持续多少毫秒)
		*/
		int GetIdleTime();
		/**
		* \brief 是否发生错误
		*/
		bool IsError();
		/**
		* \brief 是否空闲状态
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
		/**工作线程*/
		SYS_UTL::CThreadBox m_Thread;
		/**空闲时间戳*/
		unsigned int m_uiIdleTimestamp;
		/**锁*/
		SYS_UTL::CCritSec m_lock;
		/**读取任务缓冲区*/
		struct  {
			int iBufferLen;
			int iDataLen;
			BYTE buff[1024 * 8];
			BYTE* lpBuff;
		}m_Task;
		/**内部发生错误*/
		int m_iError;
		/***/
		struct  {
			SYS_UTL::THREAD_POOL_PROCESS cb;
			void* lpUsr;
		}m_TaskCallBack;
		std::function<void(BOOL& bRun, HANDLE hWait, void* context)> m_cb;
	};

	/**
	* \brief 线程池封装类.
	*/
	class SYS_UTL_CPPAPI CThreadPool
	{
		CThreadPool(const CThreadPool &ref);
		CThreadPool &operator=(const CThreadPool &ref);
	public:
		CThreadPool();
		virtual ~CThreadPool();

		/**
		* \brief 资源初始化.
		* \param iThreadCnt 线程池线程个数上限
		* \param iTaskBufferLen 任务缓冲区总大小(字节),小于等于0x0FFFFFFF
		* \param iSingleTaskMaxLen 单个任务缓冲区最大长度(字节), 小于等于iTaskBufferLen
		* \param iMaxIdleTime 线程最大空闲时间(毫秒),达到最大空闲时间,线程会退出. =0生命周期内都存在
		*/
		int Init(int iThreadCnt, int iTaskBufferLen, int iSingleTaskMaxLen, int iMaxIdleTime);
		/**
		* \brief 停止线程池
		*/
		void UnInit();

		/**
		* \brief 启动线程池.
		*	内部优先使用 BOX_THREAD_PROCESS 回调,再次 function回调,
		* \param lpProcCallBack 任务处理回调函数
		* \param lpUsr 自定义上下文
		*/
		int Start(SYS_UTL::THREAD_POOL_PROCESS lpProcCallBack, void* lpUsr);
		int Start(std::function<void(void* lpTask, int iTaskDataLen)>);
		/**
		* \brief 停止线程池
		*/
		void Stop();

		/**
		* \brief 添加任务
		* \param lpTask 任务数据缓冲区, 不能为 NULL, 否则返回参数错误
		* \param iTaskDataLen 任务数据大小
		* \return 0成功 ,否则失败
		*/
		int Task(void* lpTask, int iTaskDataLen);

	private:

		/**控制线程处理函数*/
		static void __CtrlThreadProc(BOOL& bRun, HANDLE hWait, void* context);
		void __CtrlThreadProcLoop(BOOL& bRun, HANDLE hWait);
		/****/
		static void __ThreadCallBack(void* lpTask, int iTaskDataLen, void* lpUsr);
		void __ThreadCallBackProc(void* lpTask, int iTaskDataLen);

		/**
		* \brief 得到一个线程实例
		*/
		SYS_UTL::CThreadWrapper* __GetThread();

		/**
		* \brief 检测处理线程
		*	超过最大的空闲时间,释放线程
		*/
		void __CheckProcThread();

		/***/
		BOOL __IsInit() const { return m_bInit; }
		/***/
		BOOL __IsStartUp() const { return m_bStartUp; }

	private:
		/**线程池控制线程*/
		SYS_UTL::CThreadBox m_CtrlThread;
		/**工作线程集合*/
		std::list<SYS_UTL::CThreadWrapper*> m_lstThreads;
		/**是否初始化*/
		BOOL m_bInit;
		/**是否启动服务*/
		BOOL m_bStartUp;
		/**线程最大空闲时间(毫秒)*/
		int m_iMaxIdleTime;
		/**线程池最大线程数,上线256*/
		int m_iMaxThreadCnt;
		int m_iSingleTaskMaxLen;
		SYS_UTL::CCritSec m_lock;
		/**任务处理回调函数*/
		struct {
			SYS_UTL::THREAD_POOL_PROCESS lpPorc;
			void* lpUsr;
		}m_TaskCallBack;
		std::function<void(void* lpTask, int iTaskDataLen)> m_cb;
		/**读取任务缓冲区*/
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