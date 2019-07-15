#ifndef __SYS_UTILITIES_API_H__
#define __SYS_UTILITIES_API_H__
#include <vector>
typedef struct _SYSUTL_ONCE
{
	unsigned char ran;
	HANDLE event;
}SYSUTL_ONCE, *LPSYSUTL_ONCE;

/**
* \brief 功能函数
*/
namespace SYS_UTL
{

	/**
	* 多个线程可以尝试以一个给定的guard与一个函数指针
	* 这个函数会被调用一次,只有一次:
	*/
	SYS_UTL_CAPI void Once(LPSYSUTL_ONCE guard, void(*callback)(void));
	/*
	* 随机数 c++11
	*/
	SYS_UTL_CAPI int Random(int start, int end);

	/*
	* 是否是数字
	*/
	SYS_UTL_CAPI bool IsDigital(const char*);
	/// 创建GUID
	SYS_UTL_CAPI void CreateGuid(char* lpBuffer, int iBufferLen);

	/**
	* 字符串
	*/
	/// 转换字符串中的指定所有字符
	SYS_UTL_CAPI void CharConvert(char*, char s, char d);
	/**
	* @param src 源字符串
	* @param from 被替换的字符串
	* @param to 替换目标字符串
	* @param buff 缓冲区
	* @pram len 缓冲区大小
	*/
	SYS_UTL_CAPI void StringReplace(const char* src, const char* from, const char* to, char* buff, int len);
	/**
	* @brief 根据分隔符查找子串
	* 	注意 ,输出的,子串有可以是空字符串, 比如 x...x ,,查找 分隔符位 'x'的子串
	* @param src. 源字符串.
	* @param delims. 分隔符.
	* @param index. 第几个分隔符.从0开始.
	* @param buff. 输出缓存区.
	* @param bufflen. 输出缓存区大小.
	* @param bMutil. 多分隔符..
	* @return None
	*/
	SYS_UTL_CAPI void StrTok(const char* src,
		const char* delims,
		int index,
		char* buff,
		int bufflen,
		BOOL bMutil = TRUE);
	SYS_UTL_CAPI void StrTok2(const char*,
		const char* delims,
		std::vector<std::string>&,
		BOOL bMutil = TRUE);


	/**
	* 路径操作
	*/
	/// 获取实例路径
	SYS_UTL_CAPI char* GetCurrentPath(HINSTANCE hInstance/* = NULL*/);
	SYS_UTL_CAPI bool IsPathExists(const char*);
	SYS_UTL_CAPI void CreateFolders(const char*);
	SYS_UTL_CAPI bool DelFolders(const char*);
	SYS_UTL_CAPI void CreateFoldersFromFilePath(const char*);
	/**
	* @brief 枚举目录下的文件/子目录
	* @param pDir. 指定目录.
	* @param pExt. 扩展名数组. 如 szExt[2][16] = {".log",".exe"}
	* @param iExtNum 搜索的扩展名数量
	* @param lstDirs 子目录缓存区, 如果不需要,传入 nullptr
	* @param lstFiles 文件缓存区, 如果不需要,传入 nullptr
	* @param bRecursive 是否递归搜索
	* @return char*
	*/
	SYS_UTL_CAPI void EnumDirectoryFiles(const char* pDir,
		char pExt[][16],
		int iExtNum,
		std::list<std::string>* lstDirs,
		std::list<std::string>* lstFiles,
		bool bRecursive);
	/// 拷贝目录下文件及其子目录到新的路径下,
	SYS_UTL_CAPI int CopyFolders(const char* src, const char* des, bool bRecursive = true);
	/// 替换目录下所有同名文件
	SYS_UTL_CAPI int ReplaceFiles(const char* file, const char* des, bool bRecursive = true);

	/// 去除文件路径扩展名
	SYS_UTL_CAPI void RemoveExtension(char*);
	/// 去除文件名，得到目录
	SYS_UTL_CAPI void RemoveFileSpec(char*);
	/// 去掉路径中的目录部分,得到文件名
	SYS_UTL_CAPI void StripPath(char*);
	/// 查找路径的扩展名
	SYS_UTL_CAPI char* FindExtension(char*);
	/// 获取相对路径 FILE_ATTRIBUTE_DIRECTORY FILE_ATTRIBUTE_NORMAL
	SYS_UTL_CAPI BOOL RelativePathTo(char* pszPath, char* pszFrom, DWORD dwAttrFrom, char* pszTo, DWORD dwAttrTo);
	/// 是否是相对路径
	SYS_UTL_CAPI BOOL IsRelativePath(char* path);
	/// 转换到相对路径
	SYS_UTL_CAPI void ConverToRelative(char* from, char* to, int len);
	/// 合并两个路径 "C:" + "One\Two\Three" = "C:\One\Two\Three",,可使用相对路径 "C:\One\Two\Three" + "..\..\four" = "C:\One\four"
	SYS_UTL_CAPI void CombinePath(char* buff, const char* first, const char* second);
	/// 删除文件
	SYS_UTL_CAPI BOOL DelFile(const char*);
	/// 拷贝文件
	SYS_UTL_CAPI BOOL FileCopy(const char*, const char*, BOOL bFailIfExists);
	/// 文件大小
	SYS_UTL_CAPI unsigned long long FileSize(const char* file);

