/*
 * ex5a3.c
 *
 *  Created on: Dec 13, 2017
 *      Author: yael
 */


/*
 * ex4a3.c
 *
 *  Created on: Dec 12, 2017
 *      Author: yaelor
 */





//Includes--------------------------------------
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<unistd.h>
#define MAX_LENGTH 200
#define PROCESS_MAX 5

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

//Main-------------------------------------
int main()
{

	char input[100];
	long int allowed_type = 0;
	int msgIdserver1,
	msgIdserver2;

	int status;

	struct my_msgbuf my_msg;
	key_t keyserver1 ,
	keyserver2;

	//get key to register server
	keyserver1=getKey('a');
	msgIdserver1=getMsgId(keyserver1);

	my_msg.mytype = 1;
	my_msg.pid = getpid();

	//send msg to register
	if(msgsnd(msgIdserver1,&my_msg,sizeof(struct my_msgbuf),0)==-1){
		perror("msgsnd failed");
		exit(EXIT_FAILURE);
	}


	//get response from register server
	status = msgrcv(msgIdserver1, &my_msg, sizeof(struct my_msgbuf),allowed_type,0);
	if(status == -1){
		perror("could not receive msg from queue");
		exit (EXIT_SUCCESS);
	}
	if(my_msg.result == 2 ){
		perror("could not add client  to server\n");
		exit(EXIT_FAILURE);
	}

	//get key to application server
	keyserver2=getKey('b');
	msgIdserver2=getMsgId(keyserver2);


	while(1){

		//get input from user
		scanf("%s",input);

		 if(strcmp("e",input) == 0){
			my_msg.mytype=3;
			my_msg.pid=getpid();
			if(msgsnd(msgIdserver1,&my_msg,sizeof(struct my_msgbuf),0)==-1){
				perror("msgsnd failed");
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
		}
		else if(strcmp("n",input) == 0){
			my_msg.mkind='n';
			scanf("%s",&my_msg.mtext[0]);
		}
		else if(strcmp("s",input) == 0){
			my_msg.mkind = 's';
			scanf("%s",my_msg.mtext);

		}

		 //send input to application server
		if(msgsnd(msgIdserver2,&my_msg,sizeof(struct my_msgbuf),0)==-1){
			perror("msgsnd failed");
			exit(EXIT_FAILURE);
		}

		//get result from application server
		status = msgrcv(msgIdserver2, &my_msg, sizeof(struct my_msgbuf),allowed_type,0);
		if(status == -1){
			perror("could not receive msg from queue");
			exit (EXIT_SUCCESS);
		}
		//print result
		printf("result is %d \n",my_msg.result);

	}
	return EXIT_SUCCESS;
}


//ftok-------------------
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
			perror("msgget2 failed in reader\n");
			exit (EXIT_FAILURE);
		}
	return id;
}
