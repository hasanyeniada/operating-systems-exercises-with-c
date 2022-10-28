//Hasan YENIADA_220201024_HW3

#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>

#define NO_OF_PROCESS 5

void writeToPipe(int pid);
float readFromPipe(int pid);

float execTime[NO_OF_PROCESS] = { 300, 220, 180, 45, 255 }; //Initial estimated value of CPU bursts, at every step these will be updated 
float tempExecTime[NO_OF_PROCESS];  // temp is used to sort execTime array, changes is done on temp array to modify order array, and execTime does not affected from that changes
float exactExecTime[NO_OF_PROCESS]; //keeps exact execution times(tn) which is taken from child
int orderOfChilds[NO_OF_PROCESS]; //keeps execution order of childs

void decideOrder();  // at the end of every step, next execution orders of childs are specified in that function
void swap(int *xp, int *yp); //basic swapping function for sorting

#define BUFFER_SIZE 128 //1k
HANDLE hPipe[NO_OF_PROCESS]; //we have 5 child and 5 named pipe

int main()
{ 
	char* pipeNames[NO_OF_PROCESS] = { "\\\\.\\Pipe\\Pipe1", "\\\\.\\Pipe\\Pipe2","\\\\.\\Pipe\\Pipe3", "\\\\.\\Pipe\\Pipe4", "\\\\.\\Pipe\\Pipe5" };

	HANDLE processHandles[NO_OF_PROCESS];
	PROCESS_INFORMATION pi[NO_OF_PROCESS];
	STARTUPINFO si[NO_OF_PROCESS];
	//childs takes pipe name from command line as argument
	char* lpCommandLine[NO_OF_PROCESS] = { "Child.exe \\\\.\\Pipe\\Pipe1 0" , "Child.exe \\\\.\\Pipe\\Pipe2 1" , "Child.exe \\\\.\\Pipe\\Pipe3 2", "Child.exe \\\\.\\Pipe\\Pipe4 3", "Child.exe \\\\.\\Pipe\\Pipe5 4"};
	int i, j, pid;

	for (i = 0; i < NO_OF_PROCESS; i++) {
		SecureZeroMemory(&si[i], sizeof(STARTUPINFO));
		si[i].cb = sizeof(STARTUPINFO);
		SecureZeroMemory(&pi[i], sizeof(PROCESS_INFORMATION));

		hPipe[i] = CreateNamedPipe(  //pipes are created
			pipeNames[i],
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,         
			PIPE_UNLIMITED_INSTANCES,
			BUFFER_SIZE,       
			BUFFER_SIZE,  
			NMPWAIT_USE_DEFAULT_WAIT,
			NULL);             

		if (INVALID_HANDLE_VALUE == hPipe[i])
		{
			printf("\nError occurred while creating the pipe: %d", GetLastError());
			system("pause");
			ExitProcess(0);
		}

		if (!CreateProcess(NULL, lpCommandLine[i], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si[i], &pi[i])) { //childs are created
			printf("\nError occurred while creating child: %d", GetLastError());
			CloseHandle(hPipe[i]);
			system("pause");
			ExitProcess(0);
		}
		else
		{
			processHandles[i] = pi[i].hProcess; 
		}

		if (!ConnectNamedPipe(hPipe[i], NULL))
		{
			printf("\nError occurred while connecting to the client: %d", GetLastError());
			CloseHandle(hPipe);
			system("pause");
			ExitProcess(0);
		}
	}//end of pipes and child process creation and connection to pipes

	float alpha = 0.5; //alpha is given as 0.5
	float next;//next cpu burst time of child is kept in that variable

	for (i = 0; i < NO_OF_PROCESS; i++) 
	{
		for (int b = 0; b < NO_OF_PROCESS; b++) {
			tempExecTime[b] = execTime[b]; //execTimes keeps estimated execution times(tn+1),and I copy it to temp to prevent undesired change on it while sorting
		}		
		decideOrder(); //sorts tempExecTimes and updates orderOfChilds array to schedule childs
		
		printf("\n%d 'th round estimated execution times(Tn+1):\n ", i+1);
		printf("\n-------------ROUND %d---------------\n", i+1);
		printf("Process  EstimatedExecutionTime(Tn+1)\n");
		for (int r = 0; r < NO_OF_PROCESS; r++) {
			printf("P%d       %f\n ", r + 1, execTime[r]);
		}
		printf("\n---------------------------------\n\n");
		printf("\n%d'th round execution order<P%d, P%d, P%d, P%d, P%d>\n",i+1, orderOfChilds[0], orderOfChilds[1], orderOfChilds[2], orderOfChilds[3], orderOfChilds[4]);
		
		for (j = 0; j < NO_OF_PROCESS; j++) {

			pid = orderOfChilds[j] - 1; //pid shows which child will be executed
			
			writeToPipe(pid); //writes to child in execution and child can 

			printf("P%d started.\n", pid+1);
			
			exactExecTime[pid] = readFromPipe(pid); //exact execution time is taken from child
			
			next = (alpha * exactExecTime[pid]) + (alpha * execTime[pid]); //next cpu burst of that child is estimated
			execTime[pid] = next; //and that next estimated cpu burst is updated in execTime array 
			
			printf("P%d ended.\n", pid+1);
		}
		//step i ended, actual execution times are taken from childs and printed at below before next step
		printf("\n%d'th round actual execution times:\n", i+1);
		printf("\n---------ROUND %d-----------\n", i+1);
		printf("Process  ActualLength(tn)\n");
		for (int t = 0; t < NO_OF_PROCESS; t++) {
			printf("P%d:      %fms\n ", t + 1, exactExecTime[t]);
		}
		printf("\n--------END OF ROUND %d----------\n\n", i+1);
	}

	printf("!!!END OF EXECUTION!!!\n\n");
	for (i = 0; i < NO_OF_PROCESS; i++) {
		CloseHandle(pi[i].hThread);
		CloseHandle(pi[i].hProcess);
		CloseHandle(hPipe[i]);
	}
	
	system("pause");
	return 1;
}

