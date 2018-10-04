CC=g++
TARGETS=myftp
CFLAGS=-std=c++11

all: $(TARGETS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

myftp: myftp.o
	$(CC) -o $@ $< $(CFLAGS)


clean:
	@echo Cleaning..
	@rm myftp
	@rm myftp.o
