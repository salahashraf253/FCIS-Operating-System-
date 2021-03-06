/*	Simple command-line kernel prompt useful for
	controlling the kernel and exploring the system interactively.


KEY WORDS
==========
CONSTANTS:	WHITESPACE, NUM_OF_COMMANDS
VARIABLES:	Command, commands, name, description, function_to_execute, number_of_arguments, arguments, command_string, command_line, command_found
FUNCTIONS:	readline, cprintf, execute_command, run_command_prompt, command_kernel_info, command_help, strcmp, strsplit, start_of_kernel, start_of_uninitialized_data_section, end_of_kernel_code_section, end_of_kernel
=====================================================================================================================================================================================================
 */


#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>


#include <kern/console.h>
#include <kern/command_prompt.h>
#include <kern/memory_manager.h>
#include <kern/trap.h>
#include <kern/kdebug.h>
#include <kern/user_environment.h>
#include <kern/tests.h>

//TODO:LAB3.Hands-on: declare start address variable of "My int array"

//=============================================================

//Structure for each command
struct Command
{
	char *name;
	char *description;
	// return -1 to force command prompt to exit
	int (*function_to_execute)(int number_of_arguments, char** arguments);
};

//Functions Declaration
int execute_command(char *command_string);
int command_writemem(int number_of_arguments, char **arguments);
int command_readmem(int number_of_arguments, char **arguments);
int command_meminfo(int , char **);

//Lab2.Hands.On
//=============
//TODO: LAB2 Hands-on: declare the command function here


//LAB3.Examples
//=============
int command_kernel_base_info(int , char **);
int command_del_kernel_base(int , char **);
int command_share_page(int , char **);

//Lab4.Hands.On
//=============
int command_show_mapping(int number_of_arguments, char **arguments);
int command_set_permission(int number_of_arguments, char **arguments);
int command_share_range(int number_of_arguments, char **arguments);

//Lab5.Examples
//=============
int command_nr(int number_of_arguments, char **arguments);
int command_ap(int , char **);
int command_fp(int , char **);

//Lab5.Hands-on
//=============
int command_asp(int, char **);
int command_cfp(int, char **);

//Lab6.Examples
//=============
int command_run(int , char **);
int command_kill(int , char **);
int command_ft(int , char **);


//Array of commands. (initialized)
struct Command commands[] =
{
		{ "help", "Display this list of commands", command_help },
		{ "kernel_info", "Display information about the kernel", command_kernel_info },
		{ "wum", "writes one byte to specific location" ,command_writemem},
		{ "rum", "reads one byte from specific location" ,command_readmem},
		{ "meminfo", "Display number of free frames", command_meminfo},

		//LAB2 : Examples
		{"wum","write 1 byte in the memory at given address",command_writeMemory},
		{"rum","read 1 byte from the memory at given virtual address",command_readMemory},
		//TODO: LAB2 Hands-on: add the commands here
		{"read_block","reads the <N> bytes starting from <virtual address> and display them",
				command_read_block},
		{"create_int_array","creates an integer array with the given <array size>, display its start virtual address and allow the user to initialize its elements",
				command_create_int_array},

		//LAB3: Examples
		{ "ikb", "Lab3.Example: shows mapping info of KERNEL_BASE" ,command_kernel_base_info},
		{ "dkb", "Lab3.Example: delete the mapping of KERNEL_BASE" ,command_del_kernel_base},
		{ "shr", "Lab3.Example: share one page on another" ,command_share_page},

		//LAB4: Hands-on
		{ "sm", "Lab4.HandsOn: display the mapping info for the given virtual address", command_show_mapping},
		{ "sp", "Lab4.HandsOn: set the desired permission to a given virtual address page", command_set_permission},
		{ "sr", "Lab4.HandsOn: shares the physical frames of the first virtual range with the 2nd virtual range", command_share_range},

		//LAB5: Examples
		{ "nr", "Lab5.Example: show the number of references of the physical frame" ,command_nr},
		{ "ap", "Lab5.Example: allocate one page [if not exists] in the user space at the given virtual address", command_ap},
		{ "fp", "Lab5.Example: free one page in the user space at the given virtual address", command_fp},

		//LAB5: Hands-on
		{ "asp", "Lab5.HandsOn: allocate 2 shared pages with the given virtual addresses" ,command_asp},
		{ "cfp", "Lab5.HandsOn: count the number of free pages in the given range", command_cfp},

