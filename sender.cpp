#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// for S_IRUSR and S_IWUSR flags, include sys/stat.h
#include <sys/stat.h>
#include <cstring>
#include "msg.h"    /* For the message struct */
#include <iostream>

/* The size of the shared memory segment */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void* sharedMemPtr;

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory 
 * @param msqid - the id of the allocated message queue
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

	/* Get the id of the shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE */
	key_t key = ftok("keyfile.txt", 'a');
	
	//MIG: ERROR CHECK
	
	/* Attach to the shared memory */
	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, S_IRUSR | S_IWUSR);

	//MIG: ERROR CHECK

	
	/* Attach to the message queue */
	sharedMemPtr = shmat(shmid, NULL, 0);
	
	
	//MIG: ERROR CHECK
	
	/* Store the IDs and the pointer to the shared memory region in the corresponding function parameters */
	msqid = msgget(key, 0666);

	//MIG: ERROR CHECK
	
	
}

/**
 * Performs the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */
void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
	/* TODO: Detach from shared memory */
	shmdt(sharedMemPtr);
}

/**
 * The main send function
 * @param fileName - the name of the file
 * @return - the number of bytes sent
 */
unsigned long sendFile(const char* fileName)
{

	/* A buffer to store message we will send to the receiver. */
	message sndMsg; 
	
	/* A buffer to store message received from the receiver. */
	ackMessage rcvMsg;
		
	/* The number of bytes sent */
	unsigned long numBytesSent = 0;
	
	/* The sender's message */
	message sentMessage;
		
	/* Open the file */
	FILE* fp =  fopen(fileName, "r");

	/* Was the file open? */
	if(!fp)
	{
		perror("fopen");
		exit(-1);
	}
	
	key_t key = ftok("keyfile.txt", 'a');


	/* Read the whole file */
	while(!feof(fp))
	{
		/* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and
 		 * store them in shared memory.  fread() will return how many bytes it has
		 * actually read. This is important; the last chunk read may be less than
		 * SHARED_MEMORY_CHUNK_SIZE.
 		 */
		if((sndMsg.size = fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp)) < 0)
		{
			perror("fread");
			exit(-1);
		}
		
		/* TODO: count the number of bytes sent. */		
		numBytesSent += sndMsg.size;
			
			
		/* TODO: Send a message to the receiver telling him that the data is ready
 		 * to be read (message of type SENDER_DATA_TYPE).
 		 */
		sndMsg.mtype = SENDER_DATA_TYPE;
		msgsnd(msqid, &sndMsg, sizeof(message) - sizeof(long), 0);
		//ERROR CHECK
 		 
		
		/* Wait until the receiver sends us a message of type RECV_DONE_TYPE telling us 
 		 * that he finished saving a chunk of memory. 
 		 */
		ackMessage acknowledged;
		// ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
		msgrcv(msqid, &acknowledged, sizeof(ackMessage) - sizeof(long), RECV_DONE_TYPE, 0);
	}
	

	/** TODO: once we are out of the above loop, we have finished sending the file.
 	  * Lets tell the receiver that we have nothing more to send. We will do this by
 	  * sending a message of type SENDER_DATA_TYPE with size field set to 0. 	
	  */
	message doneMessage;
	doneMessage.mtype = SENDER_DATA_TYPE;
	doneMessage.size = 0;

	// int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
	msgsnd(msqid, &doneMessage, sizeof(message) - sizeof(long), 0);
	//MIG: ERROR CHECK
		
	/* Close the file */
	fclose(fp);
	
	return numBytesSent;
}

/*
 * Used to send the name of the file to the receiver
 * @param fileName - the name of the file to send
 */
void sendFileName(const char* fileName)
{
	/* Get the length of the file name */
	int fileNameSize = strlen(fileName);

	/* Make sure the file name does not exceed 
	 * the maximum buffer size in the fileNameMsg
	 * struct. If exceeds, then terminate with an error.
	 */
	if (fileNameSize > MAX_FILE_NAME_SIZE)
	{
		perror("File name excedes max length");
		exit(-1);
	}

	/* Create an instance of the struct representing the message
	 * containing the name of the file.
	 */
	fileNameMsg nameMsg;

	std::cout << "In sendFileName()" << std::endl;

	/* Set the file name in the message */
	strncpy(nameMsg.fileName, fileName, fileNameSize + 1);

	/* Set the message type FILE_NAME_TRANSFER_TYPE */
	nameMsg.mtype = FILE_NAME_TRANSFER_TYPE;

	/* Send the message using msgsnd */
	std::cout << "sending a message..." << std::endl;

	// starting 'sender' without recv running crashes here
	msgsnd(msqid, &nameMsg, sizeof(fileNameMsg) - sizeof(long), 0);
	
	//ERROR CHECK
	
	std::cout << "filenmae message has been sent" << std::endl;
}


int main(int argc, char** argv)
{
	bool debug = false;
	// for debugging, give filename as global variable
	std::string globalFilename = "The_LaTeX_Companion.pdf";


	/* Check the command line arguments */
	if (argc == 1) {
		std::cout << "Running in debug mode" << std::endl;
		debug = true;
	}
	else if(argc < 2)
	{
		fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
		exit(-1);
	}
		
	/* Connect to shared memory and the message queue */
	std::cout << "connecting..." << std::endl;
	init(shmid, msqid, sharedMemPtr);
	
	/* Send the name of the file */
	std::cout << "sending file..." << std::endl;

	// during debug, provide filename as global variable
	if (debug){
		// don't know how to pass a param to gdb, so using a 'canned' filename if started with no params
		// Maybe should also test if prog is called 'sender_debug'
		std::cout << "opening global filename file" << std::endl;
		sendFileName(globalFilename.c_str());
	} else {
    	sendFileName(argv[1]);
	}

	/* Send the file */
	if (debug) {
		std::cout << "In debug mode, transferring from " << globalFilename << " file." << std::endl;
		fprintf(stderr, "The number of bytes sent is %lu\n", sendFile(globalFilename.c_str()));

	} else {
		fprintf(stderr, "The number of bytes sent is %lu\n", sendFile(argv[1]));
	}
	/* Cleanup */
	cleanUp(shmid, msqid, sharedMemPtr);
		
	return 0;
}
