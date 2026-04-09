
LOCAL_CFLAGS= -I../common
LOCAL_LIBS=

PROG=slice-n-dice
OBJS=slice-n-dice.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) ./$(TARGET_EXE) FOO.log 200000 > FOO 
	$(TARGET_DIFF) FOO slice-n-dice.out
	rm -f FOO

