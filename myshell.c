#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TRUE 1
#define TOKEN_SIZE 32 //maximum length of tokens
#define INPUT_SIZE 512 //maximum length of inputs

char** getParameters(char* line, int space, bool firstIteration, char** parameters);
char* getCommand(char** parameters, bool firstIteration);

int main(int argc, char *argv[])  
{
    //Declare parameters initially
    char** parameters;
    bool firstIteration = true;
    bool endShell = false;

    //Loop for shell
    while (TRUE)
    {
        /*.......................PROMPT.......................*/
        int writeBytes;
        if (argc == 1)
        {
            writeBytes = write(1, "myshell$", 8);
        }
        else if (argv[1] != "-n")
        {
            write(3, "ERROR: command line argument not alowed", 39);
        }

        /*.......................READ.LINE.......................*/
        //Read the line
        char line[INPUT_SIZE];
        int commandBytes = read(0, line, INPUT_SIZE);

        //Check for ctrl+D input
        int i;
        /*while(line[i] != '\n')
        {
            if (line[i] == 4) 
            {
                endShell = true;
            }
            i++;
        }*/
        if (commandBytes == 0) 
        {
            break;
        }

        /*.......................PARSE.LINE.......................*/
        //Parse the parameters from the line
        int space = 0;
        char** parameters = getParameters(line, space, firstIteration, parameters);

        //Get command from the parameters
        char* command = getCommand(parameters, firstIteration);

        //Flush the output buffer for saftey
        fflush(stdout);

        /*.......................FORK.......................*/
        int status;
        if (fork()!=0)
        {
            //Parent Code
            waitpid(-1, &status, 0);
            printf("The status was: %d\n", status);
        }
        else
        {
            //Child Code
            execve(command, parameters, NULL);
        } 
    }

    //Free the dynamic memory
    if (!firstIteration)
    {
        free(parameters);
    }   
}


char** getParameters(char* line, int space, bool firstIteration, char** parameters)
{
    //Read the line to find the number of space characters
    space = 0;
    int lNum = 0;
    while (line[lNum] != '\0')
    {
        space += (line[lNum] == ' ') ? 1 : 0;
        lNum++;
    }

    //Split up the line into different strings
    char tempstr[space+2][TOKEN_SIZE];
    lNum = 0;
    int i;
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
    for (i = 0; i < space+1; i++)
    {
        parameters[i] = malloc(TOKEN_SIZE*sizeof(char));
        parameters[i] = tempstr[i];
    }
    parameters[space+1] = NULL;

    return parameters;
}

char* getCommand(char** parameters, bool firstIteration)
{
    int i;
    long int li;
    if (firstIteration)
    {
        //Get all the possible enviornment variable paths
        const char* paths = getenv("PATH");
        printf("path : %s\n", paths);
        printf("length : %ld\n", strlen(paths));
    
        //Figure out the number of 
        static int colon = 0;
        for (li = 0; li < strlen(paths); li++)
        {
            colon += (paths[li] == ':') ? 1 : 0;
            if (paths[li] == ':')
            {
                printf("%c,",paths[li]);
                colon++;
                printf("%d", colon);
            }
        }
        printf("colon : %d\n", paths[li]);
    }

        char temp_command[TOKEN_SIZE] = "/bin/";
        printf("%s\n", parameters[0]); //this is messed up
        strcat(temp_command, parameters[0]);
        char* command = temp_command;
        printf("%s %s\n",temp_command, command);
    
    return command;
}