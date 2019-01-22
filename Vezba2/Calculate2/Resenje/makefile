all: calculate
calculate: calculate.o helper.o helper2.o
	gcc -o calculate calculate.o helper.o helper2.o
calculate.o: calculate.c helper.h
	gcc -c calculate.c
helper.o: helper.c helper.h helper2.h
	gcc -c helper.c
helper2.o: helper2.c helper2.h
	gcc -c helper2.c
.PHONY: clean
clean:
	rm -f calculate *.o 
