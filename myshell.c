#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>

#define TRUE 1
#define STRING_SIZE 1024 //maximum length of strings
#define READ_BYTES 1024  //maximum number of bytes to be read or written

int main(int argc, char *argv[] )  
{
    //Declare gdparameters initially
    char** parameters;
    bool firstIteration = true;

    //Loop for shell
    while (TRUE)
    {
        /*.......................PROMPT.......................*/
        int writeBytes = write(1, "myshell$", 8);

        /*.......................READ.LINE.......................*/
        char line[READ_BYTES];
        int commandBytes = read(0, line, READ_BYTES);

        /*.......................PARSE.LINE.......................*/
        //Read the line to find the number of space characters
        int space;
        int lNum = 0;
        while (line[lNum] != '\0')
        {
            space += (line[lNum] == ' ') ? 1 : 0;
            lNum++;
        }
        
        //Split up the line into different strings
        char tempstr[space+2][STRING_SIZE];
        int i;
        lNum = 0;
        for (i = 0; i < space+1; i++)
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
        int parameterSize = ((space+2) * sizeof(char*));
        if (firstIteration)
        {
            parameters = malloc(parameterSize);
            firstIteration = false;
        }
        else
        {
            parameters = realloc(parameters,parameterSize);
        }
        
        for (i = 0; i < space+2; i++)
        {
            parameters[i] = malloc(STRING_SIZE*sizeof(char));
            parameters[i] = tempstr[i];
        }
        parameters[space+1] = NULL;

        //Get command from the parameters
        char command[STRING_SIZE] = "/bin/ls";

        /*.......................FORK.......................*/
        int status;
        if (fork()!=0)
        {
            //Parent Code
            waitpid(-1, &status, 0);
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
