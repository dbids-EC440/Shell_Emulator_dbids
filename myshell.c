#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define TRUE 1
#define TOKEN_SIZE 32 //maximum length of tokens
#define INPUT_SIZE 512 //maximum length of inputs
#define STDIN 0
#define STDOUT 1
#define STDERR 2

int main(int argc, char *argv[])  
{
    //Declare parameters initially
    char line[INPUT_SIZE];
    bool firstIteration = true;
    bool endShell = false;
    char** parameters = NULL;
    char command[TOKEN_SIZE];
    int numCommands = 0;
    char in_filename[TOKEN_SIZE];
    char out_filename[TOKEN_SIZE] = "fake_file";

    //These are bools that I use to detect presence of the meta characters
    bool isLT = false; //detects <
    bool isGT = false; //detects >
    bool isVB = false; //detects |
    bool isAM = false; //detects &

    //Loop for shell
    while (TRUE)
    {
        /*.......................PROMPT.......................*/
        int writeBytes;
        if (argc == 1)
        {
            writeBytes = write(STDOUT, "myshell$", 8);
        }
        else if (argv[1] != "-n")
        {
            write(STDERR, "ERROR: command line argument not alowed", 39);
        }

        /*.......................READ.LINE.......................*/
        //Read the line
        memset(line, 0, INPUT_SIZE);
        int commandBytes = read(STDIN, line, INPUT_SIZE);

        //Check for ctrl+D input
        /*If the line is blank them using the EOT operator(ctrl + D) should 
          result in a line which is zero bytes long*/
        if (commandBytes == 0) 
        {
            write(STDOUT, "\n", 1);
            exit(0);
        }
        //Checks for invalid read just in case
        else if (commandBytes < 0)
        {
            write(STDERR, "ERROR: reading from STDIN falied with message: ", 49);
            write(STDERR,strerror(errno), sizeof(errno));
        }

        /*.......................PARSE.LINE.......................*/
        //Read the line to find the number of space characters
        int space = 0;
        int lNum = 0;
        while (line[lNum] != '\0')
        {
            //Check for meta-chars
            switch(line[lNum])
            {
                case '&':
                    isAM = true;
                    if (line[lNum+1] == ' ')//used to stop counting of spaces
                        space--;
                    if (line[lNum-1] == ' ')
                        space--;
                    break;
                case '|':
                    isVB = true;
                    if (line[lNum+1] == ' ')
                        space--;
                    if (line[lNum-1] == ' ')
                        space--;
                    break;
                case '<':
                    isLT = true;
                    if (line[lNum+1] == ' ')
                        space--;
                    if (line[lNum-1] == ' ')
                        space--;
                    break;
                case '>':
                    isGT = true;
                    if (line[lNum+1] == ' ')
                        space--;
                    if (line[lNum-1] == ' ')
                        space--;
                    break;
                default:
                    break;   
            } 
            
            //Count number of spaces before meta-chars
            space += (line[lNum] == ' ') ? 1 : 0;
            lNum++;
        }

        //Split up the line into different strings
        char tempstr[space+2][TOKEN_SIZE];
        if (!isAM && !isVB && !isGT && !isLT)
        {
            //Default splitting of line with no meta characters
            lNum = 0;
            for (int i = 0; i < space+1; i++)
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
        }
        else
        {
            //Splitting of line until meta-char
            lNum = 0;
            for (int i = 0; i < space+1; i++)
            {
                int tNum = 0;
                while (line[lNum] != '\0' && line[lNum] != ' ' && line[lNum] != '\n'
                && line[lNum] != '&' && line[lNum] != '|' && line[lNum] != '<' && line[lNum] != '>')
                {
                    tempstr[i][tNum] = line[lNum]; 
                    tNum++;
                    lNum++;           
                }
                tempstr[i][tNum] = '\0';
                if (line[lNum] == ' ') lNum++;
            }

            //Then deal with each meta-char
            bool moreMeta = true;
            while(moreMeta)
            {
                switch(line[lNum])
                {

                case '&':
                
                    printf("not yet");
                    exit(0);
                
                    break;
                case '|':
                
                    printf("not yet");
                    exit(0);
                
                    break;
                case '<':
                    
                    //First elmimate spaces before the filename
                    if (line[lNum] == '<')
                        lNum++;
                    while (line[lNum] == ' ') 
                        lNum++;

                    //Check for double <<
                    if(line[lNum] == '<')
                    {
                        write(STDERR, "ERROR: Only one imput redirection is allowed for a single command\n", 66);
                        exit(0);
                    }

                    //Then get the whole filename
                    int fNum = 0;
                    memset(in_filename, 0, TOKEN_SIZE);
                    while (line[lNum] != '\0' && line[lNum] != ' ' && line[lNum] != '\n'
                    && line[lNum] != '&' && line[lNum] != '|' && line[lNum] != '>')
                    {
                        *(in_filename+fNum) = *(line+lNum);
                        lNum++;
                        fNum++;
                    }

                    //Then check for more meta characters
                    if(line[lNum] == '\0' || line[lNum] == '\n')
                    {
                        moreMeta = false;
                    }
                    else 
                        //Check for second < after the first
                        if(line[lNum] == '<')
                        {
                            write(STDERR, "ERROR: Only one imput redirection is allowed for a single command", 65);
                            exit(0);
                        }

                        //Move to the next non space char
                        while (line[lNum] == ' ')
                            lNum++;
                    break;
                case '>':
                    moreMeta = false;
                    break;
                }
            }
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
        for (int i = 0; i < space+1; i++)
        {
            parameters[i] = malloc(TOKEN_SIZE*sizeof(char));
            parameters[i] = tempstr[i];
        }
        parameters[space+1] = NULL;

        //Get command from the parameters
        strcpy(command, parameters[0]);

        //Flush the output buffer for saftey
        fflush(stdout);

        /*.......................FORK.......................*/
        int status;
        int fd[2];
        if (fork()!=0)
        {
            /*..........PARENT..........*/
            //Wait for the child
            waitpid(-1, &status, 0);
            printf("The status was: %d\n", status);

            //clear the meta char bools
            if (isAM) isAM = false;
            if (isLT) isLT = false;
            if (isGT) isGT = false;
            if (isVB) isVB = false;
        }
        else
        {
            /*..........CHILD..........*/
            //Redirect STDIN if < character was used
            if (isLT)
            {
                fd[0] = open(in_filename, O_RDONLY);
                close(STDIN);
                dup2(fd[0], STDIN);
                close(fd[0]);
            }
            //Redirect STDOUT if > character was used
            if (isGT)
            {
                fd[1] = open(out_filename, O_WRONLY);
                close(STDOUT);
                dup2(fd[0], STDOUT);
                close(fd[0]);  
            }
            //Put in background if AM was used, THIS MIGHT NOT BE NEEDED HERE AND ONLY IN PARENT
            if (isAM)
            {
                execlp("ls", "ls"); //place holder
            }
            //Pipe if VB was used
            if (isVB)
            {
                execlp("ls", "ls"); //place holder
            }
            else
            {
                execvp(command, parameters);
            }
        } 
    }
    //Free the dynamic memory
    if (!firstIteration)
    {
        free(parameters);
    }
    
    return 0;   
}
