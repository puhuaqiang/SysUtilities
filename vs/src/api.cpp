#include "../SysUtilities/stdafx.h"
#include "../include/api.h"
#include <random>
#include <time.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <io.h>
#include <map>
#include <list>
#include <vector>

#include <dwmapi.h> 
#pragma comment(lib, "dwmapi.lib")

#include <ShellAPI.h>
#include <Shlwapi.h> // path api
#pragma comment(lib,"Shlwapi.lib")

#include <rpcdce.h> // UUID
#pragma comment(lib,"Rpcrt4.lib")

namespace SYS_UTL
{

	BOOL Is64() {
		SYSTEM_INFO si;
		GetNativeSystemInfo(&si);
		if ((si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ||
			(si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)) {
			return TRUE;
		}
		return FALSE;
	}

	SYS_UTL_CAPI void Once(LPSYSUTL_ONCE guard, void(*callback)(void))
	{
		/* Fast case - avoid WaitForSingleObject. */
		if (guard->ran) {
			return;
		}

		__once_inner(guard, callback);
	}

	int Random(int start, int end)
	{
		static std::default_random_engine generator((int)time(0));
		static std::uniform_int_distribution<int> distribution(start, end);
		int dice_roll = distribution(generator);
		return dice_roll;
	}

	bool IsDigital(const char* lpStr)
	{
		if (NULL == lpStr)
		{
			return false;
		}
		int len = strlen(lpStr);
		if (len <= 0)
		{
			return false;
		}
		for (int i = 0; i < len; i++)
		{
			if (!isdigit(lpStr[i]))
			{
				return false;
			}
		}
		return true;
	}

	void CreateGuid(char* lpBuffer, int iBufferLen)
	{
		if (NULL == lpBuffer)
		{
			return;
		}
		char szGuid[128];
		UUID guid = { 0 };
		UuidCreate(&guid);
		ZeroMemory(szGuid, sizeof(szGuid));
		_snprintf_s(szGuid, 128, "%x-%x-%x-%x%x-%x%x%x%x%x%x",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1],
			guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
		strncpy_s(lpBuffer, iBufferLen, szGuid, min(strlen(szGuid), iBufferLen - 1));
	}

	void CharConvert(char* str, char s, char d)
	{
		if (NULL == str) 
		{
			return;
		}
		int len = strlen(str);
		for (int i = 0; i < len; i++){
			if (str[i] == s) {
				str[i] = d;
			}
		}
	}

	void StringReplace(const char* src, const char* from, const char* to, char* buff, int len)
	{
		if (NULL == src || NULL == from || NULL == to || NULL == buff || len <= 0)
		{
			return;
		}
		int toLen = strlen(to);
		std::vector<std::string> v;
		StrTok2(src, from, v, FALSE);
		buff[0] = '\0';

		int iCnt = v.size();
		for (int i = 0; i < iCnt; i++)
		{
			strncat_s(buff, len, v.at(i).data(), min(len - 1 - strlen(buff), v.at(i).length()));
			if (i + 1 < iCnt){
				strncat_s(buff, len, to, min((int)(len - 1 - strlen(buff)), toLen));
			}
		}
	}

	void StrTok(const char* src, const char* delims, int index, char* buff, int bufflen, BOOL bMutil /*= TRUE*/)
	{
		const int ciBuffLen = 1024;
		int iBuffLen = ciBuffLen, iProIndex = 0;
		char szDelims[64], szSrc[ciBuffLen];
		char* pSrc = NULL;
		char* pDelims = szDelims;
		char *strToken = nullptr;
		char *next_token = nullptr;
		if (delims == NULL || src == NULL || buff == NULL || bufflen <= 0)
		{
			return;
		}
		_snprintf_s(szDelims, _TRUNCATE, "%s", delims);
		if (strlen(src) > ciBuffLen){
			iBuffLen = PAD_SIZE(strlen(src));
			pSrc = new char[iBuffLen];
			if (pSrc == NULL){
				return;
			}
		}
		else {
			pSrc = szSrc;
		}
		strncpy_s(pSrc, iBuffLen, src, min(iBuffLen - 1, (int)strlen(src)));

		if (bMutil){
			strToken = strtok_s(pSrc, szDelims, &next_token);//linux - strtok_r
			if (strToken == NULL){
				if (iBuffLen > ciBuffLen){
					delete[] pSrc;
				}
				return;
			}
			while (strToken != NULL) {
				strncpy_s(buff, bufflen, strToken, min(bufflen - 1, (int)strlen(strToken)));
				if (iProIndex == index){
					break;
				}
				iProIndex++;
				strToken = strtok_s(nullptr, szDelims, &next_token);
			}
		}
		else {
			int len = 0, iIdx = 0;
			bool bFind = false;
			strToken = pSrc;
			next_token = strstr(strToken, pDelims);
			while (next_token != NULL) {
				len = next_token - strToken;
				if (iIdx == index){
					strncpy_s(buff, bufflen, strToken, min(bufflen - 1, len));
					bFind = true;
					break;
				}
				iIdx++;
				strToken = next_token + strlen(pDelims);
				next_token = strstr(strToken, pDelims);
			}
			if (!bFind && strlen(strToken) > 0){
				if (iIdx == index){
					strncpy_s(buff, bufflen, strToken, min((int)(bufflen - 1), (int)strlen(strToken)));
				}
			}
		}

		if (iBuffLen > ciBuffLen){
			delete[] pSrc;
		}
	}

	void StrTok2(const char* src, const char* delims, std::vector<std::string>& vStr, BOOL bMutil /*= TRUE*/)
	{
		const int ciBuffLen = 1024;
		int iBuffLen = ciBuffLen;
		char szDelims[64], szSrc[ciBuffLen];
		char* pSrc = NULL;
		char* pDelims = szDelims;
		char *strToken = nullptr;
		char *next_token = nullptr;
		if (delims == NULL || src == NULL)
		{
			return;
		}
		_snprintf_s(szDelims, _TRUNCATE, "%s", delims);
		if (strlen(src) > ciBuffLen){
			iBuffLen = PAD_SIZE(strlen(src));
			pSrc = new char[iBuffLen];
			if (pSrc == NULL){
				return;
			}
		}
		else {
			pSrc = szSrc;
		}
		strncpy_s(pSrc, iBuffLen, src, min(iBuffLen - 1, (int)strlen(src)));

		if (bMutil){
			strToken = strtok_s(pSrc, szDelims, &next_token);
			if (strToken == NULL){
				if (iBuffLen > ciBuffLen){
					delete[] pSrc;
				}
				return;
			}
			while (strToken != NULL) {
				vStr.emplace_back(strToken);
				/*if (strlen(strToken) > 0){
				vStr.emplace_back(strToken);
				}*/
				strToken = strtok_s(nullptr, szDelims, &next_token);
			}
		}
		else {
			int len = 0;
			strToken = pSrc;
			next_token = strstr(strToken, pDelims);
			while (next_token != NULL) {
				len = next_token - strToken;
				vStr.emplace_back(strToken, len);
				strToken = next_token + strlen(pDelims);
				next_token = strstr(strToken, pDelims);
			}
			if (strToken != nullptr/*strlen(strToken) > 0*/){
				vStr.emplace_back(strToken);
			}
		}

		if (iBuffLen > ciBuffLen){
			delete[] pSrc;
		}
	}

	char* GetCurrentPath(HINSTANCE hInstance/* = NULL*/)
	{
		static char gszCurrentPath[MAX_PATH] = { 0 };
		static HINSTANCE ghIns = NULL;
		if (strlen(gszCurrentPath) <= 0 || ghIns != hInstance){
			ghIns = hInstance;
			DWORD dwLength = GetModuleFileName(ghIns, gszCurrentPath, MAX_PATH);
			if (dwLength != 0 && gszCurrentPath[0] != _T('\0'))	{
				char *p = _tcsrchr(gszCurrentPath, _T('\\'));
				if (p){
					//*(++p) = _T('\0');
					*(p) = _T('\0');
				}
			}
		}
		return gszCurrentPath;
	}

	bool IsPathExists(const char* path)
	{
		if (NULL == path)
		{
			return false;
		}
		if (_access(path, 0) == 0)
		{
			return true;
		}
		return false;
	}

	void CreateFolders(const char* path)
	{
		if (nullptr == path) 
		{
			return;
		}
#ifdef _WIN32
		char tmp[MAX_PATH];
		char* p = nullptr;
		char* ptr = nullptr;
		tmp[0] = '\0';
		strncpy_s(tmp, sizeof(tmp)-1, path, min(strlen(path), sizeof(tmp)-1));
		CharConvert(tmp, '/', '\\');
		ptr = tmp;
		do
		{
			p = strchr(ptr, '\\');
			if (nullptr == p) 
			{
				if (!IsPathExists(tmp)) 
				{
					CreateDirectory(tmp, nullptr);
				}
				break;
			}
			else {
				*p = '\0';
				if (!IsPathExists(tmp)) 
				{
					CreateDirectory(tmp, nullptr);
				}
				*p = '\\';
			}
			ptr = p + 1;
		} while (true);
#else

#endif
	}

	bool DelFolders(const char* folders)
	{
		if (folders == NULL)
		{
			return false;
		}
		std::list<std::string> lstDirs;
		std::list<std::string> lstFiles;
		EnumDirectoryFiles(folders, NULL, 0, &lstDirs, &lstFiles, true);
		bool bFail = false;
		for (auto& it : lstFiles)
		{
			if (!DeleteFile(it.c_str())) 
			{
				bFail = true;
			}
		}
		auto it = lstDirs.crbegin();
		auto itEnd = lstDirs.crend();
		for (; it != itEnd; it++){
			if (!RemoveDirectory(it->c_str())) 
			{
				bFail = true;
			}
		}
		if (IsPathExists(folders))
		{
			if (!RemoveDirectory(folders)) 
			{
				bFail = true;
			}
		}
		return !bFail;
	}

	void CreateFoldersFromFilePath(const char* file)
	{
		if (NULL == file) 
		{
			return;
		}
#ifdef _WIN32
		char tmp[MAX_PATH];
		char* p = nullptr;
		char* ptr = nullptr;
		tmp[0] = '\0';
		strncpy_s(tmp, sizeof(tmp), file, min(sizeof(tmp)-1, strlen(file)));
		CharConvert(tmp, '/', '\\');
		RemoveFileSpec(tmp);
		ptr = tmp;
		do
		{
			p = strchr(ptr, '\\');
			if (nullptr == p) {
				if (!IsPathExists(tmp)) {
					CreateDirectory(tmp, nullptr);
				}
				break;
			}
			else {
				*p = '\0';
				if (!IsPathExists(tmp)) {
					CreateDirectory(tmp, nullptr);
				}
				*p = '\\';
			}
			ptr = p + 1;
		} while (true);
#else

#endif
	}

	void EnumDirectoryFiles(const char* pDir, char pExt[][16], int iExtNum, std::list<std::string>* lstDirs, std::list<std::string>* lstFiles, bool bRecursive)
	{
#ifdef _WIN32
		if ((pDir == NULL) || strlen(pDir) == 0){
			DBG_E;
			return;
		}
		if (!IsPathExists(pDir)){
			DBG_I("err par:%s", pDir);
			return;
		}

		WIN32_FIND_DATAA FindFileData;
		BOOL IsFinded = TRUE;
		char szPath[MAX_PATH], szSubDir[MAX_PATH];
		char szDir[MAX_PATH], szFile[MAX_PATH];

		szDir[0] = '\0';
		strncpy_s(szDir, sizeof(szDir)-1, pDir, min(strlen(pDir), sizeof(szDir)-1));
		if (szDir[strlen(szDir) - 1] == '\\' || szDir[strlen(szDir) - 1] == '/'){
			szDir[strlen(szDir) - 1] = '\0';
		}

		memset(&FindFileData, 0, sizeof(WIN32_FIND_DATAA));
		_snprintf_s(szPath, _TRUNCATE, "%s\\*.*", szDir);
		HANDLE hFile = FindFirstFile(szPath, &FindFileData);
		if (hFile == INVALID_HANDLE_VALUE) {
			DBG_I("err LINE:%d", __LINE__);
			return;
		}
		while (IsFinded){
			if (strcmp(FindFileData.cFileName, ".") && strcmp(FindFileData.cFileName, "..")) 
			{

				_snprintf_s(szFile, _TRUNCATE, "%s\\%s", szDir, FindFileData.cFileName);
				if (!PathIsDirectory(szFile))
				{
					if (lstFiles){
						char* p = PathFindExtension(FindFileData.cFileName);//.txt
						if (pExt != NULL && p != NULL && iExtNum > 0)
						{
							for (int i = 0; i < iExtNum; i++)
							{
								if (!strcmp(p, pExt[i]))
								{
									lstFiles->push_back(szFile);
									break;
								}
							}
						}
						else {
							lstFiles->push_back(szFile);
						}
					}
				}
				else {
					if (lstDirs)
					{
						lstDirs->push_back(szFile);
					}
					if (bRecursive){
						_snprintf_s(szSubDir, _TRUNCATE, "%s\\%s", szDir, FindFileData.cFileName);
						EnumDirectoryFiles(szSubDir, pExt, iExtNum, lstDirs, lstFiles, bRecursive);
					}
				}
			}
			IsFinded = FindNextFile(hFile, &FindFileData);
		}
		if (hFile != INVALID_HANDLE_VALUE) {
			FindClose(hFile);
		}
#else
#endif
	}

	int CopyFolders(const char* src, const char* des, bool bRecursive /*= true*/)
	{
		if (NULL == src || NULL == des) {
			DBG_E;
			return -1;
		}
		if (strlen(src) == 0 || strlen(des) == 0) {
			DBG_E;
			return -1;
		}
		int err = 0;
		char tmp[MAX_PATH], relative[MAX_PATH];
		std::list<std::string> lstDirs, lstFiles;

		EnumDirectoryFiles(src, nullptr, 0, &lstDirs, &lstFiles, bRecursive);

		for (auto& it : lstDirs)
		{
			//相对路径
			RelativePathTo(tmp,
				(char*)src,
				FILE_ATTRIBUTE_DIRECTORY,
				(char*)it.data(),
				FILE_ATTRIBUTE_DIRECTORY);
			relative[0] = '\0';
			if (!IsRelativePath(tmp)){
				ConverToRelative(tmp, relative, MAX_PATH);
			}
			else {
				strncpy_s(relative, sizeof(relative), tmp, min(strlen(tmp), sizeof(relative)-1));
			}
			//合并路径
			tmp[0] = '\0';
			CombinePath(tmp, des, relative);
			if (!IsPathExists(tmp)){
				CreateFolders(tmp);
			}
			if (!IsPathExists(tmp)){
				DBG_E;
				err = -2;
			}
		}
		for (auto& it : lstFiles)
		{
			//相对路径
			RelativePathTo(tmp,
				(char*)src,
				FILE_ATTRIBUTE_DIRECTORY,
				(char*)it.data(),
				FILE_ATTRIBUTE_NORMAL);
			relative[0] = '\0';
			if (!IsRelativePath(tmp)) {
				ConverToRelative(tmp, relative, MAX_PATH);
			}
			else {
				strncpy_s(relative, sizeof(relative), tmp, min(strlen(tmp), sizeof(relative)-1));
			}

			//合并路径
			tmp[0] = '\0';
			CombinePath(tmp, des, relative);
			//DEBUG_INFO("合并路径 :%s", tmp);
			if (IsPathExists(tmp)) {
				DelFile(tmp);
			}
			if (!FileCopy(it.c_str(), tmp, FALSE)){
				err = -3;
				DBG_I("拷贝文件[%s] 到[%s] ..失败 LINE:%d", it.c_str(), tmp, __LINE__);
			}
		}
		return err;
	}

	int ReplaceFiles(const char* file, const char* des, bool bRecursive /*= true*/)
	{
		if (NULL == file || NULL == des) 
		{
			DBG_E;
			return -1;
		}
		if (strlen(file) == 0 || strlen(des) == 0) 
		{
			DBG_E;
			return -1;
		}
		int err = 0;
		const char* p = nullptr;
		char szName[MAX_PATH];
		std::list<std::string> lstFiles;
		strncpy_s(szName, MAX_PATH, file, min(strlen(file), sizeof(szName)-1));
		StripPath(szName);
		if (strlen(szName) == 0) {
			DBG_E;
			return -1;
		}

		EnumDirectoryFiles(des, nullptr, 0, nullptr, &lstFiles, bRecursive);
		for (auto& it : lstFiles){
			p = strstr(it.data(), szName);
			if (NULL == p){
				continue;
			}
			DelFile(it.data());
			if (!FileCopy(file, it.c_str(), FALSE)){
				err = -2;
				DBG_I("拷贝文件[%s] 到[%s] ..失败", file, it.c_str());
			}
		}
		return err;
	}

	void RemoveExtension(char* path)
	{
#ifdef _WIN32
		if (NULL == path || strlen(path) <= 0)
		{
			return;
		}
		PathRemoveExtension(path);
#else

#endif // _WIN32
	}

	void RemoveFileSpec(char* path)
	{
#ifdef _WIN32
		if (NULL == path || strlen(path) <= 0)
		{
			return;
		}
		PathRemoveFileSpec(path);
#else

#endif // _WIN32
	}

	void StripPath(char* path)
	{
#ifdef _WIN32
		if (NULL == path || strlen(path) <= 0)
		{
			return;
		}
		PathStripPath(path);
#else

#endif // _WIN32
	}

	char* FindExtension(char* path)
	{
#ifdef _WIN32
		if (NULL == path || strlen(path) <= 0)
		{
			return NULL;
		}
		return PathFindExtension(path);
#else

#endif // _WIN32
	}

	BOOL RelativePathTo(char* pszPath, char* pszFrom, DWORD dwAttrFrom, char* pszTo, DWORD dwAttrTo)
	{
		return PathRelativePathTo(pszPath, pszFrom, FILE_ATTRIBUTE_DIRECTORY, pszTo, FILE_ATTRIBUTE_DIRECTORY);
	}

	BOOL IsRelativePath(char* path)
	{
		return PathIsRelative(path);
	}

	void ConverToRelative(char* from, char* to, int len)
	{
		char tmp[256];
		tmp[0] = '\0';
		if (NULL == from || NULL == to) 
		{
			return;
		}
		_snprintf_s(tmp, _TRUNCATE, ".%s", from);
		strncpy_s(to, len, tmp, min(strlen(tmp), len-1));
	}

	void CombinePath(char* buff, const char* first, const char* second)
	{
		if (NULL == buff || NULL == first || NULL == second)
		{
			return;
		}
		PathCombine(buff, first, second);
	}

	BOOL DelFile(const char* file)
	{
		if (nullptr == file) 
		{
			return FALSE;
		}
		BOOL ret = DeleteFile(file);
		if (!ret) {
			DBG_I("ERROR [FUNCTION]DelFile file:%s err:%d", file, GetLastError());
		}
		return ret;
	}

	BOOL FileCopy(const char* from, const char* to, BOOL bFailIfExists)
	{
		if (nullptr == from || nullptr == to) 
		{
			return FALSE;
		}
		return CopyFile(from, to, bFailIfExists);
	}

	unsigned long long FileSize(const char* file)
	{
#ifdef _WIN32
		if (NULL == file) 
		{
			return 0;
		}

		HANDLE hFile = nullptr;
		LARGE_INTEGER stFileSize;
		memset(&stFileSize, 0, sizeof(LARGE_INTEGER));
		hFile = CreateFile(file, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == nullptr) 
		{
			return 0;
		}
		GetFileSizeEx(hFile, &stFileSize);
		//printf("File size are %lld bytes and %4.2f KB and %4.2f MB and %4.2f GB\n", FileSize.QuadPart, (float)FileSize.QuadPart / 1024, (float)FileSize.QuadPart / (1024 * 1024), (float)FileSize.QuadPart / (1024 * 1024 * 1024));
		CloseHandle(hFile);
		return stFileSize.QuadPart;
#else

#endif // _WIN32
	}

	bool PathCopy(const char *_pFrom, const char *_pTo)
	{
		char szTo[MAX_PATH] = { 0 }, szFrom[MAX_PATH] = { 0 };
#if 1 //路径后2个\0\0
		memcpy(szTo, _pTo, lstrlen(_pTo));
		memcpy(szFrom, _pFrom, lstrlen(_pFrom));
#else
		Strcpy(szTo, MAX_PATH, _pTo);
		Strcpy(szFrom, MAX_PATH, _pFrom);
#endif

		SHFILEOPSTRUCT FileOp = { 0 };
		FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
		FileOp.pFrom = szFrom;
		FileOp.pTo = szTo;
		FileOp.wFunc = FO_COPY;
		int ret = SHFileOperation(&FileOp);
		if (ret != 0)
		{
			DBG_I("err:%d", ret);
		}
		return ret == 0;
	}

	bool PathReName(const char *_pFrom, const char *_pTo)
	{
		char szTo[MAX_PATH] = { 0 }, szFrom[MAX_PATH] = { 0 };
#if 1 //路径后2个\0\0
		memcpy(szTo, _pTo, lstrlen(_pTo));
		memcpy(szFrom, _pFrom, lstrlen(_pFrom));
#else
		Strcpy(szTo, MAX_PATH, _pTo);
		Strcpy(szFrom, MAX_PATH, _pFrom);
#endif
		SHFILEOPSTRUCT FileOp = { 0 };
		FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
		FileOp.pFrom = szFrom;
		FileOp.pTo = szTo;
		FileOp.wFunc = FO_RENAME;
		int ret = SHFileOperation(&FileOp);
		if (ret != 0)
		{
			DBG_I("err:%d", ret);
		}
		return ret == 0;
	}

	bool PathDelete(const char* _pFrom)
	{
		//SHFileOperation将永久删除文件，除非您在由lpFileOp指向的SHFILEOPSTRUCT结构的fFlags成员中设置FOF_ALLOWUNDO标志
		// 删除是递归的，除非您在lpFileOp中设置FOF_NORECURSION标志
		char szFrom[MAX_PATH] = { 0 };
		memcpy(szFrom, _pFrom, lstrlen(_pFrom));

		SHFILEOPSTRUCT FileOp = { 0 };
		FileOp.pFrom = szFrom;
		FileOp.pTo = NULL;//一定要是NULL
		FileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;;
		FileOp.wFunc = FO_DELETE; //删除操作
		int ret = SHFileOperation(&FileOp);
		if (ret != 0)
		{
			DBG_I("err:%d", ret);
		}
		return ret == 0;
	}

	bool PathMove(const char *_pFrom, const char *_pTo)
	{
		char szTo[MAX_PATH] = { 0 }, szFrom[MAX_PATH] = { 0 };
		memcpy(szTo, _pTo, lstrlen(_pTo));
		memcpy(szFrom, _pFrom, lstrlen(_pFrom));

		SHFILEOPSTRUCT FileOp = { 0 };
		FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
		FileOp.pFrom = szFrom;
		FileOp.pTo = szTo;
		FileOp.wFunc = FO_MOVE;
		int ret = SHFileOperation(&FileOp);
		if (ret != 0)
		{
			DBG_I("err:%d", ret);
		}
		return ret == 0;
	}

	int GBKToUtf8(char* buff, int iBuffLen, const char* pSrc, int iSrcLen)
	{
		if (buff == nullptr || pSrc == nullptr){
			return 0;
		}
		unsigned short* wszUtf8 = nullptr;
		char *szUtf8 = nullptr;
		char tmp1[1024 * 64], tmp2[1024 * 64];
		const int cilen = 1024 * 64;
		int iNeedLen1 = cilen, iNeedLen2 = cilen;
#if 1
		int len = MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)pSrc, iSrcLen, NULL, 0);
		if (len > cilen){
			wszUtf8 = new unsigned short[len];
			iNeedLen1 = len;
		}
		else{
			wszUtf8 = (unsigned short*)tmp1;
		}
		if (wszUtf8 == NULL){
			return 0;
		}

