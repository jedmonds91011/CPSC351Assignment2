#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <cstring>
#include "msg.h"    /* For the message struct */
#include <iostream>

using namespace std;

/* The size of the shared memory segment */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void *sharedMemPtr = NULL;

// count for signal handler 
int count = 1;


/**
 * The function for receiving the name of the file
 * @return - the name of the file received from the sender
 */
string recvFileName()
{
	/* The file name received from the sender */
	string fileName; 
        
	/* : declare an instance of the fileNameMsg struct to be
	 * used for holding the message received from the sender.
    */

	fileNameMsg nameMsg;
	
    /*: Receive the file name using msgrcv() */
	if(msgrcv(msqid, &nameMsg, sizeof(fileNameMsg) - sizeof(long), FILE_NAME_TRANSFER_TYPE, 0)<0)
	{
		perror("msgsnd");
		exit(-1);
	}
	
	//fileName = string(nameMsg.fileName);
	
	/* : return the received file name */
	
    return nameMsg.fileName;
}
 /**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory 
 * @param msqid - the id of the shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */
void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
	
	/*  
    1. Create a file called keyfile.txt containing string "Hello world" (you may do
 	    so manually or from the code).
	2. Use ftok("keyfile.txt", 'a') in order to generate the key.
	3. Use will use this key in the TODO's below. Use the same key for the queue
	   and the shared memory segment. This also serves to illustrate the difference
 	   between the key and the id used in message queues and shared memory. The key is
	   like the file name and the id is like the file object.  Every System V object 
	   on the system has a unique id, but different objects may have the same key.
	*/
	

	/* : Allocate a shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE. */
	key_t key = ftok("keyfile.txt", 'a');
		if(key < 0)
	{
		perror("ftok");
		exit(-1);
	}
	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
		if(shmid < 0)
	{
		perror("shmget");
		exit(-1);
	}

	// print shared memory id, so I can kill it when this thing breaks
	// gdb is intercepting Ctrl-C signal, so sometimes process doesn't 
	// finish normally.
	std::cout << "shmid: " << shmid << std::endl;
	
	/* : Attach to the shared memory */
	 sharedMemPtr = shmat(shmid, NULL, 0);
	 
	 /*  Error checks */
	if(((void*)sharedMemPtr) < 0)
	{
		perror("shmat");
		exit(-1); 
	}
	
	/* Create a message queue */
	 msqid = msgget(key, 0666 | IPC_CREAT);
	 	if(msqid < 0)
	{
		perror("msgget");
		exit(-1);
	}
	  
	/* TODO: Store the IDs and the pointer to the shared memory region in the corresponding parameters */
	
}
 

/**
 * The main loop
 * @param fileName - the name of the file received from the sender.
 * @return - the number of bytes received
 */
unsigned long mainLoop(const char* fileName)
{
	/* The size of the message received from the sender */
	int msgSize = -1;
	
	/* The number of bytes received */
	int numBytesRecv = 0;
	
	/* The string representing the file name received from the sender */
	string recvFileNameStr = fileName;
	
	/* append __recv to the end of file name */
	//string appendStr = "__recv";
	
	recvFileNameStr += "__recv";
	
	/* Open the file for writing */
	FILE* fp = fopen(recvFileNameStr.c_str(), "w");
			
	/* Error checks */
	if(!fp)
	{
		perror("fopen");	
		exit(-1);
	}
		

	/* Keep receiving until the sender sets the size to 0, indicating that
 	 * there is no more data to send.
 	 */	
	while(msgSize != 0)
	{	

		/* Receive the message and get the value of the size field. The message will be of 
		 * of type SENDER_DATA_TYPE. That is, a message that is an instance of the message struct with 
		 * mtype field set to SENDER_DATA_TYPE (the macro SENDER_DATA_TYPE is defined in 
		 * msg.h).  If the size field of the message is not 0, then we copy that many bytes from 
		 * the shared memory segment to the file. Otherwise, if 0, then we close the file 
		 * and exit.
		 *
		 * NOTE: the received file will always be saved into the file called
		 * <ORIGINAL FILENAME__recv>. For example, if the name of the original
		 * file is song.mp3, the name of the received file is going to be song.mp3__recv.
		 */
		message rcvMsg;
		msgrcv(msqid, &rcvMsg, sizeof(message) - sizeof(long), SENDER_DATA_TYPE, 0);
		// msgSize is an int. Why are we trying to copy a string into it?
		// strcpy(msgSize, rcvMsg.size);
		
		/* Copy the message size */
		msgSize = rcvMsg.size;	
		
		/* If the sender is not telling us that we are done, then get to work */
		if(msgSize != 0)
		{
			/* TODO: count the number of bytes received */
			numBytesRecv += msgSize;
			
			/* Save the shared memory to file */
			if(fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < 0)
			{
				perror("fwrite");
			}
			
			/* Tell the sender that we are ready for the next set of bytes. 
 			 * I.e., send a message of type RECV_DONE_TYPE. That is, a message
			 * of type ackMessage with mtype field set to RECV_DONE_TYPE. 
 			 */
			ackMessage sndAckMessage;
			sndAckMessage.mtype = RECV_DONE_TYPE;
			msgsnd(msqid, &sndAckMessage, sizeof(ackMessage) - sizeof(long), 0);
		}
		/* We are done */
		else
		{
			/* Close the file */
			fclose(fp);
		}
	}
	
	return numBytesRecv;
}



/**
 * Performs cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */
void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
	/*: Detach from shared memory */
	shmdt(sharedMemPtr);
	
	/*: Deallocate the shared memory segment */
	// needs three params
	shmctl(shmid, IPC_RMID, NULL);

	/*: Deallocate the message queue */
	msgctl(msqid, IPC_RMID, NULL);

}

/**
 * Handles the exit signal
 * @param signal - the signal type
 */
void ctrlCSignal(int signal)
{
	/* Free system V resources */
	cleanUp(shmid, msqid, sharedMemPtr);
}

/* signal handling function */

void signalHandlerFunc(int arg)
{
	/* We can take more Ctrl-Cs */
	if(count > 0)
	{	ctrlCSignal (arg);
		fprintf(stderr, "Deallocated the shared memory and message queue\n");
		exit(0);
	}
	 
	--count;
}
int main(int argc, char** argv)
{
	
	/* : Install a signal handler (see signaldemo.cpp sample file).
 	 * If user presses Ctrl-c, your program should delete the message
 	 * queue and the shared memory segment before exiting. You may add 
	 * the cleaning functionality in ctrlCSignal().
 	 */
	signal(SIGINT, signalHandlerFunc);
	/* Initialize */
	init(shmid, msqid, sharedMemPtr);
	
	/* Receive the file name from the sender */
	string fileName = recvFileName();
	
	/* Go to the main loop */
	fprintf(stderr, "The number of bytes received is: %lu\n", mainLoop(fileName.c_str()));

	/*: Detach from shared memory segment, and deallocate shared memory 
	 * and message queue (i.e. call cleanup) 
	 */
	cleanUp(shmid, msqid, sharedMemPtr);
	return 0;
}
