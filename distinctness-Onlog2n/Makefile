
LOCAL_CFLAGS= -I../common
LOCAL_LIBS=

PROG=distinctness-Onlog2n
OBJS=distinctness-Onlog2n.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO distinctness-Onlog2n.out
	rm -f FOO

