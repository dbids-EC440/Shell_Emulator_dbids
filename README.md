# Project 1 README by Devin Bidstrup
### Written for Boston University EC440: Introduction to Operating Systems
This will explain to the reader the way which this shell was programmed sequentially with how it is ordered in the myshell.c file.

# Before Main

#### #Feature Test Macros
Here the feature test macro _POSIX_C_SOURCE was defined to be 199309L as that is needed for the sa_sigaction part of the sigaction handler used below.

#### #Pre-Processor Directives
Here the TOKEN_SIZE, and INPUT_SIZE are declared to be 32 and 512 respectively.  These are limitations to the shells functionality.

#### zombieHandler
This is a handler function which when called waits for a child.  This is used by the shell when the ampersand (&) meta character is given to remove the zombie process from the process table when the SIGCHLD signal is sent by the child.

#### isNotMeta
A simple function which returns true if the input character is not one of the meta characters we are considering ('&', '|', '<', '>')

#### largest
A function which returns the largest element of an integer array, used for the space variable in order to know the maximum amount of parameters passed per command, allowing for the proper allocation of memory.

## `int main(int argc, char *argv[])`
Whether or not it is stylistically best, the majority of the code is contained within the header int main(int argc, char *argv[]).

#### Variable declarations
All variables have been described in the code with comments as to their nature.

#### Sigaction declaration
Here the struct sigaction act is declared and initialized, with the act.sa_sigaction parameter set to the function pointer of zombieHandler.  Therefore when the SIGCHLD is recieved, the act struct will call the zombieHandler function.

## `while(TRUE)`
Continues to loop the shell forever until the exit function is used via ctrl+D.

# PROMPT
The line `myshell$` is written to STDOUT for the user.  Additionally, if the user entered any command line argument besides -n, the shell program should write an error message to STDERR.

# READ LINE
First the line variable is cleared, and then the read system call is used to take input from the STDIN buffer until the user either hits enter '\n' or ctrl + D (EOT).
Then the program checks if the user entered a zero byte line, which is only possible when one presses ctrl+D, and if so exits the program.
If the read was invalid, then the read command is called again.

# PARSE LINE
This section takes the input from the user and parses it into seperate parameters to prepare for execution.

#### Clear Meta Char Bools
The bools declared before main to detect the presence of meta characters are cleared, so as to prevent the program from malfunctioning after the first command.

## Scanning the line
The goal here is to find the number of space characters in each pipe seperated command, the types of meta chars, and the number of pipes.  It uses the loop 'while (line[lNum] != '\0')' to iterate through the *whole* line and set these variables.

### `switch(line[lNum])`
This switch statement is used to check if each char is a meta character.  If so it sets the corresponding bool to true.  Additionally, to deal with space characters preceeding and following meta characters, each case statement  decrements the appropriate element of the space array if neccessary.  Finally, within this case statement there are checks to make sure that the meta characters were used correctly (e.g. ampersand used only once at the end of the line).

### Space counter
This is the section of code following the switch statement, but still within the while loop for scanning the code.  If a space character is found it increments the correct element of the space array.  Additionally the statement `lNum++` is used to increment the iterate through each character of the line.

## Split the line
The goal here is to split the line based on the spaces and meta chars and put each parameter in the array defined by the line `char tempstr[pipeNum+1][maxSpace+2][TOKEN_SIZE];`.

### Default line splitting without meta characers
Without meta characters, splitting the line is as simple as a nested for loop.  The outer for loop iterates the variable i and runs for the number of time corresponding to the number of parameters (one more than the number of spaces).  Then inside of that there is a while loop.  This while loop iterates through the line until either the end of the line char or the space char is found.  In essence this while loop will run through the line while the line iterator is still pointing to the same parameter.  During this loop each char of the line is copied over to one of the strings within the tempstr string array to contain that parameter.  Once the while loop is complete, the end of the token is set to the null char `\0`, and the for loop is iterated for the number of parameter.

### Splitting of the line with meta characters
#### Split until meta character
First there is a neted for and while loop identical to the default splitting of the line except that it ends once a meta character is pointed to by the iterator through the line string.

#### Then deal with each meta character
First there is a while statement which iterates through each meta character until there are no more.  Within that is a switch statement which selects the appropriate meta char to deal with.

