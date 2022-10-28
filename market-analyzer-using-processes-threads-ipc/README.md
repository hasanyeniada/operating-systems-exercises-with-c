# Market Analyzer App Using Processes & Threads & Inter Process Communications (IPC)

A C program that is explained below using WinAPI. 

I implemented a program that analyzes market transactions and calculate the summation of how
many items (MILK, BISCUIT, CHIPS and COKE) are sold in seven days.

First your program should create seven child processes that represent a day. 
Each child process should be responsible of a day. More than one child process should not
analyze the same day. 

Each child’s day is decided by parent process.
Then each child process should create four threads. 
Each thread should calculate the summation of an item. 
More than one thread should not count the same item.
The market transactions and items are stored in a file called market.txt. 
Therefore, the threads should read the market.txt file to count the transactions.
