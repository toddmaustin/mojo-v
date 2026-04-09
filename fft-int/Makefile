
LOCAL_CFLAGS= -I../common
LOCAL_LIBS=

PROG=fft-int
OBJS=fft-int.o ../common/mathlib.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO fft-int.out
	rm -f FOO

