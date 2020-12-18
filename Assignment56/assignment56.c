#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

//Declaration of thread functions
void *withdraw(void *input);
void *deposit(void *input);
void *transfer(void *input);

//Global variables
int *balances; //dynamic array to store balances
pthread_mutex_t lock;

//Structure for thread argument data
struct args {
	int acc1, acc2, num;
};

void *withdraw(void *input) {
	//Get argument data
	struct args *temp = (struct args *) input;
	struct args data = *temp;

	pthread_mutex_lock(&lock); //ENTRY REGION

	//CRITICAL SECTION START

	int balance = balances[data.acc1]; //get balance

	if (balance >= data.num) balance -= data.num; //if there is money then withdraw it

	balances[data.acc1] = balance; //update balance

	//CRITICAL SECTION END

	pthread_mutex_unlock(&lock); //EXIT REGION
}

void *deposit(void *input) {
	//Get argument data
	struct args *temp = (struct args *) input;
	struct args data = *temp;

	pthread_mutex_lock(&lock); //ENTRY REGION

	//CRITICAL SECTION START

	int balance = balances[data.acc1]; //get balance

	balance += data.num; //deposit money

	balances[data.acc1] = balance; //update balance

	//CRITICAL SECTION END

	pthread_mutex_unlock(&lock); //EXIT REGION
}

void *transfer(void *input) {
	//Get argument data
	struct args *temp = (struct args *) input;
	struct args data = *temp;

	pthread_mutex_lock(&lock); //ENTRY REGION

	//CRITICAL SECTION START

	int fromBal = balances[data.acc1]; //get balance for from account
	int toBal = balances[data.acc2]; //get balance for to account

	if (fromBal >= data.num) { //if there is money in from account then transfer it
		fromBal -= data.num;
		toBal += data.num;
	}

	balances[data.acc1] = fromBal; //update balance of from account
	balances[data.acc2] = toBal; //update balance of to account

	//CRITICAL SECTION END

	pthread_mutex_unlock(&lock); //EXIT REGION
}

int main(int argc, char* argv[]) {
	//Ensure correct program arguments
	if (argc != 2) {
		printf("Usage: %s input.txt\n", argv[0]);
		return -1;
	}

	//Allocate memory for threads, account balances, and thread arguments
	//These are initialized to 100 to allow for large amounts of test data,
	//I previously set them to allocate for a single element and then grow as needed,
	//however reallocating data constantly caused a lot of data leaks and conflicts,
	//so I found this was the best method
	//It should be noted that alternatively, I could have made an array for each,
	//and everytime I wanted to reallocate memory, I could have moved the current data to an array,
	//freed the old memory, allocated new memory increasing size by one, and copied the array back.
	//I thought this was a lot of unnecessary moving of data and it was better to just allocate
	//enough for most test data, so reallocation is only necessary for large input files.
	pthread_t *threads = (pthread_t *) malloc(100*sizeof(pthread_t));
	balances = (int *) malloc(100*sizeof(int));
	struct args *Input = (struct args *) malloc(100*sizeof(struct args));

	//These store potential thread errors, number of threads, and number of accounts
	int err, numThreads = 0, numAccounts = 0;

	//Initialize the mutex lock and return error if it doesn't work
	if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\nThread mutex init failed\n");
        return -1;
    }

	//Open the input file for reading only
	FILE *file = fopen(argv[1], "r");
	char line[1000]; //stores a line read
	int acc1, acc2, amount, cust, offset; //temp variables
	char temp, *lineptr = line; //temp variables

	//Loop until no lines left to read
	while (fgets(line, 1000, file)) {

		if (line[0] == 'a') { //new account

			sscanf(line, "a%d b %d", &acc1, &amount); //read formatted line saving account number and balance
			if (numAccounts >= 100) balances = (int *) realloc(balances, numAccounts * sizeof(int)); //reallocate balances array for large number of inputs
			balances[acc1-1] = amount; //store the new balance in the array
			numAccounts++;

		} else { //customer request

			lineptr = line; //reset line pointer
			sscanf(lineptr, "c%d %n", &cust, &offset); //read formatted line saving customer number and line position

			while (1) {

				lineptr += offset; //fix pointer
				sscanf(lineptr, "%c", &temp); //read transaction type
				lineptr++; //fix pointer

				//If the character read is not a transaction, it is the end of line, exit the loop
				if (temp != 'w' && temp != 'd' && temp != 't') break;

				//Otherwise, we have a new transaction to process
				numThreads++;
				//Reaalocate threads array and data struct for large number of inputs
				if (numThreads > 100) {
					threads = (pthread_t *) realloc(threads, numThreads * sizeof(pthread_t));
					Input = (struct args *) realloc(Input, numThreads * sizeof(struct args));
				}

				if (temp == 'w' || temp == 'd') { //withdrawal or deposit

					//Read formatted line and save values, reset offset
					sscanf(lineptr, " a%d %d%n", &acc1, &amount, &offset);
					if (*(lineptr+offset) == ' ') offset++;

					//Save account number, transaction amount, and current thread
					Input[numThreads-1].acc1 = acc1-1;
					Input[numThreads-1].acc2 = 0;
					Input[numThreads-1].num = amount;

					//Create threads
					if (temp == 'w')
						err = pthread_create(&threads[numThreads-1], NULL, &withdraw, &Input[numThreads-1]);
					else
						err = pthread_create(&threads[numThreads-1], NULL, &deposit, &Input[numThreads-1]);

				} else if (temp == 't') { //transfer
					
					//Read formatted line and save values, reset offset
					sscanf(lineptr, " a%d a%d %d%n", &acc1, &acc2, &amount, &offset);
					if (*(lineptr+offset) == ' ') offset++;

					//Saving both account numbers, transaction amount, and current thread
					Input[numThreads-1].acc1 = acc1-1;
					Input[numThreads-1].acc2 = acc2-1;
					Input[numThreads-1].num = amount;

					//Create threads
					err = pthread_create(&threads[numThreads-1], NULL, &transfer, &Input[numThreads-1]);
				}
				
				//Send error if any thread is not created
				if (err != 0) {
					printf("\nError creating thread %d", numThreads);
				}
			}			
		}
	}

	//Join all the threads (run them)
	for (int i = 0; i < numThreads; i++) {
		pthread_join(*(threads + i), NULL);
	}

	//Open new file for writing
	FILE *out = fopen("assignment_6_output_file.txt", "w");

	//Write final balances to output file
	for (int i = 0; i < numAccounts; i++) {
		fprintf(out, "a%d b %d\n", i+1, balances[i]);
	}

	//Destroy the mutex lock and close the input file
	pthread_mutex_destroy(&lock);
	fclose(file);
	fclose(out);

	//Free all memory
	free(balances);
	free(threads);
	free(Input);

	return 0;
	//End, hopefully this shit works :)
}
