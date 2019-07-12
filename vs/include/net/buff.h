#ifndef __SYS_UTILITIES_NET_BUFF_H__
#define __SYS_UTILITIES_NET_BUFF_H__

#include "../SysUtilities.h"
#include "../CritSec.h"


/**< 最大缓存区大小.*/
#define BUFFER_MAX_LEN 1024*1024*1024

namespace SYS_UTL{namespace NET{
/**
* @brief 环形(循环)缓存区(CRingBuffer) class.
*/
class SYS_UTL_CPPAPI CRingBuffer
{
public:
	CRingBuffer();
	/*
	* @brief 构造函数..
	* 	客户端创建套接字使用.内部会创建\连接套接字
	* @param uiCapacity 缓存大小
	*/
	CRingBuffer(size_t uiCapacity);
	virtual ~CRingBuffer();

	void Init(size_t uiCapacity);
	void UnInit();
	/*
	* @brief 启用/停止内部锁..
	* 	注意: 默认启用内部锁.
	* \param uiCapacity 缓存大小
	*/
	void SetInternalLock(bool enable) { _UseInternalLock = enable; }

	/*
	*  @brief 判断缓冲区为空..
	*  \return [bool]
	*/
	bool IsEmpty() const { return _in == _out; }

	/*
	* \return [bool]
	*/
	bool IsFull() const { return _size == (_in - _out); }

	/*
	@brief 获取缓存区起始位置..
	\return [ptr]
	*/
	const char* GetBuffer() const { return _buffer; }

	/*
	@brief 获取缓存区数据起始位置..
	\return [ptr]
	*/
	const char* GetDataStart() const { return _buffer + (_out & (_size - 1))/*_buffer + _out*/; }

	/*
	@brief 获取缓存区数据结束位置..
	\return [ptr]
	*/
	const char* GetDataEnd() const { return _buffer + (_in & (_size - 1))/*_buffer + _in*/; }

	/*
	@brief 判断写入指定大小的数据是否是物理上连续的缓存区(跳转到开始处)..
	\return [ptr]
	*/
	bool IsContinuousBuff(size_t size) const { return size <= _size - (_in & (_size - 1)); }

	/*
	@brief 获取缓存区大小..
	\return [size_t]
	*/
	size_t Capacity() const { return _size; }

	/*
	@brief 获取缓存区数据大小..
	\return [size_t]
	*/
	size_t Size();

	/*
	@brief 获取缓存区可用空间大小..
	\return [size_t]
	*/
	size_t Space();

	/*
	@brief 获取缓存区可用空间大小(物理上连续的缓存区)..
	\return [size_t]
	*/
	size_t SpaceEx();

	/*
	@brief 复位数据..
	\return [size_t]
	*/
	void Clear();

	/*
	@brief 修改缓存区数据大小.
	\param size
		缓存区数据新增指定大小字节数据
	\return [void]
	*/
	void Write(size_t size);

	/*
	@brief 向缓存区写入数据.
	\param data
		数据缓存区
	\param bytesWriteOfNumber
		待写入数据大小.
	\return [size_t]
		返回写入数据大小
	*/
	size_t Write(const char *data, size_t bytesWriteOfNumber);

	/*
	* @brief 从缓存区读取指定大小数据.
	* @param buff 数据接收缓存区
	* @param bytesReadOfNumber 待读取数据大小.
	* @return [size_t] 返回读取的数据大小
	*/
	size_t Read(char* buff, size_t bytesReadOfNumber);
	/*
	* @brief 从缓存区读取指定大小数据.
	* @param buff 指向数据缓存区的数据指针.
	* @param bytesReadOfNumber 待读取数据大小.
	* @return [size_t] 返回读取的数据大小
	*/
	size_t ReadEx(char*& buff, size_t bytesReadOfNumber);

private:
	/**< 可用数据大小. */
	size_t _len() const { return (_in - _out); }
	/**< 可用空间大小. */
	size_t _space() const { return (_size - _in + _out); }
	/**< 写入数据. */
	size_t _write(const char *data, size_t bytes);
	/**< 读取数据. */
	size_t _read(char *data, size_t bytes);
	size_t _read2(char*& buff, size_t bytesReadOfNumber);
	/**< 初始化锁. */
	void _InitLock();
	void _UnInitLock();

