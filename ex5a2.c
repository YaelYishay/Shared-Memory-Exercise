/*
 * ex5a2.c
 *
 *  Created on: Dec 13, 2017
 *      Author: yael
 */


/*
 * ex4a2.c
 *
 *  Created on: Dec 12, 2017
 *      Author: yaelor
 */


/*
 * ex4a2.c

 *
 *  Created on: Dec 10, 2017
 *      Author: yael
 */

//Includes--------------------------------------

#include<signal.h>
#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include <sys/shm.h>

int *shm_ptr;

#define MAX_LENGTH 200

//Enum & Struct-------------------------------

struct my_msgbuf{
	long mytype;
	char mkind;
	pid_t pid;
	char mtext[MAX_LENGTH];
	int result;
};

//functions-------------------------------
key_t getKey(char k);
int getMsgId(key_t key);
int getMsgIdMake(key_t key);
int processCheck(pid_t pid,int counter);
int checkPrime(char prime[]);
int checkPolindrom(char polindrom[]);
void signal_handler(int );
int *getSmhPtr(int);
int getSmId(key_t shmKey);
//Main-------------------------------------

int main(){


	struct my_msgbuf my_msg;
	long int allowed_type = 0;
	int msgId,
	shId,
	status,
	result;
	key_t key,
	shmKey;
	signal(SIGINT,signal_handler);


	shmKey = getKey('s');		//get key to shared memory
	shId = getSmId(shmKey);
	key = getKey('b');			//get key to server
	msgId = getMsgIdMake(key);

	shm_ptr = getSmhPtr(shId);	//get pointer to shared memory
	int flag = 1;

	while(flag == 1){

		//get request from client
		status = msgrcv(msgId, &my_msg, sizeof(struct my_msgbuf),allowed_type,0);
		if(status == -1){
			perror("could not receive msg from queue");
			exit (EXIT_SUCCESS);
		}


		//check in shared memory if registerd
		result = processCheck(my_msg.pid,shm_ptr[0]);
		my_msg.result = result;

		//if registered
		if(my_msg.result == 1 ){
			if(my_msg.mkind == 'n'){
				result=checkPrime(my_msg.mtext);
				my_msg.result = result;
			}
			else if(my_msg.mkind == 's'){
				result=checkPolindrom(my_msg.mtext);
				my_msg.result = result;
			}
		}
		//if not registered
		else
			my_msg.result = -1;

		if(msgsnd(msgId,&my_msg,sizeof(struct my_msgbuf),0)==-1){
			perror("msgsnd failed");
			exit(EXIT_FAILURE);
		}


	}
	return EXIT_SUCCESS;
}
//----------------------------------------------------
//signal handler
void signal_handler(int sigNum){

	int
	msgId2;
	key_t
	key2;

	if((key2 = ftok(".",'b')) == -1){
		perror("ftok() failed");
		exit(EXIT_FAILURE);
	}

	if((msgId2 = msgget(key2,0)) == -1){
		perror("new msg queue failed");
		exit (EXIT_FAILURE);
	}
	if(msgctl(msgId2,IPC_RMID,NULL)==-1){
		perror("msgctl failed");
		exit(EXIT_FAILURE);
	}



}

//check prime------------------------
//check if number is prime or not
int checkPrime(char text[]){


	int number,
	i,
	flag = 0;

	number = atoi(text);

	for(i=2; i<=number/2; ++i)
		if(number % i == 0){
			flag=1;
			break;
		}
	if (flag==0)
		return 1;
	return 0;
}

//check if string is polindrum---------

int checkPolindrom(char polindrom[]){

	// Start from leftmost and rightmost corners of str
	int l = 0;
	int h = strlen(polindrom) - 1;

	// Keep comparing characters while they are same
	while (h > l)
		if (polindrom[l++] != polindrom[h--])
			return 0;
	return 1;
}

//ftok-------------------
//get outer key
key_t getKey(char k){

	key_t key;

	key=(ftok(".", k)) ;
	if (key == -1){
		perror("ftok failed");
		exit (EXIT_FAILURE);
	}
	return key;
}

//get message id-------------

int getMsgId(key_t key){

	int id;

	id = msgget(key,0);
	if(id == -1){
		perror("msgget2 failed in reader");
		exit (EXIT_FAILURE);
	}
	return id;
}
//--------------------------
//get inner key
int getMsgIdMake(key_t key){

	int id;

	id = msgget(key,IPC_CREAT|IPC_EXCL|0600);
	if(id == -1){
		perror("msgget2 failed in server2\n");
		exit (EXIT_FAILURE);
	}
	return id;
}

//------------------------------
//get pointer to shared memory
int *getSmhPtr(int msId){

	int *shm ;
	shm = (int *) shmat ( msId, NULL, 0 ) ;
	if (shm == (int *) -1)
	{
		perror( "shmat failed" ) ;
		exit( EXIT_FAILURE ) ;
	}
	return shm;

}
//----------------------------------
//get inner key to shared memory
int getSmId(key_t shmKey){

	int shm_id;
	shm_id = shmget(shmKey, 0 , 0600) ;
	if (shm_id == -1){
		perror("could not join shared memory\n");
		exit(EXIT_SUCCESS);
	}
	return shm_id;
}
//---------------------------------------------
//check if process exits in shared memory
int processCheck(pid_t pid,int counter){


	int i;

	for( i = 1 ; i <= counter+1; i++)
		if(shm_ptr[i] == pid)
			return 1;
	return 0;
}
//-










