CC = gcc
CFLAGS = -Wall -std=c99
testFlags = -Wall -std=c99 -g
optFlags = -Wall -std=c99 -O


prodcon: producer.c consumer.c queue.c mainDriver.c mainHeader.h
	$(CC) $(CFLAGS) -o prodcon queue.c producer.c consumer.c mainDriver.c

testProdcon: producer.c consumer.c queue.c mainDriver.c	mainHeader.h
	$(CC) $(testFlags) -o testProdcon queue.c producer.c consumer.c	mainDriver.c

optimizedProdcon: producer.c consumer.c queue.c mainDriver.c mainHeader.h
	$(CC) $(optFlags) -o optimizedProdcon queue.c producer.c consumer.c	mainDriver.c