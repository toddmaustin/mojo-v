
LOCAL_CFLAGS=
LOCAL_LIBS=

PROG=linear-reg
OBJS=linear-reg.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO linear-reg.out
	rm -f FOO

