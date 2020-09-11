GCC = gcc -g

OBJS = myshell

myshell: myshell.c
	$(GCC) myshell.c -o $(OBJS)	

clean: 
	rm $(OBJS)
