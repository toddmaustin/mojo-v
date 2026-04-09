
LOCAL_CFLAGS=
LOCAL_LIBS=

PROG=parrando
OBJS=parrando.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO parrando.out
	rm -f FOO