		//memset(wszUtf8, 0, len);
		MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)pSrc, iSrcLen, (LPWSTR)wszUtf8, len);
		len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, NULL, 0, NULL, NULL);
		if (len > cilen){
			szUtf8 = new char[len];
			iNeedLen2 = len;
		}
		else{
			szUtf8 = tmp2;
		}

		if (szUtf8 == NULL){
			return 0;
		}

		//memset(szUtf8, 0, len);
		WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, szUtf8, len, NULL, NULL);
		int tmp = min(iBuffLen - 1, len);
		memcpy(buff, szUtf8, tmp);
		buff[tmp] = '\0';
		if (iNeedLen1 > cilen){
			delete[] wszUtf8;
		}
		if (iNeedLen2 > cilen){
			delete[] szUtf8;
		}
		return tmp;
#else
		int len = MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)pSrc, iSrcLen, NULL, 0);
		unsigned short * wszUtf8 = new unsigned short[len];
		if (wszUtf8 == NULL){
			return;
		}
		memset(wszUtf8, 0, len);
		MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)pSrc, iSrcLen, (LPWSTR)wszUtf8, len);
		len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, NULL, 0, NULL, NULL);
		char *szUtf8 = new char[len];
		if (szUtf8 == NULL){
			return;
		}
		memset(szUtf8, 0, len);
		WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, szUtf8, len, NULL, NULL);
		if (len < 1024)
		{
			int tmp = min(iBuffLen - 1, len);
			strncpy_s(buff, iBuffLen, szUtf8, tmp);
			buff[tmp] = '\0';
		}
		else {
			int tmp = min(iBuffLen - 1, len);
			memcpy(buff, szUtf8, tmp);
			buff[tmp] = '\0';
		}

		delete[] szUtf8;
		delete[] wszUtf8;