	///SHFileOperation
	// 路径拷贝
	SYS_UTL_CAPI bool PathCopy(const char *_pFrom, const char *_pTo);
	// 路径重命名
	SYS_UTL_CAPI bool PathReName(const char *_pFrom, const char *_pTo);
	// 路径删除 递归删除
	SYS_UTL_CAPI bool PathDelete(const char* _pFrom);
	// 路径移动 
	SYS_UTL_CAPI bool PathMove(const char *_pFrom, const char *_pTo);


	/**
	* 字符串编码格式转换
	*/
	/**
	* @brief GBK字符串转 UTF-8字符串
	* @param buff. 存放结果的缓存区.
	* @param iBuffLen. 存放结果的缓存区大小.
	* @param pSrc. 源字符串
	* @param iSrcLen. 源字符串大小,包括字符串结束符 strlen(x)+1
	* @return 返回缓存区存放转换后的数据大小
	*/
	SYS_UTL_CAPI int GBKToUtf8(char* buff, int iBuffLen, const char* pSrc, int iSrcLen);
	/**
	* @brief UTF-8字符串 转 GBK字符串
	* @param buff. 存放结果的缓存区.
	* @param iBuffLen. 存放结果的缓存区大小.
	* @param pSrc. 源字符串
	* @param iSrcLen. 源字符串大小,包括字符串结束符 strlen(x)+1
	* @return 返回缓存区存放转换后的数据大小
	*/
	SYS_UTL_CAPI int Utf8ToGBK(char* buff, int iBuffLen, const char* pSrc, int iSrcLen);

	/// WideCharToMultiByte
	SYS_UTL_CAPI char* WcharToChar(wchar_t* wc);
	SYS_UTL_CAPI wchar_t* CharToWchar(char* c);
	/// 限制在1024Bytes
	SYS_UTL_CAPI void WcharToChar2(wchar_t* wc, char* buff, int len);
	SYS_UTL_CAPI void CharToWchar2(char* c, wchar_t* buff, int len);


	/**
	* 进程
	*/
	/**
	* @brief 运行程序
	* @param cmd. 命令行.
	* @param uiTimeOut. 超时时间.
	* @return char*
	*/
	typedef struct _PROCESS_PROPERTY {
		unsigned int uiProcessID;
		HANDLE hProcessHandle;
	}PROCESS_PROPERTY, *PPROCESS_PROPERTY;
	SYS_UTL_CAPI int RunProcess(const char* cmd, PPROCESS_PROPERTY pProcessProperty = NULL, unsigned int uiTimeOut = 0, BOOL bShow = FALSE);
	/**
	* @brief 创建作业对象,主进程关闭,子进程自动关闭
	* @return 作业对象句柄
	*/
	SYS_UTL_CAPI HANDLE EnableSubProcessAutoClose(const char* lpName);
	/// 添加子进程句柄到作业中
	SYS_UTL_CAPI BOOL AddSubProcess(HANDLE hJob, HANDLE sub);



	/*
	* 注册表相关..
	*/
	/*
	* @brief 创建/打开键，如果存在则打开, close创建后是否关闭
	* @param key. 注册表根键 [HKEY_LOCAL_MACHINE]...
	* @param subkey. 子键.
	* @param close. 打开后是否关闭,如果关闭返回值一定是NULL
	* @return [HKEY]. 操作句柄.
	*/
	SYS_UTL_CAPI HKEY CreateRegKey(HKEY key, const char* subkey, bool close = true);
	SYS_UTL_CAPI int CloseRegKey(HKEY key);
	/*
	* @brief 写/修改注册表值项
	* @param key. 操作句柄
	* @param subkey. 子键.
	* @param value. 值项
	* @param val. 值
	* @param len. 值大小
	* @return [int]. 成功返回0.否则返回非0.
	*/
	SYS_UTL_CAPI int WriteRegString(HKEY key, const char* subkey, const char* value, const char* val, int len);
	SYS_UTL_CAPI int WriteRegInt(HKEY key, const char* subkey, const char* value, int val);
	// HKEY_LOCAL_MACHINE..  section.=SOFTWARE\\InControl\\Install  Entry..= install
	SYS_UTL_CAPI int ReadRegString(HKEY key, const char* subkey, const char* value, char* buff, int len);
	SYS_UTL_CAPI int ReadRegInt(HKEY key, const char* subkey, const char* value, int& v);
	/*
	* @brief 枚举注册表[子健/值项]
	* @param key. 操作句柄
	* @param subkey. 子键.
	* @param pSubKey. 存放子健队列,可以传NULL
	* @param pValues. 存放值项队列,可以传NULL
	* @return [int]. 成功返回0.否则返回非0.
	*/
	SYS_UTL_CAPI int EnumRegKey(HKEY key, const char* subkey, std::list<std::string>* pSubKey, std::list<std::string>* pValues);


	/*
	* @brief	IEEE754单精度浮点数格式
	* 	4个Byte
	*/
	SYS_UTL_CAPI BYTE GetBit(unsigned int dwValue, int iIndex);
	SYS_UTL_CAPI float ByteToFloat(BYTE*);
}

#endif