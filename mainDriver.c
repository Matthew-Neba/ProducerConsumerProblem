#ifndef mainHeader
#define mainHeader
#include "mainHeader.h"
#endif

int main (int argc, char *argv[]) {

    if (argc < 3) {
        printf("Usage: ./prodcon <number of consumers> <output file number>\n");
        exit(0);
    }

    //get number of consumers and filename
    int numberConsumers = atoi(argv[1]);

    int lengthFileString = strlen(argv[2]) + strlen("prodcon.") + strlen(".log");
    char outputFile[lengthFileString + 1];
    outputFile[0] = '\0';

    sprintf(outputFile, "prodcon.%s.log", argv[2]);

    printf("Output file: %s\n", outputFile);
}