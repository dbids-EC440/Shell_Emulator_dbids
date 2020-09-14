GCC = gcc -g -Werror

OBJS = myshell

myshell: myshell.c
	$(GCC) myshell.c -o $(OBJS)	

clean: 
	rm $(OBJS)
