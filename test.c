
#include <stdio.h>
#include <time.h>

int main() {
   time_t start, end;
   double elapsed;

   time(&start);
   // Your code here
   time(&end);

   elapsed = difftime(end, start);
   printf("The program took %.2f seconds to run.\n", elapsed);

   return 0;
}