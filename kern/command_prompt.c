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

		//TODO: LAB2 Hands-on: add the commands here


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
//dp code
int getMin2(int x,int y){
	if(x>=y)
		return x;
	else return y;
}
int getMin(int x,int y,int z){
	if(x<y&&x<z){
		return x;
	}
	else if(y<x&&y<z){
		return y;
	}
	else return z;
}
int editDistanceUsingDP(char *str1,char *str2){
	int m =strlen(str1);
	int n= strlen(str2);
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
            else if (str1[i - 1] == str2[j - 1]){
            	dp[i][j] = dp[i - 1][j - 1];
            }
            else {
            	 //dp[i][j]= 1 + getMin2(dp[i][j - 1],dp[i - 1][j - 1]); // Replace
                   dp[i][j]= 1 + getMin(dp[i][j - 1], dp[i - 1][j],dp[i - 1][j - 1]);
            }
		}
	}
	return dp[m][n];
}
char* getTheNearestCommandName(char *commandName){
	int mn=150;
	int commandNameLength=strlen(commandName);
	int numberOfEdits;
	char *nearestCommand=commandName;
	for (int i = 0; i < NUM_OF_COMMANDS; i++)
	{
	    numberOfEdits=editDistanceUsingDP(commandName , commands[i].name);
		if(numberOfEdits<mn)
		{
			mn=numberOfEdits;
			nearestCommand=commands[i].name;
		}
	}
	commandName=nearestCommand;
	//cprintf("Command Name : %d\n",strlen(commandName));
	return commandName;
} //end of DP code
void modifiedReadLine(const char *prompt,char *buf)
{
	int i=0,c,echoing ;
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
			foundFirstSpace=1;;
			char *arr=getTheNearestCommandName(buf);
			for(int j=0;j<i;j++){
				cputchar('\b');
			}
			for(int j=0;j<i;j++){
				cputchar(arr[j]);
				buf[j]=arr[j];
			}
			char space=' ';
			cputchar(space);
			buf[i++] = space;
		}
		else if (c >= ' ' && i < BUFLEN-1)
		{
			if (echoing)
				cputchar(c);
			buf[i++] = c;
		}
		else if (c == '\b' && i > 0)
		{
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


//invoke the command prompt
void run_command_prompt()
{
	//CAUTION: DON'T CHANGE OR COMMENT THIS LINE======
	TestAssignment1();
	//================================================

	char command_line[1024];

	while (1==1)
	{
		//get command line
		readline("FOS> ", command_line);
		//modifiedReadLine("FOS> ",command_line);
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
	int command_found = 0;
	int i ;
	for (i = 0; i < NUM_OF_COMMANDS; i++)
	{
		if (strcmp(arguments[0], commands[i].name) == 0)
		{
			command_found = 1;
			break;
		}
	}

	if(command_found)
	{
		int return_value;
		return_value = commands[i].function_to_execute(number_of_arguments, arguments);
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
int command_kernel_base_info(int number_of_arguments, char **arguments)
{
	//TODO: LAB3 Example: fill this function. corresponding command name is "ikb"
	//Comment the following line
	panic("Function is not implemented yet!");

	return 0;
}


int command_del_kernel_base(int number_of_arguments, char **arguments)
{
	//TODO: LAB3 Example: fill this function. corresponding command name is "dkb"
	//Comment the following line
	panic("Function is not implemented yet!");

	return 0;
}

int command_share_page(int number_of_arguments, char **arguments)
{
	//TODO: LAB3 Example: fill this function. corresponding command name is "shr"
	//Comment the following line
	panic("Function is not implemented yet!");

	return 0;
}

//===========================================================================
//Lab4.Hands.On
//=============
int command_show_mapping(int number_of_arguments, char **arguments)
{
	//TODO: LAB4 Hands-on: fill this function. corresponding command name is "sm"
	//Comment the following line
	panic("Function is not implemented yet!");

	return 0 ;
}

int command_set_permission(int number_of_arguments, char **arguments)
{
	//TODO: LAB4 Hands-on: fill this function. corresponding command name is "sp"
	//Comment the following line
	panic("Function is not implemented yet!");

	return 0 ;
}

int command_share_range(int number_of_arguments, char **arguments)
{
	//TODO: LAB4 Hands-on: fill this function. corresponding command name is "sr"
	//Comment the following line
	panic("Function is not implemented yet!");

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
	panic("Function is not implemented yet!");

	return 0;
}

//[2] Allocate Page: If the given user virtual address is mapped, do nothing. Else, allocate a single frame and map it to a given virtual address in the user space
int command_ap(int number_of_arguments, char **arguments)
{
	//TODO: LAB5 Example: fill this function. corresponding command name is "ap"
	//Comment the following line
	//panic("Function is not implemented yet!");

	uint32 va = strtol(arguments[1], NULL, 16);
	struct Frame_Info* ptr_frame_info;
	int ret = allocate_frame(&ptr_frame_info) ;
	map_frame(ptr_page_directory, ptr_frame_info, (void*)va, PERM_USER | PERM_WRITEABLE);

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
	panic("Function is not implemented yet!");

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
	int sum=0,number;
	for(int i=1;i<num_of_arguments;i+=2){
		number=strtol(arguments[i],NULL,10);

		if(strcmp(arguments[i+1],"M")==0 || strcmp(arguments[i+1],"m")==0){
			sum+=(number*1024*1024);
		}
		else if(strcmp(arguments[i+1],"K")==0||strcmp(arguments[i+1],"k")==0){
			sum+=(number*1024);
		}
		else if(strcmp(arguments[i+1],"B")==0 || strcmp(arguments[i+1],"b")==0){
			sum+=number;
		}
		else return -1;
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

