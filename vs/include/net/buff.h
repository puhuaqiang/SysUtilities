#ifndef __SYS_UTILITIES_NET_BUFF_H__
#define __SYS_UTILITIES_NET_BUFF_H__

#include "../SysUtilities.h"
#include "../CritSec.h"


/**< ��󻺴�����С.*/
#define BUFFER_MAX_LEN 1024*1024*1024

namespace SYS_UTL{namespace NET{
/**
* @brief ����(ѭ��)������(CRingBuffer) class.
*/
class SYS_UTL_CPPAPI CRingBuffer
{
public:
	CRingBuffer();
	/*
	* @brief ���캯��..
	* 	�ͻ��˴����׽���ʹ��.�ڲ��ᴴ��\�����׽���
	* @param uiCapacity �����С
	*/
	CRingBuffer(size_t uiCapacity);
	virtual ~CRingBuffer();

	void Init(size_t uiCapacity);
	void UnInit();
	/*
	* @brief ����/ֹͣ�ڲ���..
	* 	ע��: Ĭ�������ڲ���.
	* \param uiCapacity �����С
	*/
	void SetInternalLock(bool enable) { _UseInternalLock = enable; }

	/*
	*  @brief �жϻ�����Ϊ��..
	*  \return [bool]
	*/
	bool IsEmpty() const { return _in == _out; }

	/*
	* \return [bool]
	*/
	bool IsFull() const { return _size == (_in - _out); }

	/*
	@brief ��ȡ��������ʼλ��..
	\return [ptr]
	*/
	const char* GetBuffer() const { return _buffer; }

	/*
	@brief ��ȡ������������ʼλ��..
	\return [ptr]
	*/
	const char* GetDataStart() const { return _buffer + (_out & (_size - 1))/*_buffer + _out*/; }

	/*
	@brief ��ȡ���������ݽ���λ��..
	\return [ptr]
	*/
	const char* GetDataEnd() const { return _buffer + (_in & (_size - 1))/*_buffer + _in*/; }

	/*
	@brief �ж�д��ָ����С�������Ƿ��������������Ļ�����(��ת����ʼ��)..
	\return [ptr]
	*/
	bool IsContinuousBuff(size_t size) const { return size <= _size - (_in & (_size - 1)); }

	/*
	@brief ��ȡ��������С..
	\return [size_t]
	*/
	size_t Capacity() const { return _size; }

	/*
	@brief ��ȡ���������ݴ�С..
	\return [size_t]
	*/
	size_t Size();

	/*
	@brief ��ȡ���������ÿռ��С..
	\return [size_t]
	*/
	size_t Space();

	/*
	@brief ��ȡ���������ÿռ��С(�����������Ļ�����)..
	\return [size_t]
	*/
	size_t SpaceEx();

	/*
	@brief ��λ����..
	\return [size_t]
	*/
	void Clear();

	/*
	@brief �޸Ļ��������ݴ�С.
	\param size
		��������������ָ����С�ֽ�����
	\return [void]
	*/
	void Write(size_t size);

	/*
	@brief �򻺴���д������.
	\param data
		���ݻ�����
	\param bytesWriteOfNumber
		��д�����ݴ�С.
	\return [size_t]
		����д�����ݴ�С
	*/
	size_t Write(const char *data, size_t bytesWriteOfNumber);

	/*
	* @brief �ӻ�������ȡָ����С����.
	* @param buff ���ݽ��ջ�����
	* @param bytesReadOfNumber ����ȡ���ݴ�С.
	* @return [size_t] ���ض�ȡ�����ݴ�С
	*/
	size_t Read(char* buff, size_t bytesReadOfNumber);
	/*
	* @brief �ӻ�������ȡָ����С����.
	* @param buff ָ�����ݻ�����������ָ��.
	* @param bytesReadOfNumber ����ȡ���ݴ�С.
	* @return [size_t] ���ض�ȡ�����ݴ�С
	*/
	size_t ReadEx(char*& buff, size_t bytesReadOfNumber);

private:
	/**< �������ݴ�С. */
	size_t _len() const { return (_in - _out); }
	/**< ���ÿռ��С. */
	size_t _space() const { return (_size - _in + _out); }
	/**< д������. */
	size_t _write(const char *data, size_t bytes);
	/**< ��ȡ����. */
	size_t _read(char *data, size_t bytes);
	size_t _read2(char*& buff, size_t bytesReadOfNumber);
	/**< ��ʼ����. */
	void _InitLock();
	void _UnInitLock();

	CRingBuffer(CRingBuffer const &) = delete;
	CRingBuffer(CRingBuffer &&) = delete;
	CRingBuffer& operator= (CRingBuffer const &) = delete;
	CRingBuffer& operator= (CRingBuffer &&) = delete;
protected:
	/**< ������*/
	char* _buffer;
	/**< ��������С*/
	//uint32_t _size;
	size_t _size;
	/**< �������д��λ��.*/
	//uint32_t _in;
	size_t _in;
	/**< ���ݶ�ȡ�����λ��.*/
	//uint32_t _out;
	size_t _out;
	/**< ��ʱ������.*/
	struct {
		char* _buffer;
		//uint32_t _size;
		size_t _size;
	}_temp_buffer;
	SYS_UTL::CCritSec _lock;
	/**< ����/ͣ���ڲ�����ʶ.*/
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
	@brief ����/ֹͣ�ڲ���..
		ע��: Ĭ�������ڲ���.
	\param uiCapacity �����С
	*/
	void SetInternalLock(bool enable) { m_UseInternalLock = enable; }

	/*
	@brief ��ʼ����������С.
	\return 0�ɹ�,����ʧ��
	*/
	int Init(int iBlockSize);
	/*
	@brief �����ڴ��.
	\return 0�ɹ�,����ʧ��
	*/
	int UnInit();
	/*
	@brief ����ڴ��.
	\return 0�ɹ�,����ʧ��
	*/
	int Clear();
	/*
	@brief ���ڴ��д������.
	\param pDataBuff ���ݻ�����
	\param iDataLen ���ݳ���
	\param pBlockPostion ����д��λ��,�ɸ��ݸ�д���ȡ���ݻ������ݶ�ȡ�ڴ��ַ
	\return 0�ɹ�,����ʧ��
	*/
	int Write(char* pDataBuff, int iDataLen, int* pBlockPostion);
	/*
	@brief �����ڴ������д��λ�ö�ȡ����.
	\param iBlockPostion ����ƫ��λ��
	\param pDataBuff ���ݽ��ջ�����
	\param iDataLen ���ݽ��ճ���
	\return 0�ɹ�,����ʧ��
	*/
	int Read(int iBlockPostion, char* pDataBuff, int iDataLen);
	/*
	@brief �����ڴ������д��λ�û������ָ��.
	\param iBlockPostion ����ƫ��λ��
	\param pBlockAddr ���ݽ���ָ��
	\return 0�ɹ�,����ʧ��
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
	/**< ��ʼ����. */
	void _InitLock();
	void _UnInitLock();
private:
	bool m_bInit;
	/**< �ڴ���ܴ�С.*/
	int m_iBlockSize;
	/**< �ڴ����ַ.*/
	char* m_pBasePointer;
	/**< ��ǰд��λ�ã�����һ������д��λ�ã���д�������ݺ����ı�.*/
	int m_iCurrentPosition;
	/**< ����/ͣ���ڲ�����ʶ.*/
	bool m_UseInternalLock;
	SYS_UTL::CCritSec m_lock;	  //������
};
}}

#endif	//__AUTOLOCK_H__
