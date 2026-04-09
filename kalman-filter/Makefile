
LOCAL_CFLAGS= -I../common
LOCAL_LIBS=

PROG=kalman-filter
OBJS=kalman-filter.o kalmanlib.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO kalman-filter.out
	rm -f FOO

