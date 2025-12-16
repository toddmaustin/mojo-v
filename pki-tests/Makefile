# Makefile for ML-KEM-512 + AES-128-GCM demo (OpenSSL 3.6.x)
#
# Usage:
#   make
#   make run-all
#   make clean

CC      = gcc
CFLAGS  = -Wall -Wextra -O0 -g -I/opt/ssl/include
LDFLAGS = -L/opt/ssl/lib64 -Wl,-rpath,/opt/ssl/lib64
LDLIBS  = -lcrypto

TARGET  := dc-multitool
SRC     := dc-multitool.c simon.c
OBJ     := $(SRC:.c=.o)

.PHONY: all clean build test

build: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


bob: $(TARGET)
	./$(TARGET) bob

test:
	-rm -f FOO
	./$(TARGET) keygen pk-file.pem sk-file.pem >> FOO
	./$(TARGET) dcgen pk-file.pem strong ct-file.txt >> FOO
	./$(TARGET) dcchk sk-file.pem ct-file.txt >> FOO
	@echo "#"
	@echo "# NOTE: salt, sym_key_128, and contract_sig should NEVER match"
	@echo "#"
	diff FOO dc-multitool.out

clean:
	rm -f $(TARGET) $(OBJ) pk-file.pem sk-file.pem ct-file.txt FOO

