CC = gcc
CFLAGS = -Wall -std=c99
testFlags = -Wall -std=c99 -g
optFlags = -Wall -std=c99 -O


factory: producer.c consumer.c queue.c
	$(CC) $(CFLAGS) -o factory queue.c producer.c consumer.c

testFactory: producer.c consumer.c queue.c
	$(CC) $(testFlags) -o testFactory queue.c producer.c consumer.c

optimizedFactory: producer.c consumer.c queue.c
	$(CC) $(optFlags) -o optimizedFactory queue.c producer.c consumer.c