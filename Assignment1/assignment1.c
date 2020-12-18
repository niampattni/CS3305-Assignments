#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv)
{
	/************************************************** command line arguments ***********************************************
		For the purposes of this explanation, assume that "external_program.out" is located in /home/usr/A1/external_program.out 

		When you are testing your program, replace "/home/usr/A1/external_program.out" with the path to the executable file 
		generated when you compile "external_program.c"
		
		To compile assignment1.c: gcc assignment1.c -o assignment1
		To run your program: ./assignment1 /home/usr/A1/external_program.out
		argv[0] = "./assignment1"
		argv[1] = "/home/usr/A1/external_program.out"

		Number of arguments (argc) = 2

		In this assignment, the path to the external program is in argv[1]
	*************************************************************************************************************************/

	// If the path of external_program.out isn't provided by the user, the program will terminate
	if (argc != 2)
	{
		printf("Invalid arguments\n");
		exit(0);
	}

	pid_t pid; //pid for main forks
	pid_t c1pid; //pid for child 1 fork
	pid = fork(); //fork and create child 1
	char child2pid[10]; //string to pass child 2 pid as argument to external_program.out
	
	if (pid > 0) { //parent
		
		wait(NULL); //wait for child 1
		pid = fork(); //fork and create child 2
		
		if (pid == 0) {	//child 2
			printf("parent (PID %d) created child_2 (PID %d)\n", getppid(), getpid());		
			printf("child_2 (PID %d) is calling an external program external_program.out and leaving child_2..\n", getpid());
			sprintf(child2pid, "%d", getpid()); //make child 2 pid a string to pass as argument to external_program.out
			execl(argv[1], argv[1], child2pid, NULL); //exec call external_program.out from command line arguments and replace child 2 process
		}
		
	} else { //child 1
		
		printf("parent process (PID %d) created child_1 (PID %d) \n", getppid(), getpid());
		printf("parent (PID %d) is waiting for child_1 (PID %d) to complete before creating child_2\n", getppid(), getpid());
		c1pid = fork(); //fork and create child 1.1
		
		if (c1pid > 0) { //child 1
			
			wait(NULL); //wait for child 1.1
			printf("child_1 (PID %d) is now complete\n", getpid());
			
		} else { //child 1.1
			
			printf("child_1 (PID %d) created child_1.1 (PID %d)\n", getppid(), getpid());
			
		}
	}
	
	return 0;
}
