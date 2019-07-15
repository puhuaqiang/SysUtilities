#ifndef __SYS_UTILITIES_API_H__
#define __SYS_UTILITIES_API_H__
#include <vector>
typedef struct _SYSUTL_ONCE
{
	unsigned char ran;
	HANDLE event;
}SYSUTL_ONCE, *LPSYSUTL_ONCE;

/**
* \brief ���ܺ���
*/
namespace SYS_UTL
{

	/**
	* ����߳̿��Գ�����һ��������guard��һ������ָ��
	* ��������ᱻ����һ��,ֻ��һ��:
	*/
	SYS_UTL_CAPI void Once(LPSYSUTL_ONCE guard, void(*callback)(void));
	/*
	* ����� c++11
	*/
	SYS_UTL_CAPI int Random(int start, int end);

	/*
	* �Ƿ�������
	*/
	SYS_UTL_CAPI bool IsDigital(const char*);
	/// ����GUID
	SYS_UTL_CAPI void CreateGuid(char* lpBuffer, int iBufferLen);

	/**
	* �ַ���
	*/
	/// ת���ַ����е�ָ�������ַ�
	SYS_UTL_CAPI void CharConvert(char*, char s, char d);
	/**
	* @param src Դ�ַ���
	* @param from ���滻���ַ���
	* @param to �滻Ŀ���ַ���
	* @param buff ������
	* @pram len ��������С
	*/
	SYS_UTL_CAPI void StringReplace(const char* src, const char* from, const char* to, char* buff, int len);
	/**
	* @brief ���ݷָ��������Ӵ�
	* 	ע�� ,�����,�Ӵ��п����ǿ��ַ���, ���� x...x ,,���� �ָ���λ 'x'���Ӵ�
	* @param src. Դ�ַ���.
	* @param delims. �ָ���.
	* @param index. �ڼ����ָ���.��0��ʼ.
	* @param buff. ���������.
	* @param bufflen. �����������С.
	* @param bMutil. ��ָ���..
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
	* ·������
	*/
	/// ��ȡʵ��·��
	SYS_UTL_CAPI char* GetCurrentPath(HINSTANCE hInstance/* = NULL*/);
	SYS_UTL_CAPI bool IsPathExists(const char*);
	SYS_UTL_CAPI void CreateFolders(const char*);
	SYS_UTL_CAPI bool DelFolders(const char*);
	SYS_UTL_CAPI void CreateFoldersFromFilePath(const char*);
	/**
	* @brief ö��Ŀ¼�µ��ļ�/��Ŀ¼
	* @param pDir. ָ��Ŀ¼.
	* @param pExt. ��չ������. �� szExt[2][16] = {".log",".exe"}
	* @param iExtNum ��������չ������
	* @param lstDirs ��Ŀ¼������, �������Ҫ,���� nullptr
	* @param lstFiles �ļ�������, �������Ҫ,���� nullptr
	* @param bRecursive �Ƿ�ݹ�����
	* @return char*
	*/
	SYS_UTL_CAPI void EnumDirectoryFiles(const char* pDir,
		char pExt[][16],
		int iExtNum,
		std::list<std::string>* lstDirs,
		std::list<std::string>* lstFiles,
		bool bRecursive);
	/// ����Ŀ¼���ļ�������Ŀ¼���µ�·����,
	SYS_UTL_CAPI int CopyFolders(const char* src, const char* des, bool bRecursive = true);
	/// �滻Ŀ¼������ͬ���ļ�
	SYS_UTL_CAPI int ReplaceFiles(const char* file, const char* des, bool bRecursive = true);

	/// ȥ���ļ�·����չ��
	SYS_UTL_CAPI void RemoveExtension(char*);
	/// ȥ���ļ������õ�Ŀ¼
	SYS_UTL_CAPI void RemoveFileSpec(char*);
	/// ȥ��·���е�Ŀ¼����,�õ��ļ���
	SYS_UTL_CAPI void StripPath(char*);
	/// ����·������չ��
	SYS_UTL_CAPI char* FindExtension(char*);
	/// ��ȡ���·�� FILE_ATTRIBUTE_DIRECTORY FILE_ATTRIBUTE_NORMAL
	SYS_UTL_CAPI BOOL RelativePathTo(char* pszPath, char* pszFrom, DWORD dwAttrFrom, char* pszTo, DWORD dwAttrTo);
	/// �Ƿ������·��
	SYS_UTL_CAPI BOOL IsRelativePath(char* path);
	/// ת�������·��
	SYS_UTL_CAPI void ConverToRelative(char* from, char* to, int len);
	/// �ϲ�����·�� "C:" + "One\Two\Three" = "C:\One\Two\Three",,��ʹ�����·�� "C:\One\Two\Three" + "..\..\four" = "C:\One\four"
	SYS_UTL_CAPI void CombinePath(char* buff, const char* first, const char* second);
	/// ɾ���ļ�
	SYS_UTL_CAPI BOOL DelFile(const char*);
	/// �����ļ�
	SYS_UTL_CAPI BOOL FileCopy(const char*, const char*, BOOL bFailIfExists);
	/// �ļ���С
	SYS_UTL_CAPI unsigned long long FileSize(const char* file);

