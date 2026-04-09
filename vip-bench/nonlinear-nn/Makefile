
LOCAL_CFLAGS=
LOCAL_LIBS=

PROG=nonlinear-nn
OBJS=nonlinear-nn.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO nonlinear-nn.out
	rm -f FOO