		//LAB6: Examples
		{ "ft", "Lab6.Example: Free table", command_ft},
		{ "run", "Lab6.Example: Load and Run User Program", command_run},
		{ "kill", "Lab6.Example: Kill User Program", command_kill},
		{ "ver", "FOS kernel version 0.1", command_ver},
		{ "add", "Add two numbers", command_add},

		//Assignment1 commands
		//====================
		{ "bytes", "Calculate the equivalent number of bytes to the input operation", command_bytes},
		{ "ones", "calculate the ones complement of the given binary string",command_frequency_max_character},
		//Assignment1.BONUS command
		//=========================
		{ "xcs", "Execute set of commands terminated by $",command_xcs}, //need no arguments
};

//Number of commands = size of the array / size of command structure
#define NUM_OF_COMMANDS (sizeof(commands)/sizeof(commands[0]))

// ------------------------------------------------------------
// ------------------------------------------------------------
#define BUFLEN 1024

int min(int x,int y){
	if(x==y)return x;
	else if(x<y)return x;
	else return y;
}
int abs(int x){
	if(x>0)
		return x;
	else return x*-1;
}
int getMin(int x,int y,int z){
	return min(min(x,y),z);
}
//dp code
int editDistanceUsingDP(char *s1,char *s2,int m){

	//m is the size of string1
	int n= strlen(s2);
	int dp[m+1][n+1];
	for(int i=0;i<=m;i++)
	{
		for(int j=0;j<=n;j++){
			if(i==0){
				dp[i][j]=j;
			}
			else if(j==0){
				dp[i][j]=1;
			}
            else if (s1[i - 1] == s2[j - 1]){
            	dp[i][j] = dp[i - 1][j - 1];
            }
            else {
                   dp[i][j]= 1 + getMin(dp[i][j - 1], dp[i - 1][j],dp[i - 1][j - 1]);
            }
		}
	}
	return dp[m][n];
}

char* getTheNearestCommand(char commandName[],int size){
	int mn=150;
	int commandNameLength=size;
	int numberOfEdits,difference;
	char *nearestCommand=commandName;
	for (int i = 0; i < NUM_OF_COMMANDS; i++)
	{
		difference = abs(commandNameLength-strlen(commands[i].name));
		if(difference == 0 || difference == 1 || difference == 2){
		    numberOfEdits=editDistanceUsingDP(commandName , commands[i].name,size);
			if(numberOfEdits<mn)
			{
				mn=numberOfEdits;
				nearestCommand=commands[i].name;
			}
		}
	}
	commandName=nearestCommand;
	return commandName;
} //end of DP code
int getCommandIndex(char *commandName)
{
	//check if the written command is a valid command or no
	for(int i=0;i<NUM_OF_COMMANDS;i++){
		if(strcmp(commandName,commands[i].name)==0){
			return i;
		}
	}
	return -1; // command is not found
}

void modifiedReadLine(const char *prompt,char *buf)
{
	//this function is challenge 1 & made by Salah Ashraf ASU 2023
	//this function is responsible for auto correct command by
	//using edit distance , it works when you press space after command name directly
	int i=0,c,echoing ;
	int ctr=0;
	int lengthOfCommand=0;
	if(prompt!=NULL){
		cprintf("%s",prompt);
	}

	echoing =iscons(0);
	bool foundFirstSpace=0;
	while(1){
		c=getchar();
		if (c < 0){
			if (c != -E_EOF){
				cprintf("read error: %e\n", c);
			}
			return;
		}
		else if(c==' ' && foundFirstSpace == 0){
			foundFirstSpace=1;
			if(getCommandIndex(buf) == -1)
			{
				char *arr=getTheNearestCommand(buf,i);

				//delete the wrong command name
				for(int j=0;j<i;j++){
					cputchar('\b');
				}
			    int lengthOfRightCommand=strlen(arr);
				//write in the screen the right command name
			    for(int j=0;j<lengthOfRightCommand;j++){
						cputchar(arr[j]);
						buf[j]=arr[j];
				}
				lengthOfCommand=lengthOfRightCommand;
				i=lengthOfRightCommand;
			}
			char space=' ';
			cputchar(' ');
			buf[i++] = ' ';
		}
		else if (c >= ' ' && i < BUFLEN-1)
		{
			if (echoing)
				cputchar(c);
			buf[i++] = c;
		}
		else if (c == '\b' && i > 0)
		{
			if(lengthOfCommand==i-1){
				foundFirstSpace=0;
			}
			if (echoing)
				cputchar(c);
			i--;
		}
		else if (c == '\n' || c == '\r')
		{
			if (echoing)
				cputchar(c);
			buf[i] = 0;
			return;
		}
	}
}
// ------------------------------------------------------------
// ------------------------------------------------------------

