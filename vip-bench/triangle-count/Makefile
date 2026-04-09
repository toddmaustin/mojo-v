
LOCAL_CFLAGS= -I../common
LOCAL_LIBS=

PROG=triangle-count
OBJS=triangle-count.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO triangle-count.out
	rm -f FOO

