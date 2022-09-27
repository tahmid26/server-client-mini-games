#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>

//read all the quotes from dictionary.txt
//when client asks for a motivational quote, select one randomly and send it out.

#define MAXWORDS 100000
#define MAXLEN 1000

char* words[MAXWORDS];
int numWords = 0;

int main() {
   char line[MAXLEN];

   FILE* fp = fopen("dictionary.txt", "r");
   if (!fp) {
      puts("dictionary.txt cannot be opened for reading.");
      exit(1);
   }

   int i = 0;

   //read one line at a time, allocate memory, then copy the line into array
   while (fgets(line, MAXLEN, fp)) {
      char* cptr = strchr(line, '\n');
      if (cptr)
         *cptr = '\0';

      words[i] = (char*)malloc(strlen(line) + 1);
      strcpy(words[i], line);
      i++;
   }
   numWords = i;
   printf("%d words were read.\n", numWords);


   srand(getpid() + time(NULL) + getuid());

   //create a named pipe to read client's requests
   char filename[MAXLEN];
   sprintf(filename, "/tmp/%s-%d", getenv("USER"), getpid());
   mkfifo(filename, 0600);
   chmod(filename, 0622);
   printf("Send your requests to %s\n", filename);


   while (1) {
      FILE* fp = fopen(filename, "r");
      if (!fp) {
         printf("FIFO %s cannot be opened for reading.\n", filename);
         exit(2);
      }
      printf("Opened %s to read...\n", filename);

      while (fgets(line, MAXLEN, fp))
      {
         char* cptr = strchr(line, '\n');
         if (cptr)
            *cptr = '\0';

         //create a child to work with this client
         if (fork() == 0) {
            FILE* clientfp = fopen(line, "w");
            //create and send new server fifo to the client
            //for private one-on-one communcations
            char serverfifo[MAXLEN];
            sprintf(serverfifo, "/tmp/%s-%d", getenv("USER"), getpid());
            mkfifo(serverfifo, 0600);
            chmod(serverfifo, 0622);

            fprintf(clientfp, "%s\n", serverfifo);
            fflush(clientfp);

            FILE* serverfp = fopen(serverfifo, "r");


            char guess;
            char actualWord[MAXLEN];
            srand(time(NULL));
            strcpy(actualWord, words[rand() % numWords]);
            int n = strlen(actualWord);
            printf("%s\n", actualWord);

            char display[100];

            for (int i = 0; i < n; i++)
            {
               display[i] = '*';
               printf("%c", display[i]);
            }
            display[n] = NULL;
            printf("\n");

            int unexposed = n;
            int wrongGuesses = 0;

            printf("Enter a letter in word %s \n", display);
            fprintf(clientfp, "Enter a letter in word %s\n", display);
            fflush(clientfp);

            while (unexposed > 0)
            {
               if (guess != '\0')
               {
                  printf("Enter a letter in word %s \n", display);
                  fprintf(clientfp, "Enter a letter in word %s\n", display);
                  fflush(clientfp);
               }

               fgets(line, MAXLEN, serverfp);
               char* cptr = strchr(line, '\n');
               if (cptr)
                  *cptr = '\0';
               guess = line[0];

               int found = 0;
               for (int i = 0; i < n; i++) {
                  if (guess == actualWord[i]) {
                     found = 1;

                     if (guess == display[i]) {
                        fprintf(clientfp, "%c is already in the word.\n", guess);
                        fflush(clientfp);
                        printf("%c is already in the word.\n", guess);
                        break;
                     }

                     else {
                        //good guess!
                        display[i] = guess;
                        unexposed--;
                     }
                  }
               }

               if (found == 0 && guess != '\0') {
                  printf("%c is not in the word.\n", guess);
                  fprintf(clientfp, "%c is not in the word.\n", guess);
                  fflush(clientfp);
                  wrongGuesses++;
               }
               //} // extra
            }

            printf("The word is %s.\n", actualWord);
            fprintf(clientfp, "The word is %s.\n", actualWord);
            fflush(clientfp);
            printf("You missed %d times.\n", wrongGuesses);
            fprintf(clientfp, "You missed %d times.\n", wrongGuesses);
            fflush(clientfp);

            exit(0);
         }
      }
      fclose(fp);
   }
}


