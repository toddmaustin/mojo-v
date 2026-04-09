
LOCAL_CFLAGS= -I../common
LOCAL_LIBS=

PROG=gradient-descent
OBJS=gradient-descent.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO gradient-descent.out
	rm -f FOO

