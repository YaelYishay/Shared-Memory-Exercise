/*
 * SHARED MEMORY:
 * =============
 * Writen by: adi ben ezra, id = 206124000, login = adibene
 *            yael yishay , id = 305345811, login = yaelor
 *
 * Algorithm:
 * A program that allocates a block of shared
 * memory, then produces two processes that random
 * some of numbers until prime number.then save him in
 * shared memory, kill his brother, send signal for
 * his father and exit. dad processes prints the number and
 * exit too.
 *
 *
 * race time:
 *
 */

#include <stdio.h>
#include <stdlib.h> //for exit
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>//for fork
#include <wait.h>
#include <stdbool.h>
#include <time.h>

#define SHM_SIZE 1

key_t create_key();
int create_shm(key_t key);
int* create_shm_ptr(int shm_id);
void delete_and_remove(int *shm_ptr,int shm_id);
void signal_handler(int sig_num);
void do_son(key_t key);
bool isPrime(int num);

pid_t ID_PROCESSES[2];

int main()
{
	//when dad get sigusr1 from son then go signal handler
	signal(SIGUSR1,signal_handler);

	srand((unsigned) time(NULL)); 	//for random

	key_t key = create_key();

	int shm_id = create_shm(key);

	int*shm_ptr = create_shm_ptr(shm_id);

	int i;

	for(i = 0; i<2; i++){

		ID_PROCESSES[i] = fork();

		switch(ID_PROCESSES[i]){

		case -1:
			perror("fork() failed");
			exit(EXIT_FAILURE);

		case 0:
			do_son(key);
		}
	}
	//send dad sleep until get signal
	pause();

	//print prime num
	printf("%d\n",shm_ptr[0]);

	delete_and_remove(shm_ptr,shm_id);

	return EXIT_SUCCESS;
}

//dad do wait for his children
//---------------------------------------
void signal_handler(int sig_num)
{
	int i;

	for(i = 0;i < 2;i++)
		wait(NULL);

}
//create a key for the shm
//--------------------------------
key_t create_key()
{
	key_t key;
	if((key = ftok(".",'b')) == -1){
		perror("ftok() failed");
		exit(EXIT_FAILURE);
	}
	return key;
}

// shmget() create shm and return the shm id
//---------------------------------------
int create_shm(key_t key)
{
	int shm_id;

	if( (shm_id = shmget(key,
			SHM_SIZE,
			IPC_CREAT|IPC_EXCL|0600)) == -1){
		perror("shmget() failed");
		exit(EXIT_FAILURE);
	}
	return shm_id;

}
//shmat return pointer to shm
//------------------------------------------
int* create_shm_ptr(int shm_id)
{
	int* shm_ptr =(int*)shmat(shm_id,NULL,0);

	if(shm_ptr == (int*) -1) {
		perror("shmat() failed");
		exit(EXIT_FAILURE);
	}
	return shm_ptr;
}

//delete pointer and remove shm
//--------------------------------------------------
void delete_and_remove(int *shm_ptr,int shm_id)
{
	shmdt(shm_ptr); //delete pointer
	shmctl(shm_id,IPC_RMID,NULL); //remove shm
}
//-------------------------------------------
//job son
void do_son(key_t key)
{
	int shm_id,  //return from shmget
	num;         // num random
	int* shm_ptr; // pointer to shm

	if( (shm_id = shmget(key,0,0600)) == -1){
		perror("shmget() failed");
		exit(EXIT_FAILURE);
	}

	//create pointer to shm
	shm_ptr = create_shm_ptr(shm_id);

	while(1){

		num = rand()%1000;
		if(isPrime(num)){

			shm_ptr[0] = num;

			////delete pointer
			shmdt(shm_ptr);

			//kill brother
			if(getpid() == ID_PROCESSES[0])
				kill(ID_PROCESSES[1],SIGTERM);
			else
				kill(ID_PROCESSES[0],SIGTERM);

			//send siguser1 to dad
			kill(getppid(),SIGUSR1);

			exit(EXIT_SUCCESS);
		}
	}
}
//check if num is prime
//-----------------------------------
bool isPrime(int num)
{
	int i;

	for(i=2; i<=num/2; ++i){
		if(num % i == 0)
			return false;
	}
	return true;
}

