#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TRUE 1

int main(int argc, char *argv[] )  
{
    while (TRUE)
    {
        /*.......................PROMPT.......................*/
        int writeBytes = write(1, "myshell$", 8);
        //printf("\nwrote with %d bytes\n", writeBytes);

        /*.......................READ.LINE.......................*/
        char line[32];
        int commandBytes = read(0, line, 8);
        printf(line);

        /*.......................PARSE.LINE.......................*/
        //Read the line to find the snumber of space characters
        char lineChar = line[0];
        int space;
        while (lineChar != '\0')
            space += (lineChar == ' ') ? 1 : 0;

        //Declar parameters array
        char* parameters[space];
        
        //set each of the entries to one of the parameters, with the first being the command itself
        lineChar = line[0];
        for (int i = 0; i < space; i++)
        {
            while (lineChar != '\0')
            {
                
            }
        }
        parameters[0] = "ls";
        parameters[1] = ".";
        parameters[2] = NULL;
        char command[40] = "/bin/ls";
        printf(command);

        /*.......................FORK.......................*/
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