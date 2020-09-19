GCC = gcc -g -Werror -std=c99 -D_POSIX_C_SOURCE=199309L

OBJS = myshell

myshell: myshell.c
	$(GCC) myshell.c -o $(OBJS)	

clean: 
	rm $(OBJS)
