CC = gcc
CFLAGS = -Wall -std=gnu11
testFlags = -Wall -std=gnu11 -g
optFlags = -Wall -std=gnu11 -O


# if a target dependencies are other targets, then make those targets first
# if a target dependencies are files, then make sure those files exist and are up to date
# need to link pthread library since it is not a standard library


testProdcon:  mainDriver.c mainHeader.h jobQueue.c
	$(CC) $(testFlags) -o testProdcon mainDriver.c jobQueue.c jobFunctions.c -pthread
	
prodcon:  mainDriver.c mainHeader.h jobQueue.c
	$(CC) $(CFLAGS) -o prodcon mainDriver.c jobQueue.c jobFunctions.c -pthread

optimizedProdcon: mainDriver.c mainHeader.h jobQueue.c
	$(CC) $(optFlags) -o optimizedProdcon mainDriver.c jobQueue.c jobFunctions.c -pthread