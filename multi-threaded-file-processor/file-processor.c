// HASAN YENIADA -- 220201024 -- Homework2

#include <unistd.h>     
#include <sys/types.h>  
#include <errno.h>     
#include <stdio.h>     
#include <stdlib.h>     
#include <pthread.h>    
#include <string.h>     
#include <semaphore.h> 
#include <time.h>


typedef struct
{
	int threadNo;
    char *threadType; 
}THREAD_PARAMETERS;

char resourceType1a[] = "numbers1.txt"; // Type1 resource
char resourceType1b[] = "numbers2.txt"; // Type1 resource
char resourceType2[] = "resource2.txt"; // Type2 resource

void *mainThreadFunc(void *param);

void typeAworkSpace(int threadNo); // Type A threads works on that file
void typeAhelper(int threadNo, char *filename, int fileNo); // Makes cut and paste operations with pasteCuttedLine function.
void pasteCuttedLine(char *filename1,char *filename2, char *line, char *x1[2000], int len, int fileNo);

void typeBworkSpace(int threadNo);  // Type B threads works on that file
void removePrimeNumbers(char *filename, int threadNo, int fileNo); // Makes removing prime number operation.

void typeCworkSpace(int threadNo); // Type C threads works on that file
void removeNegativeNumbers(char *file, int threadNo, int fileNo);

int prime_or_not(int num); // to check whether given number is prime or not
void checkResult( char *resultingFileName ); // When threads finish their works, this function shows total number count in resource2.txt


sem_t mutexFor1a; // Binary semaphore(it is used mutex for resource1a: "numbers1.txt"), initial values = 1, means Files are available at the beginning.
sem_t mutexFor1b; // Binary semaphore(it is used mutex for resource1b: "numbers2.txt")
sem_t mutexFor2;   // Binary semaphore(it is used mutex for resource2: "resource2.txt")

int flag_res1a = 0; //these flags specify whether resource1a or resource1b are empty, 1 means files are empty, 0 means there are numbers in files.
int flag_res1b = 0; // If they are empty, typeA thread finishes its execution, but threadB or threadC can continue, because there can be still prime or nagative number at resource2.


FILE *ptrForResType1; // Type A thread will cut a line from that file
FILE *ptrForResType2; // TypeA thread will paste cutted line to that file

int negativeNumFlag = 1; //that flag specifies whether there is a negative number in resource2 or not. 0 means there is no more negative number.
int primeNumFlag = 1;    //that flag specifies whether there is a prime number in resource2 or not. 0 means there is no more prime number.
int initialFlagForRes2 = 0;  //that flag specifies threadB and threadC cannot work on resource2 at the beginning. 0 means threadB and threadC cannot modify resource2.

int main()
{
    int threadCount = 6;
	pthread_t tid[threadCount];
	pthread_attr_t attr[threadCount];
    int i;

	THREAD_PARAMETERS* lpParameter;
	lpParameter = malloc(sizeof(THREAD_PARAMETERS)* threadCount);
    //initially semaphores are 1, means that all files are available at the beginning
    if(sem_init(&mutexFor1a, 0, 1) != 0) {
        printf("Error initializing semaphores!");
    } 
    if(sem_init(&mutexFor1b, 0, 1) != 0) {
        printf("Error initializing semaphores!");
    }     
    if(sem_init(&mutexFor2, 0, 1) != 0) {
        printf("Error initializing semaphores!");
    } 

	for(i=0; i<threadCount; i++)
	{
		lpParameter[i].threadNo = i + 1;
        if(lpParameter[i].threadNo == 1) {
            lpParameter[i].threadType = "A";    
        }
        else if(lpParameter[i].threadNo == 2) {
            lpParameter[i].threadType = "A";    
        }
        else if(lpParameter[i].threadNo == 3) {
            lpParameter[i].threadType = "A";    
        }
        else if(lpParameter[i].threadNo == 4) {
            lpParameter[i].threadType = "A";    
        }
        else if(lpParameter[i].threadNo == 5) {
            lpParameter[i].threadType = "B"; 
        } 
        else if(lpParameter[i].threadNo == 6) {
            lpParameter[i].threadType = "C"; 
        } 
     
		pthread_attr_init(&attr[i]);
		if(pthread_create( &tid[i], &attr[i], mainThreadFunc, &lpParameter[i]) != 0) {
            printf("Error creating thread!");
        }
	}

    for(i=0; i<threadCount; i++)
	{
		if(pthread_join(tid[i], NULL) != 0) {
            printf("Thread cannot be exited succesfully");
        }
	}

    checkResult(resourceType2);

    if(sem_destroy(&mutexFor1a) != 0) {
        printf("Error while destroying mutex1a");
    }
    if(sem_destroy(&mutexFor1b) != 0) {
        printf("Error while destroying mutex1b");
    }    
    if(sem_destroy(&mutexFor2) != 0) {
        printf("Error while destroying mutex2");
    }

    printf("END OF EXECUTION!!!!\n");

    exit(0);
}


