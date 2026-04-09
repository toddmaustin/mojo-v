
LOCAL_CFLAGS=
LOCAL_LIBS=

PROG=eulers-number-approx
OBJS=eulers-number-approx.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO eulers-number-approx.out
	rm -f FOO

