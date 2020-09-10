GCC = gcc

OBJS = myshell

myshell: myshell.c
	$(GCC) myshell.c -o $(OBJS)	

clean: 
	rm $(OBJS)