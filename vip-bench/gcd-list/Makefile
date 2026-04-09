
LOCAL_CFLAGS= -I../common
LOCAL_LIBS=

PROG=gcd-list
OBJS=gcd-list.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO gcd-list.out
	rm -f FOO

