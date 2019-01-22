CFLAGS = -g

all: pwm_button
pwm_button: pwm_button.o
	${CC} ${CFLAGS} $^ -o $@

clean:
	rm -rfv *.o
	rm -rfv pwm_button

.PHONY: clean
