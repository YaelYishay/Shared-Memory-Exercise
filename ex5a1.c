

/*
 * MSGQ
 * ====
 * built out of two servers and a client
 * the program sends msg between the client
 * and each of the servers.
 * the servers share data by using shared memory
 * AS REQUIRED
 *
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

#define MAX_LENGTH 200
#define PROCESS_MAX 5
#define SHM_SIZE 200

int *shm_ptr;

//Enum & Struct-------------------------------

struct my_msgbuf{
	long mytype;
	char mkind;
	pid_t pid;
	char mtext[MAX_LENGTH];
	int result;
};



//Functions---------------------------------
key_t getKey(char k);
int getMsgId(key_t key);
int addprocess(pid_t pid,int counter);
int processCheck(pid_t pid,int counter);
void deleteProcess(pid_t pid,int counter);
void signal_handler(int );
int gets_m(key_t smKey);
int *getSmhPtr(int);
//Main-------------------------------------


int main(){

	signal(SIGINT,signal_handler);

	long int allowed_type = 0 ;

	int msgId,
	smId,
	status,
	counter = 0 ,
	result;

	struct my_msgbuf my_msg;
	key_t key,
	smKey;


	smKey=getKey('s');		//shared memory key
	smId = gets_m(smKey);
	key=getKey('a');		//msgq key
	msgId=getMsgId(key);

	shm_ptr = getSmhPtr(smId);	//get pointer to shared memory
	shm_ptr[0] = counter;		//save counter in first cell


	while(1){

		//get msg from client
		status = msgrcv(msgId, &my_msg,sizeof(struct my_msgbuf),allowed_type,0);
		if(status == -1){
			perror("could not receive msg from queue");
			exit (EXIT_SUCCESS);
		}

		//register program id
		if(my_msg.mytype == 1){
			result = addprocess(my_msg.pid,counter);
			if(result == 0){
				counter++;
				shm_ptr[0] = counter;
			}
			my_msg.result = result;
			status = msgsnd(msgId,&my_msg,sizeof(struct my_msgbuf),0);
			if(status == -1){
				perror("could not send msg to client \n");
				exit(EXIT_SUCCESS);
			}
		}

		//delete id from register
		else if(my_msg.mytype == 3 ){
			deleteProcess(my_msg.pid,counter);
			counter--;
			shm_ptr[0] = counter;
			puts("bye\n");
		}

	}

	return (EXIT_SUCCESS);
}
//----------------------------------------------------
//signal handler
void signal_handler(int sigNum){

	int msgId;
	key_t key;

	if((key = ftok(".",'a')) == -1){
		perror("ftok() failed");
		exit(EXIT_FAILURE);
	}

	if((msgId = msgget(key,0)) == -1){
		perror("new msg queue failed");
		exit (EXIT_FAILURE);
	}
	if(msgctl(msgId,IPC_RMID,NULL)==-1){
		perror("msgctl failed");
		exit(EXIT_FAILURE);
	}
}
//----------------------------------------------------
//add a process
int addprocess(pid_t pid,int counter){


	int result;

	result= processCheck(pid,counter);
	if(result == 1)
		return 1;

	else if (result == 0){
		if(counter >= PROCESS_MAX)
			return 2;
		else {
			shm_ptr[counter+1]=pid;
			return 0;
		}
	}


	return 4; //for compiler
}

//----------------------------------------------------
//check if process exists
int processCheck(pid_t pid,int counter){

	int i;

	for( i = 1 ; i <= counter+1; i++)
		if(shm_ptr[i] == pid)
			return 1;
	return 0;
}
//-------------------------------------------------------
//delete the process
void deleteProcess(pid_t pid,int counter){

	int i,
	j;


	for( i = 1 ; i<counter+1; i++)
		if(shm_ptr[i]== pid)
			break;


	for(j = i  ; j<counter+1; j++)
		shm_ptr[j] = shm_ptr[j+1];
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

	id = msgget(key,IPC_CREAT|IPC_EXCL|0600);
	if(id == -1){
		perror("msgget2 failed in server1\n");
		exit (EXIT_FAILURE);
	}
	return id;
}
//----------------------------
//get shared memory key
int gets_m(key_t smKey){

	int shm_id ;
	shm_id = shmget( smKey, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0600 ) ;
	if (shm_id == -1){
		perror("could not make shared memory\n");
		exit(EXIT_FAILURE);
	}
	return shm_id;
}
//--------------------
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




