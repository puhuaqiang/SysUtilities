#include "../../SysUtilities/stdafx.h"
#include "../../include/net/buff.h"
#include <atomic>
#include <memory>
#include "../../include/api.h"
#include "../../include/AutoLock.h"
//#include <algorithm> // for std::min
//判断x是否是2的次方
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x)-1)) == 0))

namespace SYS_UTL{namespace NET{
CRingBuffer::CRingBuffer()
	: _size(0)
	, _in(0)
	, _out(0)
	, _init(false)
	, _UseInternalLock(true)
{
	_temp_buffer._buffer = nullptr;
	_temp_buffer._size = 0;
	_InitLock();
}
CRingBuffer::CRingBuffer(size_t uiCapacity)
	: _size(0)
	, _in(0)
	, _out(0)
	, _init(false)
	, _UseInternalLock(true)
{
	_temp_buffer._buffer = nullptr;
	_temp_buffer._size = 0;
	_InitLock();
	Init(uiCapacity);
}

CRingBuffer::~CRingBuffer()
{
	UnInit();
	_UnInitLock();
}

void CRingBuffer::_InitLock()
{
}

void CRingBuffer::_UnInitLock()
{
}

void CRingBuffer::Init(size_t uiCapacity)
{
	if (_init) {
		return;
	}
	_size = uiCapacity;
	if (_size <= 0) {
		return;
	}
	if (_size > 1024 * 16) {
		if (!is_power_of_2(_size)){
			return;
		}
		_buffer = new char[_size];
		if (nullptr == _buffer) {
			return;
		}
	}
	else {
		_buffer = _buff;
	}
	_init = true;
}

void CRingBuffer::UnInit() {
	if (!_init) {
		return;
	}
	if ((_size > 1024 * 16) && _buffer != nullptr) {
		delete[] _buffer;
		_buffer = nullptr;
	}
	_in = 0;
	_out = 0;
	_size = 0;
	_init = false;
}

size_t CRingBuffer::Size() {
	if (!_init) {
		return 0;
	}
	if (_UseInternalLock){
		uint32_t len = 0;
		SYS_UTL::CAutoLock lck(&_lock);
		len = _len();
		return len;
	}
	else{
		return _len();
	}
}

size_t CRingBuffer::Space() {
	if (!_init) {
		return 0;
	}
	if (_UseInternalLock){
		uint32_t len = 0;
		SYS_UTL::CAutoLock lck(&_lock);
		len = _space();
		return len;
	}
	else{
		return _space();
	}
}

size_t CRingBuffer::SpaceEx()
{
	if (!_init) {
		return 0;
	}
	size_t len = 0;
	if (_UseInternalLock){
		SYS_UTL::CAutoLock lck(&_lock);
		len = _space();
		len = /*std::min*/min(len, _size - (_in & (_size - 1)));
	}else{
		len = _space();
		len = min(len, _size - (_in & (_size - 1)));
	}
	return len;
}

void CRingBuffer::Clear()
{
	if (_UseInternalLock){
		SYS_UTL::CAutoLock lck(&_lock);
		_in = _out = 0;
	}
	else{
		_in = _out = 0;
	}
}

void CRingBuffer::Write(size_t size) {
	if (!_init) {
		return;
	}
	if (_UseInternalLock){
		SYS_UTL::CAutoLock lck(&_lock);
		_in += size;
	}
	else{
		_in += size;
	}
}
size_t CRingBuffer::Write(const char *data, size_t bytesWriteOfNumber)
{
	if (!_init) {
		return 0;
	}
	size_t ret;
	if (_UseInternalLock){
		SYS_UTL::CAutoLock lck(&_lock);
		ret = _write(data, bytesWriteOfNumber);
	}
	else{
		ret = _write(data, bytesWriteOfNumber);
	}
	return ret;
}

size_t CRingBuffer::Read(char *data, size_t bytesReadOfNumber)
{
	if (!_init) {
		return 0;
	}
	size_t ret;
	if (_UseInternalLock){
		SYS_UTL::CAutoLock lck(&_lock);
		ret = _read(data, bytesReadOfNumber);
		//buffer中没有数据
		if (_in == _out)
			_in = _out = 0;
	}else{
		ret = _read(data, bytesReadOfNumber);
		//buffer中没有数据
		if (_in == _out)
			_in = _out = 0;
	}
	return ret;
}

size_t CRingBuffer::ReadEx(char*& buff, size_t bytesReadOfNumber)
{
	if (!_init) {
		return 0;
	}
	size_t ret;
	if (_UseInternalLock){
		SYS_UTL::CAutoLock lck(&_lock);
		ret = _read2(buff, bytesReadOfNumber);
		//buffer中没有数据
		if (_in == _out)
			_in = _out = 0;
	}
	else{
		ret = _read2(buff, bytesReadOfNumber);
		//buffer中没有数据
		if (_in == _out)
			_in = _out = 0;
	}
	return ret;
}

size_t CRingBuffer::_read(char *data, size_t size)
{
	assert(data);
	size_t len = 0;
	size_t size_ = min(size, _len());
	/* first get the data from fifo->out until the end of the buffer */
	len = min(size_, _size - (_out & (_size - 1)));
	memcpy(data, _buffer + (_out & (_size - 1)), len);
	/* then get the rest (if any) from the beginning of the buffer */
	memcpy(data + len, _buffer, size_ - len);
	_out += size_;
	return size_;
}

size_t CRingBuffer::_read2(char*& buff, size_t bytesReadOfNumber)
{
	size_t len = 0, ret = 0;
	size_t size_ = min(bytesReadOfNumber, _len());
	ret = size_;
	/* first get the data from fifo->out until the end of the buffer */
	len = min(size_, _size - (_out & (_size - 1)));
	buff = _buffer + (_out & (_size - 1));
	/* then get the rest (if any) from the beginning of the buffer */
	if (size_ - len > 0){
		ret = 0;
		if (size_ > _temp_buffer._size){
			if (_temp_buffer._size > 0 && (_temp_buffer._buffer != nullptr)){
				delete[] _temp_buffer._buffer;
				_temp_buffer._buffer = nullptr;
			}
			_temp_buffer._size = PAD_SIZE(size_);
			_temp_buffer._buffer = new char[_temp_buffer._size];

		}
		if (size_ <= _temp_buffer._size){
			if (_temp_buffer._buffer != nullptr){
				//DBG_I(".new char.");
				memcpy(_temp_buffer._buffer, buff, len);
				memcpy(_temp_buffer._buffer + len, _buffer, size_ - len);
				buff = _temp_buffer._buffer;
				ret = size_;
			}
		}
	}
	_out += size_;
	return ret;
}

size_t CRingBuffer::_write(const char *data, size_t size)
{
	assert(data);
	size_t len = 0;
	size_t size_ = min(size, _space());
	/* first put the data starting from fifo->in to buffer end */
	len = min(size_, _size - (_in & (_size - 1)));
	if (len < size) {
		DBG_I("分段..len:%d total:%d", len, size);
	}
	memcpy(_buffer + (_in & (_size - 1)), data, len);
	/* then put the rest (if any) at the beginning of the buffer */
	memcpy(_buffer, data + len, size_ - len);
	_in += size_;
	return size_;
}

CBoundBuffer::CBoundBuffer()
{
	m_bInit = false;
	m_iBlockSize = 0;
	m_pBasePointer = NULL;
	m_iCurrentPosition = 0;
	m_UseInternalLock = true;
	_InitLock();
}


CBoundBuffer::~CBoundBuffer()
{
	_UnInitLock();
}

int CBoundBuffer::Init(int iBlockSize)
{
	int err = 0;
	if (m_UseInternalLock){
		SYS_UTL::CAutoLock lck(&m_lock);
		err = Init(iBlockSize);
	}
	else{
		err = Init(iBlockSize);
	}
	return err;
}

int CBoundBuffer::UnInit()
{
	int err = 0;
	if (m_UseInternalLock){
		SYS_UTL::CAutoLock lck(&m_lock);
		err = _UnInit();
	}
	else{
		err = _UnInit();
	}
	return err;
}

int CBoundBuffer::Clear()
{
	int err = 0;
	if (m_UseInternalLock){
		SYS_UTL::CAutoLock lck(&m_lock);
		err = _Clear();
	}
	else{
		err = _Clear();
	}
	return err;
}

int CBoundBuffer::Write(char* pDataBuff, int iDataLen, int* pBlockPostion)
{
	int err = 0;
	if (m_UseInternalLock){
		SYS_UTL::CAutoLock lck(&m_lock);
		err = _Write(pDataBuff, iDataLen, pBlockPostion);
	}
	else{
		err = _Write(pDataBuff, iDataLen, pBlockPostion);
	}
	return err;
}

int CBoundBuffer::Read(int iBlockPostion, char* pDataBuff, int iDataLen)
{
	int err = 0;
	if (m_UseInternalLock){
		SYS_UTL::CAutoLock lck(&m_lock);
		err = _Read(iBlockPostion, pDataBuff, iDataLen);
	}
	else{
		err = _Read(iBlockPostion, pDataBuff, iDataLen);
	}
	return err;
}

int CBoundBuffer::GetAddr(int iBlockPostion, char*& pBlockAddr)
{
	int err = 0;
	if (m_UseInternalLock){
		SYS_UTL::CAutoLock lck(&m_lock);
		err = _GetAddr(iBlockPostion, pBlockAddr);
	}
	else{
		err = _GetAddr(iBlockPostion, pBlockAddr);
	}
	return err;
}

int CBoundBuffer::_Init(int iBlockSize)
{
	if (m_bInit){
		return 0;
	}

	if (iBlockSize < 1024)
	{
		iBlockSize = 1024;
	}

	if (iBlockSize >BUFFER_MAX_LEN){
		return -1;
	}

	//创建(或打开)共享数据段
	m_iBlockSize = PAD_SIZE(iBlockSize);
	m_pBasePointer = new char[m_iBlockSize];
	if (nullptr == m_pBasePointer){
		return -2;
	}
	m_bInit = true;
	return 0;
}

int CBoundBuffer::_UnInit()
{
	if (!m_bInit){
		return 0;
	}
	m_bInit = false;
	if (NULL != m_pBasePointer){
		delete[] m_pBasePointer;
		m_pBasePointer = NULL;
	}
	return 0;
}

//清空内存块
int CBoundBuffer::_Clear()
{
	m_iCurrentPosition = 0;
	return 0;
}

int CBoundBuffer::_Write(char* pDataBuff, int iDataLen, int* pBlockPostion)
{
	if (NULL == pDataBuff || iDataLen > m_iBlockSize)
	{
		return -1;
	}
	if (!m_bInit)
	{
		return -2;
	}

	char* ptr = NULL;
	int iBlockPosition = 0;
	if ((m_iBlockSize - m_iCurrentPosition) > iDataLen)
	{
		ptr = (char*)(m_pBasePointer + m_iCurrentPosition);
		iBlockPosition = m_iCurrentPosition;
		m_iCurrentPosition += iDataLen;
	}
	else
	{
		ptr = m_pBasePointer;
		iBlockPosition = 0;
		m_iCurrentPosition = iDataLen;
	}
	memcpy(ptr, pDataBuff, iDataLen);
	if (NULL != pBlockPostion)
	{
		*pBlockPostion = iBlockPosition;
	}
	return 0;
}

int CBoundBuffer::_Read(int iBlockPostion, char* pDataBuff, int iDataLen)
{
	if (iBlockPostion < 0 || iBlockPostion > m_iBlockSize)
	{
		return -1;
	}
	if (NULL == pDataBuff || (iBlockPostion + iDataLen) > m_iBlockSize)
	{
		return -2;
	}
	if (!m_bInit)
	{
		return -3;
	}

	char* ptr = m_pBasePointer + iBlockPostion;
	memcpy(pDataBuff, ptr, iDataLen);
	return 0;
}

int CBoundBuffer::_GetAddr(int iBlockPostion, char*& pBlockAddr)
{
	if (iBlockPostion < 0 || iBlockPostion > m_iBlockSize)
	{
		return -1;
	}
	if (!m_bInit)
	{
		return -2;
	}
	char* ptr = m_pBasePointer + iBlockPostion;
	if (NULL != pBlockAddr)
	{
		pBlockAddr = ptr;
	}

	return 0;
}

void CBoundBuffer::_InitLock()
{
}

void CBoundBuffer::_UnInitLock()
{
}
} //namespace SYS_UTL:NET
} //namespace SYS_UTL