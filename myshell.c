#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define TRUE 1
#define TOKEN_SIZE 32 //maximum length of tokens
#define INPUT_SIZE 512 //maximum length of inputs
#define STDIN 0
#define STDOUT 1
#define STDERR 2

int main(int argc, char *argv[])  
{
    //Declare parameters initially
    bool firstIteration = true;
    bool endShell = false;
    char** parameters = NULL;
    char command[TOKEN_SIZE];
    int numCommands = 0;
    char in_filename[TOKEN_SIZE] = "test_file";//(char *)malloc(TOKEN_SIZE*sizeof(char));
    char out_filename[TOKEN_SIZE];

    //These are bools that I use to detect the presence of the meta characters
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
        char line[INPUT_SIZE];
        int commandBytes = read(STDIN, line, INPUT_SIZE);

        //Check for ctrl+D input
        /*If the line is blank them using the EOT operator(ctrl + D) should 
          result in a line which is zero bytes long*/
        if (commandBytes == 0) 
        {
            writeBytes = write(STDOUT, "\n", 1);
            exit(0);
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
                    line[lNum+1] = '\0'; //used to stop counting of spaces
                    if (line[lNum-1] == ' ')
                        space--;
                    break;
                case '|':
                    isVB = true;
                    line[lNum+1] = '\0';
                    if (line[lNum-1] == ' ')
                        space--;
                    break;
                case '<':
                    isLT = true;
                    line[lNum+1] = '\0';
                    if (line[lNum-1] == ' ')
                        space--;
                    break;
                case '>':
                    isGT = true;
                    line[lNum+1] = '\0';
                    if (line[lNum-1] == ' ')
                        space--;
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
                    
                    //First elmimate spaces (or weirdly \0) before the filename
                    lNum++;
                    while (line[lNum] == ' ' || line[lNum] == '\0') 
                        lNum++;

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
                        //Move to the next non space char
                        do{
                            lNum++;
                        }while (line[lNum] == ' ');
                    
                        if(line[lNum] == '<')
                        {
                            write(STDERR, "ERROR: Only one imput redirection is allowed for a single command", 128);
                            exit(0);
                        }
                    break;
                case '>':

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
        //char in_filename[] = "ls_test";
        char out_filename[] = "fake_file";
        if (fork()!=0)
        {
            //Parent Code
            waitpid(-1, &status, 0);
            printf("The status was: %d\n", status);
        }
        else
        {
            //Child Code

            //Redirect STDIN if < character was used
            if (isLT)
            {
                fd[0] = open(in_filename, O_RDONLY);
                close(STDIN);
                dup2(fd[0], STDIN);
                close(fd[0]);
            }
            //Redurect STDOUT if > character was used
            if (isGT)
            {
                fd[0] = open(out_filename, O_WRONLY);
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
