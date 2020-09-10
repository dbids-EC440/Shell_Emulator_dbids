GCC = gcc

OBJS = myshell

myshell:
	$(GCC) myshell.c -o $(OBJS)	

clean: 
	rm $(OBJS) myshell