#endif
	}

	int Utf8ToGBK(char* buff, int iBuffLen, const char *pSrc, int iSrcLen)
	{
		if (buff == nullptr || pSrc == nullptr){
			return 0;
		}
		unsigned short* wszGBK = nullptr;
		char *szGBK = nullptr;
		char tmp1[1024 * 64], tmp2[1024 * 64];
		const int cilen = 1024 * 64;
		int iNeedLen1 = cilen, iNeedLen2 = cilen;

#if 1
		int len = MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)pSrc, iSrcLen, NULL, 0);
		if (len > cilen){
			wszGBK = new unsigned short[len];
			iNeedLen1 = len;
		}
		else{
			wszGBK = (unsigned short*)tmp1;
		}
		if (wszGBK == NULL){
			return 0;
		}
		//memset(wszGBK, 0, len);
		MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)pSrc, iSrcLen, (LPWSTR)wszGBK, len);
		len = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
		if (len > cilen){
			szGBK = new char[len];
			iNeedLen2 = len;
		}
		else{
			szGBK = tmp2;
		}

		if (szGBK == NULL){
			return 0;
		}
		//memset(szGBK, 0, len);
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, szGBK, len, NULL, NULL);
		int tmp = min(iBuffLen - 1, len);
		memcpy(buff, szGBK, tmp);
		buff[tmp] = '\0';
		if (iNeedLen1 > cilen){
			delete[] wszGBK;
		}
		if (iNeedLen2 > cilen){
			delete[] szGBK;
		}
		return tmp;
