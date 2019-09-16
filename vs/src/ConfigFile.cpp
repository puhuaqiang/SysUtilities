#include "../SysUtilities/stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <map>
#include <assert.h>
#include <sys/stat.h>
#include "../include/api.h"
#include "../include/ConfigFile.h"
#include "InDef.h"

using namespace SYS_UTL;
ConfigFile::ConfigFile()
{
	memset(szConfigFile, 0, sizeof(szConfigFile));
}

ConfigFile::ConfigFile(const char* path)
{
	memset(szConfigFile, 0, sizeof(szConfigFile));
	if (NULL != path)
	{
		strncpy_s(szConfigFile, sizeof(szConfigFile), path, min(sizeof(szConfigFile)-1, strlen(path)));
	}
}
ConfigFile::~ConfigFile()
{

}

void ConfigFile::SetConfigFile(const char* path)
{
	if (NULL == path)
	{
		return;
	}
	strncpy_s(szConfigFile, sizeof(szConfigFile), path, min(sizeof(szConfigFile)-1, strlen(path)));
}

int ConfigFile::ReadSectionData(const char* lpszFileName, const char* lpSection, std::list<std::pair<std::string, std::string>>& lstContent)
{
#ifdef _DEBUG
	assert(lpszFileName != NULL && strlen(lpszFileName) > 0 && (NULL != lpSection) && (strlen(lpSection) > 0));
#else
	if (lpszFileName == NULL || strlen(lpszFileName) <= 0)
	{
		DBG_E;
		return -1;
	}
	if (lpSection == NULL || strlen(lpSection) <= 0)
	{
		DBG_E;
		return -1;
	}
#endif
	char* pAppData = NULL;
	int iAppNameLen = 0, iAppDataLen = 0, iKeyNameLen = 0;
	char szReturn[1024*8], szSection[1024 * 32];
	int iTmp = 0;

	if (!IsPathExists(lpszFileName))
	{
		DBG_E;
		return -1;
	}

	lstContent.clear();

	struct _stat64	st;
	if (_stat64(lpszFileName, &st) == 0)
	{
		iTmp = (int)st.st_size;
	}

	if (iTmp <= 0)
	{
		DBG_E;
		return -1;
	}
	iTmp = PAD_SIZE(iTmp);
	pAppData = (char*)new BYTE[iTmp];

	//如果此参数为NULL,将配置文件中的所有节点名称复制到lpReturnedString参数指定的缓冲区
	iAppDataLen = GetPrivateProfileString(NULL, NULL, NULL, pAppData, iTmp, lpszFileName);
	if (iAppDataLen <= 0) {
		DBG_E;
		return -1;
	}
	char* pAppName = new char[iAppDataLen];

	if (pAppName == NULL) {
		DBG_E;
		return -1;
	}
	iAppNameLen = 0;

	for (int i = 0; i < iAppDataLen; i++)
	{
		pAppName[iAppNameLen++] = pAppData[i];//所有节点名称

		if (pAppData[i] != '\0')
		{
			continue;
		}
		//如果此参数为NULL,将配置文件中的指定的节点下的关键字复制到lpReturnedString参数指定的缓冲区
		iKeyNameLen = GetPrivateProfileString(pAppName, NULL, NULL, szSection, 1024 * 32, lpszFileName);
		if (iKeyNameLen <= 0)
		{
			//memset(pAppName, 0, iAppDataLen);
			iAppNameLen = 0;
			continue;
		}

		if (strcmp(pAppName, lpSection))
		{
			//memset(pAppName, 0, iAppDataLen);
			iAppNameLen = 0;
			continue;
		}

		char* pKeyName = new char[iKeyNameLen];
		int nKeyNameLen = 0; //每个KeyName的长度
		//ZeroMemory(pKeyName, sizeof(TCHAR)*dwKeyNameSize);

		for (int j = 0; j < iKeyNameLen; j++)
		{
			pKeyName[nKeyNameLen++] = szSection[j];//所有关键字

			if (szSection[j] != '\0')
			{
				continue;
			}
			if (GetPrivateProfileString(pAppName, pKeyName, NULL, szReturn, 1024 * 8, lpszFileName))
			{
				lstContent.emplace_back(std::pair<std::string, std::string>(pKeyName, szReturn));
			}
			memset(pKeyName, 0, iKeyNameLen);
			nKeyNameLen = 0;
		}

		delete[] pKeyName;
		memset(pAppName, 0, iAppDataLen);
		iAppNameLen = 0;

		break;
	}
	delete[] pAppName;
	return 0;
}

