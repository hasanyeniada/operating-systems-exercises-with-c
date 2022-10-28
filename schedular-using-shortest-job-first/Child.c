#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

void writeToPipe(int pid);
void readFromPipe(int pid);

HANDLE hPipe;
#define NO_OF_PROCESS 5

int main(int argc, char* argv[])
{
	int pid; //keeps child process id.
	int executionTime; //keeps actual execution time, 50 < executionTime < 300 , will be specified randomly.

	//check arguments for error
	if (argc != 3)  //if there is no 3 argument, exit.
	{
		printf("error in child %d process...now exiting %d\n", argv[0]);
		system("pause");
		exit(0);
	}

	pid = atoi(argv[2]) + 1; //last argument is process id of child.
	srand(time(NULL) + 100 * pid); 

	hPipe = CreateFile(   //Connect to the server pipe using CreateFile().
		argv[1], 
		GENERIC_READ |  GENERIC_WRITE,
		0,             
		NULL,           
		OPEN_EXISTING,  
		0,              
		NULL);        

	if (INVALID_HANDLE_VALUE == hPipe)
	{
		printf("\nError occurred in child %d while connecting to the server: %d\n",pid,  GetLastError());
		system("pause");
		ExitProcess(0);
	}

	int i;
	for (i = 0; i < NO_OF_PROCESS; i++) { // every child is executed 5 times.
		printf("ROUND %d, CHILD %d in execution!\n", i + 1, pid);
		
		readFromPipe(pid); //message of parent has taken, now child can be executed.

		executionTime = (rand() % 250) + 50; //actual length of i'th CPU burst.

		SleepEx(executionTime, TRUE);  //wait until actual length is done.
		
		printf("---CHILD %d has executed %dms.\n",pid, executionTime);
		
		writeToPipe(pid, executionTime); //send actual CPU burst time to parent, after that parent can send message to other waiting child.
		
		printf("End of ROUND %d\n\n", i + 1);
	}
	
	printf("\nEND OF CHILD %d's execution!!\n", pid);

	system("pause");
	CloseHandle(hPipe);
	return 1;
}

void writeToPipe(int pid, int exTime) //standart writing to pipe function.
{ 
	char tn[50];
	sprintf_s(tn, sizeof(tn), "%d", exTime); //actual execution time is keep in tn to send parent.

	int bytesToWrite = 0;
	int bytesWritten = 0;
	bytesToWrite = strlen(tn);
	bytesToWrite++;

	if (!WriteFile(hPipe, tn, bytesToWrite, &bytesWritten, NULL) || (strlen(tn) + 1 != bytesWritten)) //standart WriteFile.
	{
		printf("Error in writing to pipe in child %d!\n", pid);
		CloseHandle(hPipe);
		system("pause");
		exit(0);
	}
	else
	{
		printf("---CHILD %d has sent its execution time to PARENT.Now waiting others...\n", pid); //WriteFile is successful, actual execution time is sent to parent.
	}
}

void readFromPipe(int pid) //standart reading from pipe function
{
	char message[256];
	DWORD dwRead;

	if (!ReadFile(hPipe, message, strlen(message), &dwRead, NULL) || dwRead == 0) //standart ReadFile.
	{
		printf("Child %d cannot read message of parent!\n", pid);
		CloseHandle(hPipe);
		system("pause");
		exit(0);
	}
	else //ReadFiile is successful,message of parent is taken.
	{
		printf("---CHILD %d has taken the message of PARENT.Now in execution...\n", pid);
		printf("---Message of parent => %s\n", message);
	}
}

