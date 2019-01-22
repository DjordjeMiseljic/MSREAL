all: calculate
calculate: calculate.o helper.o
	gcc -o calculate calculate.o helper.o 
calculate.o: calculate.c helper.h
	gcc -c calculate.c
helper.o: helper.c helper.h
	gcc -c helper.c
.PHONY: clean
clean:
	rm -f calculate *.o 
