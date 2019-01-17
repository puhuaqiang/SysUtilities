#include "../SysUtilities/stdafx.h"
#include "../include/api.h"
#include <random>
#include <time.h>

namespace SYS_UTL
{
	int Random(int start, int end)
	{
		static std::default_random_engine generator((int)time(0));
		static std::uniform_int_distribution<int> distribution(start, end);
		int dice_roll = distribution(generator);
		return dice_roll;
	}
}