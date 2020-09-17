GCC = gcc -g -std=c99

OBJS = myshell

myshell: myshell.c
	$(GCC) myshell.c -o $(OBJS)	

clean: 
	rm $(OBJS)
