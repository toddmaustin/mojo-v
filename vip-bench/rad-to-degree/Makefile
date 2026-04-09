
LOCAL_CFLAGS=
LOCAL_LIBS=

PROG=rad-to-degree
OBJS=rad-to-degree.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO rad-to-degree.out
	rm -f FOO