//-------------------------------------------------Main Function----------------------------------
//invoke the command prompt
void run_command_prompt()
{
	//CAUTION: DON'T CHANGE OR COMMENT THIS LINE======
	//TestAssignment1();
	//================================================

	char command_line[1024];
	while (1==1)
	{
		//get command line
		//readline("FOS> ", command_line);

		//this function is challenge 1 & made by me
		//cprintf("CommandLine : %s\n",command_line);

		modifiedReadLine("FOS> ", command_line);

		//cprintf("Command Line : %s \n",command_line);

		//parse and execute the command
		if (command_line != NULL)
			if (execute_command(command_line) < 0)
				break;
	}
}

/***** Kernel command prompt command interpreter *****/

//define the white-space symbols
#define WHITESPACE "\t\r\n "

//Function to parse any command and execute it
//(simply by calling its corresponding function)
int execute_command(char *command_string)
{
	// Split the command string into whitespace-separated arguments
	int number_of_arguments;
	//allocate array of char * of size MAX_ARGUMENTS = 16 found in string.h
	char *arguments[MAX_ARGUMENTS];


	strsplit(command_string, WHITESPACE, arguments, &number_of_arguments) ;
	if (number_of_arguments == 0)
		return 0;

	// Lookup in the commands array and execute the command
	int commandIndex = getCommandIndex(arguments[0]);
//	int i ;
//	for (i = 0; i < NUM_OF_COMMANDS; i++)
//	{
//		if (strcmp(arguments[0], commands[i].name) == 0)
//		{
//			command_found = 1;
//			break;
//		}
//	}

	if(commandIndex != -1)
	{
		int return_value;
		return_value = commands[commandIndex].function_to_execute(number_of_arguments, arguments);
		return return_value;
	}
	else
	{
		//if not found, then it's unknown command
		cprintf("Unknown command '%s'\n", arguments[0]);
		return 0;
	}
}

/***** Implementations of basic kernel command prompt commands *****/

//print name and description of each command
int command_help(int number_of_arguments, char **arguments)
{
	int i;
	for (i = 0; i < NUM_OF_COMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].description);

	cprintf("-------------------\n");

	return 0;
}

int command_ver(int number_of_arguments, char **arguments)
{
	cprintf("FOS version 1\n");
	return 0;
}

int command_add(int number_of_arguments, char **arguments)
{
	uint32 n1 = strtol(arguments[1], NULL, 10);
	uint32 n2 = strtol(arguments[2], NULL, 10);
	uint32 res = n1 + n2;
	cprintf("res=%d\n", res);
	return 0;
}

//print information about kernel addresses and kernel size
int command_kernel_info(int number_of_arguments, char **arguments )
{
	extern char start_of_kernel[], end_of_kernel_code_section[], start_of_uninitialized_data_section[], end_of_kernel[];

	cprintf("Special kernel symbols:\n");
	cprintf("  Start Address of the kernel 			%08x (virt)  %08x (phys)\n", start_of_kernel, start_of_kernel - KERNEL_BASE);
	cprintf("  End address of kernel code  			%08x (virt)  %08x (phys)\n", end_of_kernel_code_section, end_of_kernel_code_section - KERNEL_BASE);
	cprintf("  Start addr. of uninitialized data section 	%08x (virt)  %08x (phys)\n", start_of_uninitialized_data_section, start_of_uninitialized_data_section - KERNEL_BASE);
	cprintf("  End address of the kernel   			%08x (virt)  %08x (phys)\n", end_of_kernel, end_of_kernel - KERNEL_BASE);
	cprintf("Kernel executable memory footprint: %d KB\n",
			(end_of_kernel-start_of_kernel+1023)/1024);
	return 0;
}


