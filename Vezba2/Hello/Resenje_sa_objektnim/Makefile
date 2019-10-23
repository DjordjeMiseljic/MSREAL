#Hello make

all:hello
hello : hello.o
	gcc -o hello hello.o
hello.o : hello.c
	gcc -c hello.c
.PHONY : clean
clean : 
	rm -f hello hello.o
