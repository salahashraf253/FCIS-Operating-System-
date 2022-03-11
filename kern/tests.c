#include <kern/tests.h>

//define the white-space symbols
#define WHITESPACE "\t\r\n "

void TestAssignment1()
{
	cprintf("\n========================\n");
	cprintf("Automatic Testing of Q1:\n");
	cprintf("========================\n");
	TestAss1Q1();
	cprintf("\n========================\n");
	cprintf("Automatic Testing of Q2:\n");
	cprintf("========================\n");
	TestAss1Q2();
}
int TestAss1Q1()
{
	//case1:
	char *args[MAX_ARGUMENTS] ;
	char cr1[100] = "bytes 5 m 3 K 8 b";
	int numOfArgs = 0;
	strsplit(cr1, WHITESPACE, args, &numOfArgs) ;
	int ret = bytes(numOfArgs, args) ;
	if (ret != 5245960 )
	{
		cprintf("[EVAL] bytes-CASE1: Failed\n");
		return ret;
	}

	//case2:
	char cr2[100] = "bytes 3 k 8 B 5 M";
	numOfArgs = 0;
	strsplit(cr2, WHITESPACE, args, &numOfArgs) ;
	ret = bytes(numOfArgs, args) ;
	if (ret != 5245960 )
	{
		cprintf("[EVAL] bytes-CASE2: Failed\n");
		return ret;
	}

	//case3:
	char cr3[100] = "bytes 9 b 7 k 1 M 6 B 4 b 9 K";
	numOfArgs = 0;
	strsplit(cr3, WHITESPACE, args, &numOfArgs) ;
	ret = bytes(numOfArgs, args) ;
	if (ret != 1064979 )
	{
		cprintf("[EVAL] bytes-CASE3: Failed\n");
		return ret;
	}

	//case4:
	char cr4[100] = "bytes 5 B 3 m";
	numOfArgs = 0;
	strsplit(cr4, WHITESPACE, args, &numOfArgs) ;
	ret = bytes(numOfArgs, args) ;
	if (ret != 3145733 )
	{
		cprintf("[EVAL] bytes-CASE4: Failed\n");
		return ret;
	}

	//case5:
	char cr5[100] = "bytes 3 m 5";
	numOfArgs = 0;
	strsplit(cr5, WHITESPACE, args, &numOfArgs) ;
	ret = bytes(numOfArgs, args) ;
	if (ret != -1 )
	{
		cprintf("[EVAL] bytes-CASE5: Failed\n");
		return ret;
	}

	cprintf("[EVAL] bytes: Succeeded\n");
	return 0;
}
int TestAss1Q2()
{
	//case1:
	char *args[MAX_ARGUMENTS] ;
	char cr1[100] = "fmc basic_phrase_for_learning_english";
	int numOfArgs = 0;
	strsplit(cr1, WHITESPACE, args, &numOfArgs) ;
	int ret = frequency_max_character(numOfArgs, args) ;
	if (ret != 4)
	{
		cprintf("[EVAL] Fmc-CASE1: Failed\n");
		return ret;
	}

	//case2:
	char cr2[100] = "fmc os_is_interface_between_software_and_hardware";
	numOfArgs = 0;
	strsplit(cr2, WHITESPACE, args, &numOfArgs) ;
	ret = frequency_max_character(numOfArgs, args) ;
	if (ret != 7)
	{
		cprintf("[EVAL] Fmc-CASE2: Failed\n");
		return ret;
	}

	//case3:
	char cr3[100] = "fmc maximum_number";
	numOfArgs = 0;
	strsplit(cr3, WHITESPACE, args, &numOfArgs) ;
	ret = frequency_max_character(numOfArgs, args) ;
	if (ret != 4)
	{
		cprintf("[EVAL] Fmc-CASE3: Failed\n");
		return ret;
	}

	//case4:
	char cr4[100] = "fmc hellohellohellohello";
	numOfArgs = 0;
	strsplit(cr4, WHITESPACE, args, &numOfArgs) ;
	ret = frequency_max_character(numOfArgs, args) ;
	if (ret != 8)
	{
		cprintf("[EVAL] Fmc-CASE4: Failed\n");
		return ret;
	}

	//case5:
	char cr5[100] = "fmc oppo";
	numOfArgs = 0;
	strsplit(cr5, WHITESPACE, args, &numOfArgs) ;
	ret = frequency_max_character(numOfArgs, args) ;
	if (ret != 2)
	{
		cprintf("[EVAL] Fmc-CASE5: Failed\n");
		return ret;
	}

	//case6:
	//char cr6[100] = "focc defenseless";
	char cr6[100] = "fmc defenseless";
	numOfArgs = 0;
	strsplit(cr6, WHITESPACE, args, &numOfArgs) ;
	ret = frequency_max_character(numOfArgs, args) ;
	if (ret != 4)
	{
		cprintf("[EVAL] Fmc-CASE6: Failed\n");
		return ret;
	}

	//case7:
	//char cr7[100] = "focc pppppppppppppppp";
	char cr7[100] = "fmc pppppppppppppppp";
	numOfArgs = 0;
	strsplit(cr7, WHITESPACE, args, &numOfArgs) ;
	ret = frequency_max_character(numOfArgs, args) ;
	if (ret != 16)
	{
		cprintf("[EVAL] Fmc-CASE7: Failed\n");
		return ret;
	}

	cprintf("[EVAL] Fmc: Succeeded\n");
	return 0;
}