void *mainThreadFunc(void *parameters)
{
	THREAD_PARAMETERS* param = (THREAD_PARAMETERS*)parameters;
    char type1[] = "A";
    char type2[] = "B";
    char type3[] = "C";

    if(strcmp(param->threadType, type1) == 0) {   //typeA
        typeAworkSpace(param->threadNo);
    }
    else if(strcmp(param->threadType, type2) == 0) { // typeB
        typeBworkSpace(param->threadNo);
    }
    
     else if(strcmp(param->threadType, type3) == 0) { // typeB
        typeCworkSpace(param->threadNo);
    }
    
    printf("END OF THREAD %d\n", param->threadNo);
    pthread_exit(0);
}

void typeAworkSpace(int threadNo) {
    time_t t;
    srand(time(NULL));
    int random;
    int i;
    sleep(1); //you can remove that line.

    while(flag_res1a == 0 || flag_res1b == 0) { // threadA works until resourceType1a and resuourceType1b are empty

        if( flag_res1a == 0 && sem_trywait(&mutexFor1a)  == 0 ) { // if resource1a is available check resource2,if resource2 is unavailable signal mutex1a
                
            if(sem_trywait(&mutexFor2)  == 0) { // if resource2 is also available, enter critical section
                    
                random = rand() % 10 + 1;
                for(i = 0; i < random; i++) 
                    {
                        if(flag_res1a == 0) {
                            typeAhelper(threadNo, resourceType1a, 1); // typeAhelper cut a line at each time and paste, 1 means resourceType1a
                            initialFlagForRes2++;
                        }
                        else{
                            break; // if file is empty, thread exit from loop and makes files available again
                        }
                    }
                    sem_post(&mutexFor2);
                    sem_post(&mutexFor1a);

                 }
                 else{ //if  resource2 is not available, do not waste resource1a and signal its mutex.
                    sem_post(&mutexFor1a);
                }
        }

        if( flag_res1b == 0 && sem_trywait(&mutexFor1b)  == 0 ) { // if resource1b is available check resource2, if resource2 is unavailable signal mutex1b
            
            if(sem_trywait(&mutexFor2)  == 0) {

                    random = rand() % 10 + 1;
                    for(i = 0; i < random; i++) 
                    {
                        if(flag_res1b == 0) {
                            typeAhelper(threadNo, resourceType1b, 2); // typeAhelper cut a line at each time and paste, 2 means resourceType1b 
                            initialFlagForRes2++;
                        }
                        else{
                            break;
                        }
                    }
                    sem_post(&mutexFor2); // At the end of cutting and pasting lines, signal mutexes, exit from critical section
                    sem_post(&mutexFor1b);

            } else{
                sem_post(&mutexFor1b); //if  resource2 is not available, do not waste resource1b and signal its mutex.
            }
         }  
        if( sem_trywait(&mutexFor2)  == 0) {  // if resource2 is available check resource1a then resource1b, if noone is available, signal mutex2

            if( flag_res1b == 0 && sem_trywait(&mutexFor1b)  == 0 ) { 
                
                    random = rand() % 10 + 1;
                    for(i = 0; i < random; i++) 
                    {
                        if(flag_res1b == 0) {
                            typeAhelper(threadNo, resourceType1b, 2); // typeAhelper cut a line at each time and paste, 2 means resourceType1b
                            initialFlagForRes2++;
                        }
                        else{
                            break;
                        }
                    }
                    sem_post(&mutexFor1b); // At the end of cutting and pasting lines, signal mutexes, exit from critical section
                    sem_post(&mutexFor2);
            }
            else if( flag_res1a == 0 && sem_trywait(&mutexFor1a)  == 0 ) { //Check res1a
                
                    random = rand() % 10 + 1;
                    for(i = 0; i < random; i++) 
                    {
                        if(flag_res1a == 0) {
                            typeAhelper(threadNo, resourceType1a, 1); // typeAhelper cut a line at each time and paste, 1 means resourceType1a
                            initialFlagForRes2++;
                        }
                        else{                        
                            break;
                        }
                    }
                    sem_post(&mutexFor1a); // At the end of cutting and pasting lines, signal mutexes, exit from critical section
                    sem_post(&mutexFor2); 
            }
            else{ //if noone is available,do not waste resource2, signal mutex2 
                sem_post(&mutexFor2);
            }
        }
                         
    }
}

