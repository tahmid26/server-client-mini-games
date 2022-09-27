#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

//read all the quotes from quotes.txt
//when client asks for a motivational quote, select one randomly and send it out.

#define MAXQUOTES 10000
#define MAXLEN 1000
char* commands[100];
int fds[100][2];
int count = 0;
int numPipes;


void runCommand(char* line) {
   //split and assemble the arguments and invoke execvp()
   //use strtok(..)

   char* arguments[MAXLEN];

   char* cmd = strtok(line, " \n");
   arguments[0] = cmd;

   int i = 1;
   //keep getting tokens (individual words)
   while ((arguments[i] = strtok(NULL, " \n")) != NULL)
      i++;

   execvp(cmd, arguments);
}


void child(int i) {
   //rewire pipes
   if (i) {
      close(0);
      dup(fds[i - 1][0]);
   }

   if (i != count - 1)
   {
      close(1);
      dup(fds[i][1]);
   }
   //close unnecessary pipes

   for (int j = 0; j < numPipes; j++)
   {
      close(fds[j][0]);
      close(fds[j][1]);
   }

   //run ith command
   runCommand(commands[i]);
}


void processLine(char* line) {
   char* pipePtr = strchr(line, '|');
   char* equalPtr = strchr(line, '=');
   if (pipePtr) { //not NULL
      // command has several sub - commands connected with pipes
      //   setup commands array
      ;
      char* temp;
      int i = 0;

      temp = strtok(line, "|");
      commands[0] = temp;
      count++;

      while (temp = strtok(NULL, "|\n")) {
         commands[count] = temp;
         count++;
      }


      // setup pipes array
      numPipes = count - 1;
      for (int i = 0; i < numPipes; i++)
         pipe(fds[i]);


      //create children-- > invoke child(i) in a loop */
      for (int i = 0; i < count; i++)
         if (!fork())
            child(i);

      for (int j = 0; j < numPipes; j++)
      {
         close(fds[j][0]);
         close(fds[j][1]);
      }

      for (int i = 0; i < count; i++)
         wait(NULL);

      exit(0);

   }
   else if (equalPtr) {
      // command has = operator, so 2 commands
      char* lhs = strtok(line, "=");
      char* rhs = strtok(NULL, "=");
      // printf("%s\n", rhs);

      char* arguments1[MAXLEN], line[MAXLEN];

      char* cmd1 = strtok(lhs, " \n");
      arguments1[0] = cmd1;

      int i = 1;
      //keep getting tokens (individual words)
      while ((arguments1[i] = strtok(NULL, " \n")) != NULL)
         i++;


      char* arguments2[MAXLEN];

      char* cmd2 = strtok(rhs, " \n");
      arguments2[0] = cmd2;

      int p = 1;
      //keep getting tokens (individual words)
      while ((arguments2[p] = strtok(NULL, " \n")) != NULL)
         p++;

      int tochild[2], toparent[2];

      pipe(tochild);
      pipe(toparent);

      if (fork() > 0)
      {
         close(1);
         dup(tochild[1]);

         close(0);
         dup(toparent[0]);

         close(tochild[0]); close(tochild[1]);
         close(toparent[0]); close(toparent[1]);

         execvp(cmd1, arguments1);
         fputs("I hope you do not see me!", stderr);

      }
      else
      {
         close(0);
         dup(tochild[0]);
         close(1);
         dup(toparent[1]);

         close(tochild[0]); close(tochild[1]);
         close(toparent[0]); close(toparent[1]);

         execvp(cmd2, arguments2);
         fputs("I hope you do not see me!", stderr);

      }
   }
   else
   //it is a simple command, no pipe or = character
      runCommand(line);
}

int main() {
   // load up all the quotes
   char line[MAXLEN];
   char* quotes[MAXQUOTES];
   int numQuotes = 0;


   FILE* fp = fopen("quotes.txt", "r");
   if (!fp) {
      puts("quotes.txt cannot be opened for reading.");
      exit(1);
   }
   int i = 0;

   //read one line at a time, allocate memory, then copy the line into array
   while (fgets(line, MAXLEN, fp)) {
      quotes[i] = (char*)malloc(strlen(line) + 1);
      strcpy(quotes[i], line);
      i++;
   }
   numQuotes = i;


   // infinite loop to serve the customer
   while (1) {
      //output a random quote to stderr
      srand(time(0));
      fputs(quotes[rand() % numQuotes], stderr);
      fprintf(stderr, "# ");
      //get the user input
      fgets(line, 1000, stdin);

      //spawn a child for taking care of it
      if (fork() == 0)
         processLine(line);

      //wait the child to finish the job!
      int x = 0;
      wait(&x);
   }
}