	///SHFileOperation
	// ·������
	SYS_UTL_CAPI bool PathCopy(const char *_pFrom, const char *_pTo);
	// ·��������
	SYS_UTL_CAPI bool PathReName(const char *_pFrom, const char *_pTo);
	// ·��ɾ�� �ݹ�ɾ��
	SYS_UTL_CAPI bool PathDelete(const char* _pFrom);
	// ·���ƶ� 
	SYS_UTL_CAPI bool PathMove(const char *_pFrom, const char *_pTo);


	/**
	* �ַ��������ʽת��
	*/
	/**
	* @brief GBK�ַ���ת UTF-8�ַ���
	* @param buff. ��Ž���Ļ�����.
	* @param iBuffLen. ��Ž���Ļ�������С.
	* @param pSrc. Դ�ַ���
	* @param iSrcLen. Դ�ַ�����С,�����ַ��������� strlen(x)+1
	* @return ���ػ��������ת��������ݴ�С
	*/
	SYS_UTL_CAPI int GBKToUtf8(char* buff, int iBuffLen, const char* pSrc, int iSrcLen);
	/**
	* @brief UTF-8�ַ��� ת GBK�ַ���
	* @param buff. ��Ž���Ļ�����.
	* @param iBuffLen. ��Ž���Ļ�������С.
	* @param pSrc. Դ�ַ���
	* @param iSrcLen. Դ�ַ�����С,�����ַ��������� strlen(x)+1
	* @return ���ػ��������ת��������ݴ�С
	*/
	SYS_UTL_CAPI int Utf8ToGBK(char* buff, int iBuffLen, const char* pSrc, int iSrcLen);

	/// WideCharToMultiByte
	SYS_UTL_CAPI char* WcharToChar(wchar_t* wc);
	SYS_UTL_CAPI wchar_t* CharToWchar(char* c);
	/// ������1024Bytes
	SYS_UTL_CAPI void WcharToChar2(wchar_t* wc, char* buff, int len);
	SYS_UTL_CAPI void CharToWchar2(char* c, wchar_t* buff, int len);


	/**
	* ����
	*/
	/**
	* @brief ���г���
	* @param cmd. ������.
	* @param uiTimeOut. ��ʱʱ��.
	* @return char*
	*/
	typedef struct _PROCESS_PROPERTY {
		unsigned int uiProcessID;
		HANDLE hProcessHandle;
	}PROCESS_PROPERTY, *PPROCESS_PROPERTY;
	SYS_UTL_CAPI int RunProcess(const char* cmd, PPROCESS_PROPERTY pProcessProperty = NULL, unsigned int uiTimeOut = 0, BOOL bShow = FALSE);
	/**
	* @brief ������ҵ����,�����̹ر�,�ӽ����Զ��ر�
	* @return ��ҵ������
	*/
	SYS_UTL_CAPI HANDLE EnableSubProcessAutoClose(const char* lpName);
	/// ����ӽ��̾������ҵ��
	SYS_UTL_CAPI BOOL AddSubProcess(HANDLE hJob, HANDLE sub);



	/*
	* ע������..
	*/
	/*
	* @brief ����/�򿪼�������������, close�������Ƿ�ر�
	* @param key. ע������ [HKEY_LOCAL_MACHINE]...
	* @param subkey. �Ӽ�.
	* @param close. �򿪺��Ƿ�ر�,����رշ���ֵһ����NULL
	* @return [HKEY]. �������.
	*/
	SYS_UTL_CAPI HKEY CreateRegKey(HKEY key, const char* subkey, bool close = true);
	SYS_UTL_CAPI int CloseRegKey(HKEY key);
	/*
	* @brief д/�޸�ע���ֵ��
	* @param key. �������
	* @param subkey. �Ӽ�.
	* @param value. ֵ��
	* @param val. ֵ
	* @param len. ֵ��С
	* @return [int]. �ɹ�����0.���򷵻ط�0.
	*/
	SYS_UTL_CAPI int WriteRegString(HKEY key, const char* subkey, const char* value, const char* val, int len);
	SYS_UTL_CAPI int WriteRegInt(HKEY key, const char* subkey, const char* value, int val);
	// HKEY_LOCAL_MACHINE..  section.=SOFTWARE\\InControl\\Install  Entry..= install
	SYS_UTL_CAPI int ReadRegString(HKEY key, const char* subkey, const char* value, char* buff, int len);
	SYS_UTL_CAPI int ReadRegInt(HKEY key, const char* subkey, const char* value, int& v);
	/*
	* @brief ö��ע���[�ӽ�/ֵ��]
	* @param key. �������
	* @param subkey. �Ӽ�.
	* @param pSubKey. ����ӽ�����,���Դ�NULL
	* @param pValues. ���ֵ�����,���Դ�NULL
	* @return [int]. �ɹ�����0.���򷵻ط�0.
	*/
	SYS_UTL_CAPI int EnumRegKey(HKEY key, const char* subkey, std::list<std::string>* pSubKey, std::list<std::string>* pValues);


	/*
	* @brief	IEEE754�����ȸ�������ʽ
	* 	4��Byte
	*/
	SYS_UTL_CAPI BYTE GetBit(unsigned int dwValue, int iIndex);
	SYS_UTL_CAPI float ByteToFloat(BYTE*);
}

#endif