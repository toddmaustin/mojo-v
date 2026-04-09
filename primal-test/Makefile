
LOCAL_CFLAGS= -I../common
LOCAL_LIBS=

PROG=primal-test
OBJS=primal-test.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO primal-test.out
	rm -f FOO