int command_readmem(int number_of_arguments, char **arguments)
{
	unsigned int address = strtol(arguments[1], NULL, 16);
	unsigned char *ptr = (unsigned char *)(address ) ;

	cprintf("value at address %x = %c\n", ptr, *ptr);

	return 0;
}

int command_writemem(int number_of_arguments, char **arguments)
{
	unsigned int address = strtol(arguments[1], NULL, 16);
	unsigned char *ptr = (unsigned char *)(address) ;

	*ptr = arguments[2][0];

	return 0;
}

int command_meminfo(int number_of_arguments, char **arguments)
{
	cprintf("Free frames = %d\n", calculate_free_frames());
	return 0;
}

//===========================================================================
//Lab2.Hands.On
//=============
//TODO: LAB2 Hands-on: write the command function here


//===========================================================================
//Lab3.Examples
//=============
//Info:
//va: virtual address
//to get index of page table (directory): we use PDX(va) OR va>>22
//to get index of page: we do va<<10 then va>>22 OR PTX(va)

int command_kernel_base_info(int number_of_arguments, char **arguments)
{
	//TODO: LAB3 Example: fill this function. corresponding command name is "ikb"
	//Comment the following line
	//panic("Function is not implemented yet!");

	uint32 virutalAddress=strtol(arguments[1],NULL,16);
	uint32 pageTableIndex=PDX(virutalAddress);
	uint32 pageIndex=PTX(virutalAddress);
	cprintf("Page table index: %d\n",pageTableIndex);
	cprintf("Page index: %d\n",pageIndex);

	//frame number of page table
	uint32 directoryEntry= ptr_page_directory[pageTableIndex];
	uint32 pageTableFrameNumber= directoryEntry>>12;
	cprintf("Frame number of page table: %d\n",pageTableFrameNumber);

	//frame number of page
	uint32* ptr_page_table=NULL;
	get_page_table(ptr_page_directory,(int*)virutalAddress,0,&ptr_page_table);
	if(ptr_page_table!=NULL){
		cprintf("page table found at virtual address:%x\n", ptr_page_table);
		uint32 pageEntry=ptr_page_table[PTX(virutalAddress)];
		uint32 pageFrameNumber=pageEntry>>12;
		cprintf("Page Entry : %d\n",pageEntry);
		cprintf("Frame number of page: %d\n",pageFrameNumber);
		cprintf("Offset : %d\n",pageFrameNumber<<20);
		int frameNo=pageFrameNumber>>12;
		cprintf("# frame: %d\n",frameNo);
		cprintf("Reference of this # frame: %d\n",frames_info[pageFrameNumber].references);
	}
	return 0;
}


int command_del_kernel_base(int number_of_arguments, char **arguments)
{
	//TODO: LAB3 Example: fill this function. corresponding command name is "dkb"
	//Comment the following line
	//panic("Function is not implemented yet!");

	return 0;
}


int command_share_page(int number_of_arguments, char **arguments)
{
	//TODO: LAB3 Example: fill this function. corresponding command name is "shr"
	//Comment the following line
	//panic("Function is not implemented yet!");

	//shr 0xF00000000 0 256000
	uint32 virtualAddress1=strtol(arguments[1],NULL,16);
	uint32 virtualAddress2=strtol(arguments[2],NULL,16);

	uint32* ptr_table1=NULL;
	uint32* ptr_table2=NULL;

	get_page_table(ptr_page_directory,(void*)virtualAddress1,0,&ptr_table1);
	if(ptr_table1!=NULL){
		get_page_table(ptr_page_directory,(void*)virtualAddress2,1,&ptr_table2);

		int pageIndexOfAddress1=PTX(virtualAddress1);
		int pageIndexOfAddress2=PTX(virtualAddress2);

		ptr_table2[pageIndexOfAddress2]=ptr_table1[pageIndexOfAddress1];
		cprintf("Sharing page is Done\n");
	}
	return 0;
}

