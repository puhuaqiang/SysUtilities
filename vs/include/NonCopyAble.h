#ifndef __SYS_UTILITIES_NONCOPYABLE_H__
#define __SYS_UTILITIES_NONCOPYABLE_H__

#include "SysUtilities.h"

namespace SYS_UTL
{
	class SYS_UTL_CPPAPI CNonCopyAble
	{
	public:
		CNonCopyAble(const CNonCopyAble&) = delete;
		void operator=(const CNonCopyAble&) = delete;

	protected:
		CNonCopyAble() = default;
		~CNonCopyAble() = default;
	};
}

#endif	//__AUTOLOCK_H__
