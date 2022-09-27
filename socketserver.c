#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>



#define MAXWORDS 100000
#define MAXLEN 1000

char* words[MAXWORDS];
int numWords = 0;


void pexit(char* errmsg) {
   fprintf(stderr, "%s\n", errmsg);
   exit(1);
}


int main() {
   char line[MAXLEN];

   /*key_t key;
   int msgid;
   message.mesg_type = 1;

   key = ftok(getenv("HOME"), 1);
   msgid = msgget(key, 0666 | IPC_CREAT);
   printf("Key %d Msgid %d\n", key, msgid); */



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


   int listenfd = 0, connfd = 0;
   struct sockaddr_in serv_addr;

   char buffer[1025];
   time_t ticks;

   if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      pexit("socket() error.");

   memset(&serv_addr, '0', sizeof(serv_addr));
   memset(buffer, '0', sizeof(buffer));

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

   int port = 4999;
   do {
      port++;
      serv_addr.sin_port = htons(port);
   } while (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0);
   printf("bind() succeeds for port #%d\n", port);

   if (listen(listenfd, 10) < 0)
      pexit("listen() error.");

   int counter = 0;

   while (1)
   {
      connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
      counter++;
      printf("connected to client %d.\n", counter);
      if (fork() > 0)
         continue;

      int count;
      //hardcode to send "ls" command output
// snprintf(buffer, sizeof(buffer), "Client %d: %.24s\r\n", counter, ctime(&ticks));
        //FILE *fcommand = popen("ls -l", "r");


      //get the output of that command and forward it to client
      // while ((n = fread(buffer, 1, sizeof(buffer), fcommand)) > 0)
//      write(connfd, buffer, n);

      // DISPLAY STRING
      char guess;
      char actualWord[MAXLEN];
      srand(time(NULL));
      strcpy(actualWord, words[rand() % numWords]);
      int len = strlen(actualWord);
      printf("%s\n", actualWord);

      char display[100];

      for (int i = 0; i < len; i++)
      {
         display[i] = '*';
         printf("%c", display[i]);
      }
      display[len] = NULL;
      printf("\n");

      int unexposed = len;
      int wrongGuesses = 0;


      snprintf(buffer, sizeof(buffer), "Enter a letter in word %s\n", display);
      write(connfd, buffer, sizeof(buffer));
      printf("%s\n", buffer);

      while (unexposed > 0 && (count = read(connfd, buffer, sizeof(buffer))) > 0)
      {
         guess = buffer[0];

         printf("From client: %s\n", guess);

         int found = 0;
         for (int i = 0; i < len; i++) {
            if (guess == actualWord[i]) {
               found = 1;

               if (guess == display[i]) {
                  snprintf(buffer, sizeof(buffer), "%c is already in the word.\n", guess);
                  write(connfd, buffer, sizeof(buffer));
                  printf("%s\n", buffer);

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

            snprintf(buffer, sizeof(buffer), "%c is not in the word.\n", guess);
            write(connfd, buffer, sizeof(buffer));
            printf("%s\n", buffer);

            wrongGuesses++;
         }

         if (guess != '\0')
         {
            snprintf(buffer, sizeof(buffer), "%s\n", display);
            write(connfd, buffer, sizeof(buffer));
            printf("%s\n", buffer);

         }
      }

      snprintf(buffer, sizeof(buffer), "The word is %s.\n", actualWord);
      write(connfd, buffer, sizeof(buffer));
      printf("%s\n", buffer);


      snprintf(buffer, sizeof(buffer), "You missed %d times.\n", wrongGuesses);
      write(connfd, buffer, sizeof(buffer));
      printf("%s\n", buffer);

      // GAME ENDED
      snprintf(buffer, sizeof(buffer), "GAME OVER");
      write(connfd, buffer, sizeof(buffer));

      

      printf("served client %d.\n", counter);
      //pclose(fcommand);
      close(connfd);
      //sleep(1);
      exit(0); //this is child server process. It is done!
   }

   


   return 0;
}