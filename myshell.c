#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h> //temp for strcat function

#define TRUE 1

int main(int argc, char *argv[] )  
{
    while (TRUE)
    {
        //PROMPT
        int writeBytes = write(1, "myshell$", 8);
        //printf("\nwrote with %d bytes\n", writeBytes);

        //READ LINE
        char line[32];
        int commandBytes = read(0, line, 8);
        printf(line);

        //PARSE LINE
            //Read the line to find the size of the parameters array
            //Then define parameters and set each of the entries to one of the parameters, with the first being the command itself
        char* parameters[3];
        parameters[0] = "ls";
        parameters[1] = ".";
        parameters[2] = NULL;
        char command[40] = "/bin/";
        strcat(command, line);

        //FORK
        int status;
        if (fork()!=0)
        {
            //Parent Code
            waitpid(-1, &status, 0);
            printf("%d", status);
        }
        else
        {
            //Child Code
            execve(command, parameters, NULL);
        }
        
    }
}