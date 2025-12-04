# Makefile for ML-KEM-512 + AES-128-GCM demo (OpenSSL 3.6.x)
#
# Usage:
#   make
#   make run-all
#   make clean

CC      = gcc
CFLAGS  = -Wall -Wextra -O0 -g -I/usr/local/ssl/include
LDFLAGS = -L/usr/local/ssl/lib64 -Wl,-rpath,/usr/local/ssl/lib64
LDLIBS  = -lssl -lcrypto

TARGET  := mlkem-demo
SRC     := mlkem-demo.c
OBJ     := $(SRC:.c=.o)

.PHONY: all clean run-all init alice bob

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run-all: $(TARGET)
	./$(TARGET) all

init: $(TARGET)
	./$(TARGET) init

alice: $(TARGET)
	./$(TARGET) alice

bob: $(TARGET)
	./$(TARGET) bob

clean:
	rm -f $(TARGET) $(OBJ) mlkem512-pk.pem mlkem512-sk.pem mlkem512-ct.txt my-msg-enc.txt

