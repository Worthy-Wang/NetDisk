#include "function.h"

//产生随机Salt
string GenerateStr(int STR_LEN)
{
	string str;
	int i, flag;

	srand(time(NULL)); //通过时间函数设置随机数种子，使得每次运行结果随机。
	for (i = 0; i < STR_LEN; i++)
	{
		flag = rand() % 3;
		switch (flag)
		{
		case 0:
			str += rand() % 26 + 'a';
			break;
		case 1:
			str += rand() % 26 + 'A';
			break;
		case 2:
			str += rand() % 10 + '0';
			break;
		}
	}
	return str;
}