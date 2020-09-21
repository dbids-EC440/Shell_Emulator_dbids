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
#define TOKEN_SIZE 32       //maximum length of tokens
#define INPUT_SIZE 512      //maximum length of inputs
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

//Just a maximum function for an array of ints.  Used for space variable below.
int largest(int arr[], int n) 
{ 
    int max = arr[0];  
    for (int i = 1; i < n; i++) 
        if (arr[i] > max) 
            max = arr[i]; 
  
    return max; 
} 

/*...................MAIN FUNCTION........................*/
int main(int argc, char *argv[])  
{
    //Declare parameters initially
    char line[INPUT_SIZE];
    bool firstIteration = true;
    bool endShell = false;
    char*** parametersPointer = NULL;
    char inFilename[TOKEN_SIZE];
    char outFilename[TOKEN_SIZE];
    int space[INPUT_SIZE / 2];                  //counts the number of spaces (and in turn arguments) for each command (possibly multiple due to pipe)
    int lNum = 0;                               //iterates through the line
    int pipeNum = 0;                            //counts the number of pipes
    int currentPipeNum = 0;                     //iterate through the pipes
    bool throwError = false;

    //These are bools that I use to detect presence of the meta characters
    bool isLT = false; //detects <
    bool isGT = false; //detects >
    bool isVB = false; //detects |
    bool isAM = false; //detects &

    //Initialize the action that is taken when the parent recieves SIGCHLD
    struct sigaction act = { 0 };
    act.sa_sigaction = &zombieHandler;  //calls the zombieHandler function for SIGCHLD
    act.sa_flags = SA_SIGINFO;
    int backgroundNum = 0;              //This counts the number of background processes since myshells inception


    //Loop for shell
    while (TRUE)
    {
        //Reset error bool used for preventing execution upon error
        throwError = false;
        
        /*.......................PROMPT.......................*/
        //Prompts the user with the shell statement
        if (argc == 1)
        {
            int writeBytes = write(STDOUT, "myshell$", 8);
        }
        //Error checks the command line arguments passed when invoking the shell
        else if (strcmp(argv[1], "-n") && (argv+2) != NULL)
        {
            write(STDERR, "ERROR: command line argument not alowed", 39);
        }

        /*.......................READ.LINE.......................*/
        //Read the line
        memset(line, 0, INPUT_SIZE);                        //clears the line
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
        
        /*..Read the line to find the number of space characters in each 
        pipe seperated command, the types of meta chars, and the number of pipes..*/
        for (int i = 0; i < (INPUT_SIZE / 2); i++)
        {
            space[i] = 0;
        }
        lNum = 0;
        pipeNum = 0;
        while (line[lNum] != '\0')
        {
            //Check for meta-chars first
            switch(line[lNum])
            {
                case '&':
                    //Check for \n after the ampersand as that should be the only possibility
                    if(line[lNum+1] != '\n')
                    {
                        write(STDERR, "ERROR: Ampersand only allowed one time at the end of the command\n", 65);
                        throwError = true;
                        break;
                    }
                    //set isAM to true and decrement space as needed
                    isAM = true;
                    if (line[lNum-1] == ' ')
                        space[pipeNum]--;
                    break;

                case '|':
                    //set isVB to true and count the number of pipes
                    isVB = true;
                    pipeNum++;
                    if (line[lNum+1] == ' ')
                        space[pipeNum]--;
                    if (line[lNum-1] == ' ')
                        space[pipeNum - 1]--;
                    break;

                case '<':
                    //Check for second < after the first
                    if(isLT)
                    {
                        write(STDERR, "ERROR: Only allowed to redirect input to one file for a single command\n", 71);
                        throwError = true;
                        break;
                    }
                    //set isLT to true and decrement space as needed
                    isLT = true;
                    if (line[lNum+1] == ' ')
                        space[pipeNum]--;
                    if (line[lNum-1] == ' ')
                        space[pipeNum]--;
                    break;

                case '>':
                    //Check for second < after the first
                    if(isGT)
                    {
                        write(STDERR, "ERROR: Only allowed to redirect output to one file for a single command\n", 72);
                        throwError = true;
                        break;
                    }
                    //set isGT to true and decrement space as needed
                    isGT = true;
                    if (line[lNum+1] == ' ')
                        space[pipeNum]--;
                    if (line[lNum-1] == ' ')
                        space[pipeNum]--;
                    break;

                default: //Possibly unneccessary, but added just in case
                    break;   
            } 
            //Count number of spaces before meta-chars
            space[pipeNum] += (line[lNum] == ' ') ? 1 : 0;
            lNum++;
        }

        /*.......Split up the line into different strings.......*/

        int maxSpace = largest(space, INPUT_SIZE/2);

        //The output parameters are stored in the variable tempstr
        char tempstr[pipeNum+1][maxSpace+2][TOKEN_SIZE];

        if (!isAM && !isVB && !isGT && !isLT)
        {
            //Default splitting of line with no meta characters
            lNum = 0;
            for (int i = 0; i < space[0]+1; i++)
            {
                int tNum = 0;
                while (line[lNum] != '\0' && line[lNum] != ' ' && line[lNum] != '\n')
                {
                    tempstr[0][i][tNum] = line[lNum]; 
                    tNum++;
                    lNum++;          
                }
                tempstr[0][i][tNum] = '\0';
                lNum++;
            }
        }
        else
        {
            //Splitting of line until meta-char
            lNum = 0;
            while (line[lNum] != '&' && line[lNum] != '|' && line[lNum] != '<' && line[lNum] != '>')
            {
                int i = 0;
                int tNum = 0;
                while (line[lNum] != '\0' && line[lNum] != ' ' && line[lNum] != '\n'
                && line[lNum] != '&' && line[lNum] != '|' && line[lNum] != '<' && line[lNum] != '>')
                {
                    tempstr[0][i][tNum] = line[lNum]; 
                    tNum++;
                    lNum++;           
                }
                tempstr[0][i][tNum] = '\0';
                if (line[lNum] == ' ') lNum++;
                i++;
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
                    //Increase the current pipe number for the tempstr array
                    currentPipeNum++;

                    //First elmimate spaces before the filename
                    if (line[lNum] == '|')
                    {
                        lNum++;
                        if (line[lNum] == '|')
                        {
                            write(STDERR, "ERROR: Cannot place two pipe characters in succession\n", 56);
                            throwError = true;
                            break;
                        }
                    }
                    while (line[lNum] == ' ') 
                        lNum++;

                    //Continue to split the line until the a meta char or 
                    int i = 0;
                    while (line[lNum] != '&' && line[lNum] != '|' && line[lNum] != '<' && line[lNum] != '>' && line[lNum] != '\n')
                    {
                        int tNum = 0;
                        while (line[lNum] != '\0' && line[lNum] != ' ' && line[lNum] != '\n'
                        && line[lNum] != '&' && line[lNum] != '|' && line[lNum] != '<' && line[lNum] != '>')
                        {
                            tempstr[currentPipeNum][i][tNum] = line[lNum]; 
                            tNum++;
                            lNum++;           
                        }
                        tempstr[currentPipeNum][i][tNum] = '\0';
                        if (line[lNum] == ' ') lNum++;
                        i++;
                    }

                    //Then check for more meta characters
                    if(line[lNum] == '\0' || line[lNum] == '\n')
                    {
                        moreMeta = false;
                    }
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
                    while (line[lNum] == currentMeta || line[lNum] == ' ')
                        lNum++;

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
        
        /*.......Declare parameters array dynamically and set equal to tempstr.......*/
        int parameterSize = ((maxSpace+2) * sizeof(char*));
        int pipeParameterSize = ((pipeNum+1) * sizeof(char**));
        currentPipeNum = 0;
        if (firstIteration)
        {
            parametersPointer = malloc(pipeParameterSize);
        }
        else
        {
            parametersPointer = realloc(parametersPointer,pipeParameterSize);
        }
        for (int j = 0; j < pipeNum+1; j++)
        {
            if (firstIteration)
            {
                parametersPointer[j] = malloc(parameterSize);
            }
            else
            {
                parametersPointer[j] = realloc(parametersPointer[j],parameterSize);
            }
            for (int i = 0; i < space[j]+1; i++)
            {
                parametersPointer[j][i] = malloc(TOKEN_SIZE*sizeof(char));
                strcpy(parametersPointer[j][i], tempstr[j][i]);
            }
            parametersPointer[j][space[j]+1] = NULL;
        }
        
        //Flush the output buffer for saftey
        fflush(stdout);

        /*.......................FORK.......................*/
        //Establish variables common to the parent and child
        currentPipeNum = 0;
        int status;
        int iofd[2];
        int pipefd[pipeNum][2];
        
        //Pipe and check for piping erros
        for (int i = 0; i < pipeNum; i++)
        {
            int pipeReturn = pipe(&pipefd[i][0]);
            if (pipeReturn)
            {
                perror("Error in pipe(): ");
            }
        }
        
        //Actually fork
        pid_t pid;
        for (currentPipeNum = 0; (currentPipeNum <= pipeNum) && !throwError; currentPipeNum++)
        {
            printf("%d\n", currentPipeNum);
            pid = fork();
            if (pid!=0)
            {
                /*..........PARENT..........*/ 
                //Associates the SIGCHILD signal with the given handler
                sigaction(SIGCHLD, &act, NULL);

                //Close all file descriptors for the parent
                if (pipeNum == currentPipeNum)
                {
                    for (int i = 0; i < pipeNum; i++)
                    {
                        close(pipefd[i][0]);
                        close(pipefd[i][1]);
                    }
                    close(iofd[0]);
                    close(iofd[1]);
                }

                //Wait for the child if not a background process, or if it is the end of the pipeline
                if (!isAM)
                {
                    waitpid(pid, &status, 0);
                    
                    //Check for Error in executed process
                    if (status != 0)
                    {
                        fprintf(stderr,"ERROR: Process failed with status %d\n", status);
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
                    if ((isVB && currentPipeNum == 0) || (!isVB)) //checks for piping conditions
                    {
                        iofd[0] = open(inFilename, O_RDONLY);
                        close(STDIN);
                        dup2(iofd[0], STDIN);
                        close(iofd[0]);
                    } 
                }
                //Redirect STDOUT if > character was used
                if (isGT)
                {
                    if ((isVB && currentPipeNum == pipeNum) || (!isVB)) //checks for piping conditons
                    {
                        iofd[1] = open(outFilename, O_WRONLY);
                        close(STDOUT);
                        dup2(iofd[1], STDOUT);
                        close(iofd[1]);  
                    }
                }
                //Pipe if VB was used
                if (isVB)
                {
                    //PIPES STDIN from prev command, if this is the second or later command
                    if(currentPipeNum > 0)
                    {
                        printf("stdin piped\n");
                        close(pipefd[currentPipeNum-1][1]);
                        close(STDIN);
                        dup2(pipefd[currentPipeNum-1][0], STDIN);
                        close(pipefd[currentPipeNum-1][0]);
                    }
                    
                    //PIPES STDOUT to next command, if there is a next pipe
                    if (currentPipeNum != pipeNum)
                    {
                        printf("stdout piped\n");
                        close(pipefd[currentPipeNum][0]);
                        close(STDOUT);
                        dup2(pipefd[currentPipeNum][1], STDOUT);
                        close(pipefd[currentPipeNum][1]);
                    }
                }
                //Finally execute
                execvp(parametersPointer[currentPipeNum][0], parametersPointer[currentPipeNum]);
            }
        } 
    }
    //Free the dynamic memory
    if (!firstIteration)
    {
        free(parametersPointer);
    }
    
    return 0;   
}