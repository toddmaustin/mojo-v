
LOCAL_CFLAGS=
LOCAL_LIBS=

PROG=rabinkarp-search
OBJS=rabinkarp-search.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	-$(TARGET_DIFF) FOO rabinkarp-search.out
	rm -f FOO

