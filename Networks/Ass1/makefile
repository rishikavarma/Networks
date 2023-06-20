.c.o:
	 gcc -g -c $?
all: usercode servercode

usercode: usercode.o
	gcc -g -o usercode usercode.o

servercode: servercode.o
	gcc -g -o servercode servercode.o

clean: 
	rm *.txt usercode servercode *.o 
	