void typeAhelper(int threadNo, char *fileCutted, int fileNo){
    
    char buf[50];
    char *temp[2000]; 
    char cuttedLine[50];

    ptrForResType1 =fopen(fileCutted,"r"); // read one of type1 resource, and cut its first line and paste to type2 resource
    if (!ptrForResType1){
        printf("Error opening file!\n");
    }

    fgets(buf,50, ptrForResType1);  // if there is no line, flags will be updated, otherwise cutting operation is done by pasteCuttedLine function
    strcpy(cuttedLine, buf); // cuttedLine will be sent to pasteCuttedLine function and operaion will be completed

    int len = 0;
    int k = 0;
    while (fgets(buf,50, ptrForResType1) != NULL) {  // other lines are sent to temp array and then they will be appended the same cleaned file
        temp[k] = malloc(sizeof(char) * 5);
        strcpy(temp[k], buf);
        len++;
        k++;
    }
    fclose(ptrForResType1);

    //CLEAN FILE ------------------
    ptrForResType1 =fopen(fileCutted,"w+"); //cleans file
    if (!ptrForResType1){
        printf("Error opening file!\n");
    }
    fclose(ptrForResType1);
    //CLEAN FILE------------------

    if(cuttedLine != NULL) {
        pasteCuttedLine(fileCutted, resourceType2, cuttedLine, temp, len, fileNo); // this function pastes cutted line and also appends other items from temp to same cleaned file
    }
    else { // if after cutting operation, file is empty, update flags
        if(fileNo == 1){
            flag_res1a = 1; 
        } 
        else if(fileNo == 2){
            flag_res1b = 1; 
        }
    }
}

void pasteCuttedLine(char *filename1, char *filename2, char *pastedLine, char *x1[2000], int len, int fileNo) {
        int i;

        ptrForResType2 =fopen(filename2,"a"); //append cutted line to resource2.
        if (!ptrForResType2){
            printf("Error opening file!\n");
        }
        fprintf(ptrForResType2,"%s", pastedLine); 
        fclose(ptrForResType2);

        if(len > 0) {
            ptrForResType1 =fopen(filename1,"a"); // append other numbers to cleaned type1 resource file.
            if (!ptrForResType1){
               printf("Error opening file!\n");
            }
            for(i = 0; i < len; i++) {
                fprintf(ptrForResType1,"%s", x1[i]);
            }      
            fclose(ptrForResType1);
        }

        if(fileNo == 1 && len == 0){ // update flags if there is no element apart from cutted line.
            flag_res1a = 1; 
        } 
        else if(fileNo == 2 && len == 0){
            flag_res1b = 1; 
        }
} 
 
void typeCworkSpace(int threadNo) { // typeC thread tries to find available file, when it find one of them, cleans negative numbers and signal its mutex.

    while(flag_res1a == 0 || flag_res1b == 0 || negativeNumFlag > 0) {
        sleep(1); // you can delete this line but this prevent quick spin of threadC and increases the speed
        if(flag_res1a == 0 && sem_trywait(&mutexFor1a)  == 0 ) { // typeC thread tries to find available file, when it find one of them, cleans negatives and signal its mutex.
            removeNegativeNumbers( "numbers1.txt", threadNo, 1);
            sem_post(&mutexFor1a);
        }
        if(flag_res1b == 0 && sem_trywait(&mutexFor1b)  == 0 ) { 
            removeNegativeNumbers( "numbers2.txt", threadNo, 2);
            sem_post(&mutexFor1b);
        }
        if( initialFlagForRes2 > 0 && sem_trywait(&mutexFor2)  == 0 ) { 
            removeNegativeNumbers( "resource2.txt", threadNo, 3);
            sem_post(&mutexFor2);
        }
    }    
}