int ConfigFile::ReadData(const char* lpszFileName,
	std::list<std::map<std::string, std::list<std::pair<std::string, std::string>>>>& lstContent)
{
#ifdef _DEBUG
		assert(lpszFileName != NULL && strlen(lpszFileName) > 0);
#else
	if (lpszFileName == NULL || strlen(lpszFileName) <= 0)
	{
		DBG_E;
		return -1;
	}
#endif
	char* pAppData = NULL;
	int iAppNameLen = 0, iAppDataLen = 0, iKeyNameLen = 0;
	char szReturn[1024 * 8], szSection[1024 * 32];
	int iTmp = 0;

	if (!IsPathExists(lpszFileName))
	{
		DBG_E;
		return -1;
	}

	lstContent.clear();

	struct _stat64	st;
	if (_stat64(lpszFileName, &st) == 0) 
	{
		iTmp = (int)st.st_size;
	}

	if (iTmp <= 0)
	{
		DBG_E;
		return -1;
	}
	iTmp = PAD_SIZE(iTmp);
	pAppData = (char*)new BYTE[iTmp];

	//如果此参数为NULL,将配置文件中的所有节点名称复制到lpReturnedString参数指定的缓冲区
	iAppDataLen = GetPrivateProfileString(NULL, NULL, NULL, pAppData, iTmp, lpszFileName);
	if (iAppDataLen <= 0) {
		DBG_E;
		return -1;
	}
	char* pAppName = new char[iAppDataLen];

	if (pAppName == NULL) {
		DBG_E;
		return -1;
	}
	iAppNameLen = 0;

	for (int i = 0; i < iAppDataLen; i++)
	{
		pAppName[iAppNameLen++] = pAppData[i];

		if (pAppData[i] != '\0')
		{
			continue;
		}
		//如果此参数为NULL,将配置文件中的指定的节点下的关键字复制到lpReturnedString参数指定的缓冲区
		iKeyNameLen = GetPrivateProfileString(pAppName, NULL, NULL, szSection, 1024 * 32, lpszFileName);
		if (iKeyNameLen <= 0)
		{
			//memset(pAppName, 0, iAppDataLen);
			iAppNameLen = 0;
			continue;
		}

		std::map<std::string, std::list<std::pair<std::string, std::string>>> mapSection;
		std::list<std::pair<std::string, std::string>> lstKeys;
		auto retIter = mapSection.emplace(pAppName, lstKeys);
		if (!retIter.second)
		{
			//memset(pAppName, 0, iAppDataLen);
			iAppNameLen = 0;
			continue;
		}

		char* pKeyName = new char[iKeyNameLen];
		int nKeyNameLen = 0; //每个KeyName的长度
		//ZeroMemory(pKeyName, sizeof(TCHAR)*dwKeyNameSize);

		for (int j = 0; j < iKeyNameLen; j++)
		{
			pKeyName[nKeyNameLen++] = szSection[j];

			if (szSection[j] != '\0')
			{
				continue;
			}
			if (GetPrivateProfileString(pAppName, pKeyName, NULL, szReturn, 1024 * 8, lpszFileName))
			{
				retIter.first->second.emplace(retIter.first->second.end(),
					std::pair<std::string, std::string>(pKeyName, szReturn));
			}
			memset(pKeyName, 0, iKeyNameLen);
			nKeyNameLen = 0;
		}

		delete[] pKeyName;
		memset(pAppName, 0, iAppDataLen);
		iAppNameLen = 0;

		lstContent.emplace(lstContent.end(), mapSection);
	}
	delete[] pAppName;
	return 0;
}

