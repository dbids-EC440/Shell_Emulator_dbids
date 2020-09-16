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
    bool firstIteration = true;
    bool endShell = false;
    char** parameters = NULL;
    char* command = NULL;

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
        //If the line is blank them using the EOT operator(ctrl + D) should result in a line which is zero bytes long
        if (commandBytes == 0) break;

        /*.......................PARSE.LINE.......................*/
        //Parse the parameters from the line
        int space = 0;
        parameters = getParameters(line, space, firstIteration, parameters);
        printf("\nparameters[0] : %s\n", command);

        //Get command from the parameters
        command = getCommand(parameters, firstIteration);
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

        //Free the dynamic memory
        if (!firstIteration)
        {
            free(parameters);
        }
    }
    
    return 0;   
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

    printf("parameters within getParameters: %s\n", parameters[0]);
    return parameters;
}

char* getCommand(char** parameters, bool firstIteration)
{
    char* command;
    static char** pathOptions;
    int numOptions = 0;

    //Set up command dynamically
    command = malloc(TOKEN_SIZE*sizeof(char));
    strcpy(command, parameters[0]);
    printf("%s\n", command);

    //Get all the possible enviornment variable paths
    const char* paths = getenv("PATH");
    printf("path : %s\n", paths);
    printf("length : %ld\n", strlen(paths));

    //Figure out the number of different paths
    for (long int li = 0; li < (strlen(paths)+1); li++)
    {
        numOptions += (paths[li] == ':') ? 1 : 0;
    }
    
    //Dynamically declare pathOptions variable to hold them
    int pathSize = ((numOptions + 1) * sizeof(char*));
    pathOptions = malloc(pathSize);
    for (int i = 0; i < numOptions+1; i++)
    {
        pathOptions[i] = malloc(TOKEN_SIZE*sizeof(char));
    }

    //Split up the paths char into different path options
    numOptions = 0;
    char tempPath[strlen(paths)];
    int tp = 0;
    for (long int li = 0; li < (strlen(paths)+1); li++)
    {
        if ((paths[li] == ':') || (paths[li] == '\0'))
        {
            pathOptions[numOptions] = tempPath;
            numOptions++;
            memset(tempPath, 0, sizeof(tempPath)); //clears the tempPath variable
            tp = 0;
        }
        else
        {
            tempPath[tp] = paths[li];
            tp++;
        }
    }

    //Concatenate each path option with the parameter
    printf("parameters[0] : %s\n", command);

    /*
    char temp_command[TOKEN_SIZE] = "/bin/";
    
    strcat(temp_command, parameters[0]);
    command = temp_command;
    printf("%s %s\n",temp_command, command);
    */

    return command;
}
