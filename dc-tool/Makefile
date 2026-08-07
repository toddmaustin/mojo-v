# Makefile for the Mojo-V data contract multi-tool (ML-KEM-512 + SIMON-128, OpenSSL 3.6.x)
#
# Usage:
#   make clean build test
#
# Set OPENSSL_DIR to the root of an OpenSSL 3.6+ installation if it is not
# in the default system paths, e.g. make OPENSSL_DIR=/opt/ssl

OPENSSL_DIR ?= /opt/ssl

CC      = gcc
CFLAGS  = -Wall -Wextra -O0 -g -I$(OPENSSL_DIR)/include
LDFLAGS = -L$(OPENSSL_DIR)/lib64 -Wl,-rpath,$(OPENSSL_DIR)/lib64
LDLIBS  = -lcrypto

TARGET  := dc-tool
SRC     := dc-tool.c simon.c
OBJ     := $(SRC:.c=.o)

.PHONY: all clean build test

all: build

build: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


test: $(TARGET)
	-rm -f FOO
	./$(TARGET) keygen pk-file.pem sk-file.pem >> FOO
	./$(TARGET) dcgen pk-file.pem strong dc-file.txt >> FOO
	./$(TARGET) dcchk sk-file.pem dc-file.txt >> FOO
	diff FOO dc-tool.out

clean:
	rm -f $(TARGET) $(OBJ) pk-file.pem sk-file.pem dc-file.txt FOO

