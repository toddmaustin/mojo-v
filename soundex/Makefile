
LOCAL_CFLAGS= -I../common
LOCAL_LIBS=

PROG=soundex
OBJS=soundex.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO soundex.out
	rm -f FOO