//===========================================================================
//Lab4.Hands.On
//=============
int getDirectoryIndex(uint32 virtualAddress){
	return PDX(virtualAddress);
}
int getPageTableIndex(uint32 virtualAddress){
	return PTX(virtualAddress);
}
int command_show_mapping(int number_of_arguments, char **arguments)
{
	//TODO: LAB4 Hands-on: fill this function. corresponding command name is "sm"
	//Comment the following line
	//panic("Function is not implemented yet!");

	//sm 0xF0000000
	cprintf("Directory Virtual Address: %x\n",ptr_page_directory);
	uint32 virtualAddress=strtol(arguments[1],NULL,16);

	int directoryIndex=PDX(virtualAddress);
	cprintf("Directory Index: %d\n",directoryIndex);

	int pageTableIndex=PTX(virtualAddress);
	cprintf("Page Table Index: %d\n",pageTableIndex);

	uint32* pageTableVirtualAddress=NULL;
	get_page_table(ptr_page_directory,(void*)virtualAddress,0,&pageTableVirtualAddress);

	uint32 pageTableEntry= ptr_page_directory[directoryIndex];
	cprintf("Page Table Entry: %x\n",pageTableEntry);
	if(pageTableEntry & PERM_PRESENT){
		cprintf("Table is present\n");
	}
	else cprintf("Table is not present\n");

	if((pageTableVirtualAddress[pageTableIndex] & PERM_PRESENT)>0){
		cprintf("Page is used\n");
	}
	else cprintf("Page is not used\n");
	return 0 ;
}

int command_set_permission(int number_of_arguments, char **arguments)
{
	//TODO: LAB4 Hands-on: fill this function. corresponding command name is "sp"
	//Comment the following line
	//panic("Function is not implemented yet!");

	uint32 virtualAddress=strtol(arguments[1],NULL,16);

	uint32* pageTableVirtualAddress=NULL;
	get_page_table(ptr_page_directory,(void*)virtualAddress,1,&pageTableVirtualAddress);

	int pageTableIndex=getPageTableIndex(virtualAddress);

	if(arguments[2][0]=='w'){
		pageTableVirtualAddress[pageTableIndex] |= PERM_WRITEABLE;
	}
	else
		pageTableVirtualAddress[pageTableIndex] &= ~PERM_WRITEABLE;
	return 0 ;
}

int command_share_range(int number_of_arguments, char **arguments)
{
	int address1=strtol(arguments[1],NULL,16);
	int address2=strtol(arguments[2],NULL,16);
	int sz = strtol(arguments[3],NULL,10);

	int mod =sz%4;
	int toShare=sz/4;
	if(mod!=0)
		toShare++;

	uint32* pageTable1Va;
	uint32* pageTable2Va;
	get_page_table(ptr_page_directory,(int*)address1,0,&pageTable1Va);
	get_page_table(ptr_page_directory,(int*)address2,1,&pageTable2Va);

	if(pageTable1Va != NULL && pageTable2Va !=NULL){
		int index1=PTX(address1);
		int index2=PTX(address2);

		int tmpVar1=address1;
		int tmpVar2=address2;

		uint32 *pageTable1;
		uint32 * pageTable2;
		for(int i=0;i<toShare;i++){
			get_page_table(ptr_page_directory,(int*)tmpVar1,0,&pageTable1);
			get_page_table(ptr_page_directory,(int*)tmpVar2,1,&pageTable2);

			pageTable2[PTX(tmpVar2)]=pageTable1[PTX(tmpVar1)];
			tmpVar1+=PAGE_SIZE;
			tmpVar2+=PAGE_SIZE;
		}
	}
	return 0;
}

//===========================================================================
//Lab5.Examples
//==============
//[1] Number of references on the given physical address
int command_nr(int number_of_arguments, char **arguments)
{
	//TODO: LAB5 Example: fill this function. corresponding command name is "nr"
	//Comment the following line
	//panic("Function is not implemented yet!");
	uint32 physicalAddress=strtol(arguments[1],NULL,16);
	struct Frame_Info *ptr_frame_info=to_frame_info(physicalAddress);
	cprintf("The number of reference are %d\n",ptr_frame_info->references);
	return 0;
}

