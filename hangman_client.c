#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXLEN 1000

int main(int argc, char* argv[]) {
   if (argc != 2) {
      puts("Usage: gclient <server-fifo-name>");
      exit(1);
   }

   // argv[1] is the server fifo name

   char clientfifo[MAXLEN];
   sprintf(clientfifo, "/tmp/%s-%d", getenv("USER"), getpid());
   mkfifo(clientfifo, 0600);
   chmod(clientfifo, 0622);

   //open argv[1] for writing, send clientfifo
   FILE* fp = fopen(argv[1], "w");
   //add \n after clientfifo to avoid potential race conditions in server
   fprintf(fp, "%s\n", clientfifo);
   fclose(fp);


   //open clientfifo for reading and read the quote & print in the screen
   FILE* clientfp = fopen(clientfifo, "r");
   //read the new server-fifo, then open it for writing! as serverfp
   char serverfifo[MAXLEN];
   fscanf(clientfp, "%s", serverfifo);
   char line[MAXLEN];
   fgets(line, MAXLEN, clientfp); //get rid of \n


   FILE* serverfp = fopen(serverfifo, "w");
   printf("Hangman:\n");


   if (fork())
      //parent - responsible for getting user input and sending to server
      while (1) {
         //get the user input - one line at a time and send it to server.
         fgets(line, MAXLEN, stdin);

         //send that line to server
        if ( fprintf(serverfp, "%s\n", line))
        {
         fflush(serverfp);
         }
         else
                break;
      }
   else
      //child - responsible for getting responses and displaying
      while (1) {
        if (fgets(line, MAXLEN, clientfp))
        {
        // line[strlen(line) - 1] = 0;
         puts(line);
         }
         else {
         break;
         }
      }

   fclose(clientfp);

   unlink(clientfifo);
}

