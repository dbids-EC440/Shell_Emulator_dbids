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
#include <signal.h>

#define TRUE 1
#define TOKEN_SIZE 32 //maximum length of tokens
#define INPUT_SIZE 512 //maximum length of inputs
#define STDIN 0
#define STDOUT 1
#define STDERR 2

//Define the sigaction struct for handling zombie processes
void zombieHandler(int sig, siginfo_t *info, void *ucontext)
{
    //Here I call wait so that the pid of the zombie is properly removed from the process table
    int wstatus;
    waitpid(-1, &wstatus, WNOHANG);
}

/*...................MAIN FUNCTION........................*/
int main(int argc, char *argv[])  
{
    //Declare parameters initially
    char line[INPUT_SIZE];
    bool firstIteration = true;
    bool endShell = false;
    char** parameters = NULL;
    char command[TOKEN_SIZE];
    int numCommands = 0;
    char inFilename[TOKEN_SIZE];
    char outFilename[TOKEN_SIZE];

    //These are bools that I use to detect presence of the meta characters
    bool isLT = false; //detects <
    bool isGT = false; //detects >
    bool isVB = false; //detects |
    bool isAM = false; //detects &

    //Initialize the action that is taken when the parent recieves SIGCHLD
    struct sigaction act = { 0 };
    act.sa_sigaction = &zombieHandler; //calls the zombieHandler function for SIGCHLD
    act.sa_flags = SA_SIGINFO;
    int backgroundNum = 0; //This counts the number of background processes since myshells inception

    //Loop for shell
    while (TRUE)
    {
        /*.......................PROMPT.......................*/
        if (argc == 1)
        {
            int writeBytes = write(STDOUT, "myshell$", 8);
        }
        else if (argv[1] != "-n" && argv[2] != "\0")
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
            exit(EXIT_SUCCESS);
        }
        //Checks for invalid read which tends to happen with background processes
        else if (commandBytes < 0)
        {
            /*int c; 
            while((c = getchar()) != '\n' && c != EOF); */
            read(STDIN, line, INPUT_SIZE);
        }

        /*.......................PARSE.LINE.......................*/
        //clear the meta char bools
            if (isAM) isAM = false;
            if (isLT) isLT = false;
            if (isGT) isGT = false;
            if (isVB) isVB = false;
        
        //Read the line to find the number of space characters
        int space = 0;
        int lNum = 0;
        int pipeNum = 0; //counts the number of pipes
        while (line[lNum] != '\0')
        {
            //Check for meta-chars
            switch(line[lNum])
            {
                case '&':
                    //Check for \n after the ampersand as that should be the only possibility
                    if(line[lNum+1] != '\n')
                    {
                        write(STDERR, "ERROR: Ampersand only allowed one time at the end of the command\n", 65);
                        exit(EXIT_FAILURE);
                    }
                    //set isAM to true and decrement space as needed
                    isAM = true;
                    if (line[lNum-1] == ' ')
                        space--;
                    break;
                case '|':
                    isVB = true;
                    pipeNum++;
                    if (line[lNum+1] == ' ')
                        space--;
                    if (line[lNum-1] == ' ')
                        space--;
                    break;
                case '<':
                    //Check for second < after the first
                    if(isLT)
                    {
                        write(STDERR, "ERROR: Only allowed to redirect input to one file for a single command\n", 71);
                        exit(EXIT_FAILURE);
                    }
                    //set isLT to true and decrement space as needed
                    isLT = true;
                    if (line[lNum+1] == ' ')
                        space--;
                    if (line[lNum-1] == ' ')
                        space--;
                    break;
                case '>':
                    //Check for second < after the first
                    if(isGT)
                    {
                        write(STDERR, "ERROR: Only allowed to redirect output to one file for a single command\n", 72);
                        exit(EXIT_FAILURE);
                    }
                    //set isGT to true and decrement space as needed
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
            int fNum = 0;
            char currentMeta = '>';
            char oppositeMeta = '<';
            char* currentFilename[TOKEN_SIZE];
            while(moreMeta)
            {
                switch(line[lNum])
                {
                case '|':
                    printf("not yet");
                    exit(EXIT_FAILURE);
            
                    break;
                case '&':
                    moreMeta = false;

                    break;
                case '<':
                case '>':

                    //Change vars between < and > operations
                    if (line[lNum] == '<')
                    {
                        currentMeta = '<';
                        oppositeMeta = '>';
                        *currentFilename = inFilename;
                    }
                    else
                    {
                        currentMeta = '>';
                        oppositeMeta = '<';
                        *currentFilename = outFilename;
                    }
                
                    //First elmimate spaces before the filename
                    if (line[lNum] == currentMeta)
                        lNum++;
                    while (line[lNum] == ' ') 
                        lNum++;

                    //Check for double <<
                    if(line[lNum] == currentMeta)
                    {
                        write(STDERR, "ERROR: Only allowed to redirect input or output to one file for a single command\n", 81);
                        exit(EXIT_FAILURE);
                    }

                    //Then get the whole filename
                    fNum = 0;
                    memset(*currentFilename, 0, TOKEN_SIZE);
                    while (line[lNum] != '\0' && line[lNum] != ' ' && line[lNum] != '\n'
                    && line[lNum] != '&' && line[lNum] != '|' && line[lNum] != oppositeMeta)
                    {
                        *(*(currentFilename)+fNum) = *(line+lNum);
                        lNum++;
                        fNum++;
                    }

                    //Then check for more meta characters
                    if(line[lNum] == '\0' || line[lNum] == '\n')
                    {
                        moreMeta = false;
                    }
                    else 
                    {
                        //Move to the next non space char
                        while (line[lNum] == ' ')
                            lNum++;
                    }
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
        pid_t pid = fork();
        if (pid!=0)
        {
            /*..........PARENT..........*/ 
            //Associates the SIGCHILD signal with the given handler
            sigaction(SIGCHLD, &act, NULL);

            if (!isAM)
            {
                //Wait for the child if not a background process
                waitpid(-1, &status, 0);
                
                //Check for Error in executed process
                if (status != 0)
                {
                    fprintf(stderr,"ERROR: The status was %d\n", status);
                    exit(EXIT_FAILURE);
                } 
            }
            else
            {
                //Displays the backgroundNum and PID of the background process
                printf("[%d] %d\n", ++backgroundNum, pid);
            }     
        }
        else
        {
            /*..........CHILD..........*/
            //Redirect STDIN if < character was used
            if (isLT)
            {
                fd[0] = open(inFilename, O_RDONLY);
                close(STDIN);
                dup2(fd[0], STDIN);
                close(fd[0]);
            }
            //Redirect STDOUT if > character was used
            if (isGT)
            {
                fd[1] = open(outFilename, O_WRONLY);
                close(STDOUT);
                dup2(fd[0], STDOUT);
                close(fd[0]);  
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