int ConfigFile::ReadStr(const char* lpszFileName,
	const char* lpszSection,
	const char* lpszValueName,
	char* lpszValue,
	int iValueBuffLen,
	char* lpszDefaultValue /*= NULL*/)
{
	if (NULL == lpszValue || iValueBuffLen <= 0 || NULL == lpszFileName || NULL == lpszSection || NULL == lpszValueName)
	{
		DBG_E;
		return -1;
	}
	if (!IsPathExists(lpszFileName))
	{
		DBG_I("文件:%s,不存在", lpszFileName);
		return -1;
	}
	::GetPrivateProfileString(lpszSection, lpszValueName, lpszDefaultValue == NULL ? "" : lpszDefaultValue, lpszValue, iValueBuffLen, lpszFileName);
	return 0;
}

int ConfigFile::ReadInt(const char* lpszFileName,
	const char* lpszSection,
	const char* lpszValueName,
	int& iValue,
	int iDefaultValue /*= 0*/)
{
	if (NULL == lpszFileName || NULL == lpszSection || NULL == lpszValueName)
	{
		DBG_E;
		return -1;
	}
	if (!IsPathExists(lpszFileName))
	{
		DBG_I("文件:%s,不存在", lpszFileName);
		return -1;
	}
	iValue = ::GetPrivateProfileInt(lpszSection, lpszValueName, iDefaultValue, lpszFileName);
	return 0;
}

int ConfigFile::WriteStr(const char* lpszFileName,
	const char* lpszSection,
	const char* lpszValueName,
	const char* lpszValue)
{
	if (NULL == lpszFileName)
	{
		DBG_E;
		return -1;
	}
	/*if (!IsPathExists(lpszFileName))
	{
		DBG_E;
		return -1;
	}*/
	::WritePrivateProfileString(lpszSection, lpszValueName, lpszValue, lpszFileName);
	return 0;
}

int ConfigFile::WriteInt(const char* lpszFileName,
	const char* lpszSection,
	const char* lpszValueName,
	int iValue)
{
	if (NULL == lpszFileName)
	{
		DBG_E;
		return -1;
	}
	/*if (!IsPathExists(lpszFileName))
	{
		DBG_E;
		return -1;
	}*/
	char szValue[32] = { 0 };
	_snprintf_s(szValue, _TRUNCATE, "%d", iValue);
	::WritePrivateProfileString(lpszSection, lpszValueName, szValue, lpszFileName);
	return 0;
}

int ConfigFile::DeleteKey(const char* lpszFileName,
	const char* lpszSection,
	const char* lpszKeyName)
{
	if (NULL == lpszFileName || NULL == lpszSection || NULL == lpszKeyName)
	{
		DBG_E;
		return -1;
	}
	if (!IsPathExists(lpszFileName))
	{
		DBG_E;
		return -1;
	}
	::WritePrivateProfileString(lpszSection, lpszKeyName, NULL, lpszFileName);
	return 0;
}

int ConfigFile::DeleteNode(const char* lpszFileName,
	const char* lpszSection)
{
	if (NULL == lpszFileName || NULL == lpszSection)
	{
		DBG_E;
		return -1;
	}
	if (!IsPathExists(lpszFileName))
	{
		DBG_E;
		return -1;
	}
	::WritePrivateProfileString(lpszSection, NULL, NULL, lpszFileName);
	return 0;
}

int ConfigFile::Clear(const char* lpszFileName)
{
	std::list<std::map<std::string, std::list<std::pair<std::string, std::string>>>> lstContent;
	int ret = ConfigFile::ReadData(lpszFileName, lstContent);
	if (ret != 0)
	{
		return ret;
	}
	for (auto& it : lstContent)
	{
		for (auto& itNode : it)
		{
			ConfigFile::DeleteNode(lpszFileName, itNode.first.data());
		}
	}
	return 0;
}