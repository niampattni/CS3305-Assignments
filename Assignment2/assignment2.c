#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// Reading and writing end of the pipe
#define READ_END 0
#define WRITE_END 1

int main(int argc, char **argv)
{

	/*************************************** command line arguments ***************************************
		To compile assignment2.c: gcc assignment2.c -o assignment2
		To run your program: ./assignment2 "CS" "3305" " is fun!"
		argv[0] = "./assignment2"
		argv[1] = "CS"
		argv[2] = "3305"
		argv[3] = " is fun!"

		Number of arguments (argc) = 4

		In this assignment, the parent process reads argv[1] and the child process reads argv[2] and argv[3]
		e.g. In the parent process: char *x = argv[1];
	*******************************************************************************************************/

	// If the user does not pass X, Y and Z, the program will terminate
	if (argc != 4)
	{
		printf("Invalid arguments\n");
		exit(0);
	}
	
	int status; //for opening pipe
	pid_t pid; //for opening fork
	int port[2]; //reading and writing ports
	char *cat1, *cat2; //concatenated string 1 and 2
	int length = strlen(argv[2]) + strlen(argv[3]); //length to be written/read

	status = pipe(port); //create pipe
	printf("A pipe is created for communication between parent (PID %d) and child\n", getpid());
	if (status < 0) exit(0); //error trap pipe

	pid = fork(); //create fork
	
	if (pid > 0) { //parent

		wait(NULL); //wait for child

		char tmp[length]; //temp array for reading data
		read(port[0],&tmp,length); //read data from pipe (13 bytes)
		printf("parent (PID %d) reads Y' from the pipe (Y' = \"%s\")\n", getpid(), tmp);

		cat2 = strcat(argv[1],tmp); //concatenate read data with first argument
		printf("parent (PID %d) concatenates X and Y' to generate the string: %s\n", getpid(), cat2);

	} else { //child

		printf("parent (PID %d) created a child (PID %d)\n", getppid(), getpid());
		printf("parent (PID %d) receives X = \"%s\" from the user\n", getppid(), argv[1]);
		printf("child (PID %d) receives Y = \"%s\" and Z = \"%s\" from the user\n", getpid(), argv[2], argv[3]);

		cat1 = strcat(argv[2], argv[3]); //concatenate argument 2 and argument 3
		printf("child (PID %d) concatenates Y and Z to generate Y'= \"%s\"\n", getpid(), cat1);

		write(port[1],cat1,length); //write data to pipe (13 bytes)
		printf("child (PID %d) writes Y' into the pipe\n", getpid());

	}
	return 0;
}
