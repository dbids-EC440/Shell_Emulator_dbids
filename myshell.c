#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TRUE 1
#define STRING_SIZE 32 //maximum length of strings
#define RWBYTES 8  //maximum number of bytes to be read or written
#define LINE_LENGTH 32 //maximum length of input lines

int main(int argc, char *argv[] )  
{
    while (TRUE)
    {
        /*.......................PROMPT.......................*/
        int writeBytes = write(1, "myshell$", RWBYTES);
        //printf("\nwrote with %d bytes\n", writeBytes);

        /*.......................READ.LINE.......................*/
        char line[LINE_LENGTH];
        int commandBytes = read(0, line, RWBYTES);
        //printf(line);

        /*.......................PARSE.LINE.......................*/
        //Read the line to find the number of space characters
        char lineChar = line[0];
        int space;
        int l = 0;
        while (lineChar != '\0')
        {
            space += (lineChar == ' ') ? 1 : 0;
            l++;
            lineChar = line[l];
        }
           
        //Declare parameters array
        char* parameters[space+2];
        
        //Set each of the entries to one of the parameters, with the first being the command itself
        lineChar = line[0];
        for (int i = 0; i < space+1; i++)
        {
            int j = 0;
            char tempstr[STRING_SIZE];
            while (lineChar != '\0' && lineChar != ' ')
            {
                tempstr[j] = lineChar;
                j++;
                lineChar = line[j];
            }
            tempstr[j] = '\0';
            parameters[i] = tempstr;
        }
        parameters[space+1] = NULL;
	
        for (int i = 0; i < space+2; i++)
        {
            printf(parameters[i]);
        }

        /*parameters[0] = "ls";
        parameters[1] = ".";
        parameters[2] = NULL;*/
        char command[STRING_SIZE] = "/bin/ls";
        //printf(command);

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