//[2] Allocate Page: If the given user virtual address is mapped, do nothing. Else, allocate a single frame and map it to a given virtual address in the user space
int command_ap(int number_of_arguments, char **arguments)
{
	//TODO: LAB5 Example: fill this function. corresponding command name is "ap"
	//Comment the following line
	//panic("Function is not implemented yet!");

	uint32 virtualAddress = strtol(arguments[1], NULL, 16);
	uint32 * ptr_page_table = NULL;
	struct Frame_Info * ptr_frame_info=get_frame_info(ptr_page_directory,(void*)virtualAddress,&ptr_page_table);
	if(ptr_frame_info!=NULL){
		return 0;	//the frame is connected already
	}
	int ret =allocate_frame(&ptr_frame_info);
	if(ret == E_NO_MEM){
		return -1;
	}
	ret =map_frame(ptr_page_directory,ptr_frame_info,(void*)virtualAddress,PERM_USER | PERM_WRITEABLE);
	if(ret==E_NO_MEM){
		free_frame(ptr_frame_info);
		return -1;
	}
	return 0 ;
}

//[3] Free Page: Un-map a single page at the given virtual address in the user space
int command_fp(int number_of_arguments, char **arguments)
{
	//TODO: LAB5 Example: fill this function. corresponding command name is "fp"
	//Comment the following line
	//panic("Function is not implemented yet!");

	uint32 va = strtol(arguments[1], NULL, 16);
	// Un-map the page at this address
	unmap_frame(ptr_page_directory, (void*)va);

	return 0;
}

//===========================================================================
//Lab5.Hands-on
//==============
//[1] Allocate Shared Pages
int command_asp(int number_of_arguments, char **arguments)
{
	//TODO: LAB5 Hands-on: fill this function. corresponding command name is "asp"
	//Comment the following line
	//panic("Function is not implemented yet!");

	return 0;
}


//[2] Count Free Pages in Range
int command_cfp(int number_of_arguments, char **arguments)
{
	//TODO: LAB5 Hands-on: fill this function. corresponding command name is "cfp"
	//Comment the following line
	panic("Function is not implemented yet!");

	return 0;
}

//===========================================================================
//Lab6.Examples
//=============
int command_run(int number_of_arguments, char **arguments)
{
	//[1] Create and initialize a new environment for the program to be run
	struct UserProgramInfo* ptr_program_info = env_create(arguments[1]);
	if(ptr_program_info == 0) return 0;

	//[2] Run the created environment using "env_run" function
	env_run(ptr_program_info->environment);
	return 0;
}

int command_kill(int number_of_arguments, char **arguments)
{
	//[1] Get the user program info of the program (by searching in the "userPrograms" array
	struct UserProgramInfo* ptr_program_info = get_user_program_info(arguments[1]) ;
	if(ptr_program_info == 0) return 0;

	//[2] Kill its environment using "env_free" function
	env_free(ptr_program_info->environment);
	ptr_program_info->environment = NULL;
	return 0;
}

int command_ft(int number_of_arguments, char **arguments)
{
	//TODO: LAB6 Example: fill this function. corresponding command name is "ft"
	//Comment the following line

	return 0;
}
/****************************************************************/

//========================================================
/*ASSIGNMENT-1*/
//========================================================
//Q1: Calculate equivalent bytes
/*DON'T change this function*/
int command_bytes(int number_of_arguments, char **arguments)
{
	//DON'T WRITE YOUR LOGIC HERE, WRITE INSIDE THE bytes() FUNCTION
	int res = bytes(number_of_arguments, arguments);
	cprintf("The number of bytes is %d \n",res);
	return 0;
}
/*---------------------------------------------------------*/
/*FILL this function
 * arguments[1]: number
 * arguments[2] unit of number 1
 * arguments[n] number
 * arguments[n+1] unit of number n
 * Return:
 * 		The result of the calculation.
 */
int bytes(int num_of_arguments, char** arguments)
{
	//Assignment1.Q1
	//put your logic here and change the return according to your logic
	//...
	//11110 & 01
	if(!(num_of_arguments &1)){
		return -1;
	}
	int sum=0,number,units;
	char symbol;
	for(int i=1;i<num_of_arguments;i+=2)
	{
		number=strtol(arguments[i],NULL,10);
	    symbol = arguments[i+1][0];
		switch(symbol){
		case 'm':
		case 'M':
			units=1024*1024;
			break;
		case 'k':
		case 'K':
			units=1024;
			break;
		case 'b':
		case 'B':
			units=1;
			break;
		default :
			return -1;
		}
		sum+=(number*units);
	}
	//cprintf("Sum = %d\n", sum);
	return sum;
}


