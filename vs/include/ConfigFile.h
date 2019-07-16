#ifndef __SYS_UTILITIES_CONFIGFILE_H__
#define __SYS_UTILITIES_CONFIGFILE_H__

#include <list>
#include <map>
namespace SYS_UTL{
/**
\brief 操作配置文件. Window(INI)
*/
class SYS_UTL_CPPAPI ConfigFile
{
public:
	/**
	\brief 构造函数,
	*/
	ConfigFile();
	/**
	\brief 构造函数
	\param path 文件名,全路径
	*/
	ConfigFile(const char* path);
	~ConfigFile();

	/**
	\brief 读取整个配置文件到内存中.
	\return Error.h
	*/
	void SetConfigFile(const char* path);

	/**
	\brief 读取整个配置文件到内存中.
	\return Error.h
	*/
	const char* GetConfigFile() const { return szConfigFile; };

	/**
	\brief 读取整个配置文件到内存中.
	\return Error.h
	*/
	static int ReadSectionData(const char* lpszFileName,
		const char* lpSection,
		std::list<std::pair<std::string, std::string>>& lstContent);
	int ReadSectionData(const char* lpSection,
		std::list<std::pair<std::string, std::string>>& lstContent){
		return ConfigFile::ReadSectionData(this->GetConfigFile(), lpSection, lstContent);
	}

	/**
	\brief 读取整个配置文件到内存中.
	\return Error.h
	*/
	static int ReadData(const char* lpszFileName,
		std::list<std::map<std::string, std::list<std::pair<std::string, std::string>>>>& lstContent);
	int ReadData(std::list<std::map<std::string, std::list<std::pair<std::string, std::string>>>>& lstContent){
		return ConfigFile::ReadData(this->GetConfigFile(), lstContent);
	}

	/**
	\brief 读取字符串.
	\param lpszFileName 配置文件全路径.
	\param lpszSection 小节.
	\return 存在返回TRUE，否则FALSE
	*/
	static int ReadStr(const char* lpszFileName,
		const char* lpszSection,
		const char* lpszValueName,
		char* lpszValue,
		int iValueBuffLen,
		char* lpszDefaultValue = NULL);
	int ReadStr(const char* lpszSection,
		const char* lpszValueName,
		char* lpszValue,
		int iValueBuffLen,
		char* lpszDefaultValue = NULL){
		return ConfigFile::ReadStr(this->GetConfigFile(), lpszSection, lpszValueName, lpszValue, iValueBuffLen, lpszDefaultValue);
	}

	/**
	\brief 读取整型值.
	\param lpszFileName 配置文件全路径.
	\param lpszSection 小节.
	\return 存在返回TRUE，否则FALSE
	*/
	static int ReadInt(const char* lpszFileName,
		const char* lpszSection,
		const char* lpszValueName,
		int& iValue,
		int iDefaultValue = 0);
	int ReadInt(const char* lpszSection,
		const char* lpszValueName,
		int& iValue,
		int iDefaultValue = 0){
		return ConfigFile::ReadInt(this->GetConfigFile(), lpszSection, lpszValueName, iValue, iDefaultValue);
	}

	/**
	\brief 写整型值.
	\param lpszFileName 配置文件全路径.
	\param lpszSection 小节.
	\return 存在返回TRUE，否则FALSE
	*/
	static int WriteStr(const char* lpszFileName,
		const char* lpszSection,
		const char* lpszValueName,
		const char* lpszValue);
	int WriteStr(const char* lpszSection,
		const char* lpszValueName,
		const char* lpszValue){
		return ConfigFile::WriteStr(this->GetConfigFile(), lpszSection, lpszValueName, lpszValue);
	}

	/**
	\brief 写整型值.
	\param lpszFileName 配置文件全路径.
	\param lpszSection 小节.
	\return 存在返回TRUE，否则FALSE
	*/
	static int WriteInt(const char* lpszFileName,
		const char* lpszSection,
		const char* lpszValueName,
		int iValue);
	int WriteInt(const char* lpszSection,
		const char* lpszValueName,
		int iValue){
		return ConfigFile::WriteInt(this->GetConfigFile(), lpszSection, lpszValueName, iValue);
	}

	/**
	\brief 删除关键字.
	\param lpszFileName 配置文件全路径.
	\param lpszSection 小节.
	\return 存在返回TRUE，否则FALSE
	*/
	static int DeleteKey(const char* lpszFileName,
		const char* lpszSection,
		const char* lpszKeyName);
	int DeleteKey(const char* lpszSection,
		const char* lpszKeyName){
		return ConfigFile::DeleteKey(this->GetConfigFile(), lpszSection, lpszKeyName);
	}

	/**
	\brief 删除小节.
	\param lpszFileName 配置文件全路径.
	\param lpszSection 小节.
	\return 存在返回TRUE，否则FALSE
	*/
	static int DeleteNode(const char* lpszFileName,
		const char* lpszSection);
	int DeleteNode(const char* lpszSection){
		return ConfigFile::DeleteNode(this->GetConfigFile(), lpszSection);
	}

	/**
	\brief 删除小节.
	\param lpszFileName 配置文件全路径.
	\param lpszSection 小节.
	\return 存在返回TRUE，否则FALSE
	*/
	static int Clear(const char* lpszFileName);
	int Clear(){
		return ConfigFile::Clear(this->GetConfigFile());
	}

	operator bool()
	{
		return strlen(this->szConfigFile) > 0;
	}

private:
	ConfigFile(ConfigFile const & rhs);
	ConfigFile& operator= (ConfigFile const& rhs);
private:
	//const int iPathLen;
	/// 文件
	char szConfigFile[256];
};
} //namespace SYS_UTL
#endif