##### `case '|':` or Pipe
First I increase the variable currentPipeNum which is used to index into the tempstr array.  Then the line is iterated through while the iterator points to either the pipe character or the space character, throwing an error if  two or more pipes are placed in succession.  Then another loop is called similar to the default line splitting which sets the tempstr[currentPipeNum] string array to the parameters between this pipe char and the next meta char or end of the line.  Finally it checks to see if there are more meta chars, and if not sets moreMeta to false to exit the while(moreMeta) loop.

##### `case '&':` or Launch Process in Background
Here more meta is simply set to false as this doesn't affect the parameters, and I have already error checked for the characters placement.

#### `case '<': or case '>':` or Read/Write to File
Here I combine the read and write from file sections of the line splitter, as they are identical except for a few key variables.  To facilitate this, the variable currentMeta is first set to the meta character currently pointed to by line[lNum].  Additionally the currentFilename pointer is set to point to the string corresponding to the given action.
Then the spaces are eliminated before the filename, similar to the pipe command.  Finally the filename is cleared and then set within a while loop.  This loop iterates until a whitespace character or another meta character is detected, and sets the corresponding filename string equal to the characters of the line pointed to by the iterator during this time.  Finally it checks to see if there are more meta chars, and if not sets moreMeta to false to exit the while(moreMeta) loop.

### Declare parameters array dynamically and set equal to tempstr
This section dynamically allocated memory for the char*** parametersPointer array of string arrays.  This array will contain all of the parameters needed by execvp in the following section to run the commands.  First the outermost part is allocated based on the number of pipes in the program.  Then the second outermost layer is allocated based on the maximum number of parameters for a given command.  Finally the innermost section is allocated to be the size of the maximum token length, and set equal to the corresponding string in the tempstr array. Finally the last element of each string array was set to be a null pointer as required by exec.  This array was needed to allow for reallocation upon subsequent loops, and because the exec system call requires the input of a string literal.  

# FORK

### Pipe
Here each of the pipes are set up by the pipe system call, for which the file descriptors are stored in the `pipefd[pipeNum][2]` array.  Should there be an error in the pipe system call, the string pointed to by errno will additionally be displayed using the perror function.

### Fork Loop
With this for loop the fork system call is used to fork a child process from the parent (Note that this is within an if statement used to insure that the pid if that of the parent).

#### Parent
Within the parent the sigaction system call is used to associate the SIGCHLD systen call with the act sigaction struct.  This means that when the parent recieves the SIGCHLD system call it will execute the zombieHandler function and wait to collect the exit status from the child, removing it from the process table.  This sigaction call is neccessary to prevent zombie processes.
The parent also checks the process of the executed function, and if it is not zero (therefore there has been an error in the execution within the child) then the status is displayed.
If the child process will be run as a background process, then the a fun, bash-like command is printed to STDOUT.
Note that the parent does not wait here.  This was to prevent the parent waiting for the first child process to finish before executing again, an error which took me over two days to trouble shoot (otherwise I would have been done early).

### Children

##### < character
The child first checks to see if a < character was used.  If so, and it is the first command, then the input is redirected from STDIN to the read file descriptor, described step by step in the comments to the side of the code.

##### > character
The child next checks to see if a > character was used.  If so, and it is the last command, then the output is redirected from STDOUT to the write file descriptor, described step by step in the comments to the side of the code.

##### | character
Next the child checks to see if the pipe command was used.  Then if it is the second command or later it takes input from the pipe as opposed to STDIN.  Note here that the pipe is referred to by currentPipeNum - 1.  This is because the child should read from the pipe outputted to by the previous child (when currentPipeNum was currentPipeNum - 1).  If it is the not the last command, the child writes its output to the pipe as opposed to STDOUT.  Here the pipe is referred to by currentPipeNum.  Again, the details to these two piping actions are commented in the code, and all actions are error checked by the program.

##### Execute
The program execute the commands given in the parametersPointer array based on the currentPipeNum, and error checks for failed execution.

#### Parent Again?
Here the parent closes the pipe file descriptors after the last child that needed them has finished.  This is within the loop, but outside of the fork.

### Final Loop
This last loop is where the parent waits or the children due to the aformentioned desire to allow the children to run in parallel.  Note that the parent only waits if these were not meant to be background processes.

#### free(parametersPointer)
Neccessary due to nature of dynamic memory.

# Known Bugs
1. One may have to press ctrl + D a number of times before the program exits.  The root cause of this bug is unknown to me at the time, but I will update the code if you have any ideas.