#else
		int len = MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)pSrc, iSrcLen, NULL, 0);
		unsigned short * wszGBK = new unsigned short[len];
		if (wszGBK == NULL){
			return;
		}
		memset(wszGBK, 0, len);
		MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)pSrc, iSrcLen, (LPWSTR)wszGBK, len);
		len = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
		char *szGBK = new char[len];
		if (szGBK == NULL){
			return;
		}
		memset(szGBK, 0, len);
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, szGBK, len, NULL, NULL);
		if (len < 1024)
		{
			int tmp = min(iBuffLen - 1, len);
			strncpy_s(buff, iBuffLen, szGBK, tmp);
			buff[tmp] = '\0';
		}
		else {
			int tmp = min(iBuffLen - 1, len);
			memcpy(buff, szGBK, tmp);
			buff[tmp] = '\0';
		}

		delete[] szGBK;
		delete[] wszGBK;
#endif
	}

	char* WcharToChar(wchar_t* wc){
		int len = WideCharToMultiByte(CP_ACP, 0, wc, wcslen(wc), NULL, 0, NULL, NULL);
		char* p = new char[len + 1];
		if (nullptr == p) {
			return nullptr;
		}
		WideCharToMultiByte(CP_ACP, 0, wc, wcslen(wc), p, len, NULL, NULL);
		p[len] = '\0';
		return p;
	}
	wchar_t* CharToWchar(char* c){
		int len = MultiByteToWideChar(CP_ACP, 0, c, strlen(c), NULL, 0);
		wchar_t* p = new wchar_t[len + 1];
		if (nullptr == p) {
			return nullptr;
		}
		MultiByteToWideChar(CP_ACP, 0, c, strlen(c), p, len);
		p[len] = '\0';
		return p;
	}
	void WcharToChar2(wchar_t* wc, char* buff, int len){
		char _buff[1024];
		int _len = WideCharToMultiByte(CP_ACP, 0, wc, wcslen(wc), NULL, 0, NULL, NULL);
		if (_len >= 1024) {
			return;
		}
		WideCharToMultiByte(CP_ACP, 0, wc, wcslen(wc), _buff, 1024, NULL, NULL);
		_buff[_len] = '\0';
		strncpy_s(buff, sizeof(buff), _buff, min(sizeof(buff)-1, strlen(_buff)));
	}

	void CharToWchar2(char* c, wchar_t* buff, int len){
		wchar_t _buff[1024];
		int _len = MultiByteToWideChar(CP_ACP, 0, c, strlen(c), NULL, 0);
		if (_len >= 1024) {
			return;
		}
		MultiByteToWideChar(CP_ACP, 0, c, strlen(c), _buff, 1024);
		_buff[_len] = '\0';
		wcsncpy_s(buff, 1024, _buff, min((unsigned int)len, wcslen(_buff)));
	}

	int RunProcess(const char* cmd, PPROCESS_PROPERTY pProcessProperty /*= NULL*/, unsigned int uiTimeOut /*= 0*/, BOOL bShow /*= FALSE*/) {
		int ret = 0;
		if (cmd == NULL)
		{
			ret = -1;
			return ret;
		}
		STARTUPINFO	stStartInfo;
		PROCESS_INFORMATION	stProcessInfo = { 0 };

		memset(&stStartInfo, 0, sizeof(STARTUPINFO));
		stStartInfo.cb = sizeof(STARTUPINFO);
		stStartInfo.dwFlags = STARTF_USESHOWWINDOW;
		stStartInfo.wShowWindow = bShow ? SW_SHOW : SW_HIDE;
		BOOL bRet = CreateProcess(NULL, (char*)cmd, NULL, NULL, FALSE, 0, NULL, NULL, &stStartInfo, &stProcessInfo);
		if (stProcessInfo.hProcess == NULL)
		{
			ret = -2;
			return ret;
		}

		if (pProcessProperty != NULL) {
			pProcessProperty->hProcessHandle = stProcessInfo.hProcess;
			pProcessProperty->uiProcessID = stProcessInfo.dwProcessId;
		}

		if (uiTimeOut > 0)
		{
			DWORD dwTimeout = WaitForSingleObject(stProcessInfo.hProcess, uiTimeOut);
			if (dwTimeout == WAIT_TIMEOUT)
			{
				ret = -3;
				return ret;
			}
		}
		else {
			if (pProcessProperty == NULL) {
				CloseHandle(stProcessInfo.hProcess);
			}
		}
		return ret;
	}

	HANDLE EnableSubProcessAutoClose(const char* lpName)
	{
		//创建一个job内核对象  
		HANDLE hd = CreateJobObject(NULL, lpName);
		if (hd)
		{
			//设置job内核对象限制条件为：当job对象关闭的时候，关闭其所有子进程  
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION extLimitInfo;
			extLimitInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			BOOL retVal = SetInformationJobObject(hd,
				JobObjectExtendedLimitInformation,
				&extLimitInfo,
				sizeof(extLimitInfo));

			if (retVal)
			{
				return hd;
			}
			else{
				CloseHandle(hd);
			}
		}
		return NULL;
	}

	BOOL AddSubProcess(HANDLE hJob, HANDLE sub)
	{
		BOOL retVal = FALSE;
		//将进程加入到job容器中去。  
		if (hJob && sub)
		{
			retVal = AssignProcessToJobObject(hJob, sub);
		}
		return retVal;
	}

	HKEY CreateRegKey(HKEY hKey, const char* subkey, bool close /*= true*/) {
		HKEY hAppKey = NULL;
		DWORD dw = 0;
		REGSAM samDesiredOpen = KEY_ALL_ACCESS;

		if (NULL == subkey) {
			DBG_E;
			return NULL;
		}
		if (Is64()) {
			samDesiredOpen |= KEY_WOW64_32KEY;
		}
		if (RegCreateKeyEx(hKey, subkey, 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, samDesiredOpen, NULL, &hAppKey, &dw) == ERROR_SUCCESS)
		{
			if (close) {
				RegCloseKey(hAppKey);
				hAppKey = NULL;
			}
			return hAppKey;
			//dw =.
			//REG_CREATED_NEW_KEY，0x00000001L 该键是新创建的键
			//REG_OPENED_EXISTING_KEY，0x00000002L 该键是已经存在的键
		}
		return NULL;
	}
	int WriteRegString(HKEY hKey, const char* subkey, const char* value, const char* val, int len) {
		if (NULL == value) {
			DBG_E;
			return -1;
		}
		bool bClose = false;
		HKEY _hKey = NULL;
		REGSAM samDesiredOpen = KEY_WRITE;
		if (Is64()) {
			samDesiredOpen |= KEY_WOW64_32KEY;
		}
		if (subkey != NULL) {
			if (RegOpenKeyEx(hKey, subkey, 0, samDesiredOpen, &_hKey) != ERROR_SUCCESS) {
				return -2;
			}
			bClose = true;
		}
		else {
			_hKey = hKey;
		}
		if (_hKey == NULL) {
			DBG_E;
			return -2;
		}
		int lRetCode = RegSetValueEx(_hKey, value, 0, REG_SZ, (BYTE *)val, len);
		if (bClose) {
			RegCloseKey(_hKey);
		}
		return lRetCode == ERROR_SUCCESS ? 0 : -3;
	}

	int CloseRegKey(HKEY hKey) {
		if (hKey != NULL) {
			RegCloseKey(hKey);
		}
		return 0;
	}
	int WriteRegInt(HKEY hKey, const char* subkey, const char* value, int val) {
		HKEY _hKey = NULL;
		if (NULL == value) {
			DBG_E;
			return -1;
		}
		REGSAM samDesiredOpen = KEY_WRITE;
		if (Is64()) {
			samDesiredOpen |= KEY_WOW64_32KEY;
		}
		bool bClose = false;
		if (subkey != NULL) {
			if (RegOpenKeyEx(hKey, subkey, 0, samDesiredOpen, &_hKey) != ERROR_SUCCESS) {
				DBG_E;
				return -2;
			}
			bClose = true;
		}
		else {
			_hKey = hKey;
		}
		if (_hKey == NULL) {
			DBG_E;
			return -3;
		}
		int lRetCode = RegSetValueEx(_hKey, value, 0, REG_DWORD, (BYTE *)&val, sizeof(int));
		if (bClose) {
			RegCloseKey(_hKey);
		}
		return lRetCode == ERROR_SUCCESS ? 0 : -4;
	}

	int ReadRegString(HKEY hKey, const char* subkey, const char* value, char* buff, int len) {
		HKEY _hKey = NULL;
		DWORD DataType = REG_SZ, BuffLen = 1024;
		char str[1024];
		bool bClose = false;
		if ((NULL == value) || (NULL == buff)) {
			DBG_E;
			return -1;
		}

		REGSAM samDesiredOpen = KEY_READ;
		if (Is64()) {
			samDesiredOpen |= KEY_WOW64_32KEY;
		}

		if (subkey != NULL) {
			if (RegOpenKeyEx(hKey, subkey, 0, samDesiredOpen, &_hKey) != ERROR_SUCCESS) {
				DBG_E;
				return -2;
			}
			bClose = true;
		}
		else {
			_hKey = hKey;
		}
		if (_hKey == NULL){
			DBG_E;
			return -3;
		}

		str[0] = '\0';
		//RegQueryValueEx(_hKey, value, 0, &DataType, NULL, &BuffLen);在不知道长度的时候,可以先读取需要长度.
		int lRetCode = RegQueryValueEx(_hKey, value, 0, &DataType, (BYTE*)&str, &BuffLen);
		if (lRetCode == ERROR_SUCCESS) {
			strncpy_s(buff, len, str, min(len - 1, strlen(str)));
		}
		if (bClose) {
			RegCloseKey(_hKey);
		}
		return lRetCode == ERROR_SUCCESS ? 0 : -4;
	}

	int ReadRegInt(HKEY hKey, const char* subkey, const char* value, int& v) {
		HKEY _hKey = NULL;
		DWORD DataType = REG_DWORD, BuffLen = 4;
		bool bClose = false;
		if (NULL == value) {
			DBG_E;
			return -1;
		}
		REGSAM samDesiredOpen = KEY_READ;
		if (Is64()) {
			samDesiredOpen |= KEY_WOW64_32KEY;
			//samDesiredOpen |= KEY_WOW64_64KEY; 64.app
		}

		if (subkey != NULL) {
			if (RegOpenKeyEx(hKey, subkey, 0, samDesiredOpen, &_hKey) != ERROR_SUCCESS) {
				DBG_E;
				return -2;
			}
			bClose = true;
		}
		else {
			_hKey = hKey;
		}

		if (_hKey == NULL){
			DBG_E;
			return -3;
		}

		int lRetCode = RegQueryValueEx(_hKey, value, 0, &DataType, (BYTE*)&v, &BuffLen);
		if (bClose) {
			RegCloseKey(_hKey);
		}
		return lRetCode == ERROR_SUCCESS ? 0 : -4;
	}

	int EnumRegKey(HKEY key, const char* subkey, std::list<std::string>* pSubKey,
		std::list<std::string>* pValues) {
		char achKey[256];   // buffer for subkey name
		DWORD cbName; // size of name string 
		char achClass[MAX_PATH] = TEXT("");  // buffer for class name 
		DWORD cchClassName = MAX_PATH;  // size of class string 
		DWORD cSubKeys = 0; // number of subkeys 
		DWORD cbMaxSubKey; // longest subkey size 
		DWORD cchMaxClass; // longest class string 
		DWORD cValues; // number of values for key 
		DWORD cchMaxValue; // longest value name 
		DWORD cbMaxValueData; // longest value data 
		DWORD cbSecurityDescriptor; // size of security descriptor 
		FILETIME ftLastWriteTime; // last write time 

		DWORD i, retCode;

		char achValue[256];
		DWORD cchValue = 256;

		HKEY _hKey = NULL;
		REGSAM samDesiredOpen = KEY_READ;
		if (Is64()) {
			samDesiredOpen |= KEY_WOW64_32KEY;
			//samDesiredOpen |= KEY_WOW64_64KEY; 64.app
		}

		bool bClose = false;
		if (subkey != NULL) {
			if (RegOpenKeyEx(key, subkey, 0, samDesiredOpen, &_hKey) != ERROR_SUCCESS) {
				DBG_E;
				return -1;
			}
			bClose = true;
		}
		else {
			_hKey = key;
		}

		if (_hKey == NULL){
			DBG_E;
			return -2;
		}

		// Get the class name and the value count. 
		retCode = RegQueryInfoKey(
			_hKey, // key handle 
			achClass, // buffer for class name 
			&cchClassName, // size of class string 
			NULL, // reserved 
			&cSubKeys, // number of subkeys 
			&cbMaxSubKey, // longest subkey size 
			&cchMaxClass, // longest class string 
			&cValues, // number of values for this key 
			&cchMaxValue, // longest value name 
			&cbMaxValueData, // longest value data 
			&cbSecurityDescriptor, // security descriptor 
			&ftLastWriteTime); // last write time 
		if (retCode != ERROR_SUCCESS) {
			DBG_E;
			return -3;
		}
		// Enumerate the subkeys, until RegEnumKeyEx fails.
		if (cSubKeys){
			//printf("\nNumber of subkeys: %d\n", cSubKeys);

			if (pSubKey != nullptr) {

				for (i = 0; i < cSubKeys; i++)
				{
					cbName = 256;
					retCode = RegEnumKeyEx(_hKey, i,
						achKey,
						&cbName,
						NULL,
						NULL,
						NULL,
						&ftLastWriteTime);
					if (retCode == ERROR_SUCCESS)
					{
						pSubKey->emplace_back(achKey);
						//_tprintf(TEXT("(%d) %s\n"), i + 1, achKey);
					}
				}
			}
		}

		// Enumerate the key values. 

		if (cValues)
		{
			//printf("\nNumber of values: %d\n", cValues);

			if (nullptr != pValues) {
				for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
				{
					cchValue = 256;
					achValue[0] = '\0';
					retCode = RegEnumValue(_hKey, i,
						achValue,
						&cchValue,
						NULL,
						NULL,
						NULL,
						NULL);

					if (retCode == ERROR_SUCCESS)
					{
						pValues->emplace_back(achValue);
						//_tprintf(TEXT("(%d) %s\n"), i + 1, achValue);
					}
				}
			}
		}
		return 0;
	}

	BYTE GetBit(unsigned int dwValue, int iIndex)
	{
		unsigned int dwTemp = dwValue;
		dwTemp = dwTemp << (32 - iIndex);
		dwTemp = dwTemp >> 31;
		return (BYTE)dwTemp;
	}

	float ByteToFloat(BYTE* buff)
	{
		//得到三部分数，直接相乘
		int sign = 1;//为正数
		float exp = 0;//阶数
		float mantissa = 1;//尾数,默认为1
		unsigned int dwVal = 0;
		memcpy(&dwVal, buff, sizeof(unsigned int));
		if (dwVal == 0){
			return 0.0;
		}

		// 求符号位
		if (GetBit(dwVal, 32) == 0)
			sign = 1;
		else
			sign = -1;

		//求阶码
		int iPar = 0;
		unsigned int dwPar = 0;
		dwPar = dwVal;
		dwPar <<= 1;
		dwPar >>= 24;
		exp = (float)(dwPar - 127);
		exp = (float)pow(2, exp);

		//求尾码
		for (int i = 1; i < 24; i++)
		{
			mantissa += (float)(GetBit(dwVal, i)* pow(2, 0 - (24 - i)));
		}
		return sign * exp * mantissa;
	}
}