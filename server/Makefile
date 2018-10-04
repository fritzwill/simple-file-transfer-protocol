CC= gcc
TARGETS=myftpd
CFLAGS=-std=gnu99

all: $(TARGETS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

myftpd: myftpd.o
	$(CC) -o $@ $< $(CFLAGS)

clean:
	@echo Cleaning..
	@rm myftpd
	@rm myftpd.o
