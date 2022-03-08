#ifndef FOS_KERN_MONITOR_H
#define FOS_KERN_MONITOR_H
#ifndef FOS_KERNEL
# error "This is a FOS kernel header; user programs should not #include it"
#endif

#include <inc/types.h>
//FOS
// Function to activate the kernel command prompt
void run_command_prompt();

// Declaration of functions that implement command prompt commands.
int command_help(int , char **);
int command_kernel_info(int , char **);
int command_ver(int, char**);
int command_add(int, char**);

/*Assignment-1 Commands*/
int command_bytes(int, char**);
int bytes(int, char**);

int command_frequency_max_character(int, char**);
int frequency_max_character(int, char**);

/*ASSIGNMENT-1 BONUS*/
int command_xcs(int , char **);
void ExecuteCommands();


#endif	// !FOS_KERN_MONITOR_H