//========================================================
//Q2:Find the ones complement in a given binary string
/*DON'T change this function*/
int command_frequency_max_character(int number_of_arguments, char **arguments)
{
	//DON'T WRITE YOUR LOGIC HERE, WRITE INSIDE THE frequency_max_character() FUNCTION
	int fmc = frequency_max_character(number_of_arguments, arguments);
	cprintf("The frequency of the maximum character is = %d\n", fmc);
	return 0;
}
/*---------------------------------------------------------*/
/*FILL this function
 * arguments[1]: input string
 * Return:
 * 		The length of the longest word.
 */

int charToInt(char c){
	//ASCII value of _ is 95
	return c-94;
}
int getMaximum(int num1,int num2){
	if(num1>num2)
		return num1;
	else
		return num2;
}
int frequency_max_character(int num_of_arguments, char** arguments)
{
	//Assignment1.Q2
	//put your logic here and change the return according to your logic
	//...
	int mx=-1;
	int freq[30]={0};

	for(int i=0;i<strlen(arguments[1]);i++){
		//freq[(int)((int)(arguments[1][i])-96)]++;
		int index=charToInt(arguments[1][i]);
		freq[index]++;
		//cprintf("%d \n ",index);
		mx=getMaximum(mx,freq[index]);
	}
	return mx;
}

//========================================================
/*ASSIGNMENT-1: BONUS*/
//========================================================
//B: Execute Set of Concatenated Commands
///This command should allow the user to enter a set of commands (max 10 commands), and execute them after pressing $.

/*DON'T change this function*/
int command_xcs(int number_of_arguments, char **arguments )
{
	//DON'T WRITE YOUR LOGIC HERE, WRITE INSIDE THE ExecuteCommands() FUNCTION
	ExecuteCommands();

	return 0;
}
/*---------------------------------------------------------*/

/*FILL this function*/
void ExecuteCommands()
{
	//Assignment1.BONUS
	//put your logic here
	//should allow the user to enter a set of commands (max 10 commands), and execute them after pressing $.
	cprintf("Enter a set of commands (max 10), one per line, terminated by '$': \n");
	char *endingCharacter="$";
	char *arguments[MAX_ARGUMENTS];
	bool endCommands=0;
	int ctr=0,number_of_arguments=0;
	char input[1024];
	char commands[12][1024]={};
	while(ctr!=10 && !endCommands){
		readline("FOS> ",input );
		if(strcmp(input,endingCharacter)==0){
			endCommands=1;
			//break;
		}
		else {
			for(int i=0;i<strlen(input);i++){
				commands[ctr][i]=input[i];
			}
		}
		ctr++;
	}
	//cprintf("The number of commands = %d \n", ctr);
	for(int i=0;i<ctr;i++){
		execute_command(commands[i]);
	}
}
// ----------------------------------------Lab 2 hands on Commands ----------------------------------------

//write_mem 0xf00000000 c
int command_writeMemory(int number_of_arguments,char **arguments){
	char *ptr;
	ptr=(char*)(strtol(arguments[1],NULL,16));

	*ptr=arguments[2][0];
	//cprintf("%x --- %c \n",ptr,*ptr);
	return 0;

}
//read_mem 0xf00000000
int command_readMemory(int number_of_arguments,char **arguments){
	unsigned int address=strtol(arguments[1],NULL,16);
	unsigned int *ptr=(unsigned int*)(address);
	cprintf("Value at address %x is %d\n",ptr,*ptr);
	return 0;
}
int command_read_block(int number_of_arguments, char **arguments){
	unsigned int address = strtol(arguments[1],NULL,16);
	unsigned char *ptr=(unsigned char*)(address);
	int blockSize=strtol(arguments[2],NULL,10);

	for(int i=0;i<blockSize;i++){
		cprintf("val @ va %x = %c\n",ptr,*ptr);
		ptr++;
	}
	return 0;
}
//create_int_array 1 2 3 4 5
int *ptr=(int*)0xF1000000;
int command_create_int_array(int number_of_arguments,char**arguments){
	cprintf("The start address of allocated array is : %x\n",ptr);

	for(int i=1;i<number_of_arguments;i++){
		unsigned int num=strtol(arguments[i],NULL,10);
		*ptr=num;
		//cprintf("ptr : %x , value : %d\n",ptr,*ptr);
		ptr++;
	}
	return 0;
}
// ---------------------------------------- Lab 4 Hands on commands  ----------------------------------------
