
LOCAL_CFLAGS= -I../common
LOCAL_LIBS=

PROG=minspan
OBJS=minspan.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO minspan.out
	rm -f FOO

