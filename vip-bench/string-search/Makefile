
LOCAL_CFLAGS=
LOCAL_LIBS=

PROG=string-search
OBJS=string-search.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	-$(TARGET_DIFF) FOO string-search.out
	rm -f FOO

