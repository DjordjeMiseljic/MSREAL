sources=$(wildcard *.c)
objs=$(sources:.c=.o)

all: calculate
calculate: $(objs)
	@echo -n "Building output binary: "
	@echo $@
	$(CC) -o $@ $(objs)
%.o: %.c
	@echo -n "Compiling source into: "
	@echo $@
	$(CC) -c $<
%.d: %.c
	@echo -n  "Creating dependancy: "
	@echo $@
	@rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@;

-include $(sources:.c=.d)

.PHONY: clean
clean:
	@rm -rf calculated *.o *.d
	@echo "Clean done.."