void removeNegativeNumbers(char *filenameC, int threadNo, int fileNo) {
    FILE *noNegativeFile;

    char buf[50];
    char *tempC[2000];

    int num; 
    int len = 0;
    int i;
    int negativeCounter = 0; // to control whether resource2 has negative number or not

    noNegativeFile =fopen(filenameC,"r");
    if (!noNegativeFile){
        printf("Error opening file!\n");
    }
    
    int k = 0;
    while (fgets(buf,50, noNegativeFile) != NULL) {
        num = atoi(buf);
        if(num >= 0){  // if number which read from file is positive, send it to temp array
            tempC[k] = malloc(sizeof(char) * 5);
            strcpy(tempC[k], buf);
            len++;
        }
        else{
            if(fileNo == 3 && num != 0) { // if threadB keep resource2, control negative number count.
                negativeCounter++;
            }
            k--;
        }
        k++;
    }
    fclose(noNegativeFile);

    //CLEAN FILE ------------------
    noNegativeFile =fopen(filenameC,"w+");
    if (!noNegativeFile){
        printf("Error opening file!\n");
    }
    fclose(noNegativeFile);
    //CLEAN FILE------------------

    if(len > 0) {
        noNegativeFile = fopen(filenameC, "a");

        if (!noNegativeFile){
            printf("Error opening file!\n");
        }

        for(i = 0; i < len; i++) {
            fprintf(noNegativeFile, "%s", tempC[i]);
        }     
    fclose(noNegativeFile);
    }  

    if(fileNo == 1 && len == 0) {flag_res1a = 1; } //update flags
    else if(fileNo == 2 && len == 0) {flag_res1b = 1; }

    if(fileNo == 3){ // to update resource2 negative number flag
        negativeNumFlag = negativeCounter; 
    }
}

void typeBworkSpace(int threadNo) { // typeB thread tries to find available file, when it find one of them, cleans primes and signal its mutex.

    while(flag_res1a == 0 || flag_res1b == 0 || primeNumFlag > 0) {
        sleep(1);// you can delete this line but this prevent quick spin of threadB and increases the speed
        if( flag_res1b == 0 && sem_trywait(&mutexFor1b)  == 0 ) { 
            removePrimeNumbers( resourceType1b, threadNo, 2);
            sem_post(&mutexFor1b);
        }

        if( flag_res1a == 0 && sem_trywait(&mutexFor1a)  == 0 ) { 
            removePrimeNumbers( resourceType1a, threadNo, 1);
            sem_post(&mutexFor1a);
        }
        
        if( initialFlagForRes2 > 0 && sem_trywait(&mutexFor2)  == 0 ) { 
            removePrimeNumbers( resourceType2, threadNo, 3);
            sem_post(&mutexFor2);

        }
    }  
}

void removePrimeNumbers(char *filenameB, int threadNo, int fileNo) {
    FILE *noPrimeFile;
    char buf[50];
    char *tempB[2000]; 

    int len = 0;
    int i;
    int num; 

    int primeCounter = 0; // to control whether resource2 has prime number or not
  
    noPrimeFile =fopen(filenameB, "r");
    if (!noPrimeFile){
        printf("Error opening file %s!\n", filenameB);
    }

    int k = 0;
    while (fgets(buf,50, noPrimeFile) != NULL) {
        num = atoi(buf);
        
        if( prime_or_not(num) == 0 || num <= 1 ) { // if number which read from file is positive, send it to temp array
            tempB[k] = malloc(sizeof(char) * 5);
            strcpy(tempB[k], buf);
            len++;
        }
        else{
            if(fileNo == 3){
                primeCounter++; // if threadC keep resource2, control prime number count.
            }

           k--;
        }
        k++;
    }
    fclose(noPrimeFile);

    //CLEAN FILE ------------------
    noPrimeFile =fopen(filenameB,"w+");
    if (!noPrimeFile){
        printf("Error opening file %s!\n", filenameB);
    }
    fclose(noPrimeFile);
    //CLEAN FILE------------------

    if(len > 0) {  //append non prime numbers to cleaned file
        noPrimeFile = fopen(filenameB, "a");
        if (!noPrimeFile){
            printf("Error opening file %s!\n", filenameB);
        }
        for(i = 0; i < len; i++) {
            fprintf(noPrimeFile,"%s", tempB[i]);
        }     
    fclose(noPrimeFile);
    }  

    if(fileNo == 1 && len == 0){flag_res1a = 1; } //update flags
    else if(fileNo == 2 && len == 0){flag_res1b = 1; }

    if(fileNo == 3){
        primeNumFlag = primeCounter; // to update resource2 prime number flag
    }
    
}

int prime_or_not(int num) { // 0 = not a prime number, 1 = prime number
    int i;
    int bool_prime = 1;
    for(i = 2; i < num; i++) {
        if(num % i == 0){
            bool_prime = 0;
            return bool_prime;
        }

    }
    return bool_prime;

}

void checkResult( char *resultingFileName ) { 
    char bufResult[50];

    FILE *lastFile;
    int elementCount= 0;

    int num; 

    lastFile =fopen(resultingFileName,"r");
    if (!lastFile){
        printf("Error opening file!\n");
    }
    while (fgets(bufResult,50, lastFile) != NULL) {
        num = atoi(bufResult);
        elementCount++;
    }
    printf("\nElement Count: : %d\n", elementCount);
}