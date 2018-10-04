CC= gcc
TARGETS=myftp
CFLAGS=-std=gnu99

all: $(TARGETS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

myftp: myftp.o
	$(CC) -o $@ $< $(CFLAGS)


clean:
	@echo Cleaning..
	@rm myftp
	@rm myftp.o