void writeToPipe(int pid) // standart writing to pipe function
{
	int i = 0;
	int bytesToWrite = 0;
	int bytesWritten = 0;
	
	char buffer[256];
	sprintf_s(buffer, sizeof(buffer), "CHILD %d, You can start to execution!", pid+1);
	
	bytesToWrite = strlen(buffer);
	bytesToWrite++;

	if (!WriteFile(hPipe[pid], buffer, bytesToWrite, &bytesWritten, NULL) || (strlen(buffer) + 1 != bytesWritten) )
	{
		printf("Error in writing to pipe in parent!\n");
		CloseHandle(hPipe[pid]);
		system("pause");
		exit(0);
	}
}

float readFromPipe(int pid) // standart reading from pipe function
{
	int i;
	DWORD dwRead;
	char exactExeTime[50]; //keeps child's actual execution time

	int Tn;
	if (!ReadFile(hPipe[pid], exactExeTime, strlen(exactExeTime), &dwRead, NULL) || dwRead == 0) {
		printf("Error while reading from pipe in parent!\n");
		CloseHandle(hPipe[pid]);
		system("pause");
		exit(0);
	}
	else {
		Tn = atof(exactExeTime); //EXACT EXECUTION TIME WHICH IS TAKEN FROM CHILD
	}
	return Tn;
}

void swap(int *num1, int *num2)
{
	int temp = *num1;
	*num1 = *num2;
	*num2 = temp;
}

void decideOrder() //basic selection sort function
{
	int i, j, minimumNo;

	for (i = 0; i < NO_OF_PROCESS - 1; i++) //sorts predicted next CPU burst times
	{
		minimumNo = i;
		for (j = i + 1; j < NO_OF_PROCESS; j++)
			if (tempExecTime[j] < tempExecTime[minimumNo])
				minimumNo = j;
		swap(&tempExecTime[minimumNo], &tempExecTime[i]);
	}

	for (i = 0; i < 5; i++) //update orderOfChilds array to send message to childs in correct order
		for (j = 0; j < NO_OF_PROCESS; j++)
			if (tempExecTime[i] == execTime[j])
				orderOfChilds[i] = j + 1;
}