GCC = gcc -g -Werror -std=c99 -Wall

OBJS = myshell

myshell: myshell.c
	$(GCC) myshell.c -o $(OBJS)

clean: 
	rm $(OBJS)
