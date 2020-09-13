#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>

#define TOKEN_SIZE 32 //maximum length of tokens
#define INPUT_SIZE 512 //maximum length of inputs

int main(int argc, char *argv[])  
{
    //Declare gdparameters initially
    char** parameters;
    bool firstIteration = true;
    bool endShell = false;
    int space = 0;

    //Loop for shell
    while (!endShell)
    {
        /*.......................PROMPT.......................*/
        int writeBytes;
        if (argc == 1)
            writeBytes = write(1, "myshell$", 8);
        else if (argv[1] != "-n")
            write(3, "ERROR: command line argument not alowed", 39);

        /*.......................READ.LINE.......................*/
        char line[INPUT_SIZE];
        int commandBytes = read(0, line, INPUT_SIZE);

        //Check for ctrl+D input
        if (line == "ctril+D")
        {
            endShell = true;
            break;
        }

        /*.......................PARSE.LINE.......................*/
        //Read the line to find the number of space characters
        space = 0;
        int lNum = 0;
        while (line[lNum] != '\0')
        {
            space += (line[lNum] == ' ') ? 1 : 0;
            lNum++;
        }
        
        //Split up the line into different strings
        char tempstr[space+1][TOKEN_SIZE];
        int i;
        lNum = 0;
        for (i = 0; i < space; i++)
        {
            int tNum = 0;
            while (line[lNum] != '\0' && line[lNum] != ' ' && line[lNum] != '\n')
            {
                tempstr[i][tNum] = line[lNum];
                tNum++;
                lNum++;
            }
            tempstr[i][tNum] = '\0';
            lNum++;
        }

        //Declare parameters array dynamically and set equal to tempstr
        int parameterSize = ((space+1) * sizeof(char*));
        if (firstIteration)
        {
            parameters = malloc(parameterSize);
            firstIteration = false;
        }
        else
        {
            parameters = realloc(parameters,parameterSize);
        }
        for (i = 0; i < space+1; i++)
        {
            parameters[i] = malloc(TOKEN_SIZE*sizeof(char));
            parameters[i] = tempstr[i];
        }
        parameters[space+1] = NULL;

        //Get command from the parameters
        char command[TOKEN_SIZE] = "/bin/ls";

        while ((getchar()) != '\n');
        fflush(stdout);

        /*.......................FORK.......................*/
        int status;
        if (fork()!=0)
        {
            //Parent Code
            waitpid(-1, &status, 0);
            printf("The status was: %d", status);
        }
        else
        {
            //Child Code
            execve(command, parameters, NULL);
        } 
    }

    //Free the dynamic memory
    free(parameters);
}