	CRingBuffer(CRingBuffer const &) = delete;
	CRingBuffer(CRingBuffer &&) = delete;
	CRingBuffer& operator= (CRingBuffer const &) = delete;
	CRingBuffer& operator= (CRingBuffer &&) = delete;
protected:
	/**< 缓存区*/
	char* _buffer;
	/**< 缓存区大小*/
	//uint32_t _size;
	size_t _size;
	/**< 数据最后写入位置.*/
	//uint32_t _in;
	size_t _in;
	/**< 数据读取的最后位置.*/
	//uint32_t _out;
	size_t _out;
	/**< 临时缓存区.*/
	struct {
		char* _buffer;
		//uint32_t _size;
		size_t _size;
	}_temp_buffer;
	SYS_UTL::CCritSec _lock;
	/**< 启用/停用内部锁标识.*/
	bool _UseInternalLock;
	bool _init;
	char _buff[1024 * 16];
};

class SYS_UTL_CPPAPI CBoundBuffer
{
public:
	CBoundBuffer();
	~CBoundBuffer();
	/*
	@brief 启用/停止内部锁..
		注意: 默认启用内部锁.
	\param uiCapacity 缓存大小
	*/
	void SetInternalLock(bool enable) { m_UseInternalLock = enable; }

	/*
	@brief 初始化缓存区大小.
	\return 0成功,否则失败
	*/
	int Init(int iBlockSize);
	/*
	@brief 销毁内存块.
	\return 0成功,否则失败
	*/
	int UnInit();
	/*
	@brief 清空内存块.
	\return 0成功,否则失败
	*/
	int Clear();
	/*
	@brief 向内存块写入数据.
	\param pDataBuff 数据缓存区
	\param iDataLen 数据长度
	\param pBlockPostion 数据写入位置,可根据该写入读取数据或获得数据读取内存地址
	\return 0成功,否则失败
	*/
	int Write(char* pDataBuff, int iDataLen, int* pBlockPostion);
	/*
	@brief 根据内存块数据写入位置读取数据.
	\param iBlockPostion 数据偏移位置
	\param pDataBuff 数据接收缓存区
	\param iDataLen 数据接收长度
	\return 0成功,否则失败
	*/
	int Read(int iBlockPostion, char* pDataBuff, int iDataLen);
	/*
	@brief 根据内存快数据写入位置获得数据指针.
	\param iBlockPostion 数据偏移位置
	\param pBlockAddr 数据接收指针
	\return 0成功,否则失败
	*/
	int GetAddr(int iBlockPostion, char*& pBlockAddr);

private:
	int _Init(int iBlockSize);
	int _UnInit();
	int _Clear();
	int _Write(char* pDataBuff, int iDataLen, int* pBlockPostion);
	int _Read(int iBlockPostion, char* pDataBuff, int iDataLen);
	int _GetAddr(int iBlockPostion, char*& pBlockAddr);
private:
	/**< 初始化锁. */
	void _InitLock();
	void _UnInitLock();
private:
	bool m_bInit;
	/**< 内存块总大小.*/
	int m_iBlockSize;
	/**< 内存基地址.*/
	char* m_pBasePointer;
	/**< 当前写入位置，即下一次数据写入位置，当写入新数据后发生改变.*/
	int m_iCurrentPosition;
	/**< 启用/停用内部锁标识.*/
	bool m_UseInternalLock;
	SYS_UTL::CCritSec m_lock;	  //互斥锁
};
}}

#endif	//__AUTOLOCK